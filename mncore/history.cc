// No copyright
#include "mncore/history.h"

#include <algorithm>
#include <string>


namespace mnian::core {

std::optional<std::vector<std::unique_ptr<History::Item>>>
History::Item::DeserializeBranch(History* owner, iDeserializer* des) {
  auto n = des->size();
  if (!n) return std::nullopt;

  std::vector<std::unique_ptr<History::Item>> branch;
  for (size_t i = 0; i < *n; ++i) {
    iDeserializer::ScopeGuard dummy_(des, i);

    auto item = Item::Deserialize(owner, des);
    if (!item) {
      des->logger().MNCORE_LOGGER_WARN("broken branch");
      des->LogLocation();
      return std::nullopt;
    }
    branch.push_back(std::move(item));
  }
  return branch;
}

std::unique_ptr<History::Item> History::Item::Deserialize(
    History* owner, iDeserializer* des) {
  assert(owner);
  assert(des);

  des->Enter(std::string("createdAt"));
  const auto created_at = des->value<time_t>(time_t{0});
  des->Leave();

  des->Enter(std::string("branch"));
  auto branch = DeserializeBranch(owner, des);
  des->Leave();

  if (!branch) {
    des->logger().MNCORE_LOGGER_WARN("broken branch");
    des->LogLocation();
    return nullptr;
  }

  des->Enter(std::string("command"));
  auto command = des->DeserializeObject<iCommand>();
  des->Leave();

  if (!command) {
    des->logger().MNCORE_LOGGER_WARN("broken command");
    des->LogLocation();
    return nullptr;
  }
  return std::unique_ptr<History::Item>(
      new Item(owner, created_at, std::move(command), std::move(*branch)));
}

void History::Item::Serialize(iSerializer* serial) const {
  assert(serial);

  iSerializer::ArrayGuard branch(serial);
  for (auto& item : branch_) {
    branch.Add(item.get());
  }

  iSerializer::MapGuard root(serial);
  root.Add("createdAt", static_cast<int64_t>(created_at_));
  root.Add("command",   command_.get());
  root.Add("branch",    &branch);
}


bool History::Deserialize(iDeserializer* des) {
  assert(des);

  // origin's branch
  std::vector<std::unique_ptr<Item>> branch;
  {
    iDeserializer::ScopeGuard dummy1_(des, std::string("origin"));

    auto n = des->size();
    if (!n) n = size_t{0};

    for (size_t i = 0; i < *n; ++i) {
      iDeserializer::ScopeGuard dummy2_(des, i);

      auto item = Item::Deserialize(this, des);
      if (!item) {
        des->logger().MNCORE_LOGGER_WARN("origin's children are broken");
        des->LogLocation();
        return false;
      }
      branch.push_back(std::move(item));
    }
  }

  // head ptr
  Item* head = nullptr;
  {
    iDeserializer::ScopeGuard dummy1_(des, std::string("head"));

    const auto n = des->size();
    if (!n) {
      des->logger().MNCORE_LOGGER_WARN("invalid head path");
      des->LogLocation();
      return false;
    }

    for (size_t i = 0; i < *n; ++i) {
      iDeserializer::ScopeGuard dummy2_(des, i);
      const size_t index = des->value<size_t>(SIZE_MAX);

      auto& b = head? head->branch(): branch;
      if (index >= b.size()) {
        des->logger().MNCORE_LOGGER_WARN("missing head");
        des->LogLocation();
        return false;
      }
      head = b[index].get();
    }
    if (!head) head = origin_.get();
  }

  // deserialization is completed, and applies the data
  origin_->DropAllBranch();
  for (auto& item : branch) origin_->Fork(std::move(item));
  head_ = head;
  return true;
}

void History::Serialize(iSerializer* serial) const {
  assert(serial);

  std::stack<size_t> path = head_->GeneratePath();

  iSerializer::ArrayGuard origin(serial);
  for (const auto& item : origin_->branch()) {
    origin.Add(item.get());
  }

  iSerializer::ArrayGuard head(serial);
  for (; !path.empty(); path.pop()) {
    head.Add(static_cast<int64_t>(path.top()));
  }

  iSerializer::MapGuard root(serial);
  root.Add("origin", &origin);
  root.Add("head",   &head);
}

}  // namespace mnian::core
