// No copyright
#include "mnian/app.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <filesystem>  // NOLINT(build/c++11)
#include <fstream>
#include <sstream>

#include <Tracy.hpp>

#include "mncore/serialize.h"

#include "mnian/app_menu.h"
#include "mnian/app_panic_popup.h"
#include "mnian/app_project.h"


namespace mnian {

static constexpr size_t kCpuWorkerCount = 4;

static constexpr const char* kFileName = "mnian.json";


App* App::instance_ = nullptr;


App::App(GLFWwindow* window, const core::DeserializerRegistry* reg) :
    iApp(&clock_, reg, &logger_, &fstore_),
    window_(window), cpu_worker_(&cpuQ(), kCpuWorkerCount),
    menu_(std::make_unique<Menu>(this)),
    panic_(std::make_unique<PanicPopup>(&lang())) {
  instance_ = this;
  BuildDefaultLang();

  if (std::filesystem::exists(kFileName)) {
    ZoneScopedN("load existing project");

    std::ifstream file(kFileName);
    if (!file) {
      TracyMessageLCS("failed to open file to read", tracy::Color::Red, true);
      Panic(lang().Translate("ERR_FILE_OPEN"));
      return;
    }

    auto des =
        core::iDeserializer::CreateJson(this, &logger(), &registry(), &file);
    if (!des) {
      TracyMessageLCS("invalid JSON", tracy::Color::Red, true);
      Panic(lang().Translate("ERR_PARSE_JSON"));
      return;
    }
    if (!Deserialize(des.get())) {
      Panic(lang().Translate("ERR_DESERIALIZE"));
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
App::~App() {
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
  panic_->Open(msg);
}

void App::Quit() {
  alive_ = false;
  Save();
}


void App::Update() {
  ZoneScoped;

  menu_->Update();

  // panic popup
  panic_->Update();
  if (panic_->aborted()) {
    alive_ = false;
  }
  if (panic_->opened()) return;

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
