// No copyright
#pragma once

#include <memory>
#include <string>

#include "mncore/file.h"


namespace mnian {

class FileStore final : public core::iFileStore {
 public:
  FileStore() = default;

  FileStore(const FileStore&) = delete;
  FileStore(FileStore&&) = delete;

  FileStore& operator=(const FileStore&) = delete;
  FileStore& operator=(FileStore&&) = delete;

 protected:
  std::shared_ptr<core::iFile> Create(const std::string&) override;
};

}  // namespace mnian
