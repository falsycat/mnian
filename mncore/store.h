// No copyright
//
// This file defines objects to manage a unique id to assign or be assigned.
#pragma once

#include <cassert>
#include <cinttypes>
#include <cstdint>
#include <unordered_map>


namespace mnian::core {

using ObjectId = uint64_t;

#define MNCORE_PRIobjid PRIu64


// An object that stores a map of an object pointer and its id.
template <typename T>
class ObjectStore final {
 public:
  // This struct manages lifetime of an item in the store.
  struct Tag final {
   public:
    Tag() = delete;
    Tag(ObjectStore* store, ObjectId id) : store_(store), id_(id) {
    }
    // allows casting for usability
    Tag(ObjectStore* store) : Tag(store, store->AllocateId()) {
    }
    ~Tag() {
      if (store_) store_->Remove(id_, ptr_);
    }

    Tag(const Tag& src) : store_(src.store_), id_(store_->AllocateId()) {
    }
    Tag(Tag&& src) : store_(src.store_), id_(src.id_) {
      src.store_ = nullptr;
    }

    Tag& operator=(const Tag&) = delete;
    Tag& operator=(Tag&&) = delete;


    void Attach(T* ptr) {
      assert(store_);
      ptr_ = ptr;
      store_->Add(id_, ptr);
    }


    ObjectStore<T>& store() const {
      return *store_;
    }
    ObjectId id() const {
      return id_;
    }

   private:
    ObjectStore<T>* store_;

    ObjectId id_;

    T* ptr_ = nullptr;
  };


  ObjectStore() = default;
  ~ObjectStore() {
    assert(map_.empty());
  }

  ObjectStore(const ObjectStore&) = delete;
  ObjectStore(ObjectStore&&) = default;

  ObjectStore& operator=(const ObjectStore&) = delete;
  ObjectStore& operator=(ObjectStore&&) = default;


  ObjectId AllocateId() {
    return next_++;
  }

  void Add(ObjectId id, T* ptr) {
    assert(!map_.contains(id));
    map_[id] = ptr;
    if (next_ <= id) next_ = id+1;
  }
  void Remove(ObjectId id, T* ptr = nullptr) {
    auto itr = map_.find(id);
    if (itr == map_.end() || (ptr && itr->second != ptr)) {
      return;
    }
    map_.erase(itr);
  }
  void Clear() {
    next_ = 0;
    map_.clear();
  }

  T* Find(ObjectId id) const {
    auto itr = map_.find(id);
    if (itr == map_.end()) return nullptr;
    return itr->second;
  }

 private:
  ObjectId next_ = 0;

  std::unordered_map<ObjectId, T*> map_;
};

}  // namespace mnian::core
