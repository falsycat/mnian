// No copyright
#pragma once

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <cassert>
#include <memory>
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
  ~App();

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

 private:
  class Menu;
  class PanicPopup;


  void BuildDefaultLang();


  bool Deserialize(core::iDeserializer* des);
  void Serialize(core::iSerializer* serial);


  static App* instance_;


  bool alive_ = true;


  GLFWwindow* window_;

  core::RealClock clock_;

  TracyLogger logger_;

  FileStore fstore_;

  CpuWorker cpu_worker_;


  std::unique_ptr<Menu>       menu_;
  std::unique_ptr<PanicPopup> panic_;
};

}  // namespace mnian
