// No copyright
#pragma once

#include "mncore/serialize.h"

#include <gmock/gmock.h>

#include <memory>
#include <string>



namespace mnian::test {

class MockSerializable : public core::iSerializable {
 public:
  MockSerializable() = default;

  MockSerializable(const MockSerializable&) = delete;
  MockSerializable(MockSerializable&&) = delete;

  MockSerializable& operator=(const MockSerializable&) = delete;
  MockSerializable& operator=(MockSerializable&&) = delete;


  MOCK_METHOD(void, Serialize, (core::iSerializer*), (const, override));
};

class MockPolymorphicSerializable : public core::iPolymorphicSerializable {
 public:
  static constexpr const char* kType = "MockPolymorphic";


  static std::unique_ptr<MockPolymorphicSerializable>
      DeserializeParam(core::iDeserializer*) {
    return std::make_unique<MockPolymorphicSerializable>();
  }


  MockPolymorphicSerializable() : iPolymorphicSerializable(kType) {
  }

  MockPolymorphicSerializable(const MockPolymorphicSerializable&) = delete;
  MockPolymorphicSerializable(MockPolymorphicSerializable&&) = delete;

  MockPolymorphicSerializable& operator=(
      const MockPolymorphicSerializable&) = delete;
  MockPolymorphicSerializable& operator=(
      MockPolymorphicSerializable&&) = delete;


  MOCK_METHOD(void, SerializeParam, (core::iSerializer*), (const, override));
};

class MockSerializer : public core::iSerializer {
 public:
  MockSerializer() = default;

  MockSerializer(const MockSerializer&) = delete;
  MockSerializer(MockSerializer&&) = delete;

  MockSerializer& operator=(const MockSerializer&) = delete;
  MockSerializer& operator=(MockSerializer&&) = delete;


  MOCK_METHOD(void, SerializeMap, (size_t), (override));
  MOCK_METHOD(void, SerializeArray, (size_t), (override));
  MOCK_METHOD(void, SerializeKey, (const std::string&), (override));
  MOCK_METHOD(void, SerializeValue, (const core::Any&), (override));
};

class MockDeserializer : public core::iDeserializer {
 public:
  MockDeserializer() = delete;
  explicit MockDeserializer(core::iLogger* logger, const core::Registry* reg) :
      iDeserializer(logger, reg) {
  }

  MockDeserializer(const MockDeserializer&) = delete;
  MockDeserializer(MockDeserializer&&) = delete;

  MockDeserializer& operator=(const MockDeserializer&) = delete;
  MockDeserializer& operator=(MockDeserializer&&) = delete;


  MOCK_METHOD(Key, DoEnter, (const Key&), (override));
  MOCK_METHOD(void, DoLeave, (), (override));


  using iDeserializer::SetUndefined;
  using iDeserializer::SetField;
  using iDeserializer::SetMapOrArray;
};

}  // namespace mnian::test
