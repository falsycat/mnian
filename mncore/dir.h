// No copyright
//
// This file declares interfaces related to a file tree, Dir.
#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "mncore/action.h"
#include "mncore/file.h"
#include "mncore/node.h"
#include "mncore/serialize.h"


namespace mnian::core {

class iDirItem;
class Dir;
class FileRef;
class NodeRef;


class iDirItemVisitor {
 public:
  iDirItemVisitor() = default;
  virtual ~iDirItemVisitor() = default;

  iDirItemVisitor(const iDirItemVisitor&) = default;
  iDirItemVisitor(iDirItemVisitor&&) = default;

  iDirItemVisitor& operator=(const iDirItemVisitor&) = default;
  iDirItemVisitor& operator=(iDirItemVisitor&&) = default;


  virtual void VisitDir(Dir*) = 0;
  virtual void VisitFile(FileRef*) = 0;
  virtual void VisitNode(NodeRef*) = 0;
};


// An observer interface for iDirItem, whose constructor registers to the
// target, and destructor unregisters if the target is still alive.
// When ObserveRemove() is called, some methods get unavailable, and this
// object should be destructed as soon as possible.
class iDirItemObserver {
 public:
  friend class iDirItem;


  iDirItemObserver() = delete;
  inline explicit iDirItemObserver(iDirItem* target);
  inline virtual ~iDirItemObserver();

  iDirItemObserver(const iDirItemObserver&) = delete;
  iDirItemObserver(iDirItemObserver&&) = delete;

  iDirItemObserver& operator=(const iDirItemObserver&) = delete;
  iDirItemObserver& operator=(iDirItemObserver&&) = delete;


  virtual void ObserveUpdate() {
  }
  virtual void ObserveMove() {
  }
  virtual void ObserveRemove() {
  }


  iDirItem& target() const {
    assert(target_);  // When the target is already deleted, target_ is nullptr.
    return *target_;
  }

 private:
  iDirItem* target_;
};


// An interface of DirItem, which composes Dir.
class iDirItem : public iActionable, public iPolymorphicSerializable {
 public:
  friend class Dir;
  friend class iDirItemObserver;


  // Returns a reason on failure, or std::nullopt on success.
  static std::optional<std::string> ValidateName(const std::string& name);


  iDirItem() = delete;
  iDirItem(ActionList&& actions, const char* type) :
      iActionable(std::move(actions)),
      iPolymorphicSerializable(type) {
  }
  ~iDirItem() override {
    for (auto observer : observers_) {
      observer->target_ = nullptr;
    }
    observers_.clear();
  }

  iDirItem(const iDirItem&) = delete;
  iDirItem(iDirItem&&) = delete;

  iDirItem& operator=(const iDirItem&) = delete;
  iDirItem& operator=(iDirItem&&) = delete;


  // Returns if the rename is done successfully.
  // Validation and  duplication check with siblings must be done in advance.
  void Rename(const std::string& name) {
    assert(!ValidateName(name));
    if (name == name_) return;

    name_ = name;
    for (auto& observer : observers_) {
      observer->ObserveMove();
    }
  }

  void Touch() const {
    for (auto& observer : observers_) {
      observer->ObserveUpdate();
    }
  }


  virtual void Visit(iDirItemVisitor* visitor) = 0;


  bool isRoot() const {
    return !parent_;
  }
  Dir& parent() const {
    assert(parent_);
    return *parent_;
  }
  const std::string& name() const {
    return name_;
  }

 private:
  Dir* parent_ = nullptr;

  std::string name_ = "";

  std::vector<iDirItemObserver*> observers_;
};


// Dir is a DirItem which owns child DirItems.
class Dir : public iDirItem {
 public:
  using ItemList = std::vector<std::unique_ptr<iDirItem>>;

  static ItemList DeserializeParam(iDeserializer*);


  Dir() = delete;
  explicit Dir(
      ActionList&& actions, const char* type = "Dir", ItemList&& items = {}) :
      iDirItem(std::move(actions), type), items_(std::move(items)) {
    for (auto& item : items_) AttachSelf(item.get());
  }
  ~Dir() {
    while (items_.size()) RemoveByIndex(0);
  }

  Dir(const Dir&) = delete;
  Dir(Dir&&) = delete;

  Dir& operator=(const Dir&) = delete;
  Dir& operator=(Dir&&) = delete;


  void Visit(iDirItemVisitor* visitor) override {
    assert(visitor);
    visitor->VisitDir(this);
  }


  // Name duplication check must be done in advance.
  iDirItem* Add(std::unique_ptr<iDirItem>&& item) {
    assert(item);

    iDirItem* ptr = item.get();
    AttachSelf(ptr);

    items_.push_back(std::move(item));

    Touch();
    return ptr;
  }

  std::unique_ptr<iDirItem> RemoveByIndex(size_t index) {
    auto item = RemoveQuietly(index);
    for (auto observer : item->observers_) {
      observer->ObserveRemove();
    }
    Touch();
    return item;
  }
  std::unique_ptr<iDirItem> Remove(const iDirItem* item) {
    auto index = FindIndexOf(item);
    assert(index);
    return RemoveByIndex(*index);
  }

