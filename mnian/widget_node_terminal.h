// No copyright
#pragma once

#include "mnian/widget.h"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mncore/app.h"
#include "mncore/conv.h"
#include "mncore/node.h"
#include "mncore/serialize.h"
#include "mncore/task.h"


namespace mnian {

class NodeTerminalWidget : public ImGuiWidget {
 public:
  static constexpr const char* kType = "mnian::NodeTerminalWidget";


  using ValueMap = std::unordered_map<
      const core::iNode::Socket*, core::SharedAny>;


  static std::unique_ptr<NodeTerminalWidget> DeserializeParam(
      core::iDeserializer*);

  static void Register(core::DeserializerRegistry*);


  NodeTerminalWidget() = delete;
  NodeTerminalWidget(core::iApp*                    app,
                     std::unique_ptr<core::iNode>&& node,
                     std::vector<core::SharedAny>&& input = {});
  NodeTerminalWidget(core::iApp*                    app,
                     core::iNode*                   node,
                     std::vector<core::SharedAny>&& input = {}) :
      NodeTerminalWidget(app, node->Clone(), std::move(input)) {
  }
  ~NodeTerminalWidget() {
    app_->wmap().Forget(this);
  }

  NodeTerminalWidget(const NodeTerminalWidget&) = delete;
  NodeTerminalWidget(NodeTerminalWidget&&) = delete;

  NodeTerminalWidget& operator=(const NodeTerminalWidget&) = delete;
  NodeTerminalWidget& operator=(NodeTerminalWidget&&) = delete;


  void Update() override;

  void SerializeParam(core::iSerializer* serial) const override;

 private:
  struct Options {
   public:
    bool autoexec = true;
  };

  class InputSetCommand;
  class InputClearCommand;


  void UpdateMenu();
  bool UpdateValue(
      const core::iNode::Socket& s, core::SharedAny* v, bool constant);

  void ExecNode();
  void CloneWidget();

  void SyncIO();

  const char* GetStateText() const;


  core::iApp* app_;

  bool dirty_ = false;  // input changed but node not executed yet

  std::atomic<bool> busy_;

  core::iNode::ProcessRef proc_;

  ValueMap unstable_input_;

  // parameters that will be saved permanently
  std::unique_ptr<core::iNode> node_;

  ValueMap input_;
  ValueMap output_;

  Options options_;
};

}  // namespace mnian
