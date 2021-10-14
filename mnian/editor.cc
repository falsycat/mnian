// No copyright
#include "mnian/editor.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <algorithm>
#include <string>

#include <Tracy.hpp>

#include "mnian/app.h"


namespace mnian {

static constexpr int64_t kWindowWidthMin  = 256;
static constexpr int64_t kWindowHeightMin = 256;


std::unique_ptr<Editor> Editor::DeserializeParam(core::iDeserializer* des) {
  ZoneScoped;

  auto ret = std::make_unique<Editor>();
  {
    core::iDeserializer::ScopeGuard _(des, std::string("widgets"));

    const auto n_ = des->size();
    if (!n_) {
      des->logger().MNCORE_LOGGER_WARN(
          "expected array but found not, all widgets have been dropped");
      des->LogLocation();
    }

    const size_t n = n_? *n_: 0;
    for (size_t i = 0; i < n; ++i) {
      core::iDeserializer::ScopeGuard __(des, i);

      auto w = des->DeserializeObject<core::iWidget>();
      if (w) {
        ret->widgets_.push_back(std::move(w));
      } else {
        des->logger().MNCORE_LOGGER_WARN(
            "deserialization failed, the widget has been dropped");
        des->LogLocation();
      }
    }
  }
  {
    core::iDeserializer::ScopeGuard _(des, std::string("settings"));

    auto text = des->value<std::string>("");
    ImGui::LoadIniSettingsFromMemory(text.data(), text.size());
  }
  {
    core::iDeserializer::ScopeGuard _(des, std::string("window"));

    des->Enter("w");
    const auto w = std::max(
        des->value<int64_t>(kWindowWidthMin), kWindowWidthMin);
    des->Leave();

    des->Enter("h");
    const auto h = std::max(
        des->value<int64_t>(kWindowHeightMin), kWindowHeightMin);
    des->Leave();

    des->Enter("x");
    const auto x = des->value<int64_t>(int64_t{0});
    des->Leave();

    des->Enter("y");
    const auto y = des->value<int64_t>(int64_t{0});
    des->Leave();

    auto win = App::instance().window();
    glfwSetWindowPos(win, static_cast<int>(x), static_cast<int>(y));
    glfwSetWindowSize(win, static_cast<int>(w), static_cast<int>(h));
  }
  return ret;
}

void Editor::SerializeParam(core::iSerializer* serial) const {
  ZoneScoped;

  core::iSerializer::ArrayGuard widgets(serial);
  for (const auto& w : widgets_) {
    widgets.Add(w.get());
  }

  size_t settinglen;
  auto setting = ImGui::SaveIniSettingsToMemory(&settinglen);

  core::iSerializer::MapGuard window(serial);
  {
    auto win = App::instance().window();
    int x, y, w, h;
    glfwGetWindowPos(win, &x, &y);
    glfwGetWindowSize(win, &w, &h);

    window.Add("x", static_cast<int64_t>(x));
    window.Add("y", static_cast<int64_t>(y));
    window.Add("w", static_cast<int64_t>(w));
    window.Add("h", static_cast<int64_t>(h));
  }

  core::iSerializer::MapGuard root(serial);
  root.Add("widgets", &widgets);
  root.Add("settings", std::string(setting, settinglen));
  root.Add("window", &window);
}

}  // namespace mnian
