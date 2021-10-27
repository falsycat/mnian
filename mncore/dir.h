// No copyright
//
// This file declares interfaces related to a file tree, Dir.
#pragma once

#include <algorithm>
#include <cassert>
#include <map>
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


  // Be called when contents of the item is updated.
  virtual void ObserveUpdate() {
  }

  // Be called when the item is added to directory tree.
  virtual void ObserveAdd() {
  }
  // Be called when the item is moved to other directory or renamed.
  virtual void ObserveMove() {
  }
  // Be called when the item is removed from directory tree to History.
  // When recovers from the History, ObserveAdd is called.
  virtual void ObserveRemove() {
  }
  // Be called when the item is deleted from memory. The item is completely
  // deleted and not recoverable anymore.
  virtual void ObserveDelete() {
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
  friend class iDirItemObserver;
  friend class Dir;


  // Returns a reason on failure, or std::nullopt on success.
  static std::optional<std::string> ValidateName(const std::string& name);


  // Deserializes a reference to an item from path expressed in string array.
  // Returns nullptr if no such item is found.
  static iDirItem* DeserializeRef(iDeserializer* des);


  iDirItem() = delete;
  iDirItem(ActionList&& actions, const char* type) :
      iActionable(std::move(actions)),
      iPolymorphicSerializable(type) {
  }
  ~iDirItem() override {
    for (auto observer : observers_) {
      observer->ObserveDelete();
      observer->target_ = nullptr;
    }
  }

  iDirItem(const iDirItem&) = delete;
  iDirItem(iDirItem&&) = delete;

  iDirItem& operator=(const iDirItem&) = delete;
  iDirItem& operator=(iDirItem&&) = delete;


  virtual void Visit(iDirItemVisitor* visitor) = 0;


  std::vector<std::string> GeneratePath() const;

  void NotifyUpdate() const {
    for (auto& observer : observers_) {
      observer->ObserveUpdate();
    }
  }


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

 protected:
  void NotifyAdd() {
    for (auto& observer : observers_) {
      observer->ObserveAdd();
    }
  }
  void NotifyMove() {
    for (auto& observer : observers_) {
      observer->ObserveMove();
    }
  }
  void NotifyRemove() {
    for (auto& observer : observers_) {
      observer->ObserveRemove();
    }
  }

 private:
  Dir* parent_ = nullptr;

  std::string name_;

  std::vector<iDirItemObserver*> observers_;
};


// Dir is a DirItem which owns child DirItems.
class Dir : public iDirItem {
 public:
  using ItemMap = std::map<std::string, std::unique_ptr<iDirItem>>;

  // Deserializes a reference to Dir from path expressed in string array.
  // Returns nullptr if no such item is found.
  static Dir* DeserializeRef(iDeserializer* des) {
    return dynamic_cast<Dir*>(iDirItem::DeserializeRef(des));
  }

  static ItemMap DeserializeParam(iDeserializer*);


  Dir() = delete;
  explicit Dir(ActionList&& actions,
               const char*  type  = "Dir",
               ItemMap&&    items = {}) :
      iDirItem(std::move(actions), type), items_(std::move(items)) {
    for (auto& item : items_) {
      const auto& name = item.first;
      auto        ptr  = item.second.get();
      ptr->name_   = name;
      ptr->parent_ = this;
      ptr->NotifyAdd();
    }
  }

  Dir(const Dir&) = delete;
  Dir(Dir&&) = delete;

  Dir& operator=(const Dir&) = delete;
  Dir& operator=(Dir&&) = delete;


  void Visit(iDirItemVisitor* visitor) final {
    assert(visitor);
    visitor->VisitDir(this);
  }


  // Name duplication check must be done in advance.
  iDirItem* Add(const std::string& name, std::unique_ptr<iDirItem>&& item) {
    assert(!items_.contains(name));
    assert(!ValidateName(name));

    item->name_   = name;
    item->parent_ = this;

    auto ret = item.get();
    items_[name] = std::move(item);
    ret->NotifyAdd();
    NotifyUpdate();
    return ret;
  }

