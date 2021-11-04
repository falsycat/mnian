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

  ItemMap items;
  iWidget::Id next = 0;

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
    if (items.find(*id) != items.end()) {
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
    if (next <= *id) {
      next = *id + 1;
    }

    item->id_ = *id;
    item->ObserveNew();

    items[*id] = std::move(item);
  }

  items_ = std::move(items);
  next_  = next;
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
