// No copyright
//
// Simple task queue implementation.
#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>  // NOLINT(build/c++11)
#include <condition_variable>  // NOLINT(build/c++11)
#include <functional>
#include <memory>
#include <mutex>  // NOLINT(build/c++11)
#include <string>
#include <utility>
#include <variant>
#include <vector>


namespace mnian::core {

class iTask {
 public:
  friend class TaskQueue;


  enum State {
    kInitial,
    kTriggered,
    kDone,
  };


  iTask() = default;
  virtual ~iTask() = default;

  iTask(const iTask&) = delete;
  iTask(iTask&&) = delete;

  iTask& operator=(const iTask&) = delete;
  iTask& operator=(iTask&&) = delete;


  // Triggers this task. After this, the task is ready when all dependencies
  // are resolved.
  void Trigger() {
    std::lock_guard<std::mutex> _(mtx_);
    if (state_ != kInitial) return;

    --deps_;
    state_ = kTriggered;
  }

  // Makes `child` to depend `this`. `this` can be in state kDone.
  void AddChild(std::shared_ptr<iTask> child) {
    std::lock_guard<std::mutex> _(mtx_);
    if (state_ == kDone) return;

    ++child->deps_;
    children_.push_back(std::move(child));
  }


  bool ready() {
    std::lock_guard<std::mutex> _(mtx_);
    return deps_ == 0;
  }

 protected:
  virtual void DoExec() = 0;

 private:
  void Resolve() {
    std::lock_guard<std::mutex> _(mtx_);
    assert(deps_);

    --deps_;
  }

  void Exec() {
    {
      std::lock_guard<std::mutex> _(mtx_);
      if (state_ != kTriggered) return;
    }
    DoExec();
    {
      std::lock_guard<std::mutex> _(mtx_);
      state_ = kDone;
      for (auto& child : children_) {
        child->Resolve();
      }
      children_.clear();
    }
  }


  std::mutex mtx_;

  std::vector<std::shared_ptr<iTask>> children_;

  State state_ = kInitial;

  size_t deps_ = size_t{1};
};


class Task : public iTask {
 public:
  using F = std::function<void(void)>;


  Task() = delete;
  explicit Task(F&& func) : func_(std::move(func)) {
  }

  Task(const Task&) = delete;
  Task(Task&&) = delete;

  Task& operator=(const Task&) = delete;
  Task& operator=(Task&&) = delete;

 protected:
  void DoExec() override {
    func_();
  }

 private:
  F func_;
};


class iLambda : public iTask {
 public:
  using Value = std::variant<
      int64_t,
      double,
      bool,
      std::shared_ptr<std::string>>;


  iLambda() = delete;
  iLambda(size_t in, size_t out) : in_(in), out_(out) {
  }

  iLambda(const iLambda&) = delete;
  iLambda(iLambda&&) = delete;

  iLambda& operator=(const iLambda&) = delete;
  iLambda& operator=(iLambda&&) = delete;


  void Connect(size_t out_i, std::shared_ptr<iLambda> in, size_t in_i) {
    assert(in);

    std::lock_guard<std::mutex> _(mtx_);
    out_[out_i].Connect(&in->in_[in_i]);
    AddChild(std::move(in));
  }


  template <typename T>
  void in(size_t i, T&& value) {
    std::lock_guard<std::mutex> _(mtx_);
    return in_[i].Set(std::move(value));
  }

 protected:
  template <typename T>
  const T& in(size_t i) {
    std::lock_guard<std::mutex> _(mtx_);
    return std::get<T>(in_[i].value());
  }

  template <typename T>
  void out(size_t i, T&& value) {
    std::lock_guard<std::mutex> _(mtx_);
    out_[i].Set(std::move(value));
  }

 private:
  class In final {
   public:
    In() = default;

    In(const In&) = default;
    In(In&&) = default;

    In& operator=(const In&) = default;
    In& operator=(In&&) = default;


    void Set(Value&& value) {
      value_ = std::move(value);
    }

    const Value& value() const {
      return value_;
    }

   private:
    Value value_;
  };

  class Out final {
   public:
    Out() = default;

    Out(const Out&) = delete;
    Out(Out&&) = default;

    Out& operator=(const Out&) = delete;
    Out& operator=(Out&&) = default;


    void Connect(In* in) {
      assert(in);

      in->Set(Value(value_));
      in_.push_back(in);
    }

    void Set(Value&& value) {
      value_ = std::move(value);
      for (auto& in : in_) {
        in->Set(Value(value_));
      }
    }

   private:
    std::vector<In*> in_;

    Value value_;
  };


  std::mutex mtx_;

  std::vector<In>  in_;
  std::vector<Out> out_;
};


class TaskQueue final {
 public:
  TaskQueue() = default;

  TaskQueue(const TaskQueue&) = delete;
  TaskQueue(TaskQueue&&) = delete;

  TaskQueue& operator=(const TaskQueue&) = delete;
  TaskQueue& operator=(TaskQueue&&) = delete;


  // Each of attached tasks will be executed by Dequeue() after it's ready.
  void Attach(std::shared_ptr<iTask> task) {
    assert(task);

    std::lock_guard<std::mutex> _(mtx_);
    tasks_.push_back(std::move(task));

    cv_.notify_all();
  }

  // Creates and Attaches new task that executes the passed function.
  void Exec(Task::F&& func) {
    auto task = std::make_shared<Task>(std::move(func));
    Attach(task);
    task->Trigger();
  }

  // Dequeues and Executes one of tasks whose ready() is true. Returns
  // true if such task is found, otherwise false.
  bool Dequeue() {
    std::shared_ptr<iTask> task;
    {
      std::lock_guard<std::mutex> _(mtx_);

      auto itr = std::find_if(tasks_.begin(), tasks_.end(),
                              [](auto& x) { return x->ready(); });
      if (itr == tasks_.end()) return false;

      task = std::move(*itr);
      tasks_.erase(itr);
    }
    task->Exec();

    std::lock_guard<std::mutex> _(mtx_);
    cv_.notify_all();
    return true;
  }

  // Wakes up threads sleeping by Sleep() forcibly.
  void WakeUp() {
    std::lock_guard<std::mutex> _(mtx_);
    cv_.notify_all();
  }

  // Blocks the current thread until WakeUp() called, timeout elapsed, or
  // the queue changed.
  template <typename Rep, typename Period>
  void Sleep(const std::chrono::duration<Rep, Period>& timeout) {
    std::unique_lock<std::mutex> k(mtx_);
    cv_.wait_for(k, timeout);
  }


  size_t size() {
    std::lock_guard<std::mutex> _(mtx_);
    return tasks_.size();
  }

 private:
  std::mutex mtx_;

  std::condition_variable cv_;

  std::vector<std::shared_ptr<iTask>> tasks_;
};

}  // namespace mnian::core
