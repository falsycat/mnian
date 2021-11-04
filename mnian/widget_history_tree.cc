// No copyright
#include "mnian/widget_history_tree.h"

#include <imgui.h>

#include <algorithm>
#include <cstring>
#include <queue>
#include <stack>
#include <vector>

#include "mnian/app.h"


namespace mnian {

void HistoryTreeWidget::Register(core::DeserializerRegistry* reg) {
  reg->RegisterType<core::iWidget, HistoryTreeWidget>();
}


void HistoryTreeWidget::Update() {
  if (ImGui::Begin(strId().c_str())) {
    if (!ImGui::IsWindowHovered()) {
      open_.clear();
    }
    UpdateItem(&app_->project().history().origin());
  }
  ImGui::End();
}

bool HistoryTreeWidget::UpdateItem(core::History::Item* item, bool skip_text) {
  const auto& style       = ImGui::GetStyle();
  const auto  text_color  = ImGui::GetStyleColorVec4(ImGuiCol_Text);
  const float indent_unit = style.IndentSpacing*1.f;
  const float bullet_size = ImGui::GetTreeNodeToLabelSpacing();
  const auto  start_gpos  = ImGui::GetCursorScreenPos();

  auto draw = ImGui::GetWindowDrawList();

  bool skip_branch = false;
  for (;;) {
    const auto& branch = item->branch();
    ImGui::PushID(item);

    ImVec4 col {1, 1, 1, 1};
    if (item == &app_->project().history().head()) {
      col = {1, 0, 0, 1};
    }

    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, col);

    ImGui::Bullet();
    if (!skip_text) {
      ImGui::Text(item->command().GetDescription().c_str());
    }

    ImGui::PopStyleColor();
    ImGui::EndGroup();
    ImGui::SameLine();

    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip(item->command().GetDescription().c_str());
      Hover(item);
    }
    if (ImGui::IsItemClicked()) {
      MoveTo(item);
    }
    ImGui::PopID();
    ImGui::NewLine();

    // draw the item's branches recursively
    const auto gpos = ImGui::GetCursorScreenPos();
    if (!skip_branch && open_.contains(item)) {
      const auto pos = ImGui::GetCursorPos();
      size_t i;
      for (i = 1; i < branch.size() && !skip_branch; ++i) {
        const float indent = indent_unit*static_cast<float>(i);
        ImGui::Indent(indent);
        if (UpdateItem(branch[branch.size()-i-1].get(), true)) {
          skip_branch = true;
        }
        ImGui::Unindent(indent);
        ImGui::SetCursorPos(pos);
      }
      skip_text   = true;
      skip_branch = true;

      const float w = static_cast<float>(i)*indent_unit;
      draw->AddLine({gpos.x  +bullet_size/2.f, gpos.y},
                    {gpos.x+w-bullet_size/2.f, gpos.y},
                    ImGui::GetColorU32(text_color));
    }

    if (branch.size() == 0) {
      draw->AddLine({gpos.x+bullet_size/2.f, start_gpos.y},
                    {gpos.x+bullet_size/2.f, gpos.y},
                    ImGui::GetColorU32(text_color));
      return skip_branch;
    }
    item = branch.back().get();
  }
}


void HistoryTreeWidget::Hover(core::History::Item* item) {
  if (open_.contains(item)) return;

  open_.clear();
  if (item->branch().size() > 1) {
    open_.insert(item);
  }

  while (!item->isOrigin()) {
    auto parent = &item->parent();
    if (parent->branch().back().get() != item) {
      open_.insert(parent);
    }
    item = parent;
  }
}

void HistoryTreeWidget::MoveTo(core::History::Item* item) {
  auto& history = app_->project().history();
  auto* head    = &history.head();

  auto lca = &item->FindLowestCommonAncestor(*head);
  while (&history.head() != lca) history.UnDo();

  std::stack<core::History::Item*> path;
  for (auto itr = item; itr != lca; itr = &itr->parent()) {
    path.push(itr);
  }

  while (!path.empty()) {
    const auto& branch = history.head().branch();

    size_t i;
    for (i = 0; branch[i].get() != path.top(); ++i) {}
    history.ReDo(i);
    path.pop();
  }
}


std::unique_ptr<HistoryTreeWidget> HistoryTreeWidget::DeserializeParam(
    core::iDeserializer* des) {
  (void) des;
  return std::make_unique<HistoryTreeWidget>(&des->app());
}

void HistoryTreeWidget::SerializeParam(core::iSerializer* serial) const {
    serial->SerializeValue(int64_t{0});
}

}  // namespace mnian
