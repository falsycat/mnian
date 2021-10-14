// No copyright
#pragma once

#include <GLFW/glfw3.h>

#include <cassert>
#include <string>

#include "mncore/app.h"
#include "mncore/clock.h"
#include "mncore/serialize.h"

#include "mnian/file.h"
#include "mnian/logger.h"


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

  void Update();


  GLFWwindow* window() const {
    return window_;
  }

 private:
  static App* instance_;


  GLFWwindow* window_;

  core::RealClock clock_;

  TracyLogger logger_;

  FileStore fstore_;
};

}  // namespace mnian
