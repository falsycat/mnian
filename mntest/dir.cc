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

TEST(iDirItem, Rename) {
  MockDirItem item;
  item.Rename("helloworld");
  ASSERT_EQ(item.name(), "helloworld");
}

TEST(iDirItem, Touch) {
  MockDirItem item;

  ::testing::StrictMock<MockDirItemObserver> observer(&item);
  EXPECT_CALL(observer, ObserveUpdate());
  item.Touch();
}


TEST(Dir, Visit) {
  core::Dir dir(core::iActionable::ActionList {});

  MockDirItemVisitor visitor;
  EXPECT_CALL(visitor, VisitDir(&dir));
  dir.Visit(&visitor);
}

TEST(Dir, Add) {
  static constexpr size_t kCount = 100;

  core::Dir dir(core::iActionable::ActionList {});

  ::testing::StrictMock<MockDirItemObserver> observer(&dir);
  EXPECT_CALL(observer, ObserveUpdate()).Times(kCount);

  std::vector<core::Dir*> subdirs;
  for (size_t i = 0; i < kCount; ++i) {
    auto subdir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
    ASSERT_TRUE(subdir);

    auto ptr = subdir.get();
    subdirs.push_back(ptr);

    ASSERT_EQ(dir.Add(std::move(subdir)), ptr);
    ASSERT_EQ(dir.size(), i+1);
  }

  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(&dir.items(i), subdirs[i]);
  }
}

TEST(Dir, Remove) {
  static constexpr size_t kCount = 100;

  std::mt19937 rnd(
      static_cast<uint32_t>(::testing::UnitTest::GetInstance()->random_seed()));

  core::Dir dir(core::iActionable::ActionList {});

  ::testing::StrictMock<MockDirItemObserver> observer(&dir);
  EXPECT_CALL(observer, ObserveUpdate()).Times(kCount*2);

  std::vector<core::Dir*> subdirs;
  for (size_t i = 0; i < kCount; ++i) {
    auto subdir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
    ASSERT_TRUE(subdir);
    subdirs.push_back(subdir.get());
    dir.Add(std::move(subdir));
  }
  ASSERT_EQ(dir.size(), kCount);

  for (size_t i = 0; i < kCount; ++i) {
    const size_t index = rnd()%(kCount-i);

    auto ptr = subdirs[index];
    subdirs.erase(subdirs.begin() + static_cast<intmax_t>(index));

    ASSERT_EQ(dir.RemoveByIndex(index).get(), ptr);
    ASSERT_EQ(dir.size(), kCount-i-1);

    for (size_t j = 0; j < kCount-i-1; ++j) {
      ASSERT_EQ(&dir.items(j), subdirs[j]);
    }
  }
}

TEST(Dir, Move) {
  static constexpr size_t kCount = 100;

  std::mt19937 rnd(
      static_cast<uint32_t>(::testing::UnitTest::GetInstance()->random_seed()));

  core::Dir src(core::iActionable::ActionList {});
  ::testing::StrictMock<MockDirItemObserver> src_observer(&src);
  EXPECT_CALL(src_observer, ObserveUpdate()).Times(kCount*2);

  core::Dir dst(core::iActionable::ActionList {});
  ::testing::StrictMock<MockDirItemObserver> dst_observer(&dst);
  EXPECT_CALL(dst_observer, ObserveUpdate()).Times(kCount*2);

  std::vector<std::unique_ptr<::testing::StrictMock<MockDirItemObserver>>>
      sub_observers;

  for (size_t i = 0; i < kCount; ++i) {
    auto subdir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
    ASSERT_TRUE(subdir);

    auto sub_observer = std::make_unique
        <::testing::StrictMock<MockDirItemObserver>>(subdir.get());
    EXPECT_CALL(*sub_observer, ObserveMove());
    sub_observers.push_back(std::move(sub_observer));

    src.Add(std::move(subdir));
  }

  std::vector<core::iDirItem*> subdirs;
  for (size_t i = 0; i < kCount; ++i) {
    const size_t index = rnd()%(kCount-i);

    auto ptr = &src.items(index);
    subdirs.push_back(ptr);
    src.Move(ptr, &dst);
  }
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(&dst.items(i), subdirs[i]);
  }
}

