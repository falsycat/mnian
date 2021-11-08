// No copyright
#include "mncore/dir.h"
#include "mntest/dir.h"

#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "mntest/file.h"
#include "mntest/node.h"


namespace mnian::test {

TEST(iDirItem, ValidateName) {
  ASSERT_FALSE(core::iDirItem::ValidateName("helloworld"));
  ASSERT_FALSE(core::iDirItem::ValidateName("hello-world"));
  ASSERT_FALSE(core::iDirItem::ValidateName("hello_world"));
  ASSERT_FALSE(core::iDirItem::ValidateName("hello-world-0123"));
  ASSERT_FALSE(core::iDirItem::ValidateName("hello_world_0123"));

  ASSERT_TRUE(core::iDirItem::ValidateName(""));
  ASSERT_TRUE(core::iDirItem::ValidateName("hello world"));
  ASSERT_TRUE(core::iDirItem::ValidateName("にゃんにゃん"));
}


TEST(Dir, Visit) {
  core::iDirItem::Store store;
  core::Dir dir(&store);

  MockDirItemVisitor visitor;
  EXPECT_CALL(visitor, VisitDir(&dir));
  dir.Visit(&visitor);
}

TEST(Dir, Add) {
  static constexpr size_t kCount = 100;

  core::iDirItem::Store store;
  core::Dir dir(&store);

  ::testing::StrictMock<MockDirItemObserver> observer(&dir);
  EXPECT_CALL(observer, ObserveUpdate()).Times(kCount);

  std::vector<core::Dir*> subdirs;
  for (size_t i = 0; i < kCount; ++i) {
    auto subdir = std::make_unique<core::Dir>(&store);
    ASSERT_TRUE(subdir);

    auto ptr = subdir.get();
    subdirs.push_back(ptr);

    ASSERT_EQ(dir.Add(std::to_string(i), std::move(subdir)), ptr);
    ASSERT_EQ(dir.items().size(), i+1);
  }

  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(dir.Find(std::to_string(i)), subdirs[i]);
  }
}

TEST(Dir, Remove) {
  static constexpr size_t kCount = 100;

  std::mt19937 rnd(
      static_cast<uint32_t>(::testing::UnitTest::GetInstance()->random_seed()));

  core::iDirItem::Store store;
  core::Dir dir(&store);

  ::testing::StrictMock<MockDirItemObserver> observer(&dir);
  EXPECT_CALL(observer, ObserveUpdate()).Times(kCount*2);

  std::vector<core::Dir*> subdirs;
  for (size_t i = 0; i < kCount; ++i) {
    auto subdir = std::make_unique<core::Dir>(&store);
    ASSERT_TRUE(subdir);
    subdirs.push_back(subdir.get());
    dir.Add(std::to_string(i), std::move(subdir));
  }
  ASSERT_EQ(dir.items().size(), kCount);

  std::vector<size_t> index;
  for (size_t i = 0; i < kCount; ++i) index.push_back(i);
  std::shuffle(index.begin(), index.end(), rnd);

  for (size_t i = 0; i < kCount; ++i) {
    auto ptr = subdirs[index[i]];
    subdirs[index[i]] = nullptr;

    ASSERT_EQ(dir.Remove(std::to_string(index[i])).get(), ptr);
    ASSERT_EQ(dir.items().size(), kCount-i-1);

    for (size_t j = 0; j < kCount; ++j) {
      if (subdirs[j]) {
        ASSERT_EQ(dir.Find(std::to_string(j)), subdirs[j]);
      } else {
        ASSERT_FALSE(dir.items().contains(std::to_string(j)));
      }
    }
  }
}

