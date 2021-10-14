// No copyright
#include "mnian/app.h"

#include <fontawesome.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

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


static constexpr const char* kPanicPopupId = "PANIC##mnian/app";

static constexpr auto kPanicPopupFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;


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
    if (!json) {
      TracyMessageLCS("invalid JSON", tracy::Color::Red, true);
      Abort("failed to parse JSON");
      return;
    }
    if (!project().Deserialize(json.get())) {
      Abort("failed to load existing project");
      return;
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


void App::Abort(const std::string& msg) {
  panic_ = msg;
}


void App::Update() {
  ZoneScoped;

  // panic popup
  if (ImGui::BeginPopupModal(kPanicPopupId, nullptr, kPanicPopupFlags)) {
    const auto window = ImGui::GetWindowSize();
    const auto region = ImGui::GetContentRegionAvail();

    const auto icon = ImGui::CalcTextSize(ICON_FA_SKULL_CROSSBONES);
    ImGui::SetCursorPosX((window.x - icon.x)/2);
    ImGui::Text(ICON_FA_SKULL_CROSSBONES);

    ImGui::Text("%s", panic_.c_str());
    ImGui::Separator();

    const auto size = ImVec2(region.x, 0);
    if (ImGui::Button("ABORT", size)) {
      alive_ = false;
    }
    ImGui::SetItemDefaultFocus();
    ImGui::EndPopup();
    return;
  }

  // opens panic popup
  if (panic_.size()) {
    ImGui::OpenPopup(kPanicPopupId);
    return;
  }

  // close window
  if (glfwWindowShouldClose(window_)) {
    alive_ = false;
    return;
  }

  // dequeue taskq
  while (mainQ().Dequeue()) continue;

  // update editor
  project().editor().Update();

  // debug
# if !defined(NDEBUG)
    ImGui::ShowDemoWindow();
# endif
}

}  // namespace mnian
