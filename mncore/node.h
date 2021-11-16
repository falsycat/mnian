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
#include "mncore/store.h"
#include "mncore/task.h"


namespace mnian::core {

class iNode;


// An observer interface for iNode, whose constructor registers to the target,
// and destructor unregisters if the target is still alive.
class iNodeObserver {
 public:
  friend class iNode;


  iNodeObserver() = delete;
  explicit iNodeObserver(iNode* target);
  virtual ~iNodeObserver();

  iNodeObserver(const iNodeObserver&) = delete;
  iNodeObserver(iNodeObserver&&) = delete;

  iNodeObserver& operator=(const iNodeObserver&) = delete;
  iNodeObserver& operator=(iNodeObserver&&) = delete;


  // Be called when the node is added to project.
  virtual void ObserveRecover() {
  }
  // Be called when the node is removed from the project but it can be recovered
  // by history operation.
  virtual void ObserveRemove() {
  }
  // Be called when the node is completely deleted.
  virtual void ObserveDelete() {
  }

  // Be called when the node's output is affected by parameter changes.
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


  using Store = ObjectStore<iNode>;
  using Tag   = Store::Tag;

  struct Socket {
   public:
    friend class iNode;


    enum Type {
      kInteger,
      kScalar,
      kString,
    };

    enum State {
      kOpen,    // shown and connectable
      kClose,   // shown and not connectable
      kHidden,  // hidden and not connectable
    };


    Socket() = delete;
    Socket(const std::string& name, Type type) : name_(name), type_(type) {
    }

    Socket(const Socket&) = default;
    Socket(Socket&&) = default;

    Socket& operator=(const Socket&) = default;
    Socket& operator=(Socket&&) = default;


    void Update(State state) {
      state_ = state;
    }

    bool Match(const Socket& other) const {
      return type_ == other.type_;
    }


    const std::string& name() const {
      return name_;
    }
    Type type() const {
      return type_;
    }
    State state() const {
      return state_;
    }

   private:
    std::string name_;

    Type type_;

    State state_ = kClose;
  };


  // Deserializes an id integer and returns a pointer to the node.
  static iNode* DeserializeRef(iDeserializer* des);


  iNode() = delete;
  iNode(ActionList&&          actions,
        const char*           type,
        Tag&&                 tag,
        std::vector<Socket>&& in,
        std::vector<Socket>&& out) :
      iActionable(std::move(actions)), iPolymorphicSerializable(type),
      tag_(std::move(tag)), input_(std::move(in)), output_(std::move(out)) {
    tag_.Attach(this);
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


  ObjectId id() const {
    return tag_.id();
  }
  const Tag& tag() const {
    return tag_;
  }

  const std::vector<Socket>& input() const {
    return input_;
  }
  const std::vector<Socket>& output() const {
    return output_;
  }

 protected:
  void NotifyRecover() {
    for (auto observer : observers_) {
      observer->ObserveRecover();
    }
  }
  void NotifyRemove() {
    for (auto observer : observers_) {
      observer->ObserveRemove();
    }
  }
  void NotifyUpdate() {
    for (auto observer : observers_) {
      observer->ObserveUpdate();
    }
  }
  void NotifySocketChange() {
    for (auto observer : observers_) {
      observer->ObserveSocketChange();
    }
  }

  Socket* input() {
    return &input_[0];
  }
  Socket* output() {
    return &output_[0];
  }

 private:
  Tag tag_;

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

}  // namespace mnian::core
