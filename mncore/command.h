// No copyright
//
// Command object can modify any in project context and revert it.
#pragma once

#include <cassert>
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


  // It's guaranteed that Apply() and Revert() are called in the following
  // sequence:
  //   Apply() -> Revert() -> Apply() -> Revert() -> ...
  virtual bool Apply()  = 0;
  virtual bool Revert() = 0;


  virtual std::string GetDescription() const {
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


  bool Apply() override {
    return true;
  }
  bool Revert() override {
    return true;
  }


  std::string GetDescription() const override {
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


  bool Apply() override {
    for (intmax_t i = 0; static_cast<size_t>(i) < commands_.size(); ++i) {
      if (!commands_[static_cast<size_t>(i)]->Apply()) {
        for (; i >= 0; --i) {
          if (!commands_[static_cast<size_t>(i)]->Revert()) {
            assert(false);  // cannot recover
          }
        }
        return false;
      }
    }
    return true;
  }
  bool Revert() override {
    for (intmax_t i = static_cast<intmax_t>(commands_.size())-1; i >= 0; --i) {
      if (!commands_[static_cast<size_t>(i)]->Revert()) {
        for (; static_cast<size_t>(i) < commands_.size(); ++i) {
          if (!commands_[static_cast<size_t>(i)]->Apply()) {
            assert(false);  // cannot recover
          }
        }
        return false;
      }
    }
    return true;
  }


  std::string GetDescription() const override {
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
  CommandList commands_;
};


// DirAddCommand is a command to add new item to an existing Dir.
class DirAddCommand : public iCommand {
 public:
  using Param = std::tuple<Dir*, std::string, std::unique_ptr<iDirItem>>;


  static std::optional<Param> DeserializeParam(iDeserializer*);


  DirAddCommand() = delete;
  DirAddCommand(const char* type, Param&& p) :
      iCommand(type),
      dir_(std::get<0>(p)),
      name_(std::get<1>(p)),
      item_(std::move(std::get<2>(p))) {
  }
  DirAddCommand(const char*                 type,
                Dir*                        dir,
                std::string                 name,
                std::unique_ptr<iDirItem>&& item) :
      DirAddCommand(type, {dir, name, std::move(item)}) {
  }

  DirAddCommand(const DirAddCommand&) = delete;
  DirAddCommand(DirAddCommand&&) = delete;

  DirAddCommand& operator=(const DirAddCommand&) = delete;
  DirAddCommand& operator=(DirAddCommand&&) = delete;


  bool Apply() override {
    if (!item_ || dir_->Find(name_)) return false;
    dir_->Add(name_, std::move(item_));
    return true;
  }
  bool Revert() override {
    assert(!item_);
    if (item_) return false;
    item_ = dir_->Remove(name_);
    return !!item_;
  }


  Dir& dir() const {
    return *dir_;
  }
  const std::string& name() const {
    return name_;
  }
  const iDirItem* item() const {
    return item_.get();
  }

 protected:
  void SerializeParam(iSerializer* serial) const override;

 private:
  Dir* dir_;

  std::string name_;

  std::unique_ptr<iDirItem> item_;
};


// DirRemoveCommand is a command to remove the item from an existing Dir.
class DirRemoveCommand : public iCommand {
 public:
  using Param = std::tuple<Dir*, std::string, std::unique_ptr<iDirItem>>;


  static std::optional<Param> DeserializeParam(iDeserializer*);


  DirRemoveCommand() = delete;
  DirRemoveCommand(const char* type, Param&& p) :
      iCommand(type),
      dir_(std::get<0>(p)),
      name_(std::get<1>(p)),
      item_(std::move(std::get<2>(p))) {
  }
  DirRemoveCommand(const char* type, Dir* dir, std::string name) :
      DirRemoveCommand(type, {dir, name, nullptr}) {
  }

  DirRemoveCommand(const DirRemoveCommand&) = delete;
  DirRemoveCommand(DirRemoveCommand&&) = delete;

  DirRemoveCommand& operator=(const DirRemoveCommand&) = delete;
  DirRemoveCommand& operator=(DirRemoveCommand&&) = delete;


  bool Apply() override {
    if (item_) return false;
    item_ = dir_->Remove(name_);
    return !!item_;
  }
  bool Revert() override {
    if (!item_ || dir_->Find(name_)) return false;
    dir_->Add(name_, std::move(item_));
    return true;
  }


  Dir& dir() const {
    return *dir_;
  }
  const std::string& name() const {
    return name_;
  }
  const iDirItem* item() const {
    return item_.get();
  }

 protected:
  void SerializeParam(iSerializer* serial) const override;

 private:
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


  bool Apply() override {
    if (!src_->Find(src_name_) || dst_->Find(dst_name_)) return false;
    src_->Move(src_name_, dst_, dst_name_);
    return true;
  }
  bool Revert() override {
    if (!dst_->Find(dst_name_) || src_->Find(src_name_)) return false;
    dst_->Move(dst_name_, src_, src_name_);
    return true;
  }


  Dir& src() const {
    return *src_;
  }
  const std::string& srcName() const {
    return src_name_;
  }
  Dir& dst() const {
    return *dst_;
  }
  const std::string& dstName() const {
    return dst_name_;
  }

 protected:
  DirMoveCommand(const char* type, Param&& p) :
      iCommand(type),
      src_(std::get<0>(p)), src_name_(std::get<1>(p)),
      dst_(std::get<2>(p)), dst_name_(std::get<3>(p)) {
  }


  void SerializeParam(iSerializer*) const override;

 private:
  Dir* src_;

  std::string src_name_;

  Dir* dst_;

  std::string dst_name_;
};


// FileRefReplaceCommand is a command to replace an entity of FileRef.
class FileRefReplaceCommand : public iCommand {
 public:
  using Param = std::tuple<FileRef*, iFile*>;


  static std::optional<Param> DeserializeParam(iDeserializer*);


  FileRefReplaceCommand() = delete;
  FileRefReplaceCommand(const char* type, FileRef* target, iFile* file) :
      FileRefReplaceCommand(type, {target, file}) {
    assert(target);
    assert(file);
  }

  FileRefReplaceCommand(const FileRefReplaceCommand&) = delete;
  FileRefReplaceCommand(FileRefReplaceCommand&&) = delete;

  FileRefReplaceCommand& operator=(const FileRefReplaceCommand&) = delete;
  FileRefReplaceCommand& operator=(FileRefReplaceCommand&&) = delete;


  bool Apply() override {
    Swap();
    return true;
  }
  bool Revert() override {
    Swap();
    return true;
  }

 protected:
  FileRefReplaceCommand(const char* type, Param&& p) :
      iCommand(type), target_(std::get<0>(p)), file_(std::get<1>(p)) {
  }


  void SerializeParam(iSerializer*) const override;

 private:
  void Swap() {
    auto temp = &target_->entity();
    target_->ReplaceEntity(file_);
    file_ = temp;
  }


  FileRef* target_;

  iFile* file_;
};


// FileRefFlagCommand is a command to modify flag bits of FileRef.
class FileRefFlagCommand : public iCommand {
 public:
  using Param = std::tuple<FileRef*, FileRef::Flag, bool>;


  static std::optional<Param> DeserializeParam(iDeserializer*);


  FileRefFlagCommand() = delete;
  FileRefFlagCommand(const char*   type,
                     FileRef*      target,
                     FileRef::Flag flag,
                     bool          set) :
      FileRefFlagCommand(type, {target, flag, set}) {
    assert(target);
    assert(flag && !(flag & (flag-1)));
  }

  FileRefFlagCommand(const FileRefFlagCommand&) = delete;
  FileRefFlagCommand(FileRefFlagCommand&&) = delete;

  FileRefFlagCommand& operator=(const FileRefFlagCommand&) = delete;
  FileRefFlagCommand& operator=(FileRefFlagCommand&&) = delete;


  bool Apply() override {
    set_? target_->SetFlag(flag_): target_->UnsetFlag(flag_);
    return true;
  }
  bool Revert() override {
    set_? target_->UnsetFlag(flag_): target_->SetFlag(flag_);
    return true;
  }

 protected:
  FileRefFlagCommand(const char* type, Param&& p) :
      iCommand(type),
      target_(std::get<0>(p)), flag_(std::get<1>(p)), set_(std::get<2>(p)) {
  }


  void SerializeParam(iSerializer*) const override;

 private:
  FileRef* target_;

  FileRef::Flag flag_;

  bool set_;
};

}  // namespace mnian::core
