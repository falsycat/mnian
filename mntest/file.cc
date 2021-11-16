// No copyright
#include "mncore/file.h"
#include "mntest/file.h"

#include <fstream>


namespace mnian::test {

class NativeFile : public ::testing::Test {
 public:
  static inline const std::string kPath = "./test-NativeFile/";

  NativeFile() = default;

 protected:
  void SetUp() override {
    ASSERT_TRUE(std::filesystem::create_directory(kPath));
    dir_created_ = true;
  }
  void TearDown() override {
    if (dir_created_) {
      ASSERT_TRUE(std::filesystem::remove_all(kPath));
    }
  }

 private:
  bool dir_created_ = false;
};

TEST_F(NativeFile, ReadTinyFile) {
  static const std::string kStr = "hello world";
  {
    std::ofstream st(kPath+"file", std::ios::binary);
    st << kStr;
  }

  auto f = core::iFile::CreateForNative(kPath+"file");
  auto k = f->Lock();

  uint8_t buf[32];
  ASSERT_EQ(k.Read(buf, sizeof(buf)), kStr.size());
  ASSERT_EQ(kStr, std::string(reinterpret_cast<char*>(buf), kStr.size()));
}

TEST_F(NativeFile, ReadHugeFile) {
  static constexpr size_t kChunkCount = 1024*4;  // = 1 MiB
  {
    uint8_t chunk[256];
    for (size_t i = 0; i < sizeof(chunk); ++i) {
      chunk[i] = static_cast<uint8_t>(i);
    }
    std::ofstream st(kPath+"file", std::ios::binary);
    for (size_t i = 0; i < kChunkCount; ++i) {
      ASSERT_TRUE(st.write(reinterpret_cast<char*>(chunk), sizeof(chunk)));
    }
  }

  auto f = core::iFile::CreateForNative(kPath+"file");
  auto k = f->Lock();

  for (size_t i = 0; i < kChunkCount*256; ++i) {
    uint8_t c;
    ASSERT_EQ(k.Read(&c, 1, i), 1);
    ASSERT_EQ(c, i%256);
  }

  uint8_t c;
  ASSERT_EQ(k.Read(&c, 1, kChunkCount*256), 0);
}

TEST_F(NativeFile, WriteTinyFile) {
  static const std::string kStr = "hello_world";

  auto f = core::iFile::CreateForNative(kPath+"file");
  auto k = f->Lock();

  ASSERT_EQ(
      k.Write(reinterpret_cast<const uint8_t*>(kStr.c_str()), kStr.size()),
      kStr.size());

  uint8_t buf[32];
  ASSERT_EQ(k.Read(buf, sizeof(buf)), kStr.size());
  ASSERT_EQ(kStr, std::string(reinterpret_cast<char*>(buf), kStr.size()));
}

TEST_F(NativeFile, WriteHugeFile) {
  uint8_t chunk[256];
  for (size_t i = 0; i < sizeof(chunk); ++i) {
    chunk[i] = static_cast<uint8_t>(i);
  }

  static constexpr size_t kChunkCount = 1024*4;  // = 1 MiB

  auto f = core::iFile::CreateForNative(kPath+"file");
  auto k = f->Lock();

  for (size_t i = 0; i < kChunkCount; ++i) {
    ASSERT_EQ(k.Write(chunk, sizeof(chunk), i*sizeof(chunk)), sizeof(chunk));
  }

  auto buf = std::make_unique<uint8_t[]>(kChunkCount*256);
  ASSERT_EQ(k.Read(buf.get(), kChunkCount*256), kChunkCount*256);
  for (size_t i = 0; i < kChunkCount*256; ++i) {
    ASSERT_EQ(buf[i], i%256);
  }
}

TEST_F(NativeFile, Truncate) {
  static constexpr size_t kTruncateSize = 4;

  static const std::string kStr = "hello_world";

  auto f = core::iFile::CreateForNative(kPath+"file");
  auto k = f->Lock();

  ASSERT_EQ(
      k.Write(reinterpret_cast<const uint8_t*>(kStr.c_str()), kStr.size()),
      kStr.size());
  k.Truncate(kTruncateSize);

  uint8_t buf[32];
  ASSERT_EQ(k.Read(buf, sizeof(buf)), kTruncateSize);
  ASSERT_EQ("hell", std::string(reinterpret_cast<char*>(buf), kTruncateSize));
}


TEST(iFile, NotifyUpdate) {
  MockFile file("test");

  ::testing::StrictMock<MockFileObserver> observer(&file);
  EXPECT_CALL(observer, ObserveUpdate());
  file.NotifyUpdate();
}

TEST(iFile_LockGuard, ReadAndWrite) {
  MockFile file("test");

  uint8_t buf[32];
  EXPECT_CALL(file, Read(buf, sizeof(buf), 0));
  EXPECT_CALL(file, Write(buf, sizeof(buf), 0));

  auto rw = file.Lock();
  rw.Read(buf, sizeof(buf), 0);
  rw.Write(buf, sizeof(buf), 0);
}

TEST(iFileObserver, AutoRemoval) {
  MockFile file("test");
  {
    ::testing::StrictMock<MockFileObserver> observer(&file);
    EXPECT_CALL(observer, ObserveUpdate());
    file.NotifyUpdate();
  }
  file.NotifyUpdate();
}

TEST(iFileStore, Load) {
  ::testing::StrictMock<MockFileStore> store;
  ON_CALL(store, Create(::testing::_)).
      WillByDefault([](const std::string& url) {
                      return std::make_unique<MockFile>(url);
                    });

  EXPECT_CALL(store, Create("hello"));
  EXPECT_CALL(store, Create("world"));

  auto hello = store.Load("hello");
  EXPECT_TRUE(hello);
  EXPECT_EQ(hello, store.Load("hello"));

  auto world = store.Load("world");
  EXPECT_TRUE(world);
  EXPECT_EQ(world, store.Load("world"));
}

}  // namespace mnian::test
