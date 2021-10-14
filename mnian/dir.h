// No copyright
#pragma once

#include "mncore/dir.h"


namespace mnian {

class GenericDir : public core::Dir {
 public:
  static constexpr const char* kType = "GenericDir";


  GenericDir() : Dir({}, kType) {
  }

  GenericDir(const GenericDir&) = delete;
  GenericDir(GenericDir&&) = delete;

  GenericDir& operator=(const GenericDir&) = delete;
  GenericDir& operator=(GenericDir&&) = delete;

 private:
  // TODO(falsycat): add actions
};

}  // namespace mnian
