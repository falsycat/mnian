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

#include "mnres/all.h"


namespace mnian {

static constexpr size_t kCpuWorkerCount = 4;

static constexpr const char* kFileName = "mnian.json";

static constexpr const char* kInitialProject = R"({
  "project": {
    "root": {
      "type" : "mnian::core::Dir",
      "param": {},
    },
    "nstore": [],
    "wstore": [
      {
        "id": 0,
        "entity": {
          "type" : "ProjectView",
          "param": {}
        },
      },
    ],
    "history": {
    },
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

  // load default language
  {
    std::stringstream st;
    st.write(
        reinterpret_cast<const char*>(mnian::res::lang::kEnglish),
        static_cast<std::streamsize>(mnian::res::lang::kEnglishSize));
    lang_.Merge(&st, &logger_);
  }

  // load project
  if (std::filesystem::exists(kFileName)) {
    ZoneScopedN("load existing project");

    std::ifstream file(kFileName);
    if (!file) {
      TracyMessageLCS("failed to open file to read", tracy::Color::Red, true);
      Panic(_("failed to open file"));
      return;
    }

    auto des =
        core::iDeserializer::CreateJson(this, &logger(), &registry(), &file);
    if (!des) {
      TracyMessageLCS("invalid JSON", tracy::Color::Red, true);
      Panic(_("failed to parse JSON"));
      return;
    }
    if (!Deserialize(des.get())) {
      Panic(_("failed to load existing project"));
      return;
    }
  } else {
    ZoneScopedN("load initial project");
    std::stringstream st(kInitialProject);

    auto des =
        core::iDeserializer::CreateJson(this, &logger(), &registry(), &st);
    assert(des);

    if (!Deserialize(des.get())) {
      assert(false);
    }
  }
}


void App::Save() {
  std::ofstream file(kFileName);
  if (!file) {
    TracyMessageLCS("failed to open file to write", tracy::Color::Red, true);
    return;
  }
  auto des = core::iSerializer::CreateJson(&file);
  Serialize(des.get());
}


void App::Panic(const std::string& msg) {
  assert(msg.size());
  panic_ = msg;
}

void App::Quit() {
  alive_ = false;
  Save();
}


void App::Update() {
  ZoneScoped;

  // app menu
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu(_("App"))) {
      if (ImGui::MenuItem(_("Save"))) { Save(); }
      if (ImGui::MenuItem(_("Quit"))) { Quit(); }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }

  // panic popup
  if (ImGui::BeginPopupModal(kPanicPopupId, nullptr, kPanicPopupFlags)) {
    const auto window = ImGui::GetWindowSize();
    const auto region = ImGui::GetContentRegionAvail();

    const auto icon = ImGui::CalcTextSize(ICON_FA_SKULL_CROSSBONES);
    ImGui::SetCursorPosX((window.x - icon.x)/2);
    ImGui::Text(ICON_FA_SKULL_CROSSBONES);

    ImGui::Text("%s", panic_.c_str());

    const auto size = ImVec2(region.x, 0);
    if (ImGui::Button(_("ABORT"), size)) {
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
    Quit();
    return;
  }

  // dequeue taskq
  while (mainQ().Dequeue()) continue;

  // update editor
  project().wstore().Update();

  // debug
# if !defined(NDEBUG)
    ImGui::ShowDemoWindow();
# endif
}


bool App::Deserialize(core::iDeserializer* des) {
  {
    core::iDeserializer::ScopeGuard _(des, std::string("window"));

    des->Enter(std::string("x"));
    const auto x = des->value(int{100});
    des->Leave();

    des->Enter(std::string("y"));
    const auto y = des->value(int{100});
    des->Leave();

    des->Enter(std::string("w"));
    const auto w = des->value(int{640});
    des->Leave();

    des->Enter(std::string("h"));
    const auto h = des->value(int{480});
    des->Leave();

    glfwSetWindowPos(window_, x, y);
    glfwSetWindowSize(window_, w, h);
  }

  {
    core::iDeserializer::ScopeGuard _(des, std::string("imgui"));

    const auto settings = des->value<std::string>("");
    ImGui::LoadIniSettingsFromMemory(settings.data(), settings.size());
  }

  {
    core::iDeserializer::ScopeGuard _(des, std::string("project"));
    if (!project().Deserialize(des)) return false;
  }
  return true;
}

void App::Serialize(core::iSerializer* serial) {
  core::iSerializer::MapGuard window(serial);
  {
    int x, y, w, h;
    glfwGetWindowPos(window_, &x, &y);
    glfwGetWindowSize(window_, &w, &h);
    window.Add("x", static_cast<int64_t>(x));
    window.Add("y", static_cast<int64_t>(y));
    window.Add("w", static_cast<int64_t>(w));
    window.Add("h", static_cast<int64_t>(h));
  }

  size_t imgui_len;
  const auto imgui = ImGui::SaveIniSettingsToMemory(&imgui_len);

  core::iSerializer::MapGuard root(serial);
  root.Add("window", &window);
  root.Add("imgui", std::string(imgui, imgui_len));
  root.Add("project", &project());
}

}  // namespace mnian