TEST(Dir, Move) {
  static constexpr size_t kCount = 100;

  std::mt19937 rnd(
      static_cast<uint32_t>(::testing::UnitTest::GetInstance()->random_seed()));

  core::iDirItem::Store store;

  core::Dir src(&store);
  ::testing::StrictMock<MockDirItemObserver> src_observer(&src);
  EXPECT_CALL(src_observer, ObserveUpdate()).Times(kCount*2);

  core::Dir dst(&store);
  ::testing::StrictMock<MockDirItemObserver> dst_observer(&dst);
  EXPECT_CALL(dst_observer, ObserveUpdate()).Times(kCount);

  std::vector<std::unique_ptr<::testing::StrictMock<MockDirItemObserver>>>
      sub_observers;

  for (size_t i = 0; i < kCount; ++i) {
    auto subdir = std::make_unique<core::Dir>(&store);
    ASSERT_TRUE(subdir);

    auto sub_observer = std::make_unique
        <::testing::StrictMock<MockDirItemObserver>>(subdir.get());

    ::testing::InSequence _;
    EXPECT_CALL(*sub_observer, ObserveAdd());
    EXPECT_CALL(*sub_observer, ObserveMove());

    sub_observers.push_back(std::move(sub_observer));
    src.Add(std::to_string(i), std::move(subdir));
  }

  std::vector<size_t> index;
  for (size_t i = 0; i < kCount; ++i) index.push_back(i);
  std::shuffle(index.begin(), index.end(), rnd);

  std::vector<core::iDirItem*> subdirs;
  for (size_t i = 0; i < kCount; ++i) {
    subdirs.push_back(
        src.Move(std::to_string(index[i]), &dst, std::to_string(i)));
  }
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(dst.Find(std::to_string(i)), subdirs[i]);
  }
}

TEST(Dir, Find) {
  static const std::vector<std::string> kNames = {
    "helloworld", "hello", "world", "hellworld",
  };

  core::iDirItem::Store store;
  core::Dir dir(&store);

  std::map<std::string, core::Dir*> subdirs;
  for (auto& name : kNames) {
    auto subdir = std::make_unique<core::Dir>(&store);
    ASSERT_TRUE(subdir);
    subdirs[name] = subdir.get();
    dir.Add(name, std::move(subdir));
  }
  for (auto& name : kNames) {
    ASSERT_EQ(dir.Find(name), subdirs[name]);
  }
}

TEST(Dir, GeneratePath) {
  static const std::vector<std::string> kNames = {"a", "b", "c", "d"};

  core::iDirItem::Store store;
  core::Dir root(&store);

  std::vector<core::Dir*> dirs = {&root};
  for (auto& name : kNames) {
    auto dir = std::make_unique<core::Dir>(&store);
    dirs.push_back(dir.get());
    dirs[dirs.size()-2]->Add(name, std::move(dir));
  }

  ASSERT_THAT(dirs[0]->GeneratePath(), ::testing::ElementsAre());
  ASSERT_THAT(dirs[1]->GeneratePath(), ::testing::ElementsAre("a"));
  ASSERT_THAT(dirs[2]->GeneratePath(), ::testing::ElementsAre("a", "b"));
  ASSERT_THAT(dirs[3]->GeneratePath(), ::testing::ElementsAre("a", "b", "c"));
  ASSERT_THAT(dirs[4]->GeneratePath(),
              ::testing::ElementsAre("a", "b", "c", "d"));
}

TEST(Dir, FindPath) {
  static const std::vector<std::string> kNames = {"a", "b", "c", "d"};

  core::iDirItem::Store store;
  core::Dir root(&store);

  std::vector<core::Dir*> dirs = {&root};
  for (auto& name : kNames) {
    auto dir = std::make_unique<core::Dir>(&store);
    dirs.push_back(dir.get());
    dirs[dirs.size()-2]->Add(name, std::move(dir));
  }

  ASSERT_EQ(root.FindPath({}),                   dirs[0]);
  ASSERT_EQ(root.FindPath({"a"}),                dirs[1]);
  ASSERT_EQ(root.FindPath({"a", "b"}),           dirs[2]);
  ASSERT_EQ(root.FindPath({"a", "b", "c"}),      dirs[3]);
  ASSERT_EQ(root.FindPath({"a", "b", "c", "d"}), dirs[4]);

  ASSERT_EQ(dirs[1]->FindPath({}),              dirs[1]);
  ASSERT_EQ(dirs[1]->FindPath({"b"}),           dirs[2]);
  ASSERT_EQ(dirs[1]->FindPath({"b", "c"}),      dirs[3]);
  ASSERT_EQ(dirs[1]->FindPath({"b", "c", "d"}), dirs[4]);

  ASSERT_EQ(dirs[2]->FindPath({}),         dirs[2]);
  ASSERT_EQ(dirs[2]->FindPath({"c"}),      dirs[3]);
  ASSERT_EQ(dirs[2]->FindPath({"c", "d"}), dirs[4]);

  ASSERT_EQ(dirs[3]->FindPath({}),    dirs[3]);
  ASSERT_EQ(dirs[3]->FindPath({"d"}), dirs[4]);

  ASSERT_EQ(dirs[4]->FindPath({}), dirs[4]);

  ASSERT_FALSE(root.FindPath({"missing", "path"}));
}

