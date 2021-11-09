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


std::optional<DirAddCommand::Param> DirAddCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter(std::string("dir"));
  auto dir = Dir::DeserializeRef(des);
  des->Leave();

  if (!dir) {
    des->logger().MNCORE_LOGGER_WARN("missing Dir");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("name"));
  auto name = des->value<std::string>();
  des->Leave();

  if (!name || iDirItem::ValidateName(*name)) {
    des->logger().MNCORE_LOGGER_WARN("invalid name");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("item"));
  auto item = des->DeserializeObject<iDirItem>();
  des->Leave();

  return std::make_tuple(dir, *name, std::move(item));
}

void DirAddCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::MapGuard root(serial);

  root.Add("dir",  static_cast<int64_t>(dir_->id()));
  root.Add("name", name_);
  if (item_) root.Add("item", item_.get());
}


std::optional<DirRemoveCommand::Param> DirRemoveCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter(std::string("dir"));
  auto dir = Dir::DeserializeRef(des);
  des->Leave();

  if (!dir) {
    des->logger().MNCORE_LOGGER_WARN("missing Dir");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("name"));
  auto name = des->value<std::string>();
  des->Leave();

  if (!name || iDirItem::ValidateName(*name)) {
    des->logger().MNCORE_LOGGER_WARN("invalid name");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("item"));
  auto item = des->DeserializeObject<iDirItem>();
  des->Leave();

  return std::make_tuple(dir, *name, std::move(item));
}

void DirRemoveCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::MapGuard root(serial);

  root.Add("dir",  static_cast<int64_t>(dir_->id()));
  root.Add("name", name_);
  if (item_) root.Add("item", item_.get());
}


std::optional<DirMoveCommand::Param> DirMoveCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter(std::string("src"));
  auto src = Dir::DeserializeRef(des);
  des->Leave();

  if (!src) {
    des->logger().MNCORE_LOGGER_WARN("missing src item");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("src_name"));
  const auto src_name = des->value<std::string>();
  des->Leave();

  if (!src_name || iDirItem::ValidateName(*src_name)) {
    des->logger().MNCORE_LOGGER_WARN("invalid src_name");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("dst"));
  auto dst = Dir::DeserializeRef(des);
  des->Leave();

  if (!dst) {
    des->logger().MNCORE_LOGGER_WARN("missing dst item");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("dst_name"));
  const auto dst_name = des->value<std::string>();
  des->Leave();

  if (!dst_name || iDirItem::ValidateName(*dst_name)) {
    des->logger().MNCORE_LOGGER_WARN("invalid dst_name");
    des->LogLocation();
    return std::nullopt;
  }
  return std::make_tuple(src, *src_name, dst, *dst_name);
}

void DirMoveCommand::SerializeParam(iSerializer* serial) const {
  assert(serial);

  iSerializer::MapGuard root(serial);
  root.Add("src", static_cast<int64_t>(src_->id()));
  root.Add("src_name", src_name_);
  root.Add("dst", static_cast<int64_t>(dst_->id()));
  root.Add("dst_name", dst_name_);
}


std::optional<FileRefReplaceCommand::Param>
FileRefReplaceCommand::DeserializeParam(iDeserializer* des) {
  des->Enter(std::string("target"));
  auto fref = FileRef::DeserializeRef(des);
  des->Leave();

  if (!fref) {
    des->logger().MNCORE_LOGGER_WARN("missing target item");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("url"));
  auto url = des->value<std::string>();
  des->Leave();

  if (!url) {
    des->logger().MNCORE_LOGGER_WARN("invalid url");
    des->LogLocation();
    return std::nullopt;
  }

  auto file = des->app().fstore().Load(*url);
  if (!file) {
    des->logger().MNCORE_LOGGER_WARN("failed to load file: "+*url);
    des->LogLocation();
    return std::nullopt;
  }
  return std::make_tuple(fref, file);
}

void FileRefReplaceCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::MapGuard root(serial);

  root.Add("target", static_cast<int64_t>(target_->id()));
  root.Add("url", file_->url());
}


std::optional<FileRefFlagCommand::Param> FileRefFlagCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter(std::string("target"));
  auto target = FileRef::DeserializeRef(des);
  des->Leave();

  if (!target) {
    des->logger().MNCORE_LOGGER_WARN("missing target");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("flag"));
  const auto flag = FileRef::DeserializeFlag(des);
  des->Leave();

  if (!flag) {
    des->logger().MNCORE_LOGGER_WARN("no flag specified");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter(std::string("set"));
  const auto set = des->value<bool>();
  des->Leave();

  if (!set) {
    des->logger().MNCORE_LOGGER_WARN("parameter 'set' is not specified");
    des->LogLocation();
    return std::nullopt;
  }
  return std::make_tuple(target, *flag, *set);
}

void FileRefFlagCommand::SerializeParam(iSerializer* serial) const {
  iSerializer::MapGuard root(serial);
  root.Add("target", static_cast<int64_t>(target_->id()));
  root.Add("flag",   FileRef::StringifyFlags(flag_));
  root.Add("set",    set_);
}

}  // namespace mnian::core
