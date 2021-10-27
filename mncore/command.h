// No copyright
//
// Command object can modify any in project context and revert it.
#pragma once

#include <memory>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "mncore/dir.h"
#include "mncore/serialize.h"


namespace mnian::core {

// An interface of Command.
class iCommand : public iPolymorphicSerializable {
 public:
  iCommand() = delete;
  explicit iCommand(const char* type) : iPolymorphicSerializable(type) {
  }

  iCommand(const iCommand&) = delete;
  iCommand(iCommand&&) = delete;

  iCommand& operator=(const iCommand&) = delete;
  iCommand& operator=(iCommand&&) = delete;


  // It's guaranteed that Apply() and Revert() must be called in the following
  // order:
  //   Apply() -> Revert() -> Apply() -> Revert() -> ...
  virtual void Apply() = 0;
  virtual void Revert() = 0;

  virtual std::string description() const {
    return "(no description)";
  }
};


class NullCommand : public iCommand {
 public:
  explicit NullCommand(const char*        type,
                       const std::string& desc = "(null command)") :
      iCommand(type), desc_(desc) {
  }

  NullCommand(const NullCommand&) = delete;
  NullCommand(NullCommand&&) = delete;

  NullCommand& operator=(const NullCommand&) = delete;
  NullCommand& operator=(NullCommand&&) = delete;


  void Apply() override {
  }
  void Revert() override {
  }


  std::string description() const override {
    return desc_;
  }

 protected:
  void SerializeParam(iSerializer* serial) const override;

 private:
  std::string desc_;
};


// SquashedCommand is a sequence of one or more commands.
class SquashedCommand : public iCommand {
 public:
  using CommandList = std::vector<std::unique_ptr<iCommand>>;


  static std::optional<CommandList> DeserializeParam(iDeserializer* des);


  SquashedCommand() = delete;
  SquashedCommand(const char* type, CommandList&& commands) :
      iCommand(type), commands_(std::move(commands)) {
  }

  SquashedCommand(const SquashedCommand&) = delete;
  SquashedCommand(SquashedCommand&&) = delete;

  SquashedCommand& operator=(const SquashedCommand&) = delete;
  SquashedCommand& operator=(SquashedCommand&&) = delete;


  void Apply() override {
    for (auto& cmd : commands_) {
      cmd->Apply();
    }
  }
  void Revert() override {
    auto itr = commands_.end();
    while (itr > commands_.begin()) {
      --itr;
      (*itr)->Revert();
    }
  }


  std::string description() const override {
    return "(squashed command)";
  }

  iCommand& commands(size_t i) const {
    assert(i < commands_.size());
    return *commands_[i];
  }
  size_t size() const {
    return commands_.size();
  }

 protected:
  void SerializeParam(iSerializer* serial) const override;

 private:
  std::string description_;

  CommandList commands_;
};


// DirCommand is a command to modify Dir.
class DirCommand : public iCommand {
 public:
  enum Verb {
    kAdd,
    kRemove,
  };

  using Param = std::tuple<
      Verb, Dir*, std::string, std::unique_ptr<iDirItem>>;


  static std::optional<Param> DeserializeParam(iDeserializer*);


  DirCommand() = delete;
  DirCommand(const char* type, Param&& p) :
      iCommand(type),
      verb_(std::get<0>(p)),
      dir_(std::get<1>(p)),
      name_(std::get<2>(p)),
      item_(std::move(std::get<3>(p))) {
  }
  DirCommand(const char*                 type,
             Dir*                        dir,
             const std::string&          name,
             std::unique_ptr<iDirItem>&& item) :
      DirCommand(type, {kAdd, dir, name, std::move(item)}) {
    assert(dir_);
    assert(!iDirItem::ValidateName(name_));
    assert(item_);
  }
  DirCommand(const char* type, Dir* dir, const std::string& name) :
      DirCommand(type, {kRemove, dir, name, nullptr}) {
    assert(dir_);
    assert(!iDirItem::ValidateName(name_));
  }

  DirCommand(const DirCommand&) = delete;
  DirCommand(DirCommand&&) = delete;

  DirCommand& operator=(const DirCommand&) = delete;
  DirCommand& operator=(DirCommand&&) = delete;


  void Apply() override {
    switch (verb_) {
    case kAdd:    Add();    break;
    case kRemove: Remove(); break;
    }
  }
  void Revert() override {
    switch (verb_) {
    case kAdd:    Remove(); break;
    case kRemove: Add();    break;
    }
  }

 protected:
  void SerializeParam(iSerializer*) const override;

 private:
  void Add() {
    dir_->Add(name_, std::move(item_));
  }
  void Remove() {
    item_ = dir_->Remove(name_);
  }


  const Verb verb_;

  Dir* dir_;

  std::string name_;

  std::unique_ptr<iDirItem> item_;
};


// DirMoveCommand is a command to move items between Dirs.
class DirMoveCommand : public iCommand {
 public:
  using Param = std::tuple<Dir*, std::string, Dir*, std::string>;


  static std::optional<Param> DeserializeParam(iDeserializer*);


  DirMoveCommand() = delete;
  DirMoveCommand(const char* type, Param&& p) :
      iCommand(type),
      src_(std::get<0>(p)), src_name_(std::get<1>(p)),
      dst_(std::get<2>(p)), dst_name_(std::get<3>(p)) {
  }
  DirMoveCommand(const char*                 type,
                 Dir*                        src,
                 const std::string&          src_name,
                 Dir*                        dst,
                 const std::string&          dst_name) :
      DirMoveCommand(type, {src, src_name, dst, dst_name}) {
    assert(src);
    assert(!iDirItem::ValidateName(src_name));
    assert(dst);
    assert(!iDirItem::ValidateName(dst_name));
  }

  DirMoveCommand(const DirMoveCommand&) = delete;
  DirMoveCommand(DirMoveCommand&&) = delete;

  DirMoveCommand& operator=(const DirMoveCommand&) = delete;
  DirMoveCommand& operator=(DirMoveCommand&&) = delete;


  void Apply() override {
    src_->Move(src_name_, dst_, dst_name_);
  }
  void Revert() override {
    dst_->Move(dst_name_, src_, src_name_);
  }

 protected:
  void SerializeParam(iSerializer*) const override;

 private:
  Dir* src_;

  std::string src_name_;

  Dir* dst_;

  std::string dst_name_;
};

}  // namespace mnian::core
