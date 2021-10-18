// No copyright
#include "mncore/node.h"
#include "mntest/node.h"

#include <gtest/gtest.h>


namespace mnian::test {

TEST(iNodeObserver, Lifetime) {
  auto node = std::make_unique<::testing::StrictMock<MockNode>>();

  ::testing::StrictMock<MockNodeObserver> observer1(node.get());
  {
    ::testing::StrictMock<MockNodeObserver> observer2(node.get());
  }
  EXPECT_CALL(observer1, ObserveDelete());
  node = nullptr;
}

TEST(iNodeObserver, ObserveNewAndDelete) {
  auto node = std::make_unique<::testing::StrictMock<MockNode>>();

  ::testing::StrictMock<MockNodeObserver> observer(node.get());
  EXPECT_CALL(observer, ObserveNew());
  EXPECT_CALL(observer, ObserveDelete());

  core::NodeStore store;
  store.Add(std::move(node));
}

}  // namespace mnian::test
