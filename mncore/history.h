// No copyright
//
// This file declares classes related to History, which is a tree structure of
// Command that represents all controls user has done or reverted.
#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <map>
#include <utility>
#include <vector>

#include "mncore/clock.h"
#include "mncore/command.h"


namespace mnian::core {

class History;
class HistoryItem;


class iHistoryObserver {
 public:
  iHistoryObserver() = delete;
  inline explicit iHistoryObserver(History* target);
  inline virtual ~iHistoryObserver();

  iHistoryObserver(const iHistoryObserver&) = delete;
  iHistoryObserver(iHistoryObserver&&) = delete;

  iHistoryObserver& operator=(const iHistoryObserver&) = delete;
  iHistoryObserver& operator=(iHistoryObserver&&) = delete;


  virtual void ObserveDrop() = 0;
  virtual void ObserveFork() = 0;
  virtual void ObserveMove() = 0;

  History& target() const {
    return *target_;
  }

 private:
  History* target_;
};


class History : public iSerializable {
 public:
  friend class iHistoryObserver;
  friend class HistoryItem;


  // History can have 2^63 items.
  using ItemId = int64_t;

  using ItemMap = std::map<ItemId, std::unique_ptr<HistoryItem>>;


  History() = delete;
  explicit History(iClock* clock) : clock_(clock) {
    assert(clock_);

    Clear();
  }
  ~History() {
    Clear();
  }

  History(const History&) = delete;
  History(History&&) = delete;

  History& operator=(const History&) = delete;
  History& operator=(History&&) = delete;


  // Creates new item, then forks from head(), and finally ReDo()
  void Exec(std::unique_ptr<iCommand>&& command);

  // head() must have one or more branch.
  void ReDo(size_t index = SIZE_MAX);

  // head() mustn't be a root.
  void UnDo();

  // Drops all history and Makes NullCommand root.
  void Clear();


  // Returns true if the current history tree is properly replaced by new one,
  // otherwise false and changes nothing.
  bool Deserialize(iDeserializer* des);

  void Serialize(iSerializer*) const override;


  iClock& clock() const {
    return *clock_;
  }

  const ItemMap& items() const {
    return items_;
  }
  HistoryItem& root() const {
    return *root_;
  }
  HistoryItem& head() const {
    return *head_;
  }

 private:
  std::unique_ptr<HistoryItem> DeserializeItem(
      iDeserializer*, const ItemMap&);

  HistoryItem* CreateItem(std::unique_ptr<iCommand>&&);

  void DropItem(HistoryItem* item);


  iClock* clock_;

  ItemMap items_;

  HistoryItem* root_ = nullptr;
  HistoryItem* head_ = nullptr;

  ItemId next_ = 0;

  std::vector<iHistoryObserver*> observers_;
};


class HistoryItem : public iSerializable {
 public:
  friend class History;


  using Id = History::ItemId;

  using ItemList = std::vector<HistoryItem*>;


  // Constructor is private.
  HistoryItem() = delete;

  HistoryItem(const HistoryItem&) = delete;
  HistoryItem(HistoryItem&&) = delete;

  HistoryItem& operator=(const HistoryItem&) = delete;
  HistoryItem& operator=(HistoryItem&&) = delete;


  void Mark() {
    marked_ = true;
  }
  void Unmark() {
    marked_ = false;
  }


  // Creates new branch from the command.
  void Fork(std::unique_ptr<iCommand>&& command) {
    assert(command);

    auto item = owner_->CreateItem(std::move(command));
    assert(item);

    item->parent_ = this;
    branch_.push_back(item);

    for (auto observer : owner_->observers_) {
      observer->ObserveFork();
    }
  }


  // Makes this item root.
  // head() must be a descendant of `this`.
  void MakeRoot() {
    assert(IsAncestorOf(owner_->head()));

    if (!parent_) return;

    RemoveFromParent();
    auto prev_root = owner_->root_;
    owner_->root_ = this;
    owner_->DropItem(prev_root);

    for (auto observer : owner_->observers_) {
      observer->ObserveDrop();
    }
  }

  // Drops `this` and its descendants from the history tree.
  // head() mustn't be a descendant of `this`.
  void DropSelf() {
    assert(parent_);
    assert(!IsAncestorOf(owner_->head()));

    RemoveFromParent();

    auto owner = owner_;
    owner->DropItem(this);  // `this` is deleted here.

    for (auto observer : owner->observers_) {
      observer->ObserveDrop();
    }
  }

  // Drops all branches that `this` has from history tree.
  void DropAllBranch() {
    auto branch = std::move(branch_);
    branch_.clear();

    for (auto item : branch) {
      item->parent_ = nullptr;
    }
    for (auto item : branch) {
      owner_->DropItem(item);
    }
    for (auto observer : owner_->observers_) {
      observer->ObserveDrop();
    }
  }


  bool IsAncestorOf(const HistoryItem& other) const {
    for (const HistoryItem* itr = &other; itr; itr = itr->parent_) {
      if (itr == this) return true;
    }
    return false;
  }

  bool IsDescendantOf(const HistoryItem& other) const {
    return other.IsAncestorOf(*this);
  }

  HistoryItem& FindLowestCommonAncestor(const HistoryItem& other) const {
    for (const HistoryItem* itr = this; ; itr = itr->parent_) {
      assert(itr);
      if (other.IsDescendantOf(*itr)) {
        return *const_cast<HistoryItem*>(itr);
      }
    }
  }


  void Serialize(iSerializer*) const override;


  History& owner() const {
    return *owner_;
  }
  Id id() const {
    return id_;
  }
  time_t createdAt() const {
    return created_at_;
  }
  iCommand& command() const {
    return *command_;
  }
  bool marked() const {
    return marked_;
  }

  bool isRoot() const {
    return !parent_;
  }
  HistoryItem& parent() const {
    assert(parent_);
    return *parent_;
  }
  const ItemList& branch() const {
    return branch_;
  }

 private:
  HistoryItem(History* owner,
              Id id,
              time_t created_at,
              std::unique_ptr<iCommand>&& command,
              bool marked = false) :
      owner_(owner), id_(id), created_at_(created_at),
      command_(std::move(command)), marked_(marked) {
    assert(owner_);
    assert(command_);
  }

  void RemoveFromParent() {
    if (!parent_) return;

    auto& parbranch = parent_->branch_;
    auto self = std::find(parbranch.begin(), parbranch.end(), this);

    assert(self != parbranch.end());
    parent_->branch_.erase(self);
    parent_ = nullptr;
  }


  History* owner_;

  Id id_;

  time_t created_at_;

  std::unique_ptr<iCommand> command_;

  bool marked_ = false;


  HistoryItem* parent_ = nullptr;

  ItemList branch_;
};


iHistoryObserver::iHistoryObserver(History* target) : target_(target) {
  assert(target_);
  target_->observers_.push_back(this);
}
iHistoryObserver::~iHistoryObserver() {
  if (!target_) return;
  auto& obs = target_->observers_;
  obs.erase(std::remove(obs.begin(), obs.end(), this), obs.end());
}

}  // namespace mnian::core
