// No copyright
//
// Context data for application or project lifetime.
#pragma once

#include <memory>
#include <string>

#include "mncore/clock.h"
#include "mncore/dir.h"
#include "mncore/file.h"
#include "mncore/history.h"
#include "mncore/lang.h"
#include "mncore/logger.h"
#include "mncore/serialize.h"
#include "mncore/task.h"
#include "mncore/widget.h"


namespace mnian::core {


class iApp {
 public:
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


    Dir& root() {
      return *root_;
    }
    NodeStore& nstore() {
      return nstore_;
    }
    WidgetStore& wstore() {
      return wstore_;
    }
    History& history() {
      return history_;
    }

   private:
    std::unique_ptr<Dir> root_;

    NodeStore nstore_;

    WidgetStore wstore_;

    History history_;
  };


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


  virtual void Save() = 0;

  virtual void Panic(const std::string&) = 0;
  virtual void Quit() = 0;


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

  Lang& lang() {
    return lang_;
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

  Lang lang_;

  TaskQueue main_, cpu_, gl3_;
};

}  // namespace mnian::core
