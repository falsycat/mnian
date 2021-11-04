// No copyright
#include "mncore/app.h"

#include <cassert>
#include <utility>


namespace mnian::core {

bool iApp::Project::Deserialize(iDeserializer* des) {
  // Deserializes a root.
  des->Enter(std::string("root"));
  auto root = des->DeserializeObject<iDirItem>();
  des->Leave();

  if (!root) {
    des->logger().MNCORE_LOGGER_ERROR("root is broken");
    des->LogLocation();
    return false;
  }

  auto root_dir_ptr = dynamic_cast<Dir*>(root.get());
  if (!root_dir_ptr) {
    des->logger().MNCORE_LOGGER_ERROR("root is not Dir");
    des->LogLocation();
    return false;
  }

  root.release();
  root_ = std::unique_ptr<Dir>(root_dir_ptr);

  // Deserializes nstore.
  des->Enter(std::string("nstore"));
  const bool nstore = nstore_.Deserialize(des);
  des->Leave();

  if (!nstore) {
    des->logger().MNCORE_LOGGER_ERROR("nstore is broken");
    des->LogLocation();
    return false;
  }

  // Deserializes wstore.
  des->Enter(std::string("wstore"));
  const bool wstore = wstore_.Deserialize(des);
  des->Leave();

  if (!wstore) {
    des->logger().MNCORE_LOGGER_WARN("wstore is broken");
    des->LogLocation();

    // Returns without loading history because History might refer missing
    // widgets.
    return true;
  }

  // Deserializes history.
  des->Enter(std::string("history"));
  const bool history = history_.Deserialize(des);

  des->Leave();
  if (!history) {
    des->logger().MNCORE_LOGGER_WARN("history is broken");
    des->LogLocation();
  }
  return true;
}

void iApp::Project::Serialize(iSerializer* serial) const {
  iSerializer::MapGuard map(serial);
  map.Add("root",    root_.get());
  map.Add("nstore",  &nstore_);
  map.Add("wstore",  &wstore_);
  map.Add("history", &history_);
}

}  // namespace mnian::core
