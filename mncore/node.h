// No copyright
//
//
#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mncore/action.h"
#include "mncore/serialize.h"
#include "mncore/task.h"


namespace mnian::core {

class iNode;


// An observer interface for iNode, whose constructor registers to the target,
// and destructor unregisters if the target is still alive.
class iNodeObserver {
 public:
  friend class iNode;


  iNodeObserver() = delete;
  inline explicit iNodeObserver(iNode* target);
  inline virtual ~iNodeObserver();

  iNodeObserver(const iNodeObserver&) = delete;
  iNodeObserver(iNodeObserver&&) = delete;

  iNodeObserver& operator=(const iNodeObserver&) = delete;
  iNodeObserver& operator=(iNodeObserver&&) = delete;


  // Be called when the node is registered to NodeStore.
  virtual void ObserveNew() {
  }

  // Be called when the node is deleted completely from the entire project.
  // There's no chance for the node to be used anymore.
  virtual void ObserveDelete() {
  }

  // Be called when the node is added to the project including a case in
  // recovering from History.
  virtual void ObserveActivate() {
  }

  // Be called when the node is removed from the project. However it's
  // still alive in History and can be recovered.
  virtual void ObserveDeactivate() {
  }

  // Be called when the node's output is affected by changes of parameters or
  // child nodes.
  virtual void ObserveUpdate() {
  }

  // Be called when the node's input or output socket gets open or close.
  virtual void ObserveSocketChange() {
  }


  iNode& target() const {
    return *target_;
  }

 private:
  iNode* target_;
};


// An interface of Node, which takes input and produces output async.
class iNode : public iActionable, public iPolymorphicSerializable {
 public:
  friend class iNodeObserver;
  friend class NodeStore;


  using Id = uint64_t;

  struct Socket {
   public:
    friend iNode;


    enum Type {
      kInteger,
      kScalar,
      kString,
    };


    Socket() = delete;
    Socket(const std::string& name, Type type) : name_(name), type_(type) {
    }

    Socket(const Socket&) = default;
    Socket(Socket&&) = default;

    Socket& operator=(const Socket&) = default;
    Socket& operator=(Socket&&) = default;


    bool Match(const Socket& other) const {
      return type_ == other.type_;
    }


    const std::string& name() const {
      return name_;
    }
    Type type() const {
      return type_;
    }
    bool opened() const {
      return open_;
    }

   private:
    std::string name_;

    Type type_;

    bool open_ = false;
  };


  iNode() = delete;
  iNode(ActionList&&          actions,
        const char*           type,
        std::vector<Socket>&& in,
        std::vector<Socket>&& out) :
      iActionable(std::move(actions)), iPolymorphicSerializable(type),
      input_(std::move(in)), output_(std::move(out)) {
  }
  ~iNode() {
    for (auto observer : observers_) {
      observer->target_ = nullptr;
      observer->ObserveDelete();
    }
  }

  iNode(const iNode&) = delete;
  iNode(iNode&&) = delete;

  iNode& operator=(const iNode&) = delete;
  iNode& operator=(iNode&&) = delete;


  virtual std::unique_ptr<iNode> Clone() = 0;

  virtual std::shared_ptr<iTask> QueueTask() = 0;


  void NotifyActivate() {
    for (auto observer : observers_) {
      observer->ObserveActivate();
    }
  }
  void NotifyDeactivate() {
    for (auto observer : observers_) {
      observer->ObserveDeactivate();
    }
  }


  Id id() const {
    return id_;
  }
  const std::vector<Socket>& input() const {
    return input_;
  }
  const std::vector<Socket>& output() const {
    return output_;
  }

 protected:
  void NotifyUpdate() {
    for (auto observer : observers_) {
      observer->ObserveUpdate();
    }
  }


  void OpenInput(size_t i) {
    input_[i].open_ = true;
    NotifySocketChange();
  }
  void CloseInput(size_t i) {
    input_[i].open_ = false;
    NotifySocketChange();
  }

  void OpenOutput(size_t i) {
    output_[i].open_ = true;
    NotifySocketChange();
  }
  void CloseOutput(size_t i) {
    output_[i].open_ = false;
    NotifySocketChange();
  }

 private:
  void NotifyNew() {
    for (auto observer : observers_) {
      observer->ObserveNew();
    }
  }
  void NotifySocketChange() {
    for (auto observer : observers_) {
      observer->ObserveSocketChange();
    }
  }


  Id id_;

  std::vector<Socket> input_;
  std::vector<Socket> output_;

  std::vector<iNodeObserver*> observers_;
};


// An interface of factory that creates Node instance.
class iNodeFactory : public iActionable {
 public:
  iNodeFactory() = delete;
  iNodeFactory(ActionList&& actions, const std::string& name) :
      iActionable(std::move(actions)), name_(name) {
  }

  iNodeFactory(const iNodeFactory&) = delete;
  iNodeFactory(iNodeFactory&&) = delete;

  iNodeFactory& operator=(const iNodeFactory&) = delete;
  iNodeFactory& operator=(iNodeFactory&&) = delete;


  virtual std::unique_ptr<iNode> Create() = 0;


  const std::string& name() const {
    return name_;
  }

 private:
  std::string name_;
};


// A store for all nodes in the entire of project including History.
class NodeStore final : public iSerializable {
 public:
  NodeStore() = default;

  NodeStore(const NodeStore&) = delete;
  NodeStore(NodeStore&&) = delete;

  NodeStore& operator=(const NodeStore&) = delete;
  NodeStore& operator=(NodeStore&&) = delete;


  // Adds new node to this store.
  iNode* Add(std::unique_ptr<iNode>&& node) {
    const auto id = next_++;

    auto ptr = node.get();
    items_[id] = std::move(node);

    ptr->id_ = id;
    ptr->NotifyNew();
    return ptr;
  }
  iNode* Create(iNodeFactory* f) {
    return Add(f->Create());
  }

  // Deletes the node completely. This means that the node will never be used
  // anymore.
  bool Drop(iNode::Id id) {
    auto itr = items_.find(id);
    if (itr == items_.end()) return false;
    items_.erase(itr);
    return true;
  }
  bool Drop(const iNode* node) {
    return Drop(node->id());
  }

  // Drops all nodes.
  void Clear() {
    items_.clear();
    next_ = 0;
  }


  iNode* Find(iNode::Id id) const {
    auto itr = items_.find(id);
    if (itr == items_.end()) return nullptr;
    return itr->second.get();
  }


  bool Deserialize(iDeserializer*);
  void Serialize(iSerializer*) const override;

 private:
  std::unordered_map<iNode::Id, std::unique_ptr<iNode>> items_;

  iNode::Id next_;
};


iNodeObserver::iNodeObserver(iNode* target) : target_(target) {
  assert(target_);
  target_->observers_.push_back(this);
}
iNodeObserver::~iNodeObserver() {
  if (!target_) return;
  auto& obs = target_->observers_;
  obs.erase(std::remove(obs.begin(), obs.end(), this), obs.end());
}

}  // namespace mnian::core
