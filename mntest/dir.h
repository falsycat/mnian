// No copyright
#pragma once

#include "mncore/dir.h"

#include <gmock/gmock.h>


namespace mnian::test {

class MockDirItemVisitor : public core::iDirItemVisitor {
 public:
  MockDirItemVisitor() = default;

  MockDirItemVisitor(const MockDirItemVisitor&) = delete;
  MockDirItemVisitor(MockDirItemVisitor&&) = delete;

  MockDirItemVisitor& operator=(const MockDirItemVisitor&) = delete;
  MockDirItemVisitor& operator=(MockDirItemVisitor&&) = delete;


  MOCK_METHOD(void, VisitDir, (core::Dir*), (override));
  MOCK_METHOD(void, VisitFile, (core::iFile*), (override));
};

class MockDirItemObserver : public core::iDirItemObserver {
 public:
  MockDirItemObserver() = delete;
  explicit MockDirItemObserver(core::iDirItem* target) :
      iDirItemObserver(target) {
  }

  MockDirItemObserver(const MockDirItemObserver&) = delete;
  MockDirItemObserver(MockDirItemObserver&&) = delete;

  MockDirItemObserver& operator=(const MockDirItemObserver&) = delete;
  MockDirItemObserver& operator=(MockDirItemObserver&&) = delete;


  MOCK_METHOD(void, ObserveUpdate, (), (override));
  MOCK_METHOD(void, ObserveMove, (), (override));
  MOCK_METHOD(void, ObserveRemove, (), (override));
};

class MockDirItem : public core::iDirItem {
 public:
  MockDirItem() : iDirItem(ActionList {}) {
  }

  MockDirItem(const MockDirItem&) = delete;
  MockDirItem(MockDirItem&&) = delete;

  MockDirItem& operator=(const MockDirItem&) = delete;
  MockDirItem& operator=(MockDirItem&&) = delete;


  MOCK_METHOD(void, Visit, (core::iDirItemVisitor*), (override));
  MOCK_METHOD(void, Serialize, (core::iSerializer*), (const, override));
};

}  // namespace mnian::test