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

class DirTreeWidget::DirCommand final : public core::DirCommand {
 public:
  static constexpr const char* kType = "mnian::DirTreeWidget::DirCommand";


  static std::unique_ptr<DirCommand> DeserializeParam(
      core::iDeserializer* des) {
    des->Enter(std::string("super"));
    auto p = core::DirCommand::DeserializeParam(des);
    des->Leave();
    if (!p) return nullptr;

    des->Enter(std::string("widget"));
    auto w = des->app().project().wstore().
        DeserializeWidgetRef<DirTreeWidget>(des);
    des->Leave();
    if (!w) return nullptr;

    return std::unique_ptr<DirCommand>(new DirCommand(w, std::move(*p)));
  }


  DirCommand(DirTreeWidget*                    w,
             core::Dir*                        dir,
             const std::string&                name,
             std::unique_ptr<core::iDirItem>&& item) :
      core::DirCommand(kType, dir, name, std::move(item)), w_(w) {
    assert(w_);
  }
  DirCommand(DirTreeWidget* w, core::Dir* dir, const std::string& name) :
      core::DirCommand(kType, dir, name), w_(w) {
    assert(w_);
  }

  DirCommand(const DirCommand&) = delete;
  DirCommand(DirCommand&&) = delete;

  DirCommand& operator=(const DirCommand&) = delete;
  DirCommand& operator=(DirCommand&&) = delete;


  void Apply() override {
    if (verb() == kRemove) {
      w_->Deselect(dir().Find(name()));
    }

    core::DirCommand::Apply();

    if (verb() == kAdd) {
      w_->Feature(dir().Find(name()));
    }
  }
  void Revert() override {
    if (verb() == kAdd) {
      w_->Deselect(dir().Find(name()));
    }

    core::DirCommand::Revert();

    if (verb() == kRemove) {
      w_->Feature(dir().Find(name()));
    }
  }


  std::string description() const override {
    return _("Removes an item from the directory.");
  }

 protected:
  void SerializeParam(core::iSerializer* serial) const override {
    serial->SerializeMap(size_t{2});

    serial->SerializeKey("super");
    core::DirCommand::SerializeParam(serial);

    serial->SerializeKey("widget");
    serial->SerializeValue(static_cast<int64_t>(w_->id()));
  }

 private:
  DirCommand(DirTreeWidget* w, Param&& p) :
      core::DirCommand(kType, std::move(p)), w_(w) {
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
