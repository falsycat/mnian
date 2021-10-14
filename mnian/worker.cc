// No copyright
#include "mnian/worker.h"

#include <string.h>

#include <Tracy.hpp>


namespace mnian {

static constexpr size_t kSleepTimeout = 50;

CpuWorker::CpuWorker(core::TaskQueue* q, size_t n) : q_(q), threads_(n) {
  size_t i = 0;
  for (auto& th : threads_) {
    th = std::thread([this, i]() { Main(i); });
    ++i;
  }
}
CpuWorker::~CpuWorker() {
  alive_ = false;
  q_->WakeUp();
  for (auto& th : threads_) {
    th.join();
  }
}

void CpuWorker::Main(size_t index) {
# ifdef TRACY_ENABLE
    // this can cause a tiny memory leak that can be ignored
    auto name = new char[32];
    snprintf(name, 32, "CPU worker %zu", index);  // NOLINT(runtime/printf)
# endif

  tracy::SetThreadName(name);
  while (alive_ || q_->size()) {
    FrameMarkStart(name);
    if (!q_->Dequeue()) {
      q_->Sleep(std::chrono::milliseconds(kSleepTimeout));
    }
    FrameMarkEnd(name);
  }
}

}  // namespace mnian
