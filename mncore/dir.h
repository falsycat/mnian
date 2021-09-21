// No copyright
//
// This file declares interfaces related to a file tree, Dir.
#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "mncore/action.h"
#include "mncore/file.h"
#include "mncore/serialize.h"


namespace mnian::core {

class iDirItem;
class Dir;


class iDirItemVisitor {
 public:
  iDirItemVisitor() = default;
  virtual ~iDirItemVisitor() = default;

  iDirItemVisitor(const iDirItemVisitor&) = default;
  iDirItemVisitor(iDirItemVisitor&&) = default;

  iDirItemVisitor& operator=(const iDirItemVisitor&) = default;
  iDirItemVisitor& operator=(iDirItemVisitor&&) = default;


  virtual void VisitDir(Dir*) = 0;
  virtual void VisitFile(iFile*) = 0;
};


// An observer interface for iDirItem, whose constructor registers to the
// target, and destructor unregisters if the target is still alive.
// When ObserveRemove() is called, some methods get unavailable, and this
// object should be destructed as soon as possible.
class iDirItemObserver {
 public:
  friend class iDirItem;


  iDirItemObserver() = delete;
  explicit iDirItemObserver(iDirItem* target);
  virtual ~iDirItemObserver();

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
class iDirItem : public iActionable, public iSerializable {
 public:
  friend class Dir;
  friend class iDirItemObserver;


  // Returns a reason on failure, or an empty on success.
  static std::string ValidateName(const std::string& name);


  iDirItem() = delete;
  explicit iDirItem(ActionList&& actions) : iActionable(std::move(actions)) {
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
    assert(ValidateName(name).empty());
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
  Dir() = delete;
  explicit Dir(ActionList&& actions) : iDirItem(std::move(actions)) {
  }
  ~Dir() {
    while (items_.size()) RemoveByIndex(0);
  }

  Dir(const Dir&) = delete;
  Dir(Dir&&) = delete;

  Dir& operator=(const Dir&) = delete;
  Dir& operator=(Dir&&) = delete;

  iDirItem& operator[](size_t index) const {
    assert(index < items_.size());
    return *items_[index];
  }


  void Visit(iDirItemVisitor* visitor) override {
    assert(visitor);
    visitor->VisitDir(this);
  }


  // Name duplication check must be done in advance.
  iDirItem* Add(std::unique_ptr<iDirItem>&& item) {
    assert(item);
    assert(!item->parent_);

    item->parent_ = this;

    iDirItem* ptr = item.get();
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


  void Serialize(iSerializer* serializer) const override;


  size_t size() const {
    return items_.size();
  }

 private:
  std::unique_ptr<iDirItem> RemoveQuietly(size_t index) {
    assert(index < items_.size());

    auto target = std::move(items_[index]);
    assert(target);
    target->parent_ = nullptr;

    items_.erase(items_.begin() + static_cast<int64_t>(index));
    return target;
  }


  std::vector<std::unique_ptr<iDirItem>> items_;
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


  FileRef() = delete;
  FileRef(ActionList&& actions, iFile* file, Flags flags) :
      iDirItem(std::move(actions)),
      file_(file), flags_(flags), observer_(this, file) {
    assert(file_);
  }

  FileRef(const FileRef&) = delete;
  FileRef(FileRef&&) = delete;

  FileRef& operator=(const FileRef&) = delete;
  FileRef& operator=(FileRef&&) = delete;


  void Visit(iDirItemVisitor* visitor) override {
    visitor->VisitFile(file_);
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


  void Serialize(iSerializer*) const override;


  iFile& entity() const {
    return *file_;
  }

  bool readable() const {
    return flags_ & kReadable;
  }
  bool writable() const {
    return flags_ & kWritable;
  }

 private:
  class FileObserver : public iFileObserver {
   public:
    FileObserver(FileRef* owner, iFile* file) :
        iFileObserver(file), owner_(owner) {
      assert(owner);
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

}  // namespace mnian::core
