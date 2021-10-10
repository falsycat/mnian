// No copyright
//
// This file declares classes about getting time.
#pragma once

#include <cassert>
#include <ctime>


namespace mnian::core {

// An interface of Clock that can get current time.
class iClock {
 public:
  iClock() = default;
  explicit iClock(time_t now) : now_(now) {
  }
  virtual ~iClock() = default;

  iClock(const iClock&) = default;
  iClock(iClock&&) = default;

  iClock& operator=(const iClock&) = default;
  iClock& operator=(iClock&&) = default;


  time_t now() const {
    return now_;
  }

 protected:
  void Tick(time_t now) {
    assert(now_ <= now);
    now_ = now;
  }

 private:
  time_t now_ = time_t{0};
};


// An implementation of Clock which user can set the current time.
class ManualClock final : public iClock {
 public:
  ManualClock() = default;
  explicit ManualClock(time_t now) : iClock(now) {
  }

  ManualClock(const ManualClock&) = default;
  ManualClock(ManualClock&&) = default;

  ManualClock& operator=(const ManualClock&) = default;
  ManualClock& operator=(ManualClock&&) = default;


  using iClock::Tick;
};


// An implementation of Clock which retrieves time of the real.
class RealClock final : public iClock {
 public:
  RealClock() = default;

  RealClock(const RealClock&) = default;
  RealClock(RealClock&&) = default;

  RealClock& operator=(const RealClock&) = default;
  RealClock& operator=(RealClock&&) = default;


  void Tick() {
    iClock::Tick(std::time(nullptr));
  }
};

}  // namespace mnian::core
