// No copyright
#include "mncore/lang.h"


namespace mnian::core {

void Lang::Deserialize(iDeserializer* des) {
  Clear();

  const auto size = des->size();
  if (!size) {
    des->logger().MNCORE_LOGGER_WARN("expected a map");
    des->LogLocation();
    return;
  }

  for (size_t i = 0; i < *size; ++i) {
    iDeserializer::ScopeGuard _(des, i);

    const auto key = des->key();
    if (!key) {
      continue;
    }

    const auto value = des->value<std::string>();
    if (!value) {
      des->logger().MNCORE_LOGGER_WARN("expected string for translated text");
      des->LogLocation();
      continue;
    }

    if (!Add(*key, *value)) {
      des->logger().MNCORE_LOGGER_WARN("duplicated id or hash collision found");
      des->LogLocation();
      continue;
    }
  }
}

}  // namespace mnian::core
