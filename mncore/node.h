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
#include <variant>
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


  using Store = ObjectStore<iNode>;
  using Tag   = Store::Tag;

  using Value = iLambda::Value;

  class Socket;


  // Deserializes an id integer and returns a pointer to the node.
  static iNode* DeserializeRef(iDeserializer* des);


  iNode() = delete;
  iNode(ActionList&& actions, const char* type, Tag&& tag) :
      iActionable(std::move(actions)), iPolymorphicSerializable(type),
      tag_(std::move(tag)) {
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

  virtual std::shared_ptr<iLambda> QueueLambda() = 0;


  std::unordered_map<const Socket*, size_t> CreateSocketIndexMap() const;


  ObjectId id() const {
    return tag_.id();
  }
  const Tag& tag() const {
    return tag_;
  }

  size_t inputCount() const {
    return input_.size();
  }
  size_t outputCount() const {
    return input_.size();
  }

  const Socket& input(size_t i) const {
    assert(i < input_.size());
    return *input_[i];
  }
  const Socket& output(size_t i) const {
    assert(i < output_.size());
    return *output_[i];
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

  auto& input() {
    return input_;
  }
  auto& output() {
    return output_;
  }

 private:
  Tag tag_;

  std::vector<std::unique_ptr<Socket>> input_;
  std::vector<std::unique_ptr<Socket>> output_;

  std::vector<iNodeObserver*> observers_;
};


class iNode::Socket final {
 public:
  enum Type {
    kInteger,
    kScalar,
    kVec2,
    kVec3,
    kVec4,
    kTensor,
    kString,
  };

  // Don't forget that Meta is just a request for input supplier, and the actual
  // input is not limited strictly.
  struct Meta {
    std::string name;
    std::string description = "";

    const char* purpose = "";

    // for kInteger and kScalar
    double min = 0.;
    double max = 0.;  // When min == max, there's no restriction.

    // for kString
    bool multiline = false;
  };


  static Type GetTypeFromValue(const iLambda::Value& v) {
    if (std::holds_alternative<int64_t>(v)) {
      return kInteger;
    }
    if (std::holds_alternative<double>(v)) {
      return kScalar;
    }
    if (std::holds_alternative<std::shared_ptr<std::string>>(v)) {
      return kString;
    }
    assert(false);
    return kInteger;
  }


  Socket() = delete;
  Socket(size_t index, Meta&& meta, Value&& def) :
      index_(index),
      type_(GetTypeFromValue(def)),
      meta_(std::move(meta)),
      def_(std::move(def)) {
  }

  Socket(const Socket&) = delete;
  Socket(Socket&&) = delete;

  Socket& operator=(const Socket&) = delete;
  Socket& operator=(Socket&&) = delete;


  size_t index() const {
    return index_;
  }
  Type type() const {
    return type_;
  }
  const Meta& meta() const {
    return meta_;
  }
  const Value& def() const {
    return def_;
  }

 private:
  size_t index_;

  Type  type_;
  Meta  meta_;
  Value def_;
};

}  // namespace mnian::core
