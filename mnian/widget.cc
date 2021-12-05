// No copyright
#include "mnian/widget.h"

#include <imgui.h>

#include "mnian/app.h"


namespace mnian {

bool ImGuiWidget::MenuOfActionable(const core::iActionable& target) {
  if (!target.actions().size()) {
    ImGui::MenuItem(_("(no actions)"), NULL, false, false);
    return false;
  }

  bool ret = false;
  for (auto act : target.actions()) {
    if (ImGui::MenuItem(act->GetName().c_str())) {
      act->Exec(core::iAction::Param {core::iAction::kUnknown});
      ret = true;
    }
  }
  return ret;
}

}  // namespace mnian
