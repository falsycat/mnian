// No copyright
#include "mncore/dir.h"

#include "mncore/app.h"


namespace mnian::core {

std::optional<std::string> iDirItem::ValidateName(const std::string& name) {
  static const std::string kAllowed =
      "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz-_";
  if (name.empty()) {
    return "empty name is not allowed";
  }
  for (auto c : name) {
    if (kAllowed.find(c) == std::string::npos) {
      return "invalid char found ('A-Za-z0-9\\-_' are allowed)";
    }
  }
  return std::nullopt;
}

iDirItem* iDirItem::DeserializeRef(iDeserializer* des) {
  const auto path = des->values<std::string>();
  if (!path) return nullptr;

  auto item = des->app().project().root().FindPath(*path);
  if (!item) return nullptr;
  return item;
}

std::vector<std::string> iDirItem::GeneratePath() const {
  if (isRoot()) return {};

  std::vector<std::string> ret = {name_};

  Dir* itr = parent_;
  while (!itr->isRoot()) {
    ret.push_back(itr->name_);
    itr = itr->parent_;
  }

  std::reverse(ret.begin(), ret.end());
  return ret;
}

bool iDirItem::IsAncestorOf(const iDirItem& other) const {
  auto itr = &other;
  for (;;) {
    if (itr == this) return true;
    if (itr->isRoot()) return false;
    itr = &itr->parent();
  }
  return false;
}


std::unique_ptr<Dir> Dir::DeserializeParam(iDeserializer* des) {
  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("item list is not a map");
    des->LogLocation();
    return nullptr;
  }

  ItemMap items;
  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard _(des, i);

    const auto name = des->key();
    if (!name) {
      des->logger().MNCORE_LOGGER_WARN("no string key specified for dir item");
      des->logger().MNCORE_LOGGER_INFO("skipping the item...");
      des->LogLocation();
      continue;
    }

    auto err = iDirItem::ValidateName(*name);
    if (err) {
      des->logger().MNCORE_LOGGER_WARN(
          "no valid name specified for dir item: "+*err);
      des->logger().MNCORE_LOGGER_INFO("skipping the item...");
      des->LogLocation();
      continue;
    }

    auto item = des->DeserializeObject<iDirItem>();
    if (!item) continue;
    items[*name] = std::move(item);
  }
  return std::make_unique<Dir>(std::move(items));
}

void Dir::SerializeParam(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  for (auto& item : items_) {
    map.Add(item.first, item.second.get());
  }
}


std::string FileRef::StringifyFlags(Flags flags) {
  std::string ret;
  if (flags & kReadable) ret += 'r';
  if (flags & kWritable) ret += 'w';
  return ret;
}

std::optional<FileRef::Flags> FileRef::ParseFlags(const std::string& v) {
  Flags ret = kNone;
  for (auto c : v) {
    const auto f = ParseFlag(c);
    if (!f) return std::nullopt;
    ret = ret | *f;
  }
  return ret;
}

std::optional<FileRef::Flag> FileRef::ParseFlag(char c) {
  switch (c) {
  case 'r': return kReadable; break;
  case 'w': return kWritable; break;
  default:  return std::nullopt;
  }
}

std::unique_ptr<FileRef> FileRef::DeserializeParam(iDeserializer* des) {
  des->Enter("url");
  const auto url = des->value<std::string>();
  des->Leave();

  if (!url) {
    des->logger().MNCORE_LOGGER_WARN("invalid url");
    des->LogLocation();
    return nullptr;
  }

  des->Enter("mode");
  const auto flags = DeserializeFlags(des);
  des->Leave();

  if (!flags) {
    des->logger().MNCORE_LOGGER_WARN("invalid flags");
    des->LogLocation();
    return nullptr;
  }

  auto f = des->app().fstore().Load(*url);
  return std::make_unique<FileRef>(f, *flags);
}

void FileRef::SerializeParam(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  map.Add("url", file_->url());

  std::string mode;
  if (flags_ & kReadable) mode += 'r';
  if (flags_ & kWritable) mode += 'w';
  map.Add("mode", mode);
}


std::unique_ptr<NodeRef> NodeRef::DeserializeParam(iDeserializer* des) {
  const auto id = des->value<iNode::Id>();
  if (!id) {
    des->logger().MNCORE_LOGGER_WARN("expected integer ID");
    des->LogLocation();
    return nullptr;
  }
  if (!des->app().project().nstore().Find(*id)) {
    des->logger().MNCORE_LOGGER_WARN("missing node");
    des->LogLocation();
    return nullptr;
  }
  return std::make_unique<NodeRef>(&des->app().project().nstore(), *id);
}

void NodeRef::SerializeParam(iSerializer* serializer) const {
  serializer->SerializeValue(static_cast<int64_t>(node_->id()));
}

}  // namespace mnian::core
