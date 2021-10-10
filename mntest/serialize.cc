// No copyright
#include "mncore/serialize.h"
#include "mntest/serialize.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <variant>

#include "mntest/app.h"
#include "mntest/file.h"


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


class iDeserializer : public ::testing::Test {
 public:
  iDeserializer() :
      app_(&clock_, &reg_, &logger_, &fstore_), des_(&app_, &logger_, &reg_) {
  }

  core::ManualClock clock_;

  core::DeserializerRegistry reg_;

  core::NullLogger logger_;

  ::testing::NiceMock<MockFileStore> fstore_;

  ::testing::NiceMock<MockApp> app_;

  ::testing::StrictMock<MockDeserializer> des_;
};

TEST_F(iDeserializer, SetUndefined) {
  des_.SetMapOrArray(0);

  EXPECT_CALL(des_,
              DoEnter(core::iDeserializer::Key(std::string("key")))).
      WillOnce([&](auto key) {
                 des_.SetUndefined();
                 return key;
               });
  des_.Enter(std::string("key"));

  ASSERT_EQ(des_.key(), std::string("key"));
  ASSERT_FALSE(des_.key<size_t>());

  ASSERT_FALSE(des_.value<int64_t>());
  ASSERT_FALSE(des_.value<double>());
  ASSERT_FALSE(des_.value<std::string>());
  ASSERT_FALSE(des_.value<bool>());
  ASSERT_FALSE(des_.size());
  ASSERT_TRUE(des_.undefined());
}
TEST_F(iDeserializer, SetField) {
  des_.SetMapOrArray(0);

  EXPECT_CALL(des_, DoEnter(core::iDeserializer::Key(size_t{0}))).
      WillOnce([&](auto key) {
                 des_.SetField(std::string("helloworld"));
                 return key;
               });
  des_.Enter(size_t{0});

  ASSERT_FALSE(des_.key());
  ASSERT_EQ(des_.key<size_t>(), size_t{0});

  ASSERT_FALSE(des_.value<int64_t>());
  ASSERT_FALSE(des_.value<double>());
  ASSERT_EQ(des_.value<std::string>(), std::string("helloworld"));
  ASSERT_FALSE(des_.value<bool>());
  ASSERT_FALSE(des_.size());
  ASSERT_FALSE(des_.undefined());
}
TEST_F(iDeserializer, SetMapOrArray) {
  des_.SetMapOrArray(0);

  EXPECT_CALL(des_, DoEnter(core::iDeserializer::Key(size_t{0}))).
      WillOnce([&](auto key) {
                 des_.SetMapOrArray(1);
                 return key;
               });
  des_.Enter(size_t{0});

  ASSERT_FALSE(des_.key());
  ASSERT_EQ(des_.key<size_t>(), size_t{0});

  ASSERT_FALSE(des_.value<int64_t>());
  ASSERT_FALSE(des_.value<double>());
  ASSERT_FALSE(des_.value<std::string>());
  ASSERT_FALSE(des_.value<bool>());
  ASSERT_EQ(des_.size(), size_t{1});
  ASSERT_FALSE(des_.undefined());
}

TEST_F(iDeserializer, EnterUndefined) {
  ASSERT_TRUE(des_.undefined());
  ASSERT_FALSE(des_.key());

  des_.Enter(std::string("helloworld"));
  ASSERT_TRUE(des_.undefined());
  ASSERT_EQ(des_.key(), std::string("helloworld"));

  des_.Enter(size_t{0});
  ASSERT_TRUE(des_.undefined());
  ASSERT_EQ(des_.key<size_t>(), size_t{0});

  des_.Leave();
  ASSERT_TRUE(des_.undefined());
  ASSERT_EQ(des_.key(), std::string("helloworld"));

  des_.Enter(size_t{1});
  ASSERT_TRUE(des_.undefined());
  ASSERT_EQ(des_.key<size_t>(), size_t{1});

  des_.Leave();
  ASSERT_TRUE(des_.undefined());
  ASSERT_EQ(des_.key(), std::string("helloworld"));

  des_.Leave();
  ASSERT_TRUE(des_.undefined());
  ASSERT_FALSE(des_.key());
}
TEST_F(iDeserializer, EnterIndex) {
  des_.SetMapOrArray(0);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(des_, DoEnter(core::iDeserializer::Key(size_t{0}))).
        WillOnce([&](auto key) {
                   des_.SetMapOrArray(1);
                   return key;
                 });
    EXPECT_CALL(des_, DoLeave());
  }

  core::iDeserializer::ScopeGuard _(&des_, size_t{0});
}
TEST_F(iDeserializer, EnterString) {
  des_.SetMapOrArray(0);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(des_,
                DoEnter(core::iDeserializer::Key(std::string("key1")))).
        WillOnce([&](auto key) {
                   des_.SetMapOrArray(1);
                   return key;
                 });
    EXPECT_CALL(des_, DoLeave());
  }

  core::iDeserializer::ScopeGuard _(&des_, std::string("key1"));
}