TEST(Dir, IsAncestorOf) {
  core::iDirItem::Store store;
  core::Dir root(&store);

  auto hello = root.Add("hello", std::make_unique<core::Dir>(&store));
  ASSERT_TRUE(hello);

  auto world = root.Add("world", std::make_unique<core::Dir>(&store));
  ASSERT_TRUE(world);

  ASSERT_TRUE(root.IsAncestorOf(*hello));
  ASSERT_TRUE(root.IsAncestorOf(*world));

  ASSERT_FALSE(hello->IsAncestorOf(*world));
  ASSERT_FALSE(world->IsAncestorOf(*hello));

  ASSERT_FALSE(hello->IsAncestorOf(root));
  ASSERT_FALSE(world->IsAncestorOf(root));

  ASSERT_TRUE(hello->IsDescendantOf(root));
  ASSERT_TRUE(world->IsDescendantOf(root));

  ASSERT_FALSE(hello->IsDescendantOf(*world));
  ASSERT_FALSE(world->IsDescendantOf(*hello));

  ASSERT_FALSE(root.IsDescendantOf(*hello));
  ASSERT_FALSE(root.IsDescendantOf(*world));

  ASSERT_TRUE(root.IsAncestorOf(root));
  ASSERT_TRUE(hello->IsAncestorOf(*hello));
  ASSERT_TRUE(world->IsAncestorOf(*world));

  ASSERT_TRUE(root.IsDescendantOf(root));
  ASSERT_TRUE(hello->IsDescendantOf(*hello));
  ASSERT_TRUE(world->IsDescendantOf(*world));
}


TEST(FileRef, ParseFlags) {
  ASSERT_EQ(core::FileRef::ParseFlags("rrww"),
            core::FileRef::kReadable | core::FileRef::kWritable);
  ASSERT_EQ(core::FileRef::ParseFlags("rw"),
            core::FileRef::kReadable | core::FileRef::kWritable);
  ASSERT_EQ(core::FileRef::ParseFlags("r"),
            core::FileRef::kReadable);
  ASSERT_EQ(core::FileRef::ParseFlags("rr"),
            core::FileRef::kReadable);
  ASSERT_EQ(core::FileRef::ParseFlags("w"),
            core::FileRef::kWritable);
  ASSERT_EQ(core::FileRef::ParseFlags("ww"),
            core::FileRef::kWritable);
  ASSERT_EQ(core::FileRef::ParseFlags(""), 0);

  ASSERT_FALSE(core::FileRef::ParseFlags("Rw"));
  ASSERT_FALSE(core::FileRef::ParseFlags("rW"));
  ASSERT_FALSE(core::FileRef::ParseFlags("HELLO"));
  ASSERT_FALSE(core::FileRef::ParseFlags("world"));
}

TEST(FileRef, Visit) {
  MockFile file("test");

  core::iDirItem::Store store;
  core::FileRef fref(&store, &file, 0);

  ::testing::StrictMock<MockDirItemVisitor> visitor;
  EXPECT_CALL(visitor, VisitFile(&fref));
  fref.Visit(&visitor);
}

