// No copyright
#pragma once

#include <memory>

#include "mncore/editor.h"


namespace mnian {

class ProjectViewWidget : public core::iWidget {
 public:
  static constexpr const char* kType = "ProjectView";


  static std::unique_ptr<ProjectViewWidget> DeserializeParam(
      core::iDeserializer*);


  ProjectViewWidget() : iWidget(kType) {
  }

  ProjectViewWidget(const ProjectViewWidget&) = delete;
  ProjectViewWidget(ProjectViewWidget&&) = delete;

  ProjectViewWidget& operator=(const ProjectViewWidget&) = delete;
  ProjectViewWidget& operator=(ProjectViewWidget&&) = delete;


  void Update() override;

  void SerializeParam(core::iSerializer*) const override;
};

}  // namespace mnian
