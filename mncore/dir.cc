// No copyright
#include "mncore/dir.h"


namespace mnian::core {

iDirItemObserver::iDirItemObserver(iDirItem* target) : target_(target) {
  assert(target_);
  target_->observers_.push_back(this);
}

iDirItemObserver::~iDirItemObserver() {
  if (!target_) return;

  auto& obs = target_->observers_;
  obs.erase(std::remove(obs.begin(), obs.end(), this), obs.end());
}


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


Dir::ItemList Dir::DeserializeParam(iDeserializer* des) {
  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("item list is not a map");
    des->LogLocation();
    return {};
  }

  ItemList items;
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

    item->Rename(*name);
    items.push_back(std::move(item));
  }
  return items;
}

void Dir::SerializeParam(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  for (auto& item : items_) {
    map.Add(item->name(), item.get());
  }
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
  node_->Serialize(serializer);
}

}  // namespace mnian::core
