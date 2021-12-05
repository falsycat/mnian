// No copyright
#pragma once

#include "mncore/action.h"

#include <gmock/gmock.h>

#include <string>


namespace mnian::test {

class MockAction : public core::iAction {
 public:
  MockAction() = default;

  MockAction(const MockAction&) = delete;
  MockAction(MockAction&&) = delete;

  MockAction& operator=(const MockAction&) = delete;
  MockAction& operator=(MockAction&&) = delete;


  MOCK_METHOD(void, Exec, (const Param&), (const override));

  MOCK_METHOD(std::string, GetName, (), (const override));
  MOCK_METHOD(std::string, GetDescription, (), (const override));
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
