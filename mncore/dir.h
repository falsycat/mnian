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

#include "mncore/file.h"
#include "mncore/node.h"
#include "mncore/serialize.h"
#include "mncore/store.h"


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
class iDirItem : public iPolymorphicSerializable {
 public:
  friend class iDirItemObserver;
  friend class Dir;


  using Store = ObjectStore<iDirItem>;
  using Tag   = Store::Tag;


  // Returns a reason on failure, or std::nullopt on success.
  static std::optional<std::string> ValidateName(const std::string& name);


  // Deserializes a reference to an item from id.  Returns nullptr if no such
  // item is found.
  static iDirItem* DeserializeRef(iDeserializer* des);


  iDirItem() = delete;
  iDirItem(const char* type, Tag&& tag) :
      iPolymorphicSerializable(type), tag_(std::move(tag)) {
    tag_.Attach(this);
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


  virtual std::unique_ptr<iDirItem> Clone() const = 0;

  virtual void Visit(iDirItemVisitor* visitor) = 0;


  std::vector<std::string> GeneratePath() const;

  void NotifyUpdate() const {
    for (auto& observer : observers_) {
      observer->ObserveUpdate();
    }
  }


  bool IsAncestorOf(const iDirItem& other) const;
  bool IsDescendantOf(const iDirItem& other) const {
    return other.IsAncestorOf(*this);
  }


  void SerializeRef(iSerializer* serial) const {
    serial->SerializeValue(static_cast<int64_t>(id()));
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

  ObjectId id() const {
    return tag_.id();
  }
  const Tag& tag() const {
    return tag_;
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
  Tag tag_;

  Dir* parent_ = nullptr;

  std::string name_;

  std::vector<iDirItemObserver*> observers_;
};


// Dir is a DirItem which owns child DirItems.
class Dir final : public iDirItem {
 public:
  static constexpr const char* kType = "mnian::core::Dir";


  using ItemMap = std::map<std::string, std::unique_ptr<iDirItem>>;


  static Dir* DeserializeRef(iDeserializer* des) {
    return dynamic_cast<Dir*>(iDirItem::DeserializeRef(des));
  }

  static std::unique_ptr<Dir> DeserializeParam(iDeserializer*);


  Dir(Tag&& tag, ItemMap&& items = {}) :
      iDirItem(kType, std::move(tag)), items_(std::move(items)) {
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


  std::unique_ptr<iDirItem> Clone() const override {
    ItemMap clone_item;
    for (auto& item : items_) {
      clone_item[item.first] = item.second->Clone();
    }
    return std::make_unique<Dir>(Tag(tag()), std::move(clone_item));
  }

  void Visit(iDirItemVisitor* visitor) override {
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
class FileRef final : public iDirItem {
 public:
  static constexpr const char* kType = "mnian::core::FileRef";


  enum Flag : uint16_t {
    kNone     = 0,
    kReadable = 1 << 0,
    kWritable = 1 << 1,
  };
  using Flags = uint16_t;


  static std::string StringifyFlags(Flags);

  static std::optional<Flags> ParseFlags(const std::string&);
  static std::optional<Flag>  ParseFlag(char c);

  static std::optional<Flag> ParseFlag(const std::string& v) {
    return v.size()? ParseFlag(v[0]): std::nullopt;
  }

  static std::optional<Flags> DeserializeFlags(iDeserializer* des) {
    assert(des);

    const auto str = des->value<std::string>();
    if (!str) return std::nullopt;

    const auto ret = ParseFlags(*str);
    if (!ret) return std::nullopt;
    return *ret;
  }
  static std::optional<Flag> DeserializeFlag(iDeserializer* des) {
    assert(des);

    const auto str = des->value<std::string>();
    if (!str) return std::nullopt;

    const auto ret = ParseFlag(*str);
    if (!ret) return std::nullopt;
    return *ret;
  }

  static std::unique_ptr<FileRef> DeserializeParam(iDeserializer* des);

  static FileRef* DeserializeRef(iDeserializer* des) {
    return dynamic_cast<FileRef*>(iDirItem::DeserializeRef(des));
  }


  FileRef() = delete;
  FileRef(Tag&& tag, std::shared_ptr<iFile> file, Flags flags) :
      iDirItem(kType, std::move(tag)),
      file_(file), flags_(flags), observer_(this) {
    assert(file_);
  }

  FileRef(const FileRef&) = delete;
  FileRef(FileRef&&) = delete;

  FileRef& operator=(const FileRef&) = delete;
  FileRef& operator=(FileRef&&) = delete;


  std::unique_ptr<iDirItem> Clone() const override {
    return std::make_unique<FileRef>(Tag(tag()), file_, flags_);
  }

  void Visit(iDirItemVisitor* visitor) override {
    visitor->VisitFile(this);
  }


  void ReplaceEntity(std::shared_ptr<iFile> file) {
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


  std::shared_ptr<iFile> entity() const {
    return file_;
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
        iFileObserver(owner->entity().get()), owner_(owner) {
      assert(owner_);
    }

    void ObserveUpdate() override {
      owner_->NotifyUpdate();
    }

   private:
    FileRef* owner_;
  };


  std::shared_ptr<iFile> file_;

  Flags flags_;

  FileObserver observer_;
};


// NodeRef is a DirItem which wraps iNode object.
class NodeRef final : public iDirItem {
 public:
  static constexpr const char* kType = "mnian::core::NodeRef";


  static NodeRef* DeserializeRef(iDeserializer* des) {
    return dynamic_cast<NodeRef*>(iDirItem::DeserializeRef(des));
  }

  static std::unique_ptr<NodeRef> DeserializeParam(iDeserializer* des);


  NodeRef() = delete;
  NodeRef(Tag&& tag, std::unique_ptr<iNode>&& node) :
      iDirItem(kType, std::move(tag)), node_(std::move(node)), observer_(this) {
    assert(node_);
  }

  NodeRef(const NodeRef&) = delete;
  NodeRef(NodeRef&&) = delete;

  NodeRef& operator=(const NodeRef&) = delete;
  NodeRef& operator=(NodeRef&&) = delete;


  std::unique_ptr<iDirItem> Clone() const override {
    return std::make_unique<NodeRef>(Tag(tag()), node_->Clone());
  }

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
      owner_->NotifyUpdate();
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
