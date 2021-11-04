// No copyright
#pragma once

#include <memory>
#include <string>

#include "mncore/command.h"

#include "mnian/app.h"


namespace mnian {

class NullCommand : public core::NullCommand {
 public:
  static constexpr const char* kType = "mnian::NullCommand";


  static std::unique_ptr<NullCommand> DeserializeParam(
      core::iDeserializer* des) {
    const auto desc = des->value<std::string>();
    if (!desc) return nullptr;
    return std::make_unique<NullCommand>(*desc);
  }


  explicit NullCommand(const std::string& desc) :
      core::NullCommand(kType, desc) {
  }

  NullCommand(const NullCommand&) = delete;
  NullCommand(NullCommand&&) = delete;

  NullCommand& operator=(const NullCommand&) = delete;
  NullCommand& operator=(NullCommand&&) = delete;


  std::string description() const override {
    return _(core::NullCommand::description().c_str());
  }
};

}  // namespace mnian
