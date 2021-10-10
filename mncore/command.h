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
  iCommand(const char* type, const std::string& description) :
      iPolymorphicSerializable(type), description_(description) {
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


  const std::string& description() const {
    return description_;
  }

 private:
  std::string description_;
};


class NullCommand : public iCommand {
 public:
  static constexpr const char* kType = "Null";


  static std::unique_ptr<NullCommand> DeserializeParam(iDeserializer* des);


  explicit NullCommand(const std::string& description) :
      iCommand(kType, description) {
  }
  NullCommand() : NullCommand("(no description)") {
  }

  NullCommand(const NullCommand&) = delete;
  NullCommand(NullCommand&&) = delete;

  NullCommand& operator=(const NullCommand&) = delete;
  NullCommand& operator=(NullCommand&&) = delete;


  void Apply() override {
  }
  void Revert() override {
  }

 protected:
  void SerializeParam(iSerializer* serial) const override;
};


// SquashedCommand is a sequence of one or more commands.
class SquashedCommand : public iCommand {
 public:
  static constexpr const char* kType = "Squashed";


  static std::unique_ptr<SquashedCommand> DeserializeParam(iDeserializer* des);


  SquashedCommand(std::vector<std::unique_ptr<iCommand>>&& commands,
                  const std::string& description) :
      iCommand(kType, description), commands_(std::move(commands)) {
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
