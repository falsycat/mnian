// No copyright
#pragma once

#include "mncore/node.h"

#include <gmock/gmock.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>


namespace mnian::test {

class MockNodeObserver : public core::iNodeObserver {
 public:
  MockNodeObserver() = delete;
  explicit MockNodeObserver(core::iNode* target) : iNodeObserver(target) {
  }

  MockNodeObserver(const MockNodeObserver&) = delete;
  MockNodeObserver(MockNodeObserver&&) = delete;

  MockNodeObserver& operator=(const MockNodeObserver&) = delete;
  MockNodeObserver& operator=(MockNodeObserver&&) = delete;


  MOCK_METHOD(void, ObserveRecover,      (), (override));
  MOCK_METHOD(void, ObserveRemove,       (), (override));
  MOCK_METHOD(void, ObserveDelete,       (), (override));
  MOCK_METHOD(void, ObserveUpdate,       (), (override));
};

class MockNode : public core::iNode {
 public:
  static constexpr const char* kType = "MockNode";

  explicit MockNode(Tag&&                 tag,
                    std::vector<Socket>&& in  = {},
                    std::vector<Socket>&& out = {}) :
      iNode(ActionList {},
            kType,
            std::move(tag),
            std::move(in),
            std::move(out)) {
  }

  MockNode(const MockNode&) = delete;
  MockNode(MockNode&&) = delete;

  MockNode& operator=(const MockNode&) = delete;
  MockNode& operator=(MockNode&&) = delete;


  MOCK_METHOD(std::unique_ptr<core::iNode>, Clone, (), (override));
  MOCK_METHOD(std::shared_ptr<core::iLambda>, QueueLambda, (), (override));

  MOCK_METHOD(void, SerializeParam, (core::iSerializer*), (const override));


  using iNode::NotifyUpdate;
};

class MockNodeFactory : public core::iNodeFactory {
 public:
  explicit MockNodeFactory(const std::string& name = "") :
      iNodeFactory(ActionList {}, name) {
  }

  MockNodeFactory(const MockNodeFactory&) = delete;
  MockNodeFactory(MockNodeFactory&&) = delete;

  MockNodeFactory& operator=(const MockNodeFactory&) = delete;
  MockNodeFactory& operator=(MockNodeFactory&&) = delete;


  MOCK_METHOD(std::unique_ptr<core::iNode>, Create, (), (override));
};

}  // namespace mnian::test
