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


namespace {

void SerializeTestObject(core::iSerializer* serial) {
  core::iSerializer::ArrayGuard array(serial);
  core::iSerializer::MapGuard   map(serial);

  map.Add("array",  &array);
  map.Add("int",    int64_t{0});
  map.Add("double", 0.);
  map.Add("str",    std::string("helloworld"));
  map.Add("bool",   true);

  array.Add(int64_t{0});
  array.Add(0.);
  array.Add(std::string("helloworld"));
  array.Add(true);
}

TEST(iSerializer, Json) {
  static constexpr const char* kExpect =
      R"({"array":[0,0.0,"helloworld",true],"int":0,"double":0.0,"str":"helloworld","bool":true})";

  std::stringstream st;
  {
    auto serial = core::iSerializer::CreateJson(&st);
    SerializeTestObject(serial.get());
  }
  ASSERT_EQ(st.str(), kExpect);
}
TEST(iSerializer, PrettyJson) {
  static constexpr const char* kExpect = R"({
  "array": [
    0,
    0.0,
    "helloworld",
    true
  ],
  "int": 0,
  "double": 0.0,
  "str": "helloworld",
  "bool": true
})";

  std::stringstream st;
  {
    auto serial = core::iSerializer::CreatePrettyJson(&st);
    SerializeTestObject(serial.get());
  }
  ASSERT_EQ(st.str(), kExpect);
}

}  // namespace


class iDeserializer : public ::testing::Test {
 public:
  iDeserializer() : app_(&clock_, &reg_, &logger_, &fstore_) {
  }

  core::ManualClock clock_;

  core::DeserializerRegistry reg_;

  core::NullLogger logger_;

  ::testing::NiceMock<MockFileStore> fstore_;

  ::testing::NiceMock<MockApp> app_;
};

TEST_F(iDeserializer, SetUndefined) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  des.SetMapOrArray(0);

  EXPECT_CALL(des,
              DoEnter(core::iDeserializer::Key(std::string("key")))).
      WillOnce([&](auto key) {
                 des.SetUndefined();
                 return key;
               });
  des.Enter(std::string("key"));

  ASSERT_EQ(des.key(), std::string("key"));
  ASSERT_FALSE(des.key<size_t>());

  ASSERT_FALSE(des.value<int64_t>());
  ASSERT_FALSE(des.value<double>());
  ASSERT_FALSE(des.value<std::string>());
  ASSERT_FALSE(des.value<bool>());
  ASSERT_FALSE(des.size());
  ASSERT_TRUE(des.undefined());
}
TEST_F(iDeserializer, SetField) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  des.SetMapOrArray(0);

  EXPECT_CALL(des, DoEnter(core::iDeserializer::Key(size_t{0}))).
      WillOnce([&](auto key) {
                 des.SetField(std::string("helloworld"));
                 return key;
               });
  des.Enter(size_t{0});

  ASSERT_FALSE(des.key());
  ASSERT_EQ(des.key<size_t>(), size_t{0});

  ASSERT_FALSE(des.value<int64_t>());
  ASSERT_FALSE(des.value<double>());
  ASSERT_EQ(des.value<std::string>(), std::string("helloworld"));
  ASSERT_FALSE(des.value<bool>());
  ASSERT_FALSE(des.size());
  ASSERT_FALSE(des.undefined());
}
TEST_F(iDeserializer, SetMapOrArray) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  des.SetMapOrArray(0);

  EXPECT_CALL(des, DoEnter(core::iDeserializer::Key(size_t{0}))).
      WillOnce([&](auto key) {
                 des.SetMapOrArray(1);
                 return key;
               });
  des.Enter(size_t{0});

  ASSERT_FALSE(des.key());
  ASSERT_EQ(des.key<size_t>(), size_t{0});

  ASSERT_FALSE(des.value<int64_t>());
  ASSERT_FALSE(des.value<double>());
  ASSERT_FALSE(des.value<std::string>());
  ASSERT_FALSE(des.value<bool>());
  ASSERT_EQ(des.size(), size_t{1});
  ASSERT_FALSE(des.undefined());
}

