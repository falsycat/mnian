// No copyright
#pragma once

#include "mnian/widget_node_terminal.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "mncore/conv.h"


namespace mnian {

class NodeTerminalWidget::InputSetCommand : public core::iCommand {
 public:
  static constexpr const char* kType =
      "mnian::NodeTerminalWidget::InputSetCommand";


  using Pair = std::pair<const core::iNode::Socket*, core::SharedAny>;

  using Param = std::tuple<NodeTerminalWidget*, std::vector<Pair>, bool>;


  static std::optional<Param> DeserializeParam_(
      core::iDeserializer* des) {
    des->Enter(std::string("widget"));
    auto w = des->app().project().wstore().
        DeserializeWidgetRef<NodeTerminalWidget>(des);
    des->Leave();
    if (!w) return std::nullopt;

    std::vector<Pair> pairs;
    {
      core::iDeserializer::ScopeGuard dummy1_(des, std::string("pairs"));

      auto size = des->size();
      if (!size) size = size_t{0};

      for (size_t i = 0; i < *size; ++i) {
        core::iDeserializer::ScopeGuard dummy2_(des, i);

        des->Enter(std::string("index"));
        const auto index = des->value<size_t>();
        des->Leave();
        if (!index || *index >= w->node_->inputCount()) continue;

        const auto& sock = w->node_->input(*index);
        const auto  dup  = std::find_if(
            pairs.begin(), pairs.end(),
            [&sock](auto& x) { return x.first == &sock; });
        if (dup < pairs.end()) continue;

        des->Enter(std::string("value"));
        const auto value = des->value<core::SharedAny>();
        des->Leave();
        if (!value) continue;

        pairs.emplace_back(&sock, *value);
      }
    }

    des->Enter(std::string("applied"));
    const auto applied = des->value<bool>();
    des->Leave();
    if (!applied) return std::nullopt;

    return std::make_tuple(w, std::move(pairs), *applied);
  }
  static std::unique_ptr<InputSetCommand> DeserializeParam(
      core::iDeserializer* des) {
    auto p = DeserializeParam_(des);
    return p? std::make_unique<InputSetCommand>(std::move(*p)): nullptr;
  }


  InputSetCommand() = delete;
  InputSetCommand(const char* type, Param&& p) :
      iCommand(type),
      w_(std::get<0>(p)),
      pairs_(std::move(std::get<1>(p))),
      applied_(std::get<2>(p)) {
    assert(w_);
  }
  explicit InputSetCommand(Param&& p) : InputSetCommand(kType, std::move(p)) {
  }
  InputSetCommand(NodeTerminalWidget* w,
                  std::vector<Pair>&& pairs,
                  bool                applied = false) :
      InputSetCommand(kType, {w, std::move(pairs), applied}) {
  }

  InputSetCommand(const InputSetCommand&) = delete;
  InputSetCommand(InputSetCommand&&) = delete;

  InputSetCommand& operator=(const InputSetCommand&) = delete;
  InputSetCommand& operator=(InputSetCommand&&) = delete;


  bool Apply() override {
    if (applied_) return false;

    SwapValues();
    applied_ = true;
    return true;
  }
  bool Revert() override {
    if (!applied_) return false;

    SwapValues();
    applied_ = false;
    return true;
  }


  std::string GetDescription() const override {
    return _("Modify input parameters on NodeTerminal.");
  }

 protected:
  void SerializeParam(core::iSerializer* serial) const override {
    core::iSerializer::MapGuard root(serial);

    root.Add("widget", static_cast<int64_t>(w_->id()));

    root.Add(
        "pairs",
        [this, serial]() {
          auto indices = w_->node_->CreateSocketIndexMap();

          serial->SerializeArray(pairs_.size());
          for (auto& p : pairs_) {
            core::iSerializer::MapGuard obj(serial);
            obj.Add("index", static_cast<int64_t>(indices[p.first]));
            obj.Add("value", core::FromSharedAny(p.second));
          }
        });

    root.Add("applied", applied_);
  }

 private:
  void SwapValues() {
    for (auto& p : pairs_) {
      std::swap(w_->input_[p.first], p.second);
      w_->unstable_input_[p.first] = w_->input_[p.first];
    }
    w_->dirty_ = true;
  }


  NodeTerminalWidget* w_;

  std::vector<Pair> pairs_;

  bool applied_;
};

class NodeTerminalWidget::InputClearCommand : public InputSetCommand {
 public:
  static constexpr const char* kType =
      "mnian::NodeTerminalWidget::InputClearCommand";


  static std::unique_ptr<InputClearCommand> DeserializeParam(
      core::iDeserializer* des) {
    auto p = DeserializeParam_(des);
    if (!p) return nullptr;
    return p? std::make_unique<InputClearCommand>(std::move(*p)): nullptr;
  }


  InputClearCommand(const char* type, Param&& p) :
      InputSetCommand(type, std::move(p)) {
  }
  explicit InputClearCommand(Param&& p) :
      InputClearCommand(kType, std::move(p)) {
  }
  explicit InputClearCommand(NodeTerminalWidget* w) :
      InputClearCommand({w, GeneratePairs(w), false}) {
  }

  InputClearCommand(const InputClearCommand&) = delete;
  InputClearCommand(InputClearCommand&&) = delete;

  InputClearCommand& operator=(const InputClearCommand&) = delete;
  InputClearCommand& operator=(InputClearCommand&&) = delete;


  std::string GetDescription() const override {
    return _("Clear all inputs.");
  }

 private:
  static std::vector<Pair> GeneratePairs(NodeTerminalWidget* w) {
    auto& node = *w->node_;

    std::vector<Pair> pairs;
    for (size_t i = 0; i < node.inputCount(); ++i) {
      const auto& sock = node.input(i);
      pairs.emplace_back(&sock, sock.def());
    }
    return pairs;
  }
};

}  // namespace mnian
