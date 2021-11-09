// No copyright
#include "mncore/history.h"

#include <gtest/gtest.h>

#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "mntest/command.h"


namespace mnian::test {


static void CreateTree(
    core::History* history, size_t origin_branch, size_t child_branch) {
  assert(history);

  auto& origin = history->origin();
  for (size_t i = 0; i < origin_branch; ++i) {
    origin.Fork(std::make_unique<core::NullCommand>(""));

    auto& branch = *origin.branch()[i];
    for (size_t j = 0; j < child_branch; ++j) {
      branch.Fork(std::make_unique<core::NullCommand>(""));
    }
  }
}


TEST(History, Initial) {
  core::ManualClock clock;
  core::History history(&clock);

  ASSERT_EQ(&history.origin(), &history.head());
}

TEST(History, Clear) {
  core::ManualClock clock;
  core::History history(&clock);

  history.Clear();
  ASSERT_EQ(&history.origin(), &history.head());
  ASSERT_EQ(history.origin().branch().size(), 0);
}

TEST(History, ExecSequence) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  ::testing::Sequence seq;

  // Executes mock commands.
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<::testing::StrictMock<MockCommand>>();
    EXPECT_CALL(*command, Apply()).
        InSequence(seq).
        WillOnce(::testing::Return(true));

    auto ptr = command.get();
    ASSERT_TRUE(history.Exec(std::move(command)));
    ASSERT_EQ(&history.head().command(), ptr);
  }

  core::History::Item* item = &history.head();
  ASSERT_EQ(item->branch().size(), 0);

  // Checks if all items are connected properly.
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(item->parent().branch().size(), 1);
    ASSERT_EQ(item, item->parent().branch()[0].get());
    item = &item->parent();
  }
  ASSERT_EQ(&history.origin(), item);
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
    EXPECT_CALL(*command, Apply()).
        InSequence(seq).
        WillOnce(::testing::Return(true));

    commands.push_back(command.get());
    ASSERT_TRUE(history.Exec(std::move(command)));
  }

  // UnDo all.
  auto itr = commands.end();
  while (itr > commands.begin()) {
    --itr;
    EXPECT_CALL(**itr, Revert()).
        InSequence(seq).
        WillOnce(::testing::Return(true));

    ASSERT_EQ(&history.head().command(), *itr);
    ASSERT_TRUE(history.UnDo());
  }
  ASSERT_EQ(&history.origin(), &history.head());
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
    EXPECT_CALL(*command, Apply()).
        InSequence(seq).
        WillOnce(::testing::Return(true));
    EXPECT_CALL(*command, Revert()).
        InSequence(seq).
        WillOnce(::testing::Return(true));

    commands.push_back(command.get());
    ASSERT_TRUE(history.Exec(std::move(command)));
    ASSERT_TRUE(history.UnDo());
  }

  ASSERT_EQ(history.origin().branch().size(), kCount);
  ASSERT_EQ(&history.origin(), &history.head());

  // Checks branch is properly ordered.
  for (size_t i = 0; i < kCount; ++i) {
    ASSERT_EQ(&history.origin().branch()[i]->command(), commands[i]);
  }
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
    EXPECT_CALL(*command, Apply()).
        InSequence(seq).
        WillOnce(::testing::Return(true));

    commands.push_back(command.get());
    ASSERT_TRUE(history.Exec(std::move(command)));
  }

  // UnDo all.
  auto itr = commands.end();
  while (itr > commands.begin()) {
    --itr;
    EXPECT_CALL(**itr, Revert()).
        InSequence(seq).
        WillOnce(::testing::Return(true));
    ASSERT_TRUE(history.UnDo());
  }

  // ReDo all.
  for (auto command : commands) {
    EXPECT_CALL(*command, Apply()).
        InSequence(seq).
        WillOnce(::testing::Return(true));
    ASSERT_TRUE(history.ReDo());
    ASSERT_EQ(&history.head().command(), command);
  }
  ASSERT_EQ(history.head().branch().size(), 0);
}

