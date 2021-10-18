// No copyright
//
//
#include "mncore/node.h"


namespace mnian::core {

bool NodeStore::Deserialize(iDeserializer* des) {
  assert(des);

  auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_ERROR(
        "expected an array, all node is dropped");
    des->LogLocation();
    return false;
  }

  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard _(des, i);

    des->Enter(std::string("id"));
    auto id = des->value<iNode::Id>();
    des->Leave();

    if (!id) {
      des->logger().MNCORE_LOGGER_WARN("no valid id specified");
      des->LogLocation();
      continue;
    }
    if (items_.find(*id) != items_.end()) {
      des->logger().MNCORE_LOGGER_WARN("id duplication");
      des->LogLocation();
      continue;
    }

    des->Enter(std::string("entity"));
    auto item = des->DeserializeObject<iNode>();
    des->Leave();

    if (!item) {
      continue;
    }

    item->id_ = *id;
    item->NotifyNew();

    items_[*id] = std::move(item);
  }
  return true;
}

void NodeStore::Serialize(iSerializer* serial) const {
  assert(serial);

  serial->SerializeArray(items_.size());
  for (auto& item : items_) {
    iSerializer::MapGuard map(serial);
    map.Add("id", static_cast<int64_t>(item.first));
    map.Add("entity", item.second.get());
  }
}

}  // namespace mnian::core
