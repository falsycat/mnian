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

  // Deserializes a wstore.
  des->Enter(std::string("wstore"));
  if (!wstore_.Deserialize(des)) {
    des->logger().MNCORE_LOGGER_ERROR("broken wstore has been dropped");
    des->LogLocation();
    wstore_.Clear();
  }
  des->Leave();

  // Deserializes history.
  des->Enter(std::string("history"));
  if (!history_.Deserialize(des)) {
    des->logger().MNCORE_LOGGER_WARN("broken history has been dropped");
    des->LogLocation();
    history_.Clear();
  }
  des->Leave();

  return true;
}

void iApp::Project::Serialize(iSerializer* serial) const {
  iSerializer::MapGuard map(serial);
  map.Add("root",    root_.get());
  map.Add("wstore",  &wstore_);
  map.Add("history", &history_);
}

}  // namespace mnian::core
