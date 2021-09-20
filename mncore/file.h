// No copyright
//
// This file declares interfaces to wrap native files.
#pragma once

#include <algorithm>
#include <cassert>
#include <mutex>  // NOLINT(build/c++11)
#include <string>
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
    size_t Read(uint8_t* buf, size_t n, size_t offset) {
      assert(file_);
      return file_->Read(buf, n, offset);
    }
    size_t Write(const uint8_t* buf, size_t n, size_t offset) {
      assert(file_);
      return file_->Write(buf, n, offset);
    }

   private:
    iFile* file_;
  };


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


  // Returns whether the file is updated, and calls Touch() if it is.
  virtual bool Watch() const = 0;


  // Triggers all observers' ObserveUpdate() method.
  void Touch() const {
    for (auto observer : observers_) {
      observer->ObserveUpdate();
    }
  }

  LockGuard Lock() {
    return LockGuard(this);
  }


  const std::string& url() const {
    return url_;
  }

 protected:
  // It's guaranteed by caller that Read/Write methods are called under locked
  // and one is not called when other is being called.
  virtual size_t Read(uint8_t* buf, size_t size, size_t offset = 0) = 0;
  virtual size_t Write(const uint8_t* buf, size_t size, size_t offset = 0) = 0;

 private:
  const std::string url_;

  std::vector<iFileObserver*> observers_;

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


  virtual iFile* Load(const std::string& url) = 0;
};

}  // namespace mnian::core
