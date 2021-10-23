// No copyright
#pragma once

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <cassert>
#include <string>

#include "mncore/app.h"
#include "mncore/clock.h"
#include "mncore/serialize.h"

#include "mnian/file.h"
#include "mnian/lang.h"
#include "mnian/logger.h"
#include "mnian/worker.h"


namespace mnian {

class App : public core::iApp {
 public:
  static App& instance() {
    return *instance_;
  }


  App() = delete;
  App(GLFWwindow* window, const core::DeserializerRegistry* reg);

  App(const App&) = delete;
  App(App&&) = delete;

  App& operator=(const App&) = delete;
  App& operator=(App&&) = delete;


  void Save() override;

  void Panic(const std::string& msg) override;
  void Quit() override;

  void Update();


  GLFWwindow* window() const {
    return window_;
  }
  bool alive() const {
    return alive_;
  }

  Lang& lang() {
    return lang_;
  }

 private:
  static App* instance_;


  bool Deserialize(core::iDeserializer* des);
  void Serialize(core::iSerializer* serial);

  void LoadInitialProject();


  bool alive_ = true;

  std::string panic_;


  Lang lang_;

  GLFWwindow* window_;

  core::RealClock clock_;

  TracyLogger logger_;

  FileStore fstore_;

  CpuWorker cpu_worker_;
};


// gettext() shorthand
static inline const char* _(const char* id) {
  return App::instance().lang().GetText(id);
}
static inline const char* _r(const char* id) {
  return id;
}

}  // namespace mnian