TEST_F(iDeserializer, EnterUndefined) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  ASSERT_TRUE(des.undefined());
  ASSERT_FALSE(des.key());

  des.Enter(std::string("helloworld"));
  ASSERT_TRUE(des.undefined());
  ASSERT_EQ(des.key(), std::string("helloworld"));

  des.Enter(size_t{0});
  ASSERT_TRUE(des.undefined());
  ASSERT_EQ(des.key<size_t>(), size_t{0});

  des.Leave();
  ASSERT_TRUE(des.undefined());
  ASSERT_EQ(des.key(), std::string("helloworld"));

  des.Enter(size_t{1});
  ASSERT_TRUE(des.undefined());
  ASSERT_EQ(des.key<size_t>(), size_t{1});

  des.Leave();
  ASSERT_TRUE(des.undefined());
  ASSERT_EQ(des.key(), std::string("helloworld"));

  des.Leave();
  ASSERT_TRUE(des.undefined());
  ASSERT_FALSE(des.key());
}
TEST_F(iDeserializer, EnterIndex) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  des.SetMapOrArray(0);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(des, DoEnter(core::iDeserializer::Key(size_t{0}))).
        WillOnce([&](auto key) {
                   des.SetMapOrArray(1);
                   return key;
                 });
    EXPECT_CALL(des, DoLeave());
  }

  core::iDeserializer::ScopeGuard _(&des, size_t{0});
}
TEST_F(iDeserializer, EnterString) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  des.SetMapOrArray(0);
  {
    ::testing::InSequence seq_;

    EXPECT_CALL(des,
                DoEnter(core::iDeserializer::Key(std::string("key1")))).
        WillOnce([&](auto key) {
                   des.SetMapOrArray(1);
                   return key;
                 });
    EXPECT_CALL(des, DoLeave());
  }

  core::iDeserializer::ScopeGuard _(&des, std::string("key1"));
}

TEST_F(iDeserializer, GenerateLocation) {
  ::testing::StrictMock<MockDeserializer> des(&app_, &logger_, &reg_);
  ASSERT_EQ(des.GenerateLocation(), "");

  des.Enter(std::string("helloworld"));
  ASSERT_EQ(des.GenerateLocation(), "helloworld");

  des.Enter(size_t{0});
  ASSERT_EQ(des.GenerateLocation(), "helloworld[0]");

  des.Enter(std::string("hoge"));
  ASSERT_EQ(des.GenerateLocation(), "helloworld[0].hoge");

  des.Enter(std::string("fuga"));
  ASSERT_EQ(des.GenerateLocation(), "helloworld[0].hoge.fuga");

  des.Leave();
  ASSERT_EQ(des.GenerateLocation(), "helloworld[0].hoge");

  des.Leave();
  ASSERT_EQ(des.GenerateLocation(), "helloworld[0]");

  des.Leave();
  ASSERT_EQ(des.GenerateLocation(), "helloworld");

  des.Enter(size_t{1});
  ASSERT_EQ(des.GenerateLocation(), "helloworld[1]");

  des.Leave();
  ASSERT_EQ(des.GenerateLocation(), "helloworld");

  des.Leave();
  ASSERT_EQ(des.GenerateLocation(), "");
}

TEST_F(iDeserializer, Json) {
  std::stringstream st;
  st << R"({"array":[0,0.0,"helloworld",true],"int":0,"double":0.0,"str":"helloworld","bool":true})";

  auto des = core::iDeserializer::CreateJson(&app_, &logger_, &reg_, &st);
  ASSERT_TRUE(des);

  des->Enter(std::string("array"));
  {
    ASSERT_EQ(des->size(), size_t{4});

    des->Enter(size_t{0});
    ASSERT_EQ(des->value<int64_t>(), int64_t{0});
    des->Leave();

    des->Enter(size_t{1});
    ASSERT_EQ(des->value<double>(), 0.);
    des->Leave();

    des->Enter(size_t{2});
    ASSERT_EQ(des->value<std::string>(), "helloworld");
    des->Leave();

    des->Enter(size_t{3});
    ASSERT_TRUE(des->value<bool>());
    des->Leave();
  }
  des->Leave();

  des->Enter(std::string("int"));
  ASSERT_EQ(des->value<int64_t>(), int64_t{0});
  des->Leave();

  des->Enter(std::string("double"));
  ASSERT_EQ(des->value<double>(), 0.);
  des->Leave();

  des->Enter(std::string("str"));
  ASSERT_EQ(des->value<std::string>(), "helloworld");
  des->Leave();

  des->Enter(std::string("bool"));
  ASSERT_TRUE(des->value<bool>());
  des->Leave();
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


  DeserializerRegistry() : iDeserializer(), des_(&app_, &logger_, &reg_) {
  }


  ::testing::StrictMock<MockDeserializer> des_;
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
