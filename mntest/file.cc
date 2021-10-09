// No copyright
#include "mncore/file.h"
#include "mntest/file.h"


namespace mnian::test {

TEST(iFile, Touch) {
  MockFile file("test");

  ::testing::StrictMock<MockFileObserver> observer(&file);
  EXPECT_CALL(observer, ObserveUpdate());
  file.Touch();
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
    file.Touch();
  }
  file.Touch();
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
