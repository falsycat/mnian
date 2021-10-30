// No copyright
#include "mncore/command.h"
#include "mntest/command.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "mntest/dir.h"
#include "mntest/file.h"


namespace mnian::test {

TEST(SquashedCommand, ApplyAndRevert) {
  static constexpr size_t kCount = 100;

  std::vector<std::unique_ptr<::testing::StrictMock<MockCommand>>> cmds;

  for (size_t i = 0; i < kCount; ++i) {
    auto cmd = std::make_unique<::testing::StrictMock<MockCommand>>();
    cmds.push_back(std::move(cmd));
  }

  ::testing::Sequence seq;
  for (auto& cmd : cmds) {
    EXPECT_CALL(*cmd, Apply()).InSequence(seq);
  }
  auto itr = cmds.end();
  while (itr > cmds.begin()) {
    --itr;
    EXPECT_CALL(**itr, Revert()).InSequence(seq);
  }

  std::vector<std::unique_ptr<core::iCommand>> cmds_;
  for (auto& cmd : cmds) {
    cmds_.push_back(std::move(cmd));
  }

  core::SquashedCommand squashed("", std::move(cmds_));
  squashed.Apply();
  squashed.Revert();
}

TEST(SquashedCommand, ApplyAndRevertEmpty) {
  core::SquashedCommand squashed("", {});
  squashed.Apply();
  squashed.Revert();
}

TEST(DirCommand, Add) {
  core::Dir dir;

  auto item     = std::make_unique<::testing::StrictMock<MockDirItem>>();
  auto item_ptr = item.get();

  ::testing::StrictMock<MockDirItemObserver> obs(item.get());

  core::DirCommand cmd("", &dir, "hello", std::move(item));
  {
    EXPECT_CALL(obs, ObserveAdd());
    cmd.Apply();
  }
  ASSERT_EQ(dir.Find("hello"), item_ptr);

  {
    EXPECT_CALL(obs, ObserveRemove());
    cmd.Revert();
  }
  ASSERT_FALSE(dir.Find("hello"));

  {
    EXPECT_CALL(obs, ObserveAdd());
    cmd.Apply();
  }
  ASSERT_EQ(dir.Find("hello"), item_ptr);
}

TEST(DirCommand, Remove) {
  core::Dir dir;

  auto item = dir.Add(
      "hello", std::make_unique<::testing::StrictMock<MockDirItem>>());

  core::DirCommand cmd("", &dir, "hello");

  ::testing::StrictMock<MockDirItemObserver> obs(item);

  {
    EXPECT_CALL(obs, ObserveRemove());
    cmd.Apply();
  }
  ASSERT_FALSE(dir.Find("hello"));

  {
    EXPECT_CALL(obs, ObserveAdd());
    cmd.Revert();
  }
  ASSERT_EQ(dir.Find("hello"), item);

  {
    EXPECT_CALL(obs, ObserveRemove());
    cmd.Apply();
  }
  ASSERT_FALSE(dir.Find("hello"));
}

TEST(FileRefReplaceCommand, Replace) {
  ::testing::StrictMock<MockFile> f1("file://f1");
  ::testing::StrictMock<MockFile> f2("file://f2");

  core::FileRef fref(&f1, core::FileRef::kReadable);

  core::FileRefReplaceCommand cmd("", &fref, &f2);

  cmd.Apply();
  ASSERT_EQ(&fref.entity(), &f2);

  cmd.Revert();
  ASSERT_EQ(&fref.entity(), &f1);

  cmd.Apply();
  ASSERT_EQ(&fref.entity(), &f2);
}

TEST(FileRefFlagCommand, SetFlag) {
  ::testing::StrictMock<MockFile> f("file://f");

  core::FileRef fref(&f, core::FileRef::kNone);

  // This command makes it unreadable.
  core::FileRefFlagCommand cmd("", &fref, core::FileRef::kReadable, false);

  cmd.Apply();
  ASSERT_FALSE(fref.readable());

  cmd.Revert();
  ASSERT_TRUE(fref.readable());

  cmd.Apply();
  ASSERT_FALSE(fref.readable());
}

}  // namespace mnian::test
