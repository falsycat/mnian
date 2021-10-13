// No copyright
#include "mnian/widget_project_view.h"

#include <imgui.h>


namespace mnian {

void ProjectViewWidget::Update() {
  ImGui::Begin("ProjectView");
  ImGui::Text("hello world");
  ImGui::End();
}


std::unique_ptr<ProjectViewWidget> ProjectViewWidget::DeserializeParam(
    core::iDeserializer*) {
  return std::make_unique<ProjectViewWidget>();
}

void ProjectViewWidget::SerializeParam(core::iSerializer* serial) const {
  serial->SerializeValue(int64_t{0});
}

}  // namespace mnian
