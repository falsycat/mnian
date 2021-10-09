// No copyright
#include "mncore/serialize.h"
#include "mntest/serialize.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <variant>

#include "mntest/app.h"
#include "mntest/logger.h"


namespace mnian::test {

TEST(iPolymorphicSerializable, Serialize) {
  MockPolymorphicSerializable serializable;
  MockSerializer serializer;
  {
    ::testing::InSequence seq_;
    EXPECT_CALL(serializer, SerializeMap(2));
    EXPECT_CALL(serializer, SerializeKey("type"));
    EXPECT_CALL(
        serializer,
        SerializeValue(
            core::Any(std::string(MockPolymorphicSerializable::kType))));
    EXPECT_CALL(serializer, SerializeKey("param"));

    EXPECT_CALL(serializable, SerializeParam(&serializer));
  }
  serializable.Serialize(&serializer);
}

TEST(iDeserializer, SetUndefined) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  EXPECT_CALL(deserializer,
              DoEnter(core::iDeserializer::Key(std::string("key")))).
      WillOnce([&](auto key) {
                 deserializer.SetUndefined();
                 return key;
               });
  deserializer.Enter(std::string("key"));

  ASSERT_EQ(deserializer.key(), std::string("key"));
  ASSERT_FALSE(deserializer.key<size_t>());

  ASSERT_FALSE(deserializer.value<int64_t>());
  ASSERT_FALSE(deserializer.value<double>());
  ASSERT_FALSE(deserializer.value<std::string>());
  ASSERT_FALSE(deserializer.value<bool>());
  ASSERT_FALSE(deserializer.size());
  ASSERT_TRUE(deserializer.undefined());
}
TEST(iDeserializer, SetField) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  EXPECT_CALL(deserializer, DoEnter(core::iDeserializer::Key(size_t{0}))).
      WillOnce([&](auto key) {
                 deserializer.SetField(std::string("helloworld"));
                 return key;
               });
  deserializer.Enter(size_t{0});

  ASSERT_FALSE(deserializer.key());
  ASSERT_EQ(deserializer.key<size_t>(), size_t{0});

  ASSERT_FALSE(deserializer.value<int64_t>());
  ASSERT_FALSE(deserializer.value<double>());
  ASSERT_EQ(deserializer.value<std::string>(), std::string("helloworld"));
  ASSERT_FALSE(deserializer.value<bool>());
  ASSERT_FALSE(deserializer.size());
  ASSERT_FALSE(deserializer.undefined());
}
TEST(iDeserializer, SetMapOrArray) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  EXPECT_CALL(deserializer, DoEnter(core::iDeserializer::Key(size_t{0}))).
      WillOnce([&](auto key) {
                 deserializer.SetMapOrArray(1);
                 return key;
               });
  deserializer.Enter(size_t{0});

  ASSERT_FALSE(deserializer.key());
  ASSERT_EQ(deserializer.key<size_t>(), size_t{0});

  ASSERT_FALSE(deserializer.value<int64_t>());
  ASSERT_FALSE(deserializer.value<double>());
  ASSERT_FALSE(deserializer.value<std::string>());
  ASSERT_FALSE(deserializer.value<bool>());
  ASSERT_EQ(deserializer.size(), size_t{1});
  ASSERT_FALSE(deserializer.undefined());
}

TEST(iDeserializer, EnterUndefined) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);

  ASSERT_TRUE(deserializer.undefined());
  ASSERT_FALSE(deserializer.key());

  deserializer.Enter(std::string("helloworld"));
  ASSERT_TRUE(deserializer.undefined());
  ASSERT_EQ(deserializer.key(), std::string("helloworld"));

  deserializer.Enter(size_t{0});
  ASSERT_TRUE(deserializer.undefined());
  ASSERT_EQ(deserializer.key<size_t>(), size_t{0});

  deserializer.Leave();
  ASSERT_TRUE(deserializer.undefined());
  ASSERT_EQ(deserializer.key(), std::string("helloworld"));

  deserializer.Enter(size_t{1});
  ASSERT_TRUE(deserializer.undefined());
  ASSERT_EQ(deserializer.key<size_t>(), size_t{1});

  deserializer.Leave();
  ASSERT_TRUE(deserializer.undefined());
  ASSERT_EQ(deserializer.key(), std::string("helloworld"));

  deserializer.Leave();
  ASSERT_TRUE(deserializer.undefined());
  ASSERT_FALSE(deserializer.key());
}

TEST(iDeserializer, GenerateLocation) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  ASSERT_EQ(deserializer.GenerateLocation(), "");

  deserializer.Enter(std::string("helloworld"));
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld");

  deserializer.Enter(size_t{0});
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld[0]");

  deserializer.Enter(std::string("hoge"));
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld[0].hoge");

  deserializer.Enter(std::string("fuga"));
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld[0].hoge.fuga");

  deserializer.Leave();
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld[0].hoge");

  deserializer.Leave();
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld[0]");

  deserializer.Leave();
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld");

  deserializer.Enter(size_t{1});
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld[1]");

  deserializer.Leave();
  ASSERT_EQ(deserializer.GenerateLocation(), "helloworld");

  deserializer.Leave();
  ASSERT_EQ(deserializer.GenerateLocation(), "");
}

