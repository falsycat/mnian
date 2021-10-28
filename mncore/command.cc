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
  auto dir = Dir::DeserializeRef(des);
  des->Leave();

  if (!dir) {
    des->logger().MNCORE_LOGGER_WARN("missing Dir");
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


std::optional<DirMoveCommand::Param> DirMoveCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter("src");
  auto src = Dir::DeserializeRef(des);
  des->Leave();

  if (!src) {
    des->logger().MNCORE_LOGGER_WARN("missing src item");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("src_name");
  const auto src_name = des->value<std::string>();
  des->Leave();

  if (!src_name || iDirItem::ValidateName(*src_name)) {
    des->logger().MNCORE_LOGGER_WARN("invalid src_name");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("dst");
  auto dst = Dir::DeserializeRef(des);
  des->Leave();

  if (!dst) {
    des->logger().MNCORE_LOGGER_WARN("missing dst item");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("dst_name");
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

  iSerializer::ArrayGuard src(serial);
  const auto src_path = src_->GeneratePath();
  for (auto& term : src_path) src.Add(term);

  iSerializer::ArrayGuard dst(serial);
  const auto dst_path = dst_->GeneratePath();
  for (auto& term : dst_path) dst.Add(term);

  iSerializer::MapGuard root(serial);
  root.Add("src", &src);
  root.Add("src_name", src_name_);
  root.Add("dst", &dst);
  root.Add("dst_name", dst_name_);
}


std::optional<FileRefReplaceCommand::Param>
FileRefReplaceCommand::DeserializeParam(iDeserializer* des) {
  des->Enter("target");
  auto fref = FileRef::DeserializeRef(des);
  des->Leave();

  if (!fref) {
    des->logger().MNCORE_LOGGER_WARN("missing target item");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("url");
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
  iSerializer::ArrayGuard target(serial);
  const auto target_path = target_->GeneratePath();
  for (const auto& term : target_path) target.Add(term);

  iSerializer::MapGuard root(serial);
  root.Add("target", &target);
  root.Add("url", file_->url());
}


std::optional<FileRefFlagCommand::Param> FileRefFlagCommand::DeserializeParam(
    iDeserializer* des) {
  des->Enter("target");
  const auto target = FileRef::DeserializeRef(des);
  des->Leave();

  if (!target) {
    des->logger().MNCORE_LOGGER_WARN("missing target");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("flag");
  const auto flag = FileRef::DeserializeFlag(des);
  des->Leave();

  if (!flag) {
    des->logger().MNCORE_LOGGER_WARN("no flag specified");
    des->LogLocation();
    return std::nullopt;
  }

  des->Enter("set");
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
  iSerializer::ArrayGuard target(serial);
  const auto target_path = target_->GeneratePath();
  for (const auto& term : target_path) target.Add(term);

  iSerializer::MapGuard root(serial);
  root.Add("target", &target);
  root.Add("flag",   FileRef::StringifyFlags(flag_));
  root.Add("set",    set_);
}

}  // namespace mnian::core
