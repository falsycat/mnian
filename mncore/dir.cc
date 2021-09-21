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


std::string iDirItem::ValidateName(const std::string& name) {
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
  return "";
}


void Dir::Serialize(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  for (auto& item : items_) {
    map.Add(item->name(), item.get());
  }
}


void FileRef::Serialize(iSerializer* serializer) const {
  iSerializer::MapGuard map(serializer);
  map.Add("url", file_->url());

  std::string mode;
  if (flags_ & kReadable) mode += 'r';
  if (flags_ & kWritable) mode += 'w';
  map.Add("mode", mode);
}

}  // namespace mnian::core