TEST(iDeserializer_ScopeGuard, EnterIndex) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(deserializer, DoEnter(core::iDeserializer::Key(size_t{0}))).
        WillOnce([&](auto key) {
                   deserializer.SetMapOrArray(1);
                   return key;
                 });
    EXPECT_CALL(deserializer, DoLeave());
  }

  core::iDeserializer::ScopeGuard _(&deserializer, size_t{0});
}
TEST(iDeserializer_ScopeGuard, EnterString) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(deserializer,
                DoEnter(core::iDeserializer::Key(std::string("key1")))).
        WillOnce([&](auto key) {
                   deserializer.SetMapOrArray(1);
                   return key;
                 });
    EXPECT_CALL(deserializer, DoLeave());
  }

  core::iDeserializer::ScopeGuard _(&deserializer, std::string("key1"));
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
    EXPECT_CALL(serializer, SerializeValue(core::Any(int64_t{1})));

    EXPECT_CALL(serializer, SerializeKey("key2"));
    EXPECT_CALL(
        serializer, SerializeValue(core::Any(std::string("helloworld"))));

    EXPECT_CALL(serializer, SerializeKey("key3"));
    EXPECT_CALL(serializer, SerializeValue(core::Any(1.5)));
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

    EXPECT_CALL(serializer, SerializeValue(core::Any(int64_t{1})));
    EXPECT_CALL(
        serializer, SerializeValue(core::Any(std::string("helloworld"))));
    EXPECT_CALL(serializer, SerializeValue(core::Any(1.5)));
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


TEST(DeserializerRegistry, RegisterFactory) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  reg.RegisterFactory<core::iPolymorphicSerializable>(
      MockPolymorphicSerializable::kType,
      [](auto) { return std::make_unique<MockPolymorphicSerializable>(); });

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  auto product = reg.DeserializeParam<core::iPolymorphicSerializable>(
      &deserializer, MockPolymorphicSerializable::kType);
  ASSERT_TRUE(product);
  ASSERT_TRUE(dynamic_cast<MockPolymorphicSerializable*>(product.get()));

  ASSERT_FALSE(reg.DeserializeParam<core::iPolymorphicSerializable>(
          &deserializer, "hello"));
}

namespace {

class TestSerializable : public core::iPolymorphicSerializable {
 public:
  static constexpr const char* kType = "Test";

  static std::unique_ptr<TestSerializable> DeserializeParam(
      core::iDeserializer*) {
    return std::make_unique<TestSerializable>();
  }
  TestSerializable() : iPolymorphicSerializable(kType) {
  }
  void SerializeParam(core::iSerializer*) const override {
  }
};
TEST(DeserializerRegistry, RegisterType) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  reg.RegisterType<core::iPolymorphicSerializable, TestSerializable>();

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  auto product = reg.DeserializeParam<core::iPolymorphicSerializable>(
      &deserializer, TestSerializable::kType);
  ASSERT_TRUE(product);
  ASSERT_TRUE(dynamic_cast<TestSerializable*>(product.get()));

  ASSERT_FALSE(reg.DeserializeParam<core::iPolymorphicSerializable>(
          &deserializer, "hello"));
}

}  // namespace

TEST(DeserializerRegistry, Deserialize) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  reg.RegisterType<
      core::iPolymorphicSerializable, MockPolymorphicSerializable>();

  ::testing::StrictMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  EXPECT_CALL(deserializer, DoEnter(::testing::_)).
      Times(2).
      WillOnce([&](auto key) {
                 deserializer.SetField(
                     std::string(MockPolymorphicSerializable::kType));
                 return key;
               }).
      WillOnce([&](auto key) {
                 deserializer.SetField(int64_t{0});
                 return key;
               });
  EXPECT_CALL(deserializer, DoLeave()).Times(2);

  auto product =
      deserializer.DeserializeObject<core::iPolymorphicSerializable>();
  ASSERT_TRUE(product);
  ASSERT_TRUE(dynamic_cast<MockPolymorphicSerializable*>(product.get()));
}

TEST(DeserializerRegistry, DeserializeInvalid) {
  ::testing::StrictMock<MockApp> app;
  core::DeserializerRegistry reg(&app);
  core::NullLogger logger;

  reg.RegisterType<
      core::iPolymorphicSerializable, MockPolymorphicSerializable>();

  ::testing::NiceMock<MockDeserializer> deserializer(&logger, &reg);
  deserializer.SetMapOrArray(0);

  EXPECT_CALL(deserializer, DoEnter(::testing::_));
  EXPECT_CALL(deserializer, DoLeave());

  auto product =
      deserializer.DeserializeObject<core::iPolymorphicSerializable>();
  ASSERT_FALSE(product);
}

}  // namespace mnian::test
