// No copyright
//
// This file declares utilties for logging.
#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>


namespace mnian::core {

// This is an interface of logger, which is not necessary to be a thread-safe.
class iLogger {
 public:
  struct SrcLoc {
   public:
    const char* file;
    const char* func;
    size_t      line;
  };

  enum Level {
    kInfo,
    kWarn,
    kError,

    kAddition,  // used for supplement msg of the last one
  };


  iLogger() = default;
  virtual ~iLogger() = default;

  iLogger(const iLogger&) = default;
  iLogger(iLogger&&) = default;

  iLogger& operator=(const iLogger&) = default;
  iLogger& operator=(iLogger&&) = default;


  virtual void Write(
      Level level, const std::string& msg, SrcLoc loc) = 0;
};

#define MNCORE_LOGGER_SRCLOC_CURRENT  \
    ::mnian::core::iLogger::SrcLoc {__FILE__, __func__, size_t{__LINE__}}

#define MNCORE_LOGGER_WRITE(level, msg)  \
    Write((level), (msg), MNCORE_LOGGER_SRCLOC_CURRENT)

#define MNCORE_LOGGER_INFO(msg)  \
    MNCORE_LOGGER_WRITE(::mnian::core::iLogger::kInfo, (msg))
#define MNCORE_LOGGER_WARN(msg)  \
    MNCORE_LOGGER_WRITE(::mnian::core::iLogger::kWarn, (msg))
#define MNCORE_LOGGER_ERROR(msg)  \
    MNCORE_LOGGER_WRITE(::mnian::core::iLogger::kError, (msg))


// This logger broadcasts all of received messages to other loggers which are
// subscribing.
class BroadcastLogger : public iLogger {
 public:
  BroadcastLogger() = default;

  BroadcastLogger(const BroadcastLogger&) = delete;
  BroadcastLogger(BroadcastLogger&&) = delete;

  BroadcastLogger& operator=(const BroadcastLogger&) = delete;
  BroadcastLogger& operator=(BroadcastLogger&&) = delete;


  void Write(Level level, const std::string& msg, SrcLoc loc) override {
    for (auto subscriber : subscribers_) {
      subscriber->Write(level, msg, loc);
    }
  }

  void Subscribe(iLogger* logger) {
    assert(logger);
    subscribers_.push_back(logger);
  }
  bool Unsubscribe(iLogger* logger) {
    auto itr = std::find(subscribers_.begin(), subscribers_.end(), logger);
    if (itr == subscribers_.end()) {
      return false;
    }

    subscribers_.erase(itr);
    return true;
  }

 private:
  std::vector<iLogger*> subscribers_;
};

}  // namespace mnian::core
