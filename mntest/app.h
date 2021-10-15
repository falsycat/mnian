// No copyright
#pragma once

#include "mncore/app.h"

#include <gmock/gmock.h>

#include <string>


namespace mnian::test {

class MockApp : public core::iApp {
 public:
  MockApp() = delete;
  MockApp(const core::iClock*               clock,
          const core::DeserializerRegistry* reg,
          core::iLogger*                    logger,
          core::iFileStore*                 fstore) :
      iApp(clock, reg, logger, fstore) {
  }

  MockApp(const MockApp&) = delete;
  MockApp(MockApp&&) = delete;

  MockApp& operator=(const MockApp&) = delete;
  MockApp& operator=(MockApp&&) = delete;


  MOCK_METHOD(void, Save, (), (override));

  MOCK_METHOD(void, Panic, (const std::string&), (override));
  MOCK_METHOD(void, Quit, (), (override));
};

}  // namespace mnian::test
