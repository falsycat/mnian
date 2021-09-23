// No copyright
#include "mncore/logger.h"
#include "mntest/logger.h"

#include <gtest/gtest.h>

#include <memory>
#include <utility>
#include <vector>


namespace mnian::test {

TEST(BroadcastLogger, Broadcast) {
  static constexpr size_t kCount = 100;

  std::vector<std::unique_ptr<core::iLogger>> subs;

  core::BroadcastLogger broadcaster;
  for (size_t i = 0; i < kCount; ++i) {
    auto sub = std::make_unique<::testing::StrictMock<MockLogger>>();
    EXPECT_CALL(
        *sub,
        Write(core::BroadcastLogger::kWarn, std::string("msg"), ::testing::_));
    broadcaster.Subscribe(sub.get());
    subs.push_back(std::move(sub));
  }
  broadcaster.MNCORE_LOGGER_WARN("msg");
}

TEST(BroadcastLogger, Unsubscribe) {
  static constexpr size_t kCount = 100;

  std::vector<std::unique_ptr<core::iLogger>> subs;

  core::BroadcastLogger broadcaster;
  for (size_t i = 0; i < kCount; ++i) {
    auto sub = std::make_unique<::testing::StrictMock<MockLogger>>();
    broadcaster.Subscribe(sub.get());
    subs.push_back(std::move(sub));
  }
  for (auto& sub : subs) {
    broadcaster.Unsubscribe(sub.get());
  }
  broadcaster.MNCORE_LOGGER_WARN("msg");
}

}   // namespace mnian::test
