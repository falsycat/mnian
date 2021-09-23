// No copyright
#pragma once
#include "mncore/logger.h"

#include <gmock/gmock.h>

#include <string>


namespace mnian::test {

class MockLogger : public core::iLogger {
 public:
  MockLogger() = default;

  MockLogger(const MockLogger&) = delete;
  MockLogger(MockLogger&&) = delete;

  MockLogger& operator=(const MockLogger&) = delete;
  MockLogger& operator=(MockLogger&&) = delete;


  MOCK_METHOD(void, Write, (Level, const std::string&, SrcLoc), (override));
};


}  // namespace mnian::test
