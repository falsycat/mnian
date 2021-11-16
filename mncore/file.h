// No copyright
//
// This file declares interfaces to wrap native files.
#pragma once

#include <algorithm>
#include <cassert>
#include <filesystem>
#include <memory>
#include <mutex>  // NOLINT(build/c++11)
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>


namespace mnian::core {

class iFile;


// This observer can detect file updates by ObserveUpdate() method. Registration
// and unregistration are done by constructor and destructor.
// Please note that File is available as long as objects using it such as this
// observer are alive.
class iFileObserver {
 public:
  iFileObserver() = delete;
  explicit iFileObserver(iFile* target);
  virtual ~iFileObserver();

  iFileObserver(const iFileObserver&) = delete;
  iFileObserver(iFileObserver&&) = delete;

  iFileObserver& operator=(const iFileObserver&) = delete;
  iFileObserver& operator=(iFileObserver&&) = delete;


  virtual void ObserveUpdate() {
  }


  iFile& target() const {
    return *target_;
  }

 private:
  iFile* target_;
};


// This is an interface of File, which is a binary or text data usually-placed
// on permanentized drive.
// It's guaranteed that all of File won't be died until all object using File
// is destroyed.
class iFile {
 public:
  friend class iFileObserver;


  // LockGuard represents a right to read/write the file. the file is locked by
  // returning from constructor and unlocked by entering into destructor.
  struct LockGuard {
   public:
    LockGuard() = delete;
    explicit LockGuard(iFile* file) : file_(file) {
      if (file_) file_->mutex_.lock();
    }
    ~LockGuard() {
      if (file_) file_->mutex_.unlock();
    }

    LockGuard(const LockGuard&) = delete;
    LockGuard(LockGuard&& src) : file_(src.file_) {
      src.file_ = nullptr;
    }

    LockGuard& operator=(const LockGuard&) = delete;
    LockGuard& operator=(LockGuard&& src) {
      if (this != &src) {
        if (file_) file_->mutex_.unlock();
        file_ = src.file_;
        src.file_ = nullptr;
      }
      return *this;
    }


    // Both Read and Write is available under any thread as long as LockGuard is
    // still alive. However don't call them concurrently.
    size_t Read(uint8_t* buf, size_t n, size_t offset = 0) {
      assert(file_);
      return file_->Read(buf, n, offset);
    }
    size_t Write(const uint8_t* buf, size_t n, size_t offset = 0) {
      assert(file_);
      return file_->Write(buf, n, offset);
    }
    bool Truncate(size_t size) {
      assert(file_);
      return file_->Truncate(size);
    }
    bool Flush() {
      assert(file_);
      return file_->Flush();
    }
    void Watch() {
      assert(file_);
      file_->Watch();
    }

   private:
    iFile* file_;
  };


  // Creates an instance of File that can read/write native files.
  static std::unique_ptr<iFile> CreateForNative(const std::filesystem::path&);


  iFile() = delete;
  explicit iFile(const std::string& url) : url_(url) {
  }
  virtual ~iFile() {
    assert(observers_.size() == 0);
  }

  iFile(const iFile&) = delete;
  iFile(iFile&&) = delete;

  iFile& operator=(const iFile&) = delete;
  iFile& operator=(iFile&&) = delete;


  LockGuard Lock() {
    return LockGuard(this);
  }


  const std::string& url() const {
    return url_;
  }
  const std::filesystem::file_time_type& lastModified() const {
    return last_modified_;
  }

 protected:
  // Triggers all observers' ObserveUpdate() method.
  void NotifyUpdate() const {
    for (auto observer : observers_) {
      observer->ObserveUpdate();
    }
  }

  void Watch() {
    const auto prev = last_modified_;
    last_modified_ = GetLastModified();
    if (prev < last_modified_) NotifyUpdate();
  }


  virtual size_t Read(uint8_t* buf, size_t size, size_t offset = 0) = 0;
  virtual size_t Write(const uint8_t* buf, size_t size, size_t offset = 0) = 0;
  virtual bool Truncate(size_t size) = 0;
  virtual bool Flush() = 0;

  virtual std::filesystem::file_time_type GetLastModified() const = 0;

 private:
  const std::string url_;

  std::vector<iFileObserver*> observers_;

  std::filesystem::file_time_type last_modified_;

  std::mutex mutex_;
};


// FileStore can create an instance of File, and also owns all created
// instances.
class iFileStore {
 public:
  iFileStore() = default;
  virtual ~iFileStore() = default;

  iFileStore(const iFileStore&) = delete;
  iFileStore(iFileStore&&) = delete;

  iFileStore& operator=(const iFileStore&) = delete;
  iFileStore& operator=(iFileStore&&) = delete;


  iFile* Load(const std::string& url) {
    auto itr = items_.find(url);
    if (itr != items_.end()) return itr->second.get();

    auto file = Create(url);
    assert(file);

    auto ptr = file.get();
    items_[url] = std::move(file);
    return ptr;
  }

 protected:
  virtual std::unique_ptr<iFile> Create(const std::string& url) = 0;

 private:
  std::unordered_map<std::string, std::unique_ptr<iFile>> items_;
};


}  // namespace mnian::core
