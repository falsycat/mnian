// No copyright
#include "mncore/action.h"
#include "mntest/action.h"

#include <gtest/gtest.h>


namespace mnian::test {

TEST(iAction, Accessors) {
  MockAction action;
  ASSERT_STREQ(action.name().c_str(),        MockAction::kName);
  ASSERT_STREQ(action.description().c_str(), MockAction::kDescription);
}

TEST(iActionable, Accessors) {
  MockActionable actionable;

  const auto& actions = actionable.actions();
  ASSERT_EQ(actions.size(), MockActionable::kActionCount);

  ASSERT_EQ(actions[0], &actionable.action1_);
  ASSERT_EQ(actions[1], &actionable.action2_);
  ASSERT_EQ(actions[2], &actionable.action3_);
}

TEST(MockAction, SimpleExec) {
  MockAction action;
  EXPECT_CALL(action, Exec(::testing::_));

  const core::iAction::Param param = {
    .reason = core::iAction::kUnknown,
  };
  action.Exec(param);
}

}  // namespace mnian::test
