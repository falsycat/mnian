// No copyright
#include "mncore/serialize.h"


namespace mnian::core {

void iPolymorphicSerializable::Serialize(iSerializer* serializer) const {
  assert(serializer);

  serializer->SerializeMap(2);
  serializer->SerializeKeyValue("type", std::string(type_));
  serializer->SerializeKey("param");
  SerializeParam(serializer);
}


void iSerializer::MapGuard::Serialize(iSerializer* serializer) const {
  assert(serializer_ == serializer);
  (void) serializer;

  serializer_->SerializeMap(items_.size());
  for (auto& item : items_) {
    serializer_->SerializeKey(item.first);
    if (std::holds_alternative<const iSerializable*>(item.second)) {
      std::get<const iSerializable*>(item.second)->Serialize(serializer_);
    } else {
      serializer_->SerializeValue(std::move(std::get<Value>(item.second)));
    }
  }

  auto this_ = const_cast<MapGuard*>(this);
  this_->serializer_ = nullptr;
  this_->items_.clear();
}

void iSerializer::ArrayGuard::Serialize(iSerializer* serializer) const {
  assert(serializer_ == serializer);
  (void) serializer;

  serializer_->SerializeArray(items_.size());
  for (auto& item : items_) {
    if (std::holds_alternative<const iSerializable*>(item)) {
      std::get<const iSerializable*>(item)->Serialize(serializer_);
    } else {
      serializer_->SerializeValue(std::get<Value>(item));
    }
  }

  auto this_ = const_cast<ArrayGuard*>(this);
  this_->serializer_ = nullptr;
  this_->items_.clear();
}

}  // namespace mnian::core
