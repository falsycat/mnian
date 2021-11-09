// No copyright
#pragma once

#include <memory>
#include <string>

#include "mncore/command.h"

#include "mnian/app.h"


namespace mnian {

class OriginCommand : public core::iCommand {
 public:
  static constexpr const char* kType = "mnian::OriginCommand";


  static std::unique_ptr<OriginCommand> DeserializeParam(core::iDeserializer*) {
    return std::make_unique<OriginCommand>();
  }


  OriginCommand() : core::iCommand(kType) {
  }

  OriginCommand(const OriginCommand&) = delete;
  OriginCommand(OriginCommand&&) = delete;

  OriginCommand& operator=(const OriginCommand&) = delete;
  OriginCommand& operator=(OriginCommand&&) = delete;


  bool Apply() override {
    return true;
  }
  bool Revert() override {
    return true;
  }


  std::string GetDescription() const override {
    return _("origin of the history");
  }


  void SerializeParam(core::iSerializer* serial) const override {
    serial->SerializeMap(0);
  }
};

}  // namespace mnian
