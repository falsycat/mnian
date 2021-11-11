// No copyright
#include "mncore/history.h"

#include <algorithm>
#include <string>


namespace mnian::core {

std::optional<std::vector<std::unique_ptr<History::Item>>>
History::Item::DeserializeBranch(
    iDeserializer*                          des,
    History*                                owner,
    std::vector<std::unique_ptr<iCommand>>* commands) {
  assert(des);
  assert(owner);
  assert(commands);

  auto n = des->size();
  if (!n) return std::nullopt;

  std::vector<std::unique_ptr<History::Item>> branch;
  for (size_t i = 0; i < *n; ++i) {
    iDeserializer::ScopeGuard dummy_(des, i);

    auto item = Item::Deserialize(des, owner, commands);
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
    iDeserializer*                          des,
    History*                                owner,
    std::vector<std::unique_ptr<iCommand>>* commands) {
  assert(owner);
  assert(des);
  assert(commands);

  des->Enter(std::string("createdAt"));
  const auto created_at = des->value<time_t>(time_t{0});
  des->Leave();

  des->Enter(std::string("branch"));
  auto branch = DeserializeBranch(des, owner, commands);
  des->Leave();

  if (!branch) {
    des->logger().MNCORE_LOGGER_WARN("broken branch");
    des->LogLocation();
    return nullptr;
  }

  des->Enter(std::string("command"));
  auto command = des->value<size_t>();
  des->Leave();

  if (!command || *command >= commands->size() || !(*commands)[*command]) {
    des->logger().MNCORE_LOGGER_WARN("invalid command ref");
    des->LogLocation();
    return nullptr;
  }
  return std::unique_ptr<History::Item>(
      new Item(owner,
               created_at,
               std::move((*commands)[*command]),
               std::move(*branch)));
}

void History::Item::SerializePastCommands(
    std::vector<iCommand*>*                cmds,
    std::unordered_map<iCommand*, size_t>* idx,
    size_t                                 prev) const {
  auto itr = this;
  for (;;) {
    if (!itr->isOrigin()) {
      auto cmd = &itr->command();
      cmds->push_back(cmd);

      const size_t index = idx->size();
      (*idx)[cmd] = index;
    }

    const auto& b = itr->branch();
    for (size_t i = 0; i < b.size(); ++i) {
      if (i == prev) continue;
      b[i]->SerializeFutureCommands(cmds, idx);
    }

    if (itr->isOrigin()) break;
    prev = itr->index_;
    itr  = &itr->parent();
  }
}

void History::Item::SerializeFutureCommands(
    std::vector<iCommand*>*                cmds,
    std::unordered_map<iCommand*, size_t>* idx) const {
  cmds->push_back(command_.get());

  const size_t index = idx->size();
  (*idx)[command_.get()] = index;

  for (const auto& child : branch_) {
    child->SerializeFutureCommands(cmds, idx);
  }
}

void History::Item::Serialize(
    iSerializer*                                 serial,
    const std::unordered_map<iCommand*, size_t>& idx) const {
  assert(serial);

  serial->SerializeMap(size_t{3});

  serial->SerializeKey("createdAt");
  serial->SerializeValue(static_cast<int64_t>(created_at_));

  serial->SerializeKey("branch");
  serial->SerializeArray(branch_.size());
  for (const auto& child : branch_) child->Serialize(serial, idx);

  assert(idx.contains(command_.get()));

  serial->SerializeKey("command");
  serial->SerializeValue(static_cast<int64_t>(idx.at(command_.get())));
}


bool History::Deserialize(iDeserializer* des) {
  assert(des);

  // commands
  std::vector<std::unique_ptr<iCommand>> commands;
  {
    iDeserializer::ScopeGuard dummy1_(des, std::string("commands"));

    const auto n = des->size();
    if (!n) {
      des->logger().MNCORE_LOGGER_WARN("invalid command list");
      des->LogLocation();
      return false;
    }

    for (size_t i = 0; i < *n; ++i) {
      iDeserializer::ScopeGuard dummy2_(des, i);
      auto cmd = des->DeserializeObject<iCommand>();
      if (!cmd) {
        des->logger().MNCORE_LOGGER_WARN("broken command found");
        des->LogLocation();
        return false;
      }
      commands.push_back(std::move(cmd));
    }
  }

  // branch
  des->Enter(std::string("origin"));
  auto branch = Item::DeserializeBranch(des, this, &commands);
  des->Leave();

  if (!branch) {
    des->logger().MNCORE_LOGGER_WARN("broken origin");
    des->LogLocation();
    return false;
  }

  const auto surplus = std::find_if(commands.begin(), commands.end(),
                                    [](auto& x) { return !!x; });
  if (surplus != commands.end()) {
    des->logger().MNCORE_LOGGER_WARN("found unused command");
    des->LogLocation();
    return false;
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

      auto& b = head? head->branch(): *branch;
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
  for (auto& item : *branch) origin_->Fork(std::move(item));
  head_ = head;
  return true;
}

void History::Serialize(iSerializer* serial) const {
  assert(serial);

  std::unordered_map<iCommand*, size_t> idx;
  std::vector<iCommand*> cmds;
  head_->SerializePastCommands(&cmds, &idx);

  serial->SerializeMap(size_t{3});

  serial->SerializeKey("commands");
  serial->SerializeArray(cmds.size());
  for (const auto& cmd : cmds) cmd->Serialize(serial);

  serial->SerializeKey("origin");
  serial->SerializeArray(origin_->branch().size());
  for (const auto& item : origin_->branch()) {
    item->Serialize(serial, idx);
  }

  serial->SerializeKey("head");
  auto head = head_->GeneratePath();
  serial->SerializeArray(head.size());
  while (!head.empty()) {
    serial->SerializeValue(static_cast<int64_t>(head.top()));
    head.pop();
  }
}

}  // namespace mnian::core
