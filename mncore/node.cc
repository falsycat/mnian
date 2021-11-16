// No copyright
#include "mncore/node.h"

#include "mncore/app.h"


namespace mnian::core {

iNodeObserver::iNodeObserver(iNode* target) : target_(target) {
  assert(target_);
  target_->observers_.push_back(this);
}

iNodeObserver::~iNodeObserver() {
  if (!target_) return;
  auto& obs = target_->observers_;
  obs.erase(std::remove(obs.begin(), obs.end(), this), obs.end());
}


iNode* iNode::DeserializeRef(iDeserializer* des) {
  auto& store = des->app().stores().nodes();

  const auto id = des->value<ObjectId>();
  return id? store.Find(*id): nullptr;
}

}  // namespace mnian::core