  std::unique_ptr<iDirItem> Remove(const std::string& name) {
    auto itr = items_.find(name);
    if (itr == items_.end()) return nullptr;

    auto ret = std::move(itr->second);
    items_.erase(itr);
    ret->NotifyRemove();
    NotifyUpdate();
    return ret;
  }

  iDirItem* Move(const std::string& name, Dir* dst, const std::string& dname) {
    assert(dst);
    assert(!dst->items_.contains(dname));
    assert(!ValidateName(dname));

    auto itr = items_.find(name);
    if (itr == items_.end()) return nullptr;

    auto item = std::move(itr->second);
    items_.erase(itr);

    auto ptr = item.get();
    item->name_   = dname;
    item->parent_ = dst;
    dst->items_[dname] = std::move(item);

    ptr->NotifyMove();
    dst->NotifyUpdate();
    NotifyUpdate();
    return ptr;
  }

  iDirItem* Rename(const std::string& src, const std::string& dst) {
    return Move(src, this, dst);
  }


  iDirItem* Find(const std::string& name) const {
    auto itr = items_.find(name);
    if (itr == items_.end()) return nullptr;
    return itr->second.get();
  }

  iDirItem* FindPath(const std::vector<std::string>& path) const {
    iDirItem* ret = const_cast<iDirItem*>(static_cast<const iDirItem*>(this));
    for (auto& term : path) {
      const auto prev = dynamic_cast<Dir*>(ret);
      if (!prev) return nullptr;
      ret = prev->Find(term);
    }
    return ret;
  }


  const ItemMap& items() const {
    return items_;
  }

 protected:
  void SerializeParam(iSerializer* serializer) const override;

 private:
  ItemMap items_;
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

  // Deserializes a reference to FileRef from path expressed in string array.
  // Returns nullptr if no such item is found.
  static FileRef* DeserializeRef(iDeserializer* des) {
    return dynamic_cast<FileRef*>(iDirItem::DeserializeRef(des));
  }


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


  void Visit(iDirItemVisitor* visitor) final {
    visitor->VisitFile(this);
  }


  void ReplaceEntity(iFile* file) {
    assert(file);
    if (file == file_) return;

    file_ = file;
    NotifyUpdate();
  }

  void SetFlag(Flag flag) {
    if (flags_ & flag) return;

    flags_ |= flag;
    NotifyUpdate();
  }
  void UnsetFlag(Flag flag) {
    if (!(flags_ & flag)) return;

    flags_ = static_cast<Flags>(flags_ & ~flag);
    NotifyUpdate();
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
      owner_->NotifyUpdate();
    }

   private:
    FileRef* owner_;
  };


  iFile* file_;

  Flags flags_;

  FileObserver observer_;
};


// NodeRef is a DirItem which wraps iNode object.
class NodeRef : public iDirItem {
 public:
  // Deserializes a reference to NodeRef from path expressed in string array.
  // Returns nullptr if no such item is found.
  static NodeRef* DeserializeRef(iDeserializer* des) {
    return dynamic_cast<NodeRef*>(iDirItem::DeserializeRef(des));
  }


  NodeRef() = delete;
  NodeRef(ActionList&&             actions,
          const char*              type,
          NodeStore*               store,
          std::unique_ptr<iNode>&& node) :
      iDirItem(std::move(actions), type),
      store_(store), node_(store_->Add(std::move(node))), observer_(this) {
    assert(store_);
    assert(node_);
  }
  NodeRef(ActionList&&             actions,
          NodeStore*               store,
          std::unique_ptr<iNode>&& node) :
      NodeRef(std::move(actions), "NodeRef", store, std::move(node)) {
  }
  ~NodeRef() {
    store_->Drop(node_);
  }

  NodeRef(const NodeRef&) = delete;
  NodeRef(NodeRef&&) = delete;

  NodeRef& operator=(const NodeRef&) = delete;
  NodeRef& operator=(NodeRef&&) = delete;


  void Visit(iDirItemVisitor* visitor) final {
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
      owner_->NotifyUpdate();
    }

   private:
    NodeRef* owner_;
  };


  NodeStore* store_;

  iNode* node_;

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
