// No copyright
#include "mnian/popup_input.h"

#include <imgui_stdlib.h>

#include "mnian/app.h"


namespace mnian {

static constexpr ImGuiWindowFlags kWinFlags =
    ImGuiWindowFlags_NoDecoration;

static constexpr ImGuiInputTextFlags kInputFlags =
    ImGuiInputTextFlags_AutoSelectAll;


InputPopup::InputPopup(const std::string& id, const char* msg) :
    id_(id), msg_(msg) {
}


void InputPopup::Update() {
  const float em = ImGui::GetFontSize();

  ImGui::SetNextWindowSize({16*em, 0});
  if (ImGui::BeginPopup(id_.c_str(), kWinFlags)) {
    bool enter  = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Enter));
    bool cancel = ImGui::IsKeyDown(ImGui::GetKeyIndex(ImGuiKey_Escape));

    // label
    ImGui::Text(App::instance().lang().GetText(msg_));
    ImGui::Indent();

    // input box
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth());
    ImGui::InputText("", &data_, kInputFlags);
    if (!ImGui::IsItemActive()) ImGui::SetKeyboardFocusHere(-1);

    // validation message
    const auto err = ValidateData();
    const auto y   = ImGui::GetCursorPosY();
    const auto h   = ImGui::GetFrameHeight();

    ImGui::SetCursorPosY(y + h/2 - em/2);
    if (err) {
      ImGui::TextWrapped(_("invalid: %s"), err->c_str());
    } else {
      ImGui::TextWrapped(_("valid"));
      ImGui::SameLine();

      // submit button
      const char* ok = _("OK");
      ImGui::SetCursorPos({0, y});
      ImGui::SetCursorPosX(
          ImGui::GetContentRegionAvailWidth() - 2*em);

      // returns always false because the text input keeps focus
      ImGui::Button(ok, {2*em, 0});
      enter = enter || ImGui::IsItemClicked();
    }
    ImGui::SetCursorPosY(y+h);
    ImGui::Unindent();

    // submit
    if (!err && enter) {
      handler_(data_);
      handler_ = [](auto) {};
      ImGui::CloseCurrentPopup();
    }

    // cancel
    if (cancel) {
      ImGui::CloseCurrentPopup();
    }

    ImGui::EndPopup();
    open_ = false;
  }
  if (open_) {
    ImGui::OpenPopup(id_.c_str());
  }
}

}  // namespace mnian
