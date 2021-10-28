// No copyright
#pragma once

#include "mncore/command.h"

#include <gmock/gmock.h>

#include <string>


namespace mnian::test {

class MockCommand : public core::iCommand {
 public:
  static constexpr const char* kType = "Mock";


  MockCommand() : iCommand(kType) {
  }

  MockCommand(const MockCommand&) = delete;
  MockCommand(MockCommand&&) = delete;

  MockCommand& operator=(const MockCommand&) = delete;
  MockCommand& operator=(MockCommand&&) = delete;


  MOCK_METHOD(void, Apply, (), (override));
  MOCK_METHOD(void, Revert, (), (override));

  MOCK_METHOD(void, SerializeParam, (core::iSerializer*), (const override));

  MOCK_METHOD(std::string, description, (), (const override));
};

}  // namespace mnian::test
