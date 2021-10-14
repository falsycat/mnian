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


  void Load(const std::string&) override;
  void Save() override;

  void Abort(const std::string& msg);

  void Update();


  GLFWwindow* window() const {
    return window_;
  }
  bool alive() const {
    return alive_;
  }

 private:
  static App* instance_;


  bool alive_ = true;

  std::string panic_;


  GLFWwindow* window_;

  core::RealClock clock_;

  TracyLogger logger_;

  FileStore fstore_;

  CpuWorker cpu_worker_;
};

}  // namespace mnian
