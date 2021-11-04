// No copyright
#pragma once

#include <imgui.h>
#include <linalg_aliases.h>

#include <cassert>
#include <memory>
#include <unordered_set>

#include "mncore/history.h"

#include "mnian/widget.h"


namespace mnian {

class HistoryTreeWidget : public ImGuiWidget {
 public:
  static constexpr const char* kType = "mnian::HistoryTreeWidget";


  static std::unique_ptr<HistoryTreeWidget> DeserializeParam(
      core::iDeserializer*);

  static void Register(core::DeserializerRegistry*);


  HistoryTreeWidget() = delete;
  explicit HistoryTreeWidget(core::iApp* app) : ImGuiWidget(kType), app_(app) {
    assert(app_);
  }

  HistoryTreeWidget(const HistoryTreeWidget&) = delete;
  HistoryTreeWidget(HistoryTreeWidget&&) = delete;

  HistoryTreeWidget& operator=(const HistoryTreeWidget&) = delete;
  HistoryTreeWidget& operator=(HistoryTreeWidget&&) = delete;


  void Update() override;

  void SerializeParam(core::iSerializer* serial) const override;

 private:
  bool UpdateItem(core::History::Item*, bool skip_text = false);


  void Hover(core::History::Item*);

  void MoveTo(core::History::Item*);


  core::iApp* app_;

  std::unordered_set<core::History::Item*> open_;
};

}  // namespace mnian
