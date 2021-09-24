// No copyright
#include "mncore/conv.h"

#include <gtest/gtest.h>


namespace mnian::test {

#define AssertIntToInt(A, B, v) ASSERT_EQ(core::ToInt<A>(B{v}), A{v})
#define AssertIntToIntFail(A, B, v) ASSERT_FALSE(core::ToInt<A>(B{v}))

#define AssertStrToInt(A, s, v) ASSERT_EQ(core::ToInt<A>(std::string(s)), A{v})
#define AssertStrToIntFail(A, s) ASSERT_FALSE(core::ToInt<A>(std::string(s)))

#define AssertStrToFloat(A, s, v)  \
  ASSERT_EQ(core::ToFloat<A>(std::string(s)), A{v})
#define AssertStrToFloatFail(A, s)  \
  ASSERT_FALSE(core::ToFloat<A>(std::string(s)))


TEST(ToInt, UnsignedToUnsigned) {
  AssertIntToInt(uint64_t, uint32_t, UINT32_MAX);
  AssertIntToInt(uint32_t, uint32_t, UINT32_MAX);
  AssertIntToIntFail(uint16_t, uint32_t, UINT32_MAX);

  AssertIntToInt(uint64_t, uint32_t, 0);
  AssertIntToInt(uint32_t, uint32_t, 0);
  AssertIntToInt(uint16_t, uint32_t, 0);
}
TEST(ToInt, UnsignedToSigned) {
  AssertIntToInt(int64_t, uint32_t, INT32_MAX);
  AssertIntToInt(int32_t, uint32_t, INT32_MAX);
  AssertIntToIntFail(int16_t, uint32_t, INT32_MAX);

  AssertIntToInt(int64_t, uint32_t, 0);
  AssertIntToInt(int32_t, uint32_t, 0);
  AssertIntToInt(int16_t, uint32_t, 0);
}
TEST(ToInt, SignedToSigned) {
  AssertIntToInt(int64_t, int32_t, INT32_MAX);
  AssertIntToInt(int32_t, int32_t, INT32_MAX);
  AssertIntToIntFail(int16_t, int32_t, INT32_MAX);

  AssertIntToInt(int64_t, int32_t, INT32_MIN);
  AssertIntToInt(int32_t, int32_t, INT32_MIN);
  AssertIntToIntFail(int16_t, int32_t, INT32_MIN);

  AssertIntToInt(int64_t, int32_t, 0);
  AssertIntToInt(int32_t, int32_t, 0);
  AssertIntToInt(int16_t, int32_t, 0);
}
TEST(ToInt, SignedToUnsigned) {
  AssertIntToInt(uint64_t, int32_t, INT32_MAX);
  AssertIntToInt(uint32_t, int32_t, INT32_MAX);
  AssertIntToIntFail(uint16_t, int32_t, INT32_MAX);

  AssertIntToIntFail(uint64_t, int32_t, INT32_MIN);
  AssertIntToIntFail(uint32_t, int32_t, INT32_MIN);
  AssertIntToIntFail(uint16_t, int32_t, INT32_MIN);

  AssertIntToInt(uint64_t, int32_t, 0);
  AssertIntToInt(uint32_t, int32_t, 0);
  AssertIntToInt(uint16_t, int32_t, 0);
}

TEST(ToInt, FromStr) {
  AssertStrToInt(int32_t, "0", 0);
  AssertStrToInt(int32_t, "1", 1);
  AssertStrToInt(int32_t, "-1", -1);

  AssertStrToInt(int32_t, "0xFF", 0xFF);

  AssertStrToIntFail(int32_t, "helloworld");
  AssertStrToIntFail(int32_t, "1a");
  AssertStrToIntFail(uint32_t, "-1");
}


TEST(ToFloat, FromStr) {
  AssertStrToFloat(double, "0.0", 0.0);
  AssertStrToFloat(double, "1.5", 1.5);
  AssertStrToFloat(double, "-1.5", -1.5);

  AssertStrToFloat(double, "0", 0);
  AssertStrToFloat(double, "1", 1);
  AssertStrToFloat(double, "-1", -1);

  AssertStrToFloatFail(double, "NaN");
  AssertStrToFloatFail(double, "INF");
  AssertStrToFloatFail(double, "-INF");

  AssertStrToFloatFail(double, "helloworld");
  AssertStrToFloatFail(double, "1.0a");
}


TEST(ToStr, FromInt) {
  ASSERT_EQ(core::ToStr(int32_t{0}), "0");
  ASSERT_EQ(core::ToStr(int32_t{1}), "1");
  ASSERT_EQ(core::ToStr(int32_t{-1}), "-1");
}

TEST(ToStr, FromFloat) {
  ASSERT_EQ(core::ToStr(double{0.0})->substr(0, 3), "0.0");
  ASSERT_EQ(core::ToStr(double{1.5})->substr(0, 3), "1.5");
  ASSERT_EQ(core::ToStr(double{-1.5})->substr(0, 4), "-1.5");
}

TEST(ToStr, FromBool) {
  ASSERT_EQ(core::ToStr(bool{true}), "true");
  ASSERT_EQ(core::ToStr(bool{false}), "false");
}


TEST(ToBool, FromStr) {
  ASSERT_TRUE(*core::ToBool(std::string("true")));
  ASSERT_FALSE(*core::ToBool(std::string("false")));

  ASSERT_FALSE(core::ToBool(std::string("helloworld")));
  ASSERT_FALSE(core::ToBool(std::string("truee")));
  ASSERT_FALSE(core::ToBool(std::string("falsee")));
}

}  // namespace mnian::test
