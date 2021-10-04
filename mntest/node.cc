// No copyright
#include "mncore/node.h"
#include "mntest/node.h"

#include <gtest/gtest.h>


namespace mnian::test {

TEST(iNodeObserver, Lifetime) {
  auto node = std::make_unique<::testing::StrictMock<MockNode>>();

  MockNodeObserver observer1(node.get());
  {
    MockNodeObserver observer2(node.get());
  }
  EXPECT_CALL(observer1, ObserveDelete());
  node = nullptr;
}


}  // namespace mnian::test
