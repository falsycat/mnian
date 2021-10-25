// No copyright
#include "mncore/command.h"
#include "mntest/command.h"

#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>


namespace mnian::test {

TEST(NullCommand, Accessors) {
  core::NullCommand cmd("helloworld");
  ASSERT_EQ(cmd.description(), "helloworld");
}

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

  core::SquashedCommand squashed(std::move(cmds_));
  squashed.Apply();
  squashed.Revert();
}

TEST(SquashedCommand, ApplyAndRevertEmpty) {
  core::SquashedCommand squashed({});
  squashed.Apply();
  squashed.Revert();
}

}  // namespace mnian::test
