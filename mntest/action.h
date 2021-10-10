// No copyright
#pragma once

#include "mncore/action.h"

#include <gmock/gmock.h>


namespace mnian::test {

class MockAction : public core::iAction {
 public:
  static constexpr const char* kName        = "MockAction";
  static constexpr const char* kDescription = "mock action for gtest";

  static inline const Meta kDefaultMeta = {
    .name        = kName,
    .description = kDescription,
  };


  MockAction() : iAction(Meta(kDefaultMeta)) {
  }

  MockAction(const MockAction&) = delete;
  MockAction(MockAction&&) = delete;

  MockAction& operator=(const MockAction&) = delete;
  MockAction& operator=(MockAction&&) = delete;


  MOCK_METHOD1(Exec, void(const Param&));
};

class MockActionable : public core::iActionable {
 public:
  static constexpr size_t kActionCount = 3;


  MockActionable() : iActionable({&action1_, &action2_, &action3_}) {
  }

  MockActionable(const MockActionable&) = delete;
  MockActionable(MockActionable&&) = delete;

  MockActionable& operator=(const MockActionable&) = delete;
  MockActionable& operator=(MockActionable&&) = delete;


  MockAction action1_;
  MockAction action2_;
  MockAction action3_;
};

}  // namespace mnian::test
