// No copyright
#include "mncore/command.h"

#include "mncore/app.h"


namespace mnian::core {

void NullCommand::SerializeParam(iSerializer* serial) const {
  serial->SerializeValue(desc_);
}


std::optional<SquashedCommand::CommandList> SquashedCommand::DeserializeParam(
    iDeserializer* des) {
  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("array expected");
    des->LogLocation();
    return std::nullopt;
  }

  CommandList commands;
  commands.reserve(*size);

  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard _(des, i);
    auto cmd = des->DeserializeObject<iCommand>();
    if (!cmd) {
      des->logger().MNCORE_LOGGER_WARN("sub command is broken");
      des->LogLocation();
      return std::nullopt;
    }
    commands.push_back(std::move(cmd));
  }
  return commands;
}

void SquashedCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::ArrayGuard array(serial);
  for (auto& cmd : commands_) array.Add(cmd.get());
}


std::optional<DirCommand::Param> DirCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter("verb");
  const auto verb = des->value<std::string>();
  des->Leave();

  des->Enter("dir");
  auto dir_path = des->values<std::string>();
  des->Leave();

  if (!dir_path) {
    des->logger().MNCORE_LOGGER_WARN("expected a path");
    des->LogLocation();
    return std::nullopt;
  }

  auto dir_item = des->app().project().root().FindPath(*dir_path);
  if (!dir_item) {
    des->logger().MNCORE_LOGGER_WARN("missing path");
    des->LogLocation();
    return std::nullopt;
  }

  auto dir = dynamic_cast<Dir*>(dir_item);
  if (!dir) {
    des->logger().MNCORE_LOGGER_WARN("the item is not Dir");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("name");
  const auto name = des->value<std::string>();
  des->Leave();

  if (!name) {
    des->logger().MNCORE_LOGGER_WARN("expected a name");
    des->LogLocation();
    return std::nullopt;
  }
  if (iDirItem::ValidateName(*name)) {
    des->logger().MNCORE_LOGGER_WARN("invalid name");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("item");
  auto item = des->DeserializeObject<iDirItem>();
  des->Leave();

  if (verb) {
    const std::optional<Verb> v =
        *verb == "add"?    std::make_optional(kAdd):
        *verb == "remove"? std::make_optional(kRemove):
        std::nullopt;
    if (v) return std::make_tuple(*v, dir, *name, std::move(item));
  }

  des->logger().MNCORE_LOGGER_WARN("unknown verb");
  des->LogLocation();
  return std::nullopt;
}

void DirCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::ArrayGuard dir(serial);
  iSerializer::MapGuard   root(serial);

  switch (verb_) {
  case kAdd:    root.Add("verb", std::string("add"));    break;
  case kRemove: root.Add("verb", std::string("remove")); break;
  }
  root.Add("dir", &dir);
  root.Add("name", name_);
  if (item_) {
    root.Add("item", item_.get());
  }

  const auto dir_path = dir_->GeneratePath();
  for (auto& term : dir_path) dir.Add(term);
}

}  // namespace mnian::core
