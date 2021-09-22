// No copyright
#pragma once

#include "mncore/app.h"


namespace mnian::test {

class MockApp : public core::iApp {
 public:
  MockApp() = default;

  MockApp(const MockApp&) = delete;
  MockApp(MockApp&&) = delete;

  MockApp& operator=(const MockApp&) = delete;
  MockApp& operator=(MockApp&&) = delete;
};

}  // namespace mnian::test
