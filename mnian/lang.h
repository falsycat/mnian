// No copyright
#pragma once

#include <mulan.h>

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

#include "mncore/logger.h"


namespace mnian {

class Lang final {
 public:
  Lang() = default;

  Lang(const Lang&) = default;
  Lang(Lang&&) = default;

  Lang& operator=(const Lang&) = default;
  Lang& operator=(Lang&&) = default;


  void Merge(std::istream* in, core::iLogger* logger);


  const char* GetText(uint64_t id, const char* def) const {
    auto itr = cat_.find(id);
    if (itr == cat_.end()) return def;
    return itr->second.get();
  }
  const char* GetText(const char* id) const {
    return GetText(mulan::hash(id), id);
  }

 private:
  std::unordered_map<uint64_t, std::unique_ptr<char[]>> cat_;

  // To guarantee that the string is alive until program exits.
  std::vector<std::unique_ptr<char[]>> trash_;
};

}  // namespace mnian
