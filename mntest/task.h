// No copyright
#pragma once

#include "mncore/task.h"

#include <gmock/gmock.h>


namespace mnian::test {

class MockTask : public core::iTask {
 public:
  MockTask() = default;

  MockTask(const MockTask&) = delete;
  MockTask(MockTask&&) = delete;

  MockTask& operator=(const MockTask&) = delete;
  MockTask& operator=(MockTask&&) = delete;


  MOCK_METHOD(void, DoExec, (), (override));
};

class MockLambda : public core::iLambda {
 public:
  MockLambda() = delete;
  MockLambda(size_t in, size_t out) : iLambda(in, out) {
  }

  MockLambda(const MockLambda&) = delete;
  MockLambda(MockLambda&&) = delete;

  MockLambda& operator=(const MockLambda&) = delete;
  MockLambda& operator=(MockLambda&&) = delete;


  MOCK_METHOD(void, DoExec, (), (override));


  using iLambda::in;
  using iLambda::out;
};

}  // namespace mnian::test
