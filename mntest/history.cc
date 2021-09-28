// No copyright
#include "mncore/history.h"
#include "mntest/history.h"

#include <gtest/gtest.h>

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "mntest/command.h"


namespace mnian::test {


static void CreateTree(
    core::History* history, size_t root_branch, size_t child_branch) {
  assert(history);

  auto& root = history->root();
  for (size_t i = 0; i < root_branch; ++i) {
    root.Fork(std::make_unique<core::NullCommand>());

    auto& branch = *root.branch()[i];
    for (size_t j = 0; j < child_branch; ++j) {
      branch.Fork(std::make_unique<core::NullCommand>());
    }
  }
}


TEST(History, Initial) {
  core::ManualClock clock;
  core::History history(&clock);

  ASSERT_EQ(&history.root(), &history.head());
}

TEST(History, Clear) {
  core::ManualClock clock;
  core::History history(&clock);

  history.Clear();
  ASSERT_EQ(&history.root(), &history.head());
}

TEST(History, ExecSequence) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  ::testing::Sequence seq;

  // Executes mock commands.
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<::testing::StrictMock<MockCommand>>();
    EXPECT_CALL(*command, Apply()).InSequence(seq);

    auto ptr = command.get();
    history.Exec(std::move(command));
    ASSERT_EQ(&history.head().command(), ptr);
  }

  core::HistoryItem* item = &history.head();
  ASSERT_EQ(item->branch().size(), 0);

  // Checks if all items are connected properly.
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(item->parent().branch().size(), 1);
    ASSERT_EQ(item, item->parent().branch()[0]);
    item = &item->parent();
  }
  ASSERT_EQ(&history.root(), item);

  ASSERT_EQ(history.items().size(), kCount+1);
}

TEST(History, UnDoSequence) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  ::testing::Sequence seq;

  // Executes mock commands.
  std::vector<::testing::StrictMock<MockCommand>*> commands;
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<::testing::StrictMock<MockCommand>>();
    EXPECT_CALL(*command, Apply()).InSequence(seq);

    commands.push_back(command.get());
    history.Exec(std::move(command));
  }

  // UnDo all.
  auto itr = commands.end();
  while (itr > commands.begin()) {
    --itr;
    EXPECT_CALL(**itr, Revert()).InSequence(seq);

    ASSERT_EQ(&history.head().command(), *itr);
    history.UnDo();
  }
  ASSERT_EQ(&history.root(), &history.head());

  ASSERT_EQ(history.items().size(), kCount+1);
}

TEST(History, ExecWithFork) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  ::testing::Sequence seq;

  // Executes and ReDo mock commands.
  std::vector<::testing::StrictMock<MockCommand>*> commands;
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<::testing::StrictMock<MockCommand>>();
    EXPECT_CALL(*command, Apply()).InSequence(seq);
    EXPECT_CALL(*command, Revert()).InSequence(seq);

    commands.push_back(command.get());
    history.Exec(std::move(command));
    history.UnDo();
  }

  ASSERT_EQ(history.root().branch().size(), kCount);
  ASSERT_EQ(&history.root(), &history.head());

  // Checks branch is properly ordered.
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(&history.root().branch()[i]->command(), commands[i]);
  }

  ASSERT_EQ(history.items().size(), kCount+1);
}

TEST(History, ReDoSequence) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  ::testing::Sequence seq;

  // Executes mock commands.
  std::vector<::testing::StrictMock<MockCommand>*> commands;
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<::testing::StrictMock<MockCommand>>();
    EXPECT_CALL(*command, Apply()).InSequence(seq);

    commands.push_back(command.get());
    history.Exec(std::move(command));
  }

  // UnDo all.
  auto itr = commands.end();
  while (itr > commands.begin()) {
    --itr;
    EXPECT_CALL(**itr, Revert()).InSequence(seq);
    history.UnDo();
  }

  // ReDo all.
  for (auto command : commands) {
    EXPECT_CALL(*command, Apply()).InSequence(seq);
    history.ReDo();
    ASSERT_EQ(&history.head().command(), command);
  }
  ASSERT_EQ(history.head().branch().size(), 0);

  ASSERT_EQ(history.items().size(), kCount+1);
}

