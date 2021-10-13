// No copyright
#include "mnian/file.h"


namespace mnian {

std::unique_ptr<core::iFile> FileStore::Create(const std::string&) {
  return nullptr;
}

}  // namespace mnian
