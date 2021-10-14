// No copyright
#include "mnian/app.h"

#include <sstream>

#include <Tracy.hpp>

#include "mncore/serialize.h"


namespace mnian {

static constexpr size_t kCpuWorkerCount = 4;

static constexpr const char* kInitialProject = R"({
  "editor": {
    "type" : "ImGUI",
    "param": {
      "widgets": [
        {
          "type" : "ProjectView",
          "param": {
          }
        },
      ],
    },
  },
  "root": {
    "type" : "GenericDir",
    "param": {
    },
  },
  "history": {
  },
})";


App* App::instance_ = nullptr;


App::App(GLFWwindow* window, const core::DeserializerRegistry* reg) :
    iApp(&clock_, reg, &logger_, &fstore_),
    window_(window), cpu_worker_(&cpuQ(), kCpuWorkerCount) {
  instance_ = this;

  ZoneScoped;
  {
    ZoneScopedN("load initial project");
    std::stringstream st(kInitialProject);
    auto des = core::iDeserializer::CreateJson(
        this, &logger(), &registry(), &st);
    project().Deserialize(des.get());
  }
}


void App::Load(const std::string&) {
  assert(false);
}

void App::Save() {
  assert(false);
}


void App::Update() {
  ZoneScoped;

  project().editor().Update();
}

}  // namespace mnian
