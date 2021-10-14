// No copyright
#pragma once

#include <string>

#include <Tracy.hpp>

#include "mncore/logger.h"


namespace mnian {

class TracyLogger : public core::iLogger {
 public:
  static void Info(const std::string& msg) {
    TracyMessageCS(msg.data(), msg.size(), tracy::Color::White, true);
  }
  static void Warn(const std::string& msg) {
    TracyMessageCS(msg.data(), msg.size(), tracy::Color::Yellow, true);
  }
  static void Error(const std::string& msg) {
    TracyMessageCS(msg.data(), msg.size(), tracy::Color::Red, true);
  }
  static void Addition(const std::string& msg) {
    TracyMessageCS(msg.data(), msg.size(), tracy::Color::Gray, true);
  }


  TracyLogger() = default;

  TracyLogger(const TracyLogger&) = delete;
  TracyLogger(TracyLogger&&) = delete;

  TracyLogger& operator=(const TracyLogger&) = delete;
  TracyLogger& operator=(TracyLogger&&) = delete;


  void Write(Level level, const std::string& msg, SrcLoc) override {
    switch (level) {
    case kWarn:
      Warn(msg);
      break;
    case kError:
      Error(msg);
      break;
    case kAddition:
      Addition(msg);
      break;
    default:
      Info(msg);
      break;
    }
  }
};

}  // namespace mnian
