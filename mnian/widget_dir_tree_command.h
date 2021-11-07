// No copyright
#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <utility>

#include "mncore/command.h"

#include "mnian/app.h"
#include "mnian/widget_dir_tree.h"


namespace mnian {

class DirTreeWidget::DirAddCommand final : public core::DirAddCommand {
 public:
  static constexpr const char* kType = "mnian::DirTreeWidget::DirAddCommand";


  static std::unique_ptr<DirAddCommand> DeserializeParam(
      core::iDeserializer* des) {
    des->Enter(std::string("super"));
    auto p = core::DirAddCommand::DeserializeParam(des);
    des->Leave();
    if (!p) return nullptr;

    des->Enter(std::string("widget"));
    auto w = des->app().project().wstore().
        DeserializeWidgetRef<DirTreeWidget>(des);
    des->Leave();
    if (!w) return nullptr;

    return std::unique_ptr<DirAddCommand>(new DirAddCommand(w, std::move(*p)));
  }


  DirAddCommand(DirTreeWidget*                    w,
                core::Dir*                        dir,
                const std::string&                name,
                std::unique_ptr<core::iDirItem>&& item) :
      core::DirAddCommand(kType, dir, name, std::move(item)), w_(w) {
    assert(w_);
  }

  DirAddCommand(const DirAddCommand&) = delete;
  DirAddCommand(DirAddCommand&&) = delete;

  DirAddCommand& operator=(const DirAddCommand&) = delete;
  DirAddCommand& operator=(DirAddCommand&&) = delete;


  void Apply() override {
    core::DirAddCommand::Apply();
    w_->Feature(dir().Find(name()));
  }
  void Revert() override {
    w_->Deselect(dir().Find(name()));
    core::DirAddCommand::Revert();
  }


  std::string description() const override {
    return _("Adds an item to the directory.");
  }

 protected:
  void SerializeParam(core::iSerializer* serial) const override {
    serial->SerializeMap(size_t{2});

    serial->SerializeKey("super");
    core::DirAddCommand::SerializeParam(serial);

    serial->SerializeKey("widget");
    serial->SerializeValue(static_cast<int64_t>(w_->id()));
  }

 private:
  DirAddCommand(DirTreeWidget* w, Param&& p) :
      core::DirAddCommand(kType, std::move(p)), w_(w) {
    assert(w_);
  }


  DirTreeWidget* w_;
};

class DirTreeWidget::DirRemoveCommand final : public core::DirRemoveCommand {
 public:
  static constexpr const char* kType = "mnian::DirTreeWidget::DirRemoveCommand";


  static std::unique_ptr<DirRemoveCommand> DeserializeParam(
      core::iDeserializer* des) {
    des->Enter(std::string("super"));
    auto p = core::DirRemoveCommand::DeserializeParam(des);
    des->Leave();
    if (!p) return nullptr;

    des->Enter(std::string("widget"));
    auto w = des->app().project().wstore().
        DeserializeWidgetRef<DirTreeWidget>(des);
    des->Leave();
    if (!w) return nullptr;

    return std::unique_ptr<DirRemoveCommand>(
        new DirRemoveCommand(w, std::move(*p)));
  }


  DirRemoveCommand(DirTreeWidget* w, core::Dir* dir, const std::string& name) :
      core::DirRemoveCommand(kType, dir, name), w_(w) {
    assert(w_);
  }

  DirRemoveCommand(const DirRemoveCommand&) = delete;
  DirRemoveCommand(DirRemoveCommand&&) = delete;

  DirRemoveCommand& operator=(const DirRemoveCommand&) = delete;
  DirRemoveCommand& operator=(DirRemoveCommand&&) = delete;


  void Apply() override {
    w_->Deselect(dir().Find(name()));
    core::DirRemoveCommand::Apply();
  }
  void Revert() override {
    core::DirRemoveCommand::Revert();
    w_->Feature(dir().Find(name()));
  }


  std::string description() const override {
    return _("Removes an item from the directory.");
  }

 protected:
  void SerializeParam(core::iSerializer* serial) const override {
    serial->SerializeMap(size_t{2});

    serial->SerializeKey("super");
    core::DirRemoveCommand::SerializeParam(serial);

    serial->SerializeKey("widget");
    serial->SerializeValue(static_cast<int64_t>(w_->id()));
  }

 private:
  DirRemoveCommand(DirTreeWidget* w, Param&& p) :
      core::DirRemoveCommand(kType, std::move(p)), w_(w) {
    assert(w_);
  }


  DirTreeWidget* w_;
};

class DirTreeWidget::DirMoveCommand final : public core::DirMoveCommand {
 public:
  static constexpr const char* kType = "mnian::DirTreeWidget::DirMoveCommand";


  static std::unique_ptr<DirMoveCommand> DeserializeParam(
      core::iDeserializer* des) {
    auto p = core::DirMoveCommand::DeserializeParam(des);
    if (!p) return nullptr;
    return std::unique_ptr<DirMoveCommand>(new DirMoveCommand(std::move(*p)));
  }


  DirMoveCommand(core::Dir*         src,
                 const std::string& src_name,
                 core::Dir*         dst,
                 const std::string& dst_name) :
      core::DirMoveCommand(kType, src, src_name, dst, dst_name) {
  }

  DirMoveCommand(const DirMoveCommand&) = delete;
  DirMoveCommand(DirMoveCommand&&) = delete;

  DirMoveCommand& operator=(const DirMoveCommand&) = delete;
  DirMoveCommand& operator=(DirMoveCommand&&) = delete;


  std::string description() const override {
    return _("Moves an item of the directory.");
  }

 private:
  explicit DirMoveCommand(Param&& p) :
      core::DirMoveCommand(kType, std::move(p)) {
  }
};

}  // namespace mnian
