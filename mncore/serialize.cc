// No copyright
#include "mncore/serialize.h"


namespace mnian::core {

void iPolymorphicSerializable::Serialize(iSerializer* serializer) const {
  assert(serializer);

  iSerializer::MapGuard root(serializer);
  root.Add("type", std::string(type_));
  root.Add("param", [this, serializer]() { SerializeParam(serializer); });
}


void iSerializer::MapGuard::Serialize(iSerializer* serializer) const {
  assert(serializer_ == serializer);
  (void) serializer;

  serializer_->SerializeMap(items_.size());
  for (auto& item : items_) {
    serializer_->SerializeKey(item.first);
    item.second();
  }

  auto this_ = const_cast<MapGuard*>(this);
  this_->serializer_ = nullptr;
  this_->items_.clear();
}

void iSerializer::ArrayGuard::Serialize(iSerializer* serializer) const {
  assert(serializer_ == serializer);
  (void) serializer;

  serializer_->SerializeArray(items_.size());
  for (auto& item : items_) item();

  auto this_ = const_cast<ArrayGuard*>(this);
  this_->serializer_ = nullptr;
  this_->items_.clear();
}


std::string iDeserializer::GenerateLocation() const {
  std::string ans;
  for (auto& key : stack_) {
    if (std::holds_alternative<std::string>(key)) {
      ans += std::get<std::string>(key)+".";

    } else if (std::holds_alternative<size_t>(key)) {
      // Removes the last dot.
      ans  = ans.substr(0, ans.size()-1);
      ans += "["+std::to_string(std::get<size_t>(key))+"].";

    } else {
      assert(false);  // UNREACHABLE
    }
  }
  // Removes the last dot.
  return ans.substr(0, ans.size()-1);
}

}  // namespace mnian::core
