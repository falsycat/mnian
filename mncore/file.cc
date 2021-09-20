// No copyright
#include "mncore/file.h"


namespace mnian::core {

iFileObserver::iFileObserver(iFile* target) : target_(target) {
  assert(target);
  target_->observers_.push_back(this);
}
iFileObserver::~iFileObserver() {
    auto& obs = target_->observers_;
    obs.erase(std::remove(obs.begin(), obs.end(), this), obs.end());
}

}  // namespace mnian::core
