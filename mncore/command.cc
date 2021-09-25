// No copyright
#include "mncore/command.h"


namespace mnian::core {

std::unique_ptr<NullCommand> NullCommand::DeserializeParam(
    iDeserializer* des) {
  auto description = des->value<std::string>();
  if (!description) {
    des->logger().MNCORE_LOGGER_WARN("no description specified");
    des->LogLocation();
    description = "";
  }
  return std::make_unique<NullCommand>(*description);
}

void NullCommand::SerializeParam(iSerializer* serial) const {
  serial->SerializeValue(description());
}


std::unique_ptr<SquashedCommand> SquashedCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter(std::string("description"));
  auto description = des->value<std::string>();
  des->Leave();

  if (!description) {
    des->logger().MNCORE_LOGGER_WARN("no description specified");
    des->LogLocation();
    description = "";
  }

  iDeserializer::ScopeGuard _(des, std::string("commands"));

  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("array expected");
    des->LogLocation();
    return nullptr;
  }

  std::vector<std::unique_ptr<iCommand>> commands;
  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard __(des, i);
    auto cmd = des->DeserializeObject<iCommand>();
    if (!cmd) {
      des->logger().MNCORE_LOGGER_WARN("sub command is broken");
      des->LogLocation();
      return nullptr;
    }
    commands.push_back(std::move(cmd));
  }
  return std::make_unique<SquashedCommand>(std::move(commands), *description);
}

void SquashedCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::ArrayGuard commands(serial);
  iSerializer::MapGuard   root(serial);

  root.Add("description", Any(description()));
  root.Add("commands", &commands);

  for (auto& cmd : commands_) {
    commands.Add(cmd.get());
  }
}

}  // namespace mnian::core
