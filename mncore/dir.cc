// No copyright
#include "mncore/dir.h"


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


Dir::ItemMap Dir::DeserializeParam(iDeserializer* des) {
  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("item list is not a map");
    des->LogLocation();
    return {};
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
  return items;
}

void Dir::SerializeParam(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  for (auto& item : items_) {
    map.Add(item.first, item.second.get());
  }
}


std::optional<FileRef::Flags> FileRef::ParseFlags(const std::string& v) {
  Flags flags = kNone;
  for (auto c : v) {
    switch (c) {
    case 'r': flags = flags | kReadable; break;
    case 'w': flags = flags | kWritable; break;
    default: return std::nullopt;
    }
  }
  return flags;
}

void FileRef::SerializeParam(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  map.Add("url", file_->url());

  std::string mode;
  if (flags_ & kReadable) mode += 'r';
  if (flags_ & kWritable) mode += 'w';
  map.Add("mode", mode);
}


void NodeRef::SerializeParam(iSerializer* serializer) const {
  serializer->SerializeValue(static_cast<int64_t>(node_->id()));
}

}  // namespace mnian::core
