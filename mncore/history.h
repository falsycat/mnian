// No copyright
//
// This file declares classes related to History, which is a tree structure of
// Command that represents all controls user has done or reverted.
#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <stack>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mncore/clock.h"
#include "mncore/command.h"
#include "mncore/serialize.h"


namespace mnian::core {

class History : public iSerializable {
 public:
  class Item final {
   public:
    friend class History;


    static std::optional<std::vector<std::unique_ptr<Item>>> DeserializeBranch(
        iDeserializer*,
        History*,
        std::vector<std::unique_ptr<iCommand>>*);

    static std::unique_ptr<Item> Deserialize(
        iDeserializer*,
        History*,
        std::vector<std::unique_ptr<iCommand>>*);


    Item() = delete;

    Item(const Item&) = delete;
    Item(Item&&) = delete;

    Item& operator=(const Item&) = delete;
    Item& operator=(Item&&) = delete;


    void DropAllAncestors() {
      assert(IsAncestorOf(owner_->head()));
      if (!parent_) return;

      owner_->origin_ = RemoveFromParent();
    }
    void DropSelf() {
      assert(parent_);
      assert(!IsAncestorOf(owner_->head()));

      RemoveFromParent();
    }
    void DropAllBranch() {
      assert(&owner_->head() == this || !IsAncestorOf(owner_->head()));

      branch_.clear();
    }


    void Fork(std::unique_ptr<Item>&& item) {
      assert(item);

      item->parent_ = this;
      item->index_  = branch_.size();
      branch_.push_back(std::move(item));
    }
    void Fork(std::unique_ptr<iCommand>&& command) {
      Fork(std::unique_ptr<Item>(
              new Item(owner_, owner_->clock_->now(), std::move(command))));
    }

    void TouchBranch(size_t index) {
      auto target = std::move(branch_[index]);
      branch_.erase(branch_.begin() + static_cast<intmax_t>(index));
      branch_.push_back(std::move(target));

      for (size_t i = index; i < branch_.size(); ++i) {
        branch_[i]->index_ = i;
      }
    }


    bool IsAncestorOf(const Item& other) const {
      for (const Item* itr = &other; itr; itr = itr->parent_) {
        if (itr == this) return true;
      }
      return false;
    }
    bool IsDescendantOf(const Item& other) const {
      return other.IsAncestorOf(*this);
    }
    Item& FindLowestCommonAncestor(const Item& other) const {
      for (const Item* itr = this; ; itr = itr->parent_) {
        assert(itr);
        if (other.IsDescendantOf(*itr)) {
          return *const_cast<Item*>(itr);
        }
      }
    }


    std::stack<size_t> GeneratePath() const {
      std::stack<size_t> path;
      auto itr = this;
      while (!itr->isOrigin()) {
        path.push(itr->index());
        itr = &itr->parent();
      }
      return path;
    }


    void SerializePastCommands(
        std::vector<iCommand*>*                cmd,
        std::unordered_map<iCommand*, size_t>* idx,
        size_t                                 prev = SIZE_MAX) const;

    void SerializeFutureCommands(
        std::vector<iCommand*>*                cmd,
        std::unordered_map<iCommand*, size_t>* idx) const;

    void Serialize(
        iSerializer*, const std::unordered_map<iCommand*, size_t>& idx) const;


    History& owner() const {
      return *owner_;
    }
    time_t createdAt() const {
      return created_at_;
    }
    iCommand& command() const {
      return *command_;
    }

    bool isOrigin() const {
      return !parent_;
    }
    Item& parent() const {
      assert(parent_);
      return *parent_;
    }
    size_t index() const {
      return index_;
    }
    const auto& branch() const {
      return branch_;
    }

   private:
    Item(History*                             owner,
         time_t                               created_at,
         std::unique_ptr<iCommand>&&          command,
         std::vector<std::unique_ptr<Item>>&& branch = {}) :
        owner_(owner),
        created_at_(created_at),
        command_(std::move(command)),
        branch_(std::move(branch)) {
      assert(owner_);
      assert(command_);

      for (size_t i = 0; i < branch_.size(); ++i) {
        branch_[i]->parent_ = this;
        branch_[i]->index_  = i;
      }
    }


    std::unique_ptr<Item> RemoveFromParent() {
      assert(parent_);
      auto& pb = parent_->branch_;

      auto self = std::move(pb[index_]);
      assert(self.get() == this);

      pb.erase(pb.begin() + static_cast<intmax_t>(index_));
      for (size_t i = index_; i < pb.size(); ++i) {
        pb[i]->index_ = i;
      }

      parent_ = nullptr;
      return self;
    }


    History* owner_ = nullptr;

    time_t created_at_ = 0;

    std::unique_ptr<iCommand> command_;


    Item* parent_ = nullptr;

    size_t index_ = 0;  // an index of this in parent's branch

    std::vector<std::unique_ptr<Item>> branch_;
  };


  History() = delete;
  explicit History(const iClock*               clock,
                   std::unique_ptr<iCommand>&& cmd = nullptr) :
      clock_(clock),
      origin_(
          new Item(
              this,
              0,  // origin item's creation datetime is always zero
              cmd? std::move(cmd): std::make_unique<NullCommand>(""))),
      head_(origin_.get()) {
    assert(clock_);
    origin_->owner_ = this;
  }

  History(const History&) = delete;
  History(History&&) = delete;

  History& operator=(const History&) = delete;
  History& operator=(History&&) = delete;


  // Creates new item, then forks from head(), and finally ReDo()
  bool Exec(std::unique_ptr<iCommand>&& command) {
    head_->Fork(std::move(command));
    return ReDo();
  }

  // head() must have one or more branch.
  bool ReDo(size_t index = SIZE_MAX) {
    const auto& branch = head_->branch();

    const size_t n = branch.size();
    assert(n > 0);
    if (index >= n) index = n-1;

    if (!branch[index]->command().Apply()) {
      return false;
    }
    head_->TouchBranch(index);
    head_ = branch.back().get();
    return true;
  }

  // head() mustn't be a origin.
  bool UnDo() {
    assert(!head_->isOrigin());

    if (!head_->command().Revert()) return false;
    head_ = &head_->parent();
    return true;
  }

  // Drops all history and Makes NullCommand origin.
  void Clear() {
    head_ = origin_.get();
    origin_->DropAllBranch();
  }


  // Returns true if the current history tree is properly replaced by new one,
  // otherwise false and changes nothing.
  bool Deserialize(iDeserializer* des);

  void Serialize(iSerializer*) const final;


  const iClock& clock() const {
    return *clock_;
  }

  Item& origin() const {
    return *origin_;
  }
  Item& head() const {
    return *head_;
  }

 private:
  const iClock* clock_;

  std::unique_ptr<Item> origin_;

  Item* head_;
};

}  // namespace mnian::core
