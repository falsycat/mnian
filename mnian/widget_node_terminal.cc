// No copyright
#include "mnian/widget_node_terminal.h"

#include <imgui.h>
#include <imgui_stdlib.h>

#include <algorithm>
#include <cassert>
#include <chrono>  // NOLINT(build/c++11)
#include <string>
#include <unordered_set>
#include <utility>

#include <Tracy.hpp>

#include "mnian/app.h"
#include "mnian/widget_node_terminal_command.h"


namespace mnian {

void NodeTerminalWidget::Register(core::DeserializerRegistry* reg) {
  reg->RegisterType<core::iWidget, NodeTerminalWidget>();
  reg->RegisterType<core::iCommand, InputSetCommand>();
  reg->RegisterType<core::iCommand, InputClearCommand>();
}


NodeTerminalWidget::NodeTerminalWidget(core::iApp*                    app,
                                       std::unique_ptr<core::iNode>&& node,
                                       std::vector<core::SharedAny>&& input) :
    ImGuiWidget(kType),
    app_(app), node_(std::move(node)) {
  assert(app_);
  assert(node_);

  app_->wmap().Bind(this, node_.get());

  const size_t input_size = std::min(input.size(), node_->inputCount());
  for (size_t i = 0; i < input_size; ++i) {
    const auto& sock = node_->input(i);
    input_[&sock] = input[i];
  }
  unstable_input_ = input_;
}


void NodeTerminalWidget::Update() {
  ZoneScoped;
  SyncIO();

  const float em    = ImGui::GetFontSize();
  const auto  flags = ImGuiTreeNodeFlags_Leaf;

  ImGui::SetNextWindowSize(ImVec2(16*em, 20*em), ImGuiCond_FirstUseEver);
  if (ImGui::Begin(strId().c_str(), nullptr, ImGuiWindowFlags_MenuBar)) {
    UpdateMenu();

    if (ImGui::CollapsingHeader(_("node info"), flags)) {
      ImGui::Text("id   : %" MNCORE_PRIobjid, node_->id());
      ImGui::Text("type : %.*s",
                  static_cast<int>(node_->type().size()),
                  node_->type().data());
      ImGui::Text("state: %s", GetStateText());
    }
    if (ImGui::CollapsingHeader(_("input"), flags)) {
      for (size_t i = 0; i < node_->inputCount(); ++i) {
        const auto& sock  = node_->input(i);
        auto&       value = unstable_input_[&sock];
        if (UpdateValue(sock, &value, false)) {
          dirty_ = true;
          // TODO(falsycat): fuzzy processing
        }
        if (ImGui::IsItemDeactivatedAfterEdit()) {
          std::vector<InputSetCommand::Pair> pairs = {{&sock, value}};
          app_->ExecCommand(
              std::make_unique<InputSetCommand>(this, std::move(pairs)));
        }
      }
    }
    if (ImGui::CollapsingHeader(_("output"), flags)) {
      for (size_t i = 0; i < node_->outputCount(); ++i) {
        const auto& sock = node_->output(i);
        UpdateValue(sock, &output_[&sock], true);
      }
    }
    if (ImGui::CollapsingHeader(_("execution"), flags)) {
      if (proc_.busy()) {
        ImGui::ProgressBar(static_cast<float>(proc_.progress()));
        ImGui::Text(proc_.msg().c_str());
      }
    }
  }
  ImGui::End();

  // handle changes
  if (options_.autoexec && dirty_) ExecNode();
}

void NodeTerminalWidget::UpdateMenu() {
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu(_("actions"))) {
      if (ImGui::MenuItem(_("execute"))) {
        ExecNode();
      }
      if (ImGui::MenuItem(_("abort execution"), NULL, false, proc_.busy())) {
        proc_.RequestAbort();
      }
      ImGui::Separator();
      if (ImGui::MenuItem(_("clear input"))) {
        app_->ExecCommand(std::make_unique<InputClearCommand>(this));
      }
      ImGui::Separator();
      if (ImGui::MenuItem(_("clone widget"))) {
        CloneWidget();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(_("node actions"))) {
      MenuOfActionable(*node_);
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(_("options"))) {
      ImGui::MenuItem(
          _("enable auto execution on change"), NULL, &options_.autoexec);
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
}

bool NodeTerminalWidget::UpdateValue(
    const core::iNode::Socket& s, core::SharedAny* v, bool constant) {
  bool ret = false;
  switch (s.type()) {
  case core::iNode::Socket::kInteger:
    if (constant) ImGui::BeginDisabled();
    ret = ImGui::DragScalar(
        s.meta().name.c_str(),
        ImGuiDataType_S64,
        &std::get<int64_t>(*v),
        .2f,
        nullptr,
        nullptr);
    if (constant) ImGui::EndDisabled();
    break;
  case core::iNode::Socket::kScalar:
    if (constant) ImGui::BeginDisabled();
    ret = ImGui::DragScalar(
        s.meta().name.c_str(),
        ImGuiDataType_Double,
        &std::get<double>(*v),
        .2f,
        nullptr,
        nullptr);
    if (constant) ImGui::EndDisabled();
    break;
  case core::iNode::Socket::kString: {
    const auto flags =
        ImGuiInputTextFlags_NoUndoRedo |
        (constant? ImGuiInputTextFlags_ReadOnly: 0);
    ret = ImGui::InputTextMultiline(
        s.meta().name.c_str(),
        std::get<std::shared_ptr<std::string>>(*v).get(),
        ImVec2(0, ImGui::GetFontSize()*3),
        flags);
  } break;
  default:
    assert(false);
    return false;
  }
  return ret;
}


void NodeTerminalWidget::ExecNode() {
  if (busy_) {
    dirty_ = true;
    if (proc_.busy()) proc_.RequestAbort();
    return;
  }

  class Taker : public core::iLambda {
   public:
    explicit Taker(NodeTerminalWidget* w) :
        iLambda(w->output_.size(), 0), w_(w) {
      for (size_t i = 0; i < w_->node_->outputCount(); ++i) {
        const auto& sock = w_->node_->output(i);
        socks_.emplace_back(sock.index(), &sock);
      }
    }

   protected:
    void DoExec() override {
      for (const auto& p : socks_) {
        w_->output_[p.second] = in(p.first);
      }
      w_->busy_ = false;
    }

   private:
    NodeTerminalWidget* w_;

    std::vector<std::pair<size_t, const core::iNode::Socket*>> socks_;
  };

  proc_ = node_->EnqueueLambda();

  auto lambda = proc_.lambda();
  for (size_t i = 0; i < node_->inputCount(); ++i) {
    const auto& sock = node_->input(i);
    lambda->in(sock.index(), core::SharedAny(unstable_input_[&sock]));
  }

  auto taker = std::make_shared<Taker>(this);
  for (size_t i = 0; i < node_->outputCount(); ++i) {
    lambda->Connect(node_->output(i).index(), taker, i);
  }
  app_->mainQ().Attach(taker);

  busy_  = true;
  dirty_ = false;

  lambda->Trigger();
  taker->Trigger();
}

void NodeTerminalWidget::CloneWidget() {
  app_->project().wstore().Add(
      std::make_unique<NodeTerminalWidget>(app_, node_->Clone()));
}


void NodeTerminalWidget::SyncIO() {
  ZoneScoped;

  // sockets
  std::unordered_set<const core::iNode::Socket*> insocks;
  for (size_t i = 0; i < node_->inputCount(); ++i) {
    insocks.insert(&node_->input(i));
  }
  std::unordered_set<const core::iNode::Socket*> outsocks;
  for (size_t i = 0; i < node_->outputCount(); ++i) {
    outsocks.insert(&node_->output(i));
  }

  // sync input sockets
  for (const auto sock : insocks) {
    if (!input_.contains(sock)) {
      input_[sock] = sock->def();
    }
    if (!unstable_input_.contains(sock)) {
      unstable_input_[sock] = sock->def();
    }
  }

  // sync output sockets
  for (size_t i = 0; i < node_->outputCount(); ++i) {
    const auto& sock = node_->output(i);
    if (!output_.contains(&sock)) {
      output_[&sock] = sock.def();
    }
  }

  // forget removed sockets
  std::unordered_set<const core::iNode::Socket*> trash;
  for (auto p : input_) {
    if (!insocks.contains(p.first)) trash.insert(p.first);
  }
  for (auto p : unstable_input_) {
    if (!insocks.contains(p.first)) trash.insert(p.first);
  }
  for (auto p : output_) {
    if (!outsocks.contains(p.first)) trash.insert(p.first);
  }
  for (auto sock : trash) {
    input_.erase(sock);
    unstable_input_.erase(sock);
    output_.erase(sock);
  }
}


const char* NodeTerminalWidget::GetStateText() const {
  if (proc_.empty()) return _("idle");

  switch (proc_.state()) {
  case core::iNode::Process::kPending:
    return _("pending");
  case core::iNode::Process::kRunning:
    return _("running");
  case core::iNode::Process::kFinished:
    return _("finished");
  case core::iNode::Process::kAborted:
    return _("aborted");
  }
  assert(false);
  return "UNKNOWN";
}


std::unique_ptr<NodeTerminalWidget> NodeTerminalWidget::DeserializeParam(
    core::iDeserializer* des) {
  auto& app = des->app();

  des->Enter(std::string("node"));
  auto node = des->DeserializeObject<core::iNode>();
  des->Leave();

  if (!node) {
    des->logger().MNCORE_LOGGER_WARN(
        "target node is broken, NodeTerminal is dropped");
    des->LogLocation();
    return nullptr;
  }

  des->Enter(std::string("input"));
  auto input = des->values<core::SharedAny>();
  des->Leave();

  if (!input) input = std::vector<core::SharedAny>();

  return std::make_unique<NodeTerminalWidget>(
      &app, std::move(node), std::move(*input));
}

void NodeTerminalWidget::SerializeParam(core::iSerializer* serial) const {
  core::iSerializer::MapGuard root(serial);

  root.Add("node", node_.get());

  root.Add(
      "input",
      [this, serial]() {
        serial->SerializeArray(node_->inputCount());
        for (size_t i = 0; i < node_->inputCount(); ++i) {
          const auto& sock = node_->input(i);

          auto vitr = input_.find(&sock);
          if (vitr != input_.end()) {
            serial->SerializeValue(core::FromSharedAny(vitr->second));
          } else {
            serial->SerializeValue(core::FromSharedAny(sock.def()));
          }
        }
      });
}

}  // namespace mnian
