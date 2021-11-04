// No copyright
#include "mncore/history.h"

#include <algorithm>


namespace mnian::core {

void History::Exec(std::unique_ptr<iCommand>&& command) {
  head_->Fork(std::move(command));
  ReDo();
}

void History::ReDo(size_t index) {
  auto& branch = head_->branch_;

  const size_t n = branch.size();
  if (n == 0) return;
  if (index >= n) index = n-1;

  auto target = branch[index];
  target->command().Apply();
  head_ = target;

  branch.erase(branch.begin() + static_cast<intmax_t>(index));
  branch.push_back(target);

  for (auto observer : observers_) {
    observer->ObserveMove();
  }
}

void History::UnDo() {
  if (!head_->parent_) return;
  head_->command().Revert();
  head_ = head_->parent_;

  for (auto observer : observers_) {
    observer->ObserveMove();
  }
}

void History::Clear() {
  if (root_) {
    head_ = root_;
    root_->DropAllBranch();
    DropItem(root_);
  }

  next_ = 0;
  root_ = CreateItem(std::make_unique<NullCommand>("", "ORIGIN"));
  head_ = root_;

  for (auto observer : observers_) {
    observer->ObserveDrop();
  }
}

bool History::Deserialize(iDeserializer* des) {
  assert(des);

  des->Enter(std::string("root"));
  auto root = des->value<ItemId>();
  des->Leave();

  des->Enter(std::string("head"));
  auto head = des->value<ItemId>();
  des->Leave();

  if (!root || !head) {
    des->logger().MNCORE_LOGGER_WARN("ref pointers are broken");
    des->LogLocation();
    return false;
  }

  // Deserialize all items.
  iDeserializer::ScopeGuard _(des, std::string("items"));
  const auto size = des->size();

  if (!size || *size == 0) {
    des->logger().MNCORE_LOGGER_WARN("history is empty");
    des->LogLocation();
    return false;
  }

  ItemId next = 0;

  ItemMap items;
  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard __(des, *size-i-1);

    // Ignores broken items until they are required.
    auto item = DeserializeItem(des, items);
    if (!item) {
      continue;
    }

    if (item->id_ >= next) {
      next = item->id_+1;
    }
    items[item->id_] = std::move(item);
  }

  // Checks if the important nodes exist.
  auto root_itr = items.find(*root);
  if (root_itr == items.end()) {
    des->logger().MNCORE_LOGGER_WARN("root is missing");
    des->LogLocation();
    return false;
  }
  auto head_itr = items.find(*head);
  if (head_itr == items.end()) {
    des->logger().MNCORE_LOGGER_WARN("head is missing");
    des->LogLocation();
    return false;
  }

  // Applies loaded values because deserialization is succeeded.
  items_ = std::move(items);
  root_  = root_itr->second.get();
  head_  = head_itr->second.get();
  next_  = next;

  for (auto observer : observers_) {
    observer->ObserveDrop();
  }
  return true;
}

void History::Serialize(iSerializer* serial) const {
  assert(serial);

  iSerializer::ArrayGuard items(serial);
  iSerializer::MapGuard   map(serial);

  map.Add("items", &items);
  map.Add("root", root_->id_);
  map.Add("head", head_->id_);

  for (auto& item : items_) {
    items.Add(item.second.get());
  }
}

HistoryItem* History::CreateItem(std::unique_ptr<iCommand>&& command) {
  assert(command);

  assert(items_.find(next_) == items_.end());

  std::unique_ptr<HistoryItem> item(
      new HistoryItem(this, next_++, clock_->now(), std::move(command)));

  auto ret = item.get();
  items_[item->id_] = std::move(item);
  return ret;
}

std::unique_ptr<HistoryItem> History::DeserializeItem(
    iDeserializer* des, const ItemMap& items) {
  assert(des);

  des->Enter(std::string("id"));
  const auto id = des->value<ItemId>();
  des->Leave();

  des->Enter(std::string("createdAt"));
  const auto created_at = des->value<time_t>();
  des->Leave();

  des->Enter(std::string("mark"));
  const auto mark = des->value<bool>();
  des->Leave();

  des->Enter(std::string("command"));
  auto command = des->DeserializeObject<iCommand>();
  des->Leave();

  if (!id || !created_at || !command) {
    des->logger().MNCORE_LOGGER_WARN("broken history item is dropped");
    des->LogLocation();
    return nullptr;
  }

  std::unique_ptr<HistoryItem> ret(
      new HistoryItem(this, *id, *created_at, std::move(command), *mark));

  // Deserializes branches.
  iDeserializer::ScopeGuard _(des, std::string("branch"));
  const auto size = des->size();

  if (!size) {
    des->logger().MNCORE_LOGGER_INFO(
        "branch id list is broken, all branches are dropped");
    des->LogLocation();
    return ret;
  }

  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard __(des, i);

    const auto branch_id = des->value<ItemId>();

    auto itr = branch_id? items.find(*branch_id): items.end();
    if (itr == items.end()) {
      des->logger().MNCORE_LOGGER_INFO("missing branch is dropped");
      des->LogLocation();
      continue;
    }
    auto branch = itr->second.get();
    branch->parent_ = ret.get();
    ret->branch_.push_back(branch);
  }
  return ret;
}

void History::DropItem(HistoryItem* item) {
  for (auto branch : item->branch_) {
    DropItem(branch);
  }
  items_.erase(item->id_);
}


void HistoryItem::Serialize(iSerializer* serial) const {
  assert(serial);

  iSerializer::ArrayGuard branch(serial);
  for (auto item : branch_) {
    branch.Add(item->id_);
  }

  iSerializer::MapGuard map(serial);
  map.Add("id",        id_);
  map.Add("createdAt", static_cast<int64_t>(created_at_));
  map.Add("marked",    marked_);
  map.Add("command",   command_.get());
  map.Add("branch",    &branch);
}

}  // namespace mnian::core
