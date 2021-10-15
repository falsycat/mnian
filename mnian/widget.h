// No copyright
#pragma once

#include "mncore/widget.h"

#include <string>


namespace mnian {

class ImGuiWidget : public core::iWidget {
 public:
  ImGuiWidget() = delete;
  explicit ImGuiWidget(const char* type) : iWidget(type) {
  }

  ImGuiWidget(const ImGuiWidget&) = delete;
  ImGuiWidget(ImGuiWidget&&) = delete;

  ImGuiWidget& operator=(const ImGuiWidget&) = delete;
  ImGuiWidget& operator=(ImGuiWidget&&) = delete;


  const std::string& strId() const {
    return str_id_;
  }

 protected:
  void ObserveNew() override {
    str_id_ = std::string(type()) + "##" + std::to_string(id());
  }

 private:
  std::string str_id_;
};

}  // namespace mnian
