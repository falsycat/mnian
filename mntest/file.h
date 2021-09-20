// No copyright
#pragma once

#include "mncore/file.h"

#include <gmock/gmock.h>

#include <string>


namespace mnian::test {

class MockFileObserver : public core::iFileObserver {
 public:
  MockFileObserver() = delete;
  explicit MockFileObserver(core::iFile* file) : iFileObserver(file) {
  }

  MockFileObserver(const MockFileObserver&) = delete;
  MockFileObserver(MockFileObserver&&) = delete;

  MockFileObserver& operator=(const MockFileObserver&) = delete;
  MockFileObserver& operator=(MockFileObserver&&) = delete;


  MOCK_METHOD(void, ObserveUpdate, (), (override));
};

class MockFile : public core::iFile {
 public:
  MockFile() = delete;
  explicit MockFile(const std::string& url) : iFile(url) {
  }

  MockFile(const MockFile&) = delete;
  MockFile(MockFile&&) = delete;

  MockFile& operator=(const MockFile&) = delete;
  MockFile& operator=(MockFile&&) = delete;


  MOCK_METHOD(bool, Watch, (), (const override));

  MOCK_METHOD(size_t, Read, (uint8_t*, size_t, size_t), (override));
  MOCK_METHOD(size_t, Write, (const uint8_t*, size_t, size_t), (override));
};

class MockFileStore : public core::iFileStore {
 public:
  MockFileStore() = default;

  MockFileStore(const MockFileStore&) = delete;
  MockFileStore(MockFileStore&&) = delete;

  MockFileStore& operator=(const MockFileStore&) = delete;
  MockFileStore& operator=(MockFileStore&&) = delete;


  MOCK_METHOD(core::iFile*, Load, (const std::string&), (override));
};

}  // namespace mnian::test