TEST(Dir, FindIndexOf) {
  static const std::vector<std::string> kNames = {
    "helloworld", "hello", "world", "hellworld",
  };

  core::Dir dir(core::iActionable::ActionList {});

  std::map<std::string, core::Dir*> subdirs;
  for (auto& name : kNames) {
    auto subdir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
    ASSERT_TRUE(subdir);
    subdir->Rename(name);
    subdirs[name] = subdir.get();
    dir.Add(std::move(subdir));
  }
  for (auto& name : kNames) {
    auto index = dir.FindIndexOf(name);
    ASSERT_TRUE(index);
    ASSERT_EQ(&dir.items(*index), subdirs[name]);
  }
  for (auto& pair : subdirs) {
    auto index = dir.FindIndexOf(pair.second);
    ASSERT_TRUE(index);
    ASSERT_EQ(&dir.items(*index), pair.second);
  }
}

TEST(Dir, GeneratePath) {
  static const std::vector<std::string> kNames = {"a", "b", "c", "d"};

  core::Dir root(core::iActionable::ActionList {});

  std::vector<core::Dir*> dirs = {&root};
  for (auto& name : kNames) {
    auto dir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
    ASSERT_TRUE(dir);

    dir->Rename(name);
    dirs.push_back(dir.get());

    dirs[dirs.size()-2]->Add(std::move(dir));
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

  core::Dir root(core::iActionable::ActionList {});

  std::vector<core::Dir*> dirs = {&root};
  for (auto& name : kNames) {
    auto dir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
    ASSERT_TRUE(dir);

    dir->Rename(name);
    dirs.push_back(dir.get());

    dirs[dirs.size()-2]->Add(std::move(dir));
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

  core::FileRef fref({}, &file, 0);

  ::testing::StrictMock<MockDirItemVisitor> visitor;
  EXPECT_CALL(visitor, VisitFile(&fref));
  fref.Visit(&visitor);
}

TEST(FileRef, EntityUpdate) {
  MockFile file("test");

  core::FileRef fref({}, &file, 0);

  ::testing::StrictMock<MockDirItemObserver> observer(&fref);
  EXPECT_CALL(observer, ObserveUpdate());
  file.Touch();
}

TEST(FileRef, ReplaceEntity) {
  MockFile file1("test");
  MockFile file2("test");

  core::FileRef fref({}, &file1, 0);
  ASSERT_EQ(&fref.entity(), &file1);

  ::testing::StrictMock<MockDirItemObserver> observer(&fref);
  EXPECT_CALL(observer, ObserveUpdate());

  fref.ReplaceEntity(&file2);
  ASSERT_EQ(&fref.entity(), &file2);
}

TEST(FileRef, ModifyFlags) {
  MockFile file("test");

  core::FileRef fref({}, &file, 0);
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
  core::NodeStore store;
  core::NodeRef nref({}, &store, std::make_unique<MockNode>());

  ::testing::StrictMock<MockDirItemVisitor> visitor;
  EXPECT_CALL(visitor, VisitNode(&nref));
  nref.Visit(&visitor);
}

TEST(NodeRef, EntityUpdate) {
  core::NodeStore store;

  auto node     = std::make_unique<MockNode>();
  auto node_ptr = node.get();
  core::NodeRef nref({}, &store, std::move(node));

  ::testing::StrictMock<MockDirItemObserver> observer(&nref);
  EXPECT_CALL(observer, ObserveUpdate());
  node_ptr->NotifyUpdate();
}


TEST(iDirItemObserver, LosingTarget) {
  auto dir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
  ASSERT_TRUE(dir);

  MockFile file("test");
  auto fref = std::make_unique<core::FileRef>(
      core::iActionable::ActionList {}, &file, core::FileRef::kNone);
  ASSERT_TRUE(fref);

  ::testing::StrictMock<MockDirItemObserver> observer(fref.get());

  dir->Add(std::move(fref));
  EXPECT_CALL(observer, ObserveRemove());
  dir = nullptr;
}

TEST(iDirItemObserver, Quitting) {
  auto dir = std::make_unique<core::Dir>(core::iActionable::ActionList {});
  ASSERT_TRUE(dir);

  MockFile file("test");
  auto fref = std::make_unique<core::FileRef>(
      core::iActionable::ActionList {}, &file, core::FileRef::kNone);
  ASSERT_TRUE(fref);

  {
    ::testing::StrictMock<MockDirItemObserver> observer(dir.get());
  }
  dir = nullptr;
}

}  // namespace mnian::test
