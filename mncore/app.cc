// No copyright
#include "mncore/app.h"

#include <cassert>
#include <utility>


namespace mnian::core {

bool Project::Deserialize(iDeserializer* des) {
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
  auto root_dir = std::unique_ptr<Dir>(root_dir_ptr);

  // Deserializes an editor.
  des->Enter(std::string("editor"));
  auto editor = des->DeserializeObject<iEditor>();
  des->Leave();

  if (!editor) {
    des->logger().MNCORE_LOGGER_ERROR("editor is broken");
    des->LogLocation();
    return false;
  }

  // Deserializes history.
  des->Enter(std::string("history"));
  if (!history_.Deserialize(des)) {
    des->logger().MNCORE_LOGGER_WARN("broken history has been dropped");
    des->LogLocation();
    history_.Clear();
  }
  des->Leave();

  // Saves deserialized instances.
  root_   = std::move(root_dir);
  editor_ = std::move(editor);

  return true;
}

void Project::Serialize(iSerializer* serial) const {
  iSerializer::MapGuard map(serial);
  map.Add("root",    root_.get());
  map.Add("editor",  editor_.get());
  map.Add("history", &history_);
}

}  // namespace mnian::core
