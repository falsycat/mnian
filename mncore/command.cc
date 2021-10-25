// No copyright
#include "mncore/command.h"


namespace mnian::core {

std::unique_ptr<NullCommand> NullCommand::DeserializeParam(iDeserializer* des) {
  const auto desc = des->value<std::string>();
  return std::make_unique<NullCommand>(desc? *desc: std::string(""));
}

void NullCommand::SerializeParam(iSerializer* serial) const {
  serial->SerializeValue(desc_);
}


std::unique_ptr<SquashedCommand> SquashedCommand::DeserializeParam(
    iDeserializer* des) {
  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("array expected");
    des->LogLocation();
    return nullptr;
  }

  std::vector<std::unique_ptr<iCommand>> commands;
  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard _(des, i);
    auto cmd = des->DeserializeObject<iCommand>();
    if (!cmd) {
      des->logger().MNCORE_LOGGER_WARN("sub command is broken");
      des->LogLocation();
      return nullptr;
    }
    commands.push_back(std::move(cmd));
  }
  return std::make_unique<SquashedCommand>(std::move(commands));
}

void SquashedCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::ArrayGuard array(serial);
  for (auto& cmd : commands_) array.Add(cmd.get());
}

}  // namespace mnian::core
