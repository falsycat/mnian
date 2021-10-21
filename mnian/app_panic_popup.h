// No copyright
#pragma once
#include "mnian/app.h"

#include <imgui.h>

#include <string>

#include "mncore/lang.h"


namespace mnian {

class App::PanicPopup final {
 public:
  using L = core::Lang;


  static constexpr const char* kPopupId = "PANIC##mnian::App::PanicPopup";

  static constexpr auto kPanicPopupFlags =
    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;


  PanicPopup() = delete;
  explicit PanicPopup(L* lang) :
    title_(lang, L::Hash("PANIC_TITLE")),
    abort_(lang, L::Hash("PANIC_ABORT")) {
  }

  PanicPopup(const PanicPopup&) = delete;
  PanicPopup(PanicPopup&&) = delete;

  PanicPopup& operator=(const PanicPopup&) = delete;
  PanicPopup& operator=(PanicPopup&&) = delete;


  void Open(const std::string& msg) {
    open_ = true;
    msg_  = msg;
  }


  void Update() {
    if (ImGui::BeginPopupModal(kPopupId, nullptr, kPanicPopupFlags)) {
      const auto window = ImGui::GetWindowSize();
      const auto region = ImGui::GetContentRegionAvail();

      const auto icon = ImGui::CalcTextSize(title_.c());
      ImGui::SetCursorPosX((window.x - icon.x)/2);
      ImGui::Text("%s", title_.c());

      ImGui::Text("%s", msg_.c_str());

      const auto size = ImVec2(region.x, 0);
      if (ImGui::Button(abort_.c(), size)) {
        aborted_ = true;
      }
      ImGui::SetItemDefaultFocus();
      ImGui::EndPopup();
      return;
    }
    if (open_) {
      ImGui::OpenPopup(kPopupId);
      return;
    }
  }


  bool opened() const {
    return open_;
  }
  bool aborted() const {
    return aborted_;
  }

 private:
  L::Text title_;
  L::Text abort_;

  bool open_    = false;
  bool aborted_ = false;

  std::string msg_;
};

}  // namespace mnian