TEST(HistoryItem, Fork) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  std::vector<core::iCommand*> commands;

  auto& root = history.root();
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<core::NullCommand>();
    commands.push_back(command.get());
    root.Fork(std::move(command));
    ASSERT_EQ(root.branch().size(), i+1);
  }

  for (size_t i = 0; i < kCount; ++i) {
    auto& branch = *root.branch()[i];
    ASSERT_EQ(&branch.parent(), &root);
    ASSERT_EQ(branch.branch().size(), 0);
    ASSERT_EQ(&branch.command(), commands[i]);
  }
}

TEST(HistoryItem, MakeRoot) {
  static constexpr size_t kRootBranchCount  = 5;
  static constexpr size_t kChildBranchCount = 5;

  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, kRootBranchCount, kChildBranchCount);

  auto& root = history.root();

  history.ReDo(kRootBranchCount/2);
  ASSERT_TRUE(history.head().parent().isRoot());

  history.ReDo();
  ASSERT_TRUE(history.head().parent().parent().isRoot());

  auto& next_root = *root.branch()[root.branch().size()-1];
  next_root.MakeRoot();

  ASSERT_EQ(&history.root(), &next_root);
  ASSERT_EQ(next_root.branch().size(), kChildBranchCount);
  ASSERT_TRUE(next_root.isRoot());

  ASSERT_EQ(history.items().size(), kChildBranchCount+1);
}

TEST(HistoryItem, DropSelf) {
  static constexpr size_t kRootBranchCount  = 5;

  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, kRootBranchCount, 5);

  auto& root = history.root();
  for (size_t i = 0; i < kRootBranchCount; ++i) {
    root.branch()[0]->DropSelf();
    ASSERT_EQ(root.branch().size(), kRootBranchCount-i-1);
  }
  ASSERT_EQ(history.items().size(), 1);
}

TEST(HistoryItem, DropAllBranch) {
  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, 5, 5);

  auto& root = history.root();
  root.DropAllBranch();
  ASSERT_EQ(root.branch().size(), 0);

  ASSERT_EQ(history.items().size(), 1);
}

TEST(HistoryItem, IsAncestorOf) {
  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, 5, 5);

  auto& root = history.root();

  auto& child_0 = *root.branch()[0];
  auto& child_1 = *root.branch()[1];

  auto& child_0_0 = *child_0.branch()[0];
  auto& child_1_0 = *child_1.branch()[0];

  ASSERT_TRUE(root.IsAncestorOf(child_0));
  ASSERT_TRUE(root.IsAncestorOf(child_1));
  ASSERT_TRUE(root.IsAncestorOf(child_0_0));
  ASSERT_TRUE(root.IsAncestorOf(child_1_0));
  ASSERT_TRUE(root.IsAncestorOf(root));

  ASSERT_TRUE(!child_0.IsAncestorOf(root));
  ASSERT_TRUE(!child_0_0.IsAncestorOf(child_0));
  ASSERT_TRUE(!child_0_0.IsAncestorOf(child_1));
  ASSERT_TRUE(!child_0_0.IsAncestorOf(child_1_0));
}

TEST(HistoryItem, FindLowestCommonAncestor) {
  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, 5, 5);

  auto& root = history.root();

  auto& child_0 = *root.branch()[0];
  auto& child_1 = *root.branch()[1];

  auto& child_0_0 = *child_0.branch()[0];
  auto& child_1_0 = *child_1.branch()[0];

  ASSERT_EQ(&child_0.FindLowestCommonAncestor(child_0), &child_0);
  ASSERT_EQ(&child_0.FindLowestCommonAncestor(child_0_0), &child_0);
  ASSERT_EQ(&child_0.FindLowestCommonAncestor(child_1), &root);
  ASSERT_EQ(&child_0_0.FindLowestCommonAncestor(child_1_0), &root);
}

}  // namespace mnian::test
