// No copyright
#include "mnian/lang.h"

#include <utility>


namespace mnian {

void Lang::Merge(std::istream* in, core::iLogger* logger) {
  for (;;) {
    uint8_t hdr[MULAN_HDR];
    if (!in->read(reinterpret_cast<char*>(hdr), sizeof(hdr))) break;

    uint64_t id;
    uint16_t strn;
    mulan::unpack_header(hdr, &id, &strn);

    auto str = std::make_unique<char[]>(static_cast<size_t>(strn+1));
    if (!in->read(str.get(), strn)) {
      logger->MNCORE_LOGGER_WARN(
          "language catalog ended unexpectedly, "
          "incomplete item is dropped");
      return;
    }
    str[strn] = 0;

    auto itr = cat_.find(id);
    if (itr == cat_.end()) {
      cat_[id] = std::move(str);
    } else {
      trash_.push_back(std::move(itr->second));
      itr->second = std::move(str);
    }
  }
}

}  // namespace mnian
