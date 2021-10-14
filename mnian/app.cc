// No copyright
#include "mnian/app.h"

#include <filesystem>  // NOLINT(build/c++11)
#include <fstream>
#include <sstream>

#include <Tracy.hpp>

#include "mncore/serialize.h"


namespace mnian {

static constexpr size_t kCpuWorkerCount = 4;

static constexpr const char* kFileName = "mnian.json";

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

  if (std::filesystem::exists(kFileName)) {
    ZoneScopedN("load existing project");

    std::ifstream file(kFileName);
    auto json =
        core::iDeserializer::CreateJson(this, &logger(), &registry(), &file);
    if (!project().Deserialize(json.get())) {
      TracyMessageLCS(
          "failed to load existing project", tracy::Color::Red, true);
      // TODO(falsycat): show error and abort
      assert(false);
    }
  } else {
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
