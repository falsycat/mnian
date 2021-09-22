// No copyright
#pragma once

#include "mncore/serialize.h"

#include <gmock/gmock.h>

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
  MOCK_METHOD(void, SerializeValue, (const Value&), (override));
};

class MockDeserializer : public core::iDeserializer {
 public:
  MockDeserializer() = delete;
  explicit MockDeserializer(const core::DeserializerRegistry& reg) :
      iDeserializer(reg) {
  }

  MockDeserializer(const MockDeserializer&) = delete;
  MockDeserializer(MockDeserializer&&) = delete;

  MockDeserializer& operator=(const MockDeserializer&) = delete;
  MockDeserializer& operator=(MockDeserializer&&) = delete;


  MOCK_METHOD(bool, EnterByKey, (const std::string&), (override));
  MOCK_METHOD(bool, EnterByIndex, (size_t), (override));
  MOCK_METHOD(void, Leave, (), (override));

  MOCK_METHOD(std::optional<Value>, value, (), (const override));
  MOCK_METHOD(std::optional<size_t>, size, (), (const override));
};

}  // namespace mnian::test