TEST_F(iDeserializer, GenerateLocation) {
  ASSERT_EQ(des_.GenerateLocation(), "");

  des_.Enter(std::string("helloworld"));
  ASSERT_EQ(des_.GenerateLocation(), "helloworld");

  des_.Enter(size_t{0});
  ASSERT_EQ(des_.GenerateLocation(), "helloworld[0]");

  des_.Enter(std::string("hoge"));
  ASSERT_EQ(des_.GenerateLocation(), "helloworld[0].hoge");

  des_.Enter(std::string("fuga"));
  ASSERT_EQ(des_.GenerateLocation(), "helloworld[0].hoge.fuga");

  des_.Leave();
  ASSERT_EQ(des_.GenerateLocation(), "helloworld[0].hoge");

  des_.Leave();
  ASSERT_EQ(des_.GenerateLocation(), "helloworld[0]");

  des_.Leave();
  ASSERT_EQ(des_.GenerateLocation(), "helloworld");

  des_.Enter(size_t{1});
  ASSERT_EQ(des_.GenerateLocation(), "helloworld[1]");

  des_.Leave();
  ASSERT_EQ(des_.GenerateLocation(), "helloworld");

  des_.Leave();
  ASSERT_EQ(des_.GenerateLocation(), "");
}


class DeserializerRegistry : public iDeserializer {
 public:
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
};

TEST_F(DeserializerRegistry, RegisterFactory) {
  reg_.RegisterFactory<core::iPolymorphicSerializable>(
      MockPolymorphicSerializable::kType,
      [](auto) { return std::make_unique<MockPolymorphicSerializable>(); });

  des_.SetMapOrArray(0);

  auto product = reg_.DeserializeParam<core::iPolymorphicSerializable>(
      &des_, MockPolymorphicSerializable::kType);
  ASSERT_TRUE(product);
  ASSERT_TRUE(dynamic_cast<MockPolymorphicSerializable*>(product.get()));

  ASSERT_FALSE(
      reg_.DeserializeParam<core::iPolymorphicSerializable>(&des_, "hello"));
}
TEST_F(DeserializerRegistry, RegisterType) {
  reg_.RegisterType<core::iPolymorphicSerializable, TestSerializable>();
  des_.SetMapOrArray(0);

  auto product = reg_.DeserializeParam<core::iPolymorphicSerializable>(
      &des_, TestSerializable::kType);
  ASSERT_TRUE(product);
  ASSERT_TRUE(dynamic_cast<TestSerializable*>(product.get()));

  ASSERT_FALSE(
      reg_.DeserializeParam<core::iPolymorphicSerializable>(&des_, "hello"));
}

TEST_F(DeserializerRegistry, Deserialize) {
  reg_.RegisterType<
      core::iPolymorphicSerializable, MockPolymorphicSerializable>();
  des_.SetMapOrArray(0);

  EXPECT_CALL(des_, DoEnter(::testing::_)).
      Times(2).
      WillOnce([&](auto key) {
                 des_.SetField(
                     std::string(MockPolymorphicSerializable::kType));
                 return key;
               }).
      WillOnce([&](auto key) {
                 des_.SetField(int64_t{0});
                 return key;
               });
  EXPECT_CALL(des_, DoLeave()).Times(2);

  auto product =
      des_.DeserializeObject<core::iPolymorphicSerializable>();
  ASSERT_TRUE(product);
  ASSERT_TRUE(dynamic_cast<MockPolymorphicSerializable*>(product.get()));
}
TEST_F(DeserializerRegistry, DeserializeInvalid) {
  reg_.RegisterType<
      core::iPolymorphicSerializable, MockPolymorphicSerializable>();
  des_.SetMapOrArray(0);

  EXPECT_CALL(des_, DoEnter(::testing::_));
  EXPECT_CALL(des_, DoLeave());

  auto product = des_.DeserializeObject<core::iPolymorphicSerializable>();
  ASSERT_FALSE(product);
}

}  // namespace mnian::test
