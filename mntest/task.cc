// No copyright
#include "mncore/task.h"
#include "mntest/task.h"

#include <gtest/gtest.h>

#include <atomic>  // NOLINT(build/c++11)
#include <memory>
#include <thread>  // NOLINT(build/c++11)
#include <tuple>
#include <vector>

#include <iostream>


namespace mnian::test {

using TaskWorkerTestParam = std::tuple<size_t, size_t, size_t>;

class TaskWorkerTest : public ::testing::TestWithParam<TaskWorkerTestParam> {
 protected:
  void SetUp() override {
    auto [threads, delay, count] = GetParam();

    for (size_t i = 0; i < threads; ++i) {
      threads_.emplace_back([this]() { return WorkerMain(); });
    }
    delay_ = delay;
    count_ = count;
  }

  void TearDown() override {
    alive_ = false;
    queue_.WakeUp();
    for (auto& th : threads_) {
      th.join();
    }
  }


  core::TaskQueue queue_;

  size_t count_ = size_t{0};

 private:
  void WorkerMain() {
    while (alive_ || queue_.size()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(delay_));
      if (!queue_.Dequeue()) {
        queue_.Sleep(std::chrono::milliseconds(10));
      }
    }
  }


  std::vector<std::thread> threads_;

  size_t delay_ = size_t{0};

  std::atomic<bool> alive_ = true;
};

TEST_P(TaskWorkerTest, ExecSingle) {
  for (size_t i = 0; i < count_; ++i) {
    auto task = std::make_shared<::testing::StrictMock<MockTask>>();
    EXPECT_CALL(*task, DoExec());

    queue_.Attach(task);
    task->Trigger();
  }
}

TEST_P(TaskWorkerTest, ExecSequence) {
  auto X = std::make_shared<size_t>();

  std::shared_ptr<core::iTask> prev;
  for (size_t i = 0; i < count_; ++i) {
    auto task = std::make_shared<::testing::StrictMock<MockTask>>();
    EXPECT_CALL(*task, DoExec()).
        WillOnce(([X, i]() {
                   ASSERT_EQ(*X, i);
                   ++*X;
                 }));

    if (prev) {
      prev->AddChild(task);
    }

    queue_.Attach(task);
    prev = task;

    task->Trigger();
  }
}

TEST_P(TaskWorkerTest, ExecLambdaSequence) {
  std::shared_ptr<core::iLambda> prev;
  for (size_t i = 0; i < count_; ++i) {
    auto lambda = std::make_shared<::testing::StrictMock<MockLambda>>(1, 1);

    auto ptr = lambda.get();
    EXPECT_CALL(*lambda, DoExec()).
        WillOnce(([ptr, i]() {
                   const auto in  = ptr->in<int64_t>(0);
                   const auto out = in + 1;
                   ASSERT_EQ(in, static_cast<int64_t>(i));
                   ptr->out(0, out);
                 }));

    if (prev) {
      prev->Connect(0, lambda, 0);
    } else {
      lambda->in(0, int64_t{0});
    }

    queue_.Attach(lambda);
    prev = lambda;

    lambda->Trigger();
  }
}

TEST_P(TaskWorkerTest, ExecLambdaTree) {
  for (size_t i = 0; i < count_; ++i) {
    auto L0 = std::make_shared<::testing::StrictMock<MockLambda>>(2, 0);
    auto L0ptr = L0.get();

    auto L0_0 = std::make_shared<::testing::StrictMock<MockLambda>>(0, 1);
    auto L0_0ptr = L0_0.get();

    auto L0_1 = std::make_shared<::testing::StrictMock<MockLambda>>(0, 1);
    auto L0_1ptr = L0_1.get();

    EXPECT_CALL(*L0, DoExec()).
        WillOnce(([L0ptr, i]() {
                   const auto in0 = L0ptr->in<int64_t>(0);
                   const auto in1 = L0ptr->in<int64_t>(1);
                   ASSERT_EQ(in0, static_cast<int64_t>(i));
                   ASSERT_EQ(in1, static_cast<int64_t>(i+1));
                 }));

    EXPECT_CALL(*L0_0, DoExec()).
        WillOnce(([L0_0ptr, i]() {
                   L0_0ptr->out(0, static_cast<int64_t>(i));
                 }));

    EXPECT_CALL(*L0_1, DoExec()).
        WillOnce(([L0_1ptr, i]() {
                   L0_1ptr->out(0, static_cast<int64_t>(i+1));
                 }));

    L0_0->Connect(0, L0, 0);
    L0_1->Connect(0, L0, 1);

    queue_.Attach(L0);
    queue_.Attach(L0_0);
    queue_.Attach(L0_1);

    L0->Trigger();
    L0_0->Trigger();
    L0_1->Trigger();
  }
}

INSTANTIATE_TEST_SUITE_P(
    TaskQueue,
    TaskWorkerTest,
    ::testing::Values(  /* thread|       delay|       count| */
        std::make_tuple(size_t{1}, size_t{  0}, size_t{  10}),
        std::make_tuple(size_t{1}, size_t{  0}, size_t{ 100}),
        std::make_tuple(size_t{1}, size_t{  0}, size_t{1000}),
        std::make_tuple(size_t{1}, size_t{ 10}, size_t{  10}),
        std::make_tuple(size_t{1}, size_t{ 10}, size_t{ 100}),
        std::make_tuple(size_t{1}, size_t{100}, size_t{  10}),
        std::make_tuple(size_t{4}, size_t{  0}, size_t{  10}),
        std::make_tuple(size_t{4}, size_t{  0}, size_t{ 100}),
        std::make_tuple(size_t{4}, size_t{  0}, size_t{1000}),
        std::make_tuple(size_t{4}, size_t{ 10}, size_t{  10}),
        std::make_tuple(size_t{4}, size_t{ 10}, size_t{ 100}),
        std::make_tuple(size_t{4}, size_t{100}, size_t{  10}),
        std::make_tuple(size_t{8}, size_t{  0}, size_t{  10}),
        std::make_tuple(size_t{8}, size_t{  0}, size_t{ 100}),
        std::make_tuple(size_t{8}, size_t{  0}, size_t{1000}),
        std::make_tuple(size_t{8}, size_t{ 10}, size_t{  10}),
        std::make_tuple(size_t{8}, size_t{ 10}, size_t{ 100}),
        std::make_tuple(size_t{8}, size_t{100}, size_t{  10})));

}  // namespace mnian::test
