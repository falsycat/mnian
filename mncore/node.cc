// No copyright
#include "mncore/node.h"

#include "mncore/app.h"


namespace mnian::core {

iNode* iNode::DeserializeRef(iDeserializer* des) {
  auto& store = des->app().stores().nodes();

  const auto id = des->value<ObjectId>();
  return id? store.Find(*id): nullptr;
}

}  // namespace mnian::core
