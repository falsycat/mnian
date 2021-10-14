// No copyright
#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>
#include <vector>

#include "mncore/editor.h"


namespace mnian {

class Editor : public core::iEditor {
 public:
  static constexpr const char* kType = "ImGUI";


  // Call after ImGui::CreateContext() and before ImGui::NewFrame().
  static std::unique_ptr<Editor> DeserializeParam(core::iDeserializer*);


  Editor() : iEditor(kType) {
  }

  Editor(const Editor&) = delete;
  Editor(Editor&&) = delete;

  Editor& operator=(const Editor&) = delete;
  Editor& operator=(Editor&&) = delete;


  void Add(std::unique_ptr<core::iWidget>&& w) override {
    assert(w);
    widgets_.push_back(std::move(w));
  }

  void Remove(const core::iWidget* w) override {
    auto& ws = widgets_;
    ws.erase(std::remove_if(ws.begin(),
                            ws.end(),
                            [w](auto& x) { return x.get() == w; }),
             ws.end());
  }


  void Update() override {
    for (auto& w : widgets_) {
      w->Update();
    }
  }

  void SerializeParam(core::iSerializer*) const override;

 private:
  std::vector<std::unique_ptr<core::iWidget>> widgets_;
};

}  // namespace mnian