  void MoveByIndex(size_t from, Dir* dst) {
    assert(dst);

    auto item = RemoveQuietly(from);
    Touch();

    iDirItem* ptr = dst->Add(std::move(item));
    for (auto observer : ptr->observers_) {
      observer->ObserveMove();
    }
    dst->Touch();
  }
  void Move(const iDirItem* item, Dir* dst) {
    auto index = FindIndexOf(item);
    assert(index);
    MoveByIndex(*index, dst);
  }

  std::optional<size_t> FindIndexOf(const std::string& name) const {
    auto itr = std::find_if(
        items_.begin(), items_.end(),
        [&](auto& x) { return x->name() == name; });
    if (itr == items_.end()) {
      return std::nullopt;
    }
    return itr-items_.begin();
  }
  std::optional<size_t> FindIndexOf(const iDirItem* item) const {
    assert(item);
    auto itr = std::find_if(
        items_.begin(), items_.end(),
        [&](auto& x) { return x.get() == item; });
    if (itr == items_.end()) {
      return std::nullopt;
    }
    return itr-items_.begin();
  }


  iDirItem& items(size_t index) const {
    return *items_[index];
  }
  size_t size() const {
    return items_.size();
  }

 protected:
  void SerializeParam(iSerializer* serializer) const override;

 private:
  void AttachSelf(iDirItem* item) {
    assert(!item->parent_);
    item->parent_ = this;
  }

  std::unique_ptr<iDirItem> RemoveQuietly(size_t index) {
    assert(index < items_.size());

    auto target = std::move(items_[index]);
    assert(target);
    target->parent_ = nullptr;

    items_.erase(items_.begin() + static_cast<int64_t>(index));
    return target;
  }


  ItemList items_;
};


// FileRef is a DirItem which wraps iFile object.
class FileRef : public iDirItem {
 public:
  enum Flag : uint16_t {
    kNone     = 0,
    kReadable = 1 << 0,
    kWritable = 1 << 1,
  };
  using Flags = uint16_t;


  static std::optional<Flags> ParseFlags(const std::string&);


  FileRef() = delete;
  FileRef(ActionList&& actions, const char* type, iFile* file, Flags flags) :
      iDirItem(std::move(actions), type),
      file_(file), flags_(flags), observer_(this) {
    assert(file_);
  }
  FileRef(ActionList&& actions, iFile* file, Flags flags) :
      FileRef(std::move(actions), "FileRef", file, flags) {
  }

  FileRef(const FileRef&) = delete;
  FileRef(FileRef&&) = delete;

  FileRef& operator=(const FileRef&) = delete;
  FileRef& operator=(FileRef&&) = delete;


  void Visit(iDirItemVisitor* visitor) override {
    visitor->VisitFile(this);
  }


  void ReplaceEntity(iFile* file) {
    assert(file);
    if (file == file_) return;

    file_ = file;
    Touch();
  }

  void SetFlag(Flag flag) {
    if (flags_ & flag) return;

    flags_ |= flag;
    Touch();
  }
  void UnsetFlag(Flag flag) {
    if (!(flags_ & flag)) return;

    flags_ = static_cast<Flags>(flags_ & ~flag);
    Touch();
  }


  iFile& entity() const {
    return *file_;
  }

  bool readable() const {
    return flags_ & kReadable;
  }
  bool writable() const {
    return flags_ & kWritable;
  }

 protected:
  void SerializeParam(iSerializer*) const override;

 private:
  class FileObserver : public iFileObserver {
   public:
    explicit FileObserver(FileRef* owner) :
        iFileObserver(&owner->entity()), owner_(owner) {
      assert(owner_);
    }

    void ObserveUpdate() override {
      owner_->Touch();
    }

   private:
    FileRef* owner_;
  };


  iFile* file_;

  Flags flags_;

  FileObserver observer_;
};


// FileRef is a DirItem which wraps iNode object.
class NodeRef : public iDirItem {
 public:
  NodeRef() = delete;
  NodeRef(ActionList&&             actions,
          const char*              type,
          std::unique_ptr<iNode>&& node) :
      iDirItem(std::move(actions), type),
      node_(std::move(node)), observer_(this) {
    assert(node_);
  }
  NodeRef(ActionList&& actions, std::unique_ptr<iNode>&& file) :
      NodeRef(std::move(actions), "NodeRef", std::move(file)) {
  }

  NodeRef(const NodeRef&) = delete;
  NodeRef(NodeRef&&) = delete;

  NodeRef& operator=(const NodeRef&) = delete;
  NodeRef& operator=(NodeRef&&) = delete;


  void Visit(iDirItemVisitor* visitor) override {
    visitor->VisitNode(this);
  }

  iNode& entity() const {
    return *node_;
  }

 protected:
  void SerializeParam(iSerializer*) const override;

 private:
  class NodeObserver : public iNodeObserver {
   public:
    explicit NodeObserver(NodeRef* owner) :
        iNodeObserver(&owner->entity()), owner_(owner) {
      assert(owner_);
    }

    void ObserveUpdate() override {
      owner_->Touch();
    }

   private:
    NodeRef* owner_;
  };


  std::unique_ptr<iNode> node_;

  NodeObserver observer_;
};


iDirItemObserver::iDirItemObserver(iDirItem* target) : target_(target) {
  assert(target_);
  target_->observers_.push_back(this);
}
iDirItemObserver::~iDirItemObserver() {
  if (!target_) return;
  auto& obs = target_->observers_;
  obs.erase(std::remove(obs.begin(), obs.end(), this), obs.end());
}

}  // namespace mnian::core
