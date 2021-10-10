// No copyright
//
// Context data for application or project lifetime.
#pragma once

#include <memory>
#include <string>

#include "mncore/clock.h"
#include "mncore/dir.h"
#include "mncore/editor.h"
#include "mncore/file.h"
#include "mncore/history.h"
#include "mncore/logger.h"
#include "mncore/serialize.h"
#include "mncore/task.h"


namespace mnian::core {

class Project final : public iSerializable {
 public:
  Project() = delete;
  explicit Project(const iClock* clock) : history_(clock) {
  }

  Project(const Project&) = delete;
  Project(Project&&) = delete;

  Project& operator=(const Project&) = delete;
  Project& operator=(Project&&) = delete;


  bool Deserialize(iDeserializer*);

  void Serialize(iSerializer*) const override;


  History& history() {
    return history_;
  }
  Dir& root() {
    return *root_;
  }
  iEditor& editor() {
    return *editor_;
  }

 private:
  History history_;

  std::unique_ptr<Dir> root_;

  std::unique_ptr<iEditor> editor_;
};


class iApp {
 public:
  iApp() = delete;
  iApp(const iClock*               clock,
       const DeserializerRegistry* reg,
       iLogger*                    logger,
       iFileStore*                 fstore) :
      clock_(clock),
      reg_(reg),
      logger_(logger),
      fstore_(fstore),
      project_(clock_) {
    assert(clock_);
    assert(reg_);
    assert(logger_);
    assert(fstore_);
  }
  virtual ~iApp() = default;

  iApp(const iApp&) = delete;
  iApp(iApp&&) = delete;

  iApp& operator=(const iApp&) = delete;
  iApp& operator=(iApp&&) = delete;


  virtual void Load(const std::string&) = 0;
  virtual void Save() = 0;


  const iClock& clock() {
    return *clock_;
  }
  const DeserializerRegistry& registry() {
    return *reg_;
  }
  iLogger& logger() {
    return *logger_;
  }
  iFileStore& fstore() {
    return *fstore_;
  }
  Project& project() {
    return project_;
  }

  TaskQueue& mainQ() {
    return main_;
  }
  TaskQueue& cpuQ() {
    return cpu_;
  }
  TaskQueue& gl3Q() {
    return gl3_;
  }

 private:
  const iClock* clock_;

  const DeserializerRegistry* reg_;

  iLogger* logger_;

  iFileStore* fstore_;

  Project project_;

  TaskQueue main_, cpu_, gl3_;
};

}  // namespace mnian::core