TEST(History_Item, Fork) {
  static constexpr size_t kCount = 100;

  core::ManualClock clock;
  core::History history(&clock);

  std::vector<core::iCommand*> commands;

  auto& origin = history.origin();
  for (size_t i = 0; i < kCount; ++i) {
    auto command = std::make_unique<core::NullCommand>("");
    commands.push_back(command.get());
    origin.Fork(std::move(command));
    ASSERT_EQ(origin.branch().size(), i+1);
  }

  for (size_t i = 0; i < kCount; ++i) {
    auto& branch = *origin.branch()[i];
    ASSERT_EQ(&branch.parent(), &origin);
    ASSERT_EQ(branch.branch().size(), 0);
    ASSERT_EQ(&branch.command(), commands[i]);
  }
}

TEST(History_Item, DropAllAncestors) {
  static constexpr size_t kOriginBranchCount  = 5;
  static constexpr size_t kChildBranchCount = 5;

  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, kOriginBranchCount, kChildBranchCount);

  auto& origin = history.origin();

  ASSERT_TRUE(history.ReDo(kOriginBranchCount/2));
  ASSERT_TRUE(history.head().parent().isOrigin());

  ASSERT_TRUE(history.ReDo());
  ASSERT_TRUE(history.head().parent().parent().isOrigin());

  auto& next_origin = *origin.branch()[origin.branch().size()-1];
  next_origin.DropAllAncestors();

  ASSERT_EQ(&history.origin(), &next_origin);
  ASSERT_EQ(next_origin.branch().size(), kChildBranchCount);
  ASSERT_TRUE(next_origin.isOrigin());
}

TEST(History_Item, DropSelf) {
  static constexpr size_t kOriginBranchCount  = 5;

  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, kOriginBranchCount, 5);

  auto& origin = history.origin();
  for (size_t i = 0; i < kOriginBranchCount; ++i) {
    origin.branch()[0]->DropSelf();
    ASSERT_EQ(origin.branch().size(), kOriginBranchCount-i-1);
  }
}

TEST(History_Item, DropAllBranch) {
  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, 5, 5);

  auto& origin = history.origin();
  origin.DropAllBranch();
  ASSERT_EQ(origin.branch().size(), 0);
}

TEST(History_Item, IsAncestorOf) {
  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, 5, 5);

  auto& origin = history.origin();

  auto& child_0 = *origin.branch()[0];
  auto& child_1 = *origin.branch()[1];

  auto& child_0_0 = *child_0.branch()[0];
  auto& child_1_0 = *child_1.branch()[0];

  ASSERT_TRUE(origin.IsAncestorOf(child_0));
  ASSERT_TRUE(origin.IsAncestorOf(child_1));
  ASSERT_TRUE(origin.IsAncestorOf(child_0_0));
  ASSERT_TRUE(origin.IsAncestorOf(child_1_0));
  ASSERT_TRUE(origin.IsAncestorOf(origin));

  ASSERT_TRUE(!child_0.IsAncestorOf(origin));
  ASSERT_TRUE(!child_0_0.IsAncestorOf(child_0));
  ASSERT_TRUE(!child_0_0.IsAncestorOf(child_1));
  ASSERT_TRUE(!child_0_0.IsAncestorOf(child_1_0));
}

TEST(History_Item, FindLowestCommonAncestor) {
  core::ManualClock clock;
  core::History history(&clock);
  CreateTree(&history, 5, 5);

  auto& origin = history.origin();

  auto& child_0 = *origin.branch()[0];
  auto& child_1 = *origin.branch()[1];

  auto& child_0_0 = *child_0.branch()[0];
  auto& child_1_0 = *child_1.branch()[0];

  ASSERT_EQ(&child_0.FindLowestCommonAncestor(child_0), &child_0);
  ASSERT_EQ(&child_0.FindLowestCommonAncestor(child_0_0), &child_0);
  ASSERT_EQ(&child_0.FindLowestCommonAncestor(child_1), &origin);
  ASSERT_EQ(&child_0_0.FindLowestCommonAncestor(child_1_0), &origin);
}

}  // namespace mnian::test
