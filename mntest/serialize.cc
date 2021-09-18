// No copyright
#include "mncore/serialize.h"
#include "mntest/serialize.h"

#include <gtest/gtest.h>

#include <string>
#include <variant>


namespace mnian::test {

TEST(iDeserializer, ValueConversionToSigned) {
  using core::iDeserializer;

  int32_t i32;
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&i32, int64_t{-1}));
  ASSERT_EQ(i32, int32_t{-1});
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&i32, -1.5));
  ASSERT_EQ(i32, int32_t{-1});
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&i32, std::string("-1")));
  ASSERT_EQ(i32, int32_t{-1});
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&i32, int64_t{INT32_MAX}));
  ASSERT_EQ(i32, INT32_MAX);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&i32, int64_t{INT32_MIN}));
  ASSERT_EQ(i32, INT32_MIN);

  ASSERT_FALSE(iDeserializer::ConvertFromValue(&i32, int64_t{INT64_MAX}));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&i32, int64_t{0x80000000}));

  ASSERT_FALSE(iDeserializer::ConvertFromValue(
          &i32, std::string("helloworld")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(
          &i32, std::string("1abc")));
}
TEST(iDeserializer, ValueConversionToUnsigned) {
  using core::iDeserializer;

  uint32_t u32;
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&u32, int64_t{1}));
  ASSERT_EQ(u32, uint32_t{1});
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&u32, 1.5));
  ASSERT_EQ(u32, uint32_t{1});
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&u32, std::string("1")));
  ASSERT_EQ(u32, uint32_t{1});
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&u32, int64_t{UINT32_MAX}));
  ASSERT_EQ(u32, UINT32_MAX);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&u32, int64_t{0}));
  ASSERT_EQ(u32, uint32_t{0});

  ASSERT_FALSE(iDeserializer::ConvertFromValue(&u32, int64_t{-1}));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&u32, -1.5));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&u32, std::string("-1")));

  ASSERT_FALSE(iDeserializer::ConvertFromValue(&u32, int64_t{0x100000000}));

  ASSERT_FALSE(iDeserializer::ConvertFromValue(
          &u32, std::string("helloworld")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&u32, std::string("1abc")));
}
TEST(iDeserializer, ValueConversionToFloating) {
  using core::iDeserializer;

  double f;
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&f, int64_t{1}));
  ASSERT_EQ(f, 1.);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&f, 1.1));
  ASSERT_EQ(f, 1.1);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&f, std::string("1.1")));
  ASSERT_EQ(f, 1.1);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&f, std::string("1.1e+3")));
  ASSERT_EQ(f, 1.1e+3);

  ASSERT_FALSE(iDeserializer::ConvertFromValue(&f, std::string("helloworld")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&f, std::string("NaN")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&f, std::string("INF")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&f, std::string("+INF")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&f, std::string("-INF")));
}
TEST(iDeserializer, ValueConversionToBool) {
  using core::iDeserializer;

  bool b;
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&b, std::string("true")));
  ASSERT_TRUE(b);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&b, std::string("false")));
  ASSERT_FALSE(b);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&b, bool{true}));
  ASSERT_TRUE(b);
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&b, bool{false}));
  ASSERT_FALSE(b);

  ASSERT_FALSE(iDeserializer::ConvertFromValue(&b, std::string("helloworld")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&b, int64_t{1}));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&b, 1.));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&b, std::string("TRUE")));
  ASSERT_FALSE(iDeserializer::ConvertFromValue(&b, std::string("FALSE")));
}
TEST(iDeserializer, ValueConversionToString) {
  using core::iDeserializer;

  std::string s;
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&s, std::string("helloworld")));
  ASSERT_EQ(s, "helloworld");
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&s, int64_t{-1}));
  ASSERT_EQ(s, "-1");
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&s, 1.1));
  ASSERT_EQ(s.substr(0, 3), "1.1");  // Actually "1.10000..." is stored.
  ASSERT_TRUE(iDeserializer::ConvertFromValue(&s, bool{true}));
  ASSERT_EQ(s, "true");
}

TEST(MockSerializable, SimpleSerialize) {
  MockSerializable serializable;
  MockSerializer   serializer;

  EXPECT_CALL(serializable, Serialize(&serializer));
  serializable.Serialize(&serializer);
}

TEST(MockSerializer, SimpleSerialize) {
  MockSerializer serializer;
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeMap(0));
    EXPECT_CALL(serializer, SerializeArray(0));
    EXPECT_CALL(serializer, SerializeKey(::testing::_));
    EXPECT_CALL(serializer, SerializeValue(::testing::_));
    EXPECT_CALL(serializer, SerializeKey(::testing::_));
    EXPECT_CALL(serializer, SerializeValue(::testing::_));
  }
  serializer.SerializeMap(0);
  serializer.SerializeArray(0);
  serializer.SerializeKey("key1");
  serializer.SerializeValue("value1");
  serializer.SerializeKeyValue("key2", "value2");
}

TEST(MockDeserializer, SimpleDeserialize) {
  MockDeserializer deserializer;
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(deserializer, EnterByIndex(::testing::Eq(0)));
    EXPECT_CALL(deserializer, value()).
        WillOnce(::testing::Return(std::string("value1")));
    EXPECT_CALL(deserializer, size()).
        WillOnce(::testing::Return(std::nullopt));
    EXPECT_CALL(deserializer, Leave());
  }
  deserializer.EnterByIndex(0);

  const auto value = deserializer.value();
  ASSERT_TRUE(value);
  ASSERT_EQ(std::get<std::string>(*value), "value1");

  ASSERT_FALSE(deserializer.size());

  deserializer.Leave();
}

}  // namespace mnian::test
