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

TEST(iSerializer_MapGuard, SimpleAdd) {
  ::testing::StrictMock<MockSerializer> serializer;

  core::iSerializer::MapGuard map(&serializer, 3);
  map.Add("key1", int64_t{1});
  map.Add("key2", std::string("helloworld"));
  map.Add("key3", 1.5);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeMap(3));

    EXPECT_CALL(serializer, SerializeKey("key1"));
    EXPECT_CALL(
        serializer, SerializeValue(core::iSerializer::Value(int64_t{1})));

    EXPECT_CALL(serializer, SerializeKey("key2"));
    EXPECT_CALL(
        serializer,
        SerializeValue(core::iSerializer::Value(std::string("helloworld"))));

    EXPECT_CALL(serializer, SerializeKey("key3"));
    EXPECT_CALL(serializer, SerializeValue(core::iSerializer::Value(1.5)));
  }
}
TEST(iSerializer_MapGuard, NestedAdd) {
  ::testing::StrictMock<MockSerializer> serializer;
  MockSerializable serializable1;
  MockSerializable serializable2;
  MockSerializable serializable3;

  core::iSerializer::MapGuard map(&serializer, 3);
  map.Add("key1", &serializable1);
  map.Add("key2", &serializable2);
  map.Add("key3", &serializable3);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeMap(3));

    EXPECT_CALL(serializer, SerializeKey("key1"));
    EXPECT_CALL(serializable1, Serialize(&serializer));

    EXPECT_CALL(serializer, SerializeKey("key2"));
    EXPECT_CALL(serializable2, Serialize(&serializer));

    EXPECT_CALL(serializer, SerializeKey("key3"));
    EXPECT_CALL(serializable3, Serialize(&serializer));
  }
}
TEST(iSerializer_MapGuard, RecursiveAdd) {
  ::testing::StrictMock<MockSerializer> serializer;
  MockSerializable serializable1;
  MockSerializable serializable2;
  MockSerializable serializable3;

  core::iSerializer::MapGuard submap1(&serializer);
  submap1.Add("subkey1", &serializable1);

  core::iSerializer::MapGuard submap2(&serializer);
  submap2.Add("subkey2", &serializable2);

  core::iSerializer::MapGuard submap3(&serializer);
  submap3.Add("subkey3", &serializable3);

  core::iSerializer::MapGuard map(&serializer, 3);
  map.Add("key1", &submap1);
  map.Add("key2", &submap2);
  map.Add("key3", &submap3);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeMap(3));

    EXPECT_CALL(serializer, SerializeKey("key1"));
    EXPECT_CALL(serializer, SerializeMap(1));
    EXPECT_CALL(serializer, SerializeKey("subkey1"));
    EXPECT_CALL(serializable1, Serialize(&serializer));

    EXPECT_CALL(serializer, SerializeKey("key2"));
    EXPECT_CALL(serializer, SerializeMap(1));
    EXPECT_CALL(serializer, SerializeKey("subkey2"));
    EXPECT_CALL(serializable2, Serialize(&serializer));

    EXPECT_CALL(serializer, SerializeKey("key3"));
    EXPECT_CALL(serializer, SerializeMap(1));
    EXPECT_CALL(serializer, SerializeKey("subkey3"));
    EXPECT_CALL(serializable3, Serialize(&serializer));
  }
}

TEST(iSerializer_ArrayGuard, SimpleAdd) {
  ::testing::StrictMock<MockSerializer> serializer;

  core::iSerializer::ArrayGuard array(&serializer, 3);
  array.Add(int64_t{1});
  array.Add(std::string("helloworld"));
  array.Add(1.5);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeArray(3));

    EXPECT_CALL(
        serializer, SerializeValue(core::iSerializer::Value(int64_t{1})));
    EXPECT_CALL(
        serializer,
        SerializeValue(core::iSerializer::Value(std::string("helloworld"))));
    EXPECT_CALL(serializer, SerializeValue(core::iSerializer::Value(1.5)));
  }
}
TEST(iSerializer_ArrayGuard, NestedAdd) {
  ::testing::StrictMock<MockSerializer> serializer;
  MockSerializable serializable1;
  MockSerializable serializable2;
  MockSerializable serializable3;

  core::iSerializer::ArrayGuard array(&serializer, 3);
  array.Add(&serializable1);
  array.Add(&serializable2);
  array.Add(&serializable3);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeArray(3));
    EXPECT_CALL(serializable1, Serialize(&serializer));
    EXPECT_CALL(serializable2, Serialize(&serializer));
    EXPECT_CALL(serializable3, Serialize(&serializer));
  }
}
TEST(iSerializer_ArrayGuard, RecursiveAdd) {
  ::testing::StrictMock<MockSerializer> serializer;
  MockSerializable serializable1;
  MockSerializable serializable2;
  MockSerializable serializable3;

  core::iSerializer::ArrayGuard subarray1(&serializer);
  subarray1.Add(&serializable1);

  core::iSerializer::ArrayGuard subarray2(&serializer);
  subarray2.Add(&serializable2);

  core::iSerializer::ArrayGuard subarray3(&serializer);
  subarray3.Add(&serializable3);

  core::iSerializer::ArrayGuard array(&serializer, 3);
  array.Add(&subarray1);
  array.Add(&subarray2);
  array.Add(&subarray3);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(serializer, SerializeArray(3));
    EXPECT_CALL(serializer, SerializeArray(1));
    EXPECT_CALL(serializable1, Serialize(&serializer));
    EXPECT_CALL(serializer, SerializeArray(1));
    EXPECT_CALL(serializable2, Serialize(&serializer));
    EXPECT_CALL(serializer, SerializeArray(1));
    EXPECT_CALL(serializable3, Serialize(&serializer));
  }
}

}  // namespace mnian::test
