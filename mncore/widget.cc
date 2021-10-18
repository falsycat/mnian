// No copyright
#include "mncore/widget.h"

#include <string>


namespace mnian::core {

bool WidgetStore::Deserialize(iDeserializer* des) {
  assert(des);

  const auto n = des->size();
  if (!n) {
    des->logger().MNCORE_LOGGER_WARN(
        "expected array, widget store has been dropped");
    des->LogLocation();
    return false;
  }

  for (size_t i = 0; i < n; ++i) {
    iDeserializer::ScopeGuard _(des, *n-i-1);

    des->Enter(std::string("id"));
    const auto id = des->value<iWidget::Id>();
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
    auto item = des->DeserializeObject<iWidget>();
    des->Leave();

    if (!item) {
      continue;
    }

    item->id_ = *id;
    item->ObserveNew();

    items_[*id] = std::move(item);
  }
  return true;
}

void WidgetStore::Serialize(iSerializer* serial) const {
  serial->SerializeArray(items_.size());
  for (auto& item : items_) {
    iSerializer::MapGuard map(serial);
    map.Add("id", static_cast<int64_t>(item.second->id()));
    map.Add("entity", item.second.get());
  }
}

}  // namespace mnian::core
