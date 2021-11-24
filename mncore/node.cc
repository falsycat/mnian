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


std::unordered_map<const iNode::Socket*, size_t>
iNode::CreateSocketIndexMap() const {
  std::unordered_map<const iNode::Socket*, size_t> ret;
  for (size_t i = 0; i < input_.size(); ++i) {
    ret[input_[i].get()] = i;
  }
  for (size_t i = 0; i < output_.size(); ++i) {
    ret[output_[i].get()] = i;
  }
  return ret;
}

}  // namespace mnian::core