TEST(FileRef, EntityUpdate) {
  MockFile file("test");

  core::iDirItem::Store store;
  core::FileRef fref(&store, &file, 0);

  ::testing::StrictMock<MockDirItemObserver> observer(&fref);
  EXPECT_CALL(observer, ObserveUpdate());
  file.Touch();
}

TEST(FileRef, ReplaceEntity) {
  MockFile file1("test");
  MockFile file2("test");

  core::iDirItem::Store store;
  core::FileRef fref(&store, &file1, 0);
  ASSERT_EQ(&fref.entity(), &file1);

  ::testing::StrictMock<MockDirItemObserver> observer(&fref);
  EXPECT_CALL(observer, ObserveUpdate());

  fref.ReplaceEntity(&file2);
  ASSERT_EQ(&fref.entity(), &file2);
}

TEST(FileRef, ModifyFlags) {
  MockFile file("test");

  core::iDirItem::Store store;
  core::FileRef fref(&store, &file, 0);
  ASSERT_FALSE(fref.readable());
  ASSERT_FALSE(fref.writable());

  ::testing::StrictMock<MockDirItemObserver> observer(&fref);
  EXPECT_CALL(observer, ObserveUpdate()).Times(4);

  fref.SetFlag(core::FileRef::kReadable);
  ASSERT_TRUE(fref.readable());
  ASSERT_FALSE(fref.writable());

  fref.SetFlag(core::FileRef::kWritable);
  ASSERT_TRUE(fref.readable());
  ASSERT_TRUE(fref.writable());

  // The followings don't change anything, so the observer is not called.
  fref.SetFlag(core::FileRef::kReadable);
  fref.SetFlag(core::FileRef::kWritable);

  fref.UnsetFlag(core::FileRef::kWritable);
  ASSERT_TRUE(fref.readable());
  ASSERT_FALSE(fref.writable());

  fref.UnsetFlag(core::FileRef::kReadable);
  ASSERT_FALSE(fref.readable());
  ASSERT_FALSE(fref.writable());

  // The followings don't change anything, so the observer is not called.
  fref.UnsetFlag(core::FileRef::kReadable);
  fref.UnsetFlag(core::FileRef::kWritable);
}


TEST(NodeRef, Visit) {
  core::iDirItem::Store store;
  core::iNode::Store    nstore;
  core::NodeRef nref(&store, std::make_unique<MockNode>(&nstore));

  ::testing::StrictMock<MockDirItemVisitor> visitor;
  EXPECT_CALL(visitor, VisitNode(&nref));
  nref.Visit(&visitor);
}

TEST(NodeRef, EntityUpdate) {
  core::iDirItem::Store store;
  core::iNode::Store    nstore;

  auto node     = std::make_unique<MockNode>(&nstore);
  auto node_ptr = node.get();
  core::NodeRef nref(&store, std::move(node));

  ::testing::StrictMock<MockDirItemObserver> observer(&nref);
  EXPECT_CALL(observer, ObserveUpdate());
  node_ptr->NotifyUpdate();
}


TEST(iDirItemObserver, LosingTarget) {
  core::iDirItem::Store store;

  auto dir = std::make_unique<core::Dir>(&store);
  ASSERT_TRUE(dir);

  MockFile file("test");
  auto fref =
      std::make_unique<core::FileRef>(&store, &file, core::FileRef::kNone);
  ASSERT_TRUE(fref);

  ::testing::StrictMock<MockDirItemObserver> observer(fref.get());

  EXPECT_CALL(observer, ObserveAdd());
  dir->Add("hoge", std::move(fref));

  EXPECT_CALL(observer, ObserveDelete());
  dir = nullptr;
}

TEST(iDirItemObserver, Quitting) {
  core::iDirItem::Store store;

  auto dir = std::make_unique<core::Dir>(&store);
  ASSERT_TRUE(dir);

  MockFile file("test");
  auto fref =
      std::make_unique<core::FileRef>(&store, &file, core::FileRef::kNone);
  ASSERT_TRUE(fref);

  {
    ::testing::StrictMock<MockDirItemObserver> observer(dir.get());
  }
  dir = nullptr;
}

}  // namespace mnian::test
