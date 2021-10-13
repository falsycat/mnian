// No copyright
#include "mnian/editor.h"

#include <string>

#include <Tracy.hpp>


namespace mnian {

std::unique_ptr<Editor> Editor::DeserializeParam(core::iDeserializer* des) {
  auto ret = std::make_unique<Editor>();

  {
    core::iDeserializer::ScopeGuard _(des, std::string("widgets"));

    const auto n_ = des->size();
    if (!n_) {
      des->logger().MNCORE_LOGGER_WARN(
          "expected array but found not, all widgets have been dropped");
      des->LogLocation();
    }

    const size_t n = n_? *n_: 0;
    for (size_t i = 0; i < n; ++i) {
      core::iDeserializer::ScopeGuard __(des, i);

      auto w = des->DeserializeObject<core::iWidget>();
      if (w) {
        ret->widgets_.push_back(std::move(w));
      } else {
        des->logger().MNCORE_LOGGER_WARN(
            "deserialization failed, the widget has been dropped");
        des->LogLocation();
      }
    }
  }
  return ret;
}

void Editor::SerializeParam(core::iSerializer* serial) const {
  core::iSerializer::ArrayGuard widgets(serial);
  for (const auto& w : widgets_) {
    widgets.Add(w.get());
  }

  core::iSerializer::MapGuard root(serial);
  root.Add("widgets", &widgets);
}

}  // namespace mnian
