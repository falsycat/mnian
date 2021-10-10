// No copyright
#pragma once

#include "mncore/history.h"


#include <gmock/gmock.h>


namespace mnian::test {

class MockHistoryObserver : public core::iHistoryObserver {
 public:
  MockHistoryObserver() = delete;
  explicit MockHistoryObserver(core::History* history) :
      iHistoryObserver(history) {
  }

  MockHistoryObserver(const MockHistoryObserver&) = delete;
  MockHistoryObserver(MockHistoryObserver&&) = delete;

  MockHistoryObserver& operator=(const MockHistoryObserver&) = delete;
  MockHistoryObserver& operator=(MockHistoryObserver&&) = delete;


  MOCK_METHOD(void, ObserveDrop, (), (override));
  MOCK_METHOD(void, ObserveFork, (), (override));
  MOCK_METHOD(void, ObserveMove, (), (override));
};

}  // namespace mnian::test
