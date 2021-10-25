// No copyright
//
// Command object can modify any in project context and revert it.
#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

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

  virtual std::string description() const = 0;
};


class NullCommand : public iCommand {
 public:
  static constexpr const char* kType = "mnian::core::NullCommand";


  static std::unique_ptr<NullCommand> DeserializeParam(iDeserializer* des);


  explicit NullCommand(const std::string& desc = "(null command)") :
      iCommand(kType), desc_(desc) {
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
  static constexpr const char* kType = "mnian::core::SquashedCommand";


  static std::unique_ptr<SquashedCommand> DeserializeParam(iDeserializer* des);


  explicit SquashedCommand(std::vector<std::unique_ptr<iCommand>>&& commands) :
      iCommand(kType), commands_(std::move(commands)) {
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

  std::vector<std::unique_ptr<iCommand>> commands_;
};

}  // namespace mnian::core
