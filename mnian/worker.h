// No copyright
#pragma once

#include <atomic>  // NOLINT(build/c++11)
#include <thread>  // NOLINT(build/c++11)
#include <vector>

#include "mncore/task.h"


namespace mnian {

class CpuWorker {
 public:
  CpuWorker() = delete;
  CpuWorker(core::TaskQueue* q, size_t n);
  ~CpuWorker();

  CpuWorker(const CpuWorker&) = delete;
  CpuWorker(CpuWorker&&) = delete;

  CpuWorker& operator=(const CpuWorker&) = delete;
  CpuWorker& operator=(CpuWorker&&) = delete;

 private:
  void Main(size_t);


  std::atomic<bool> alive_ = true;

  core::TaskQueue* q_;

  std::vector<std::thread> threads_;
};

}  // namespace mnian
