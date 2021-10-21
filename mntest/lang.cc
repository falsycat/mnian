// No copyright
#include "mncore/lang.h"

#include <gtest/gtest.h>


namespace mnian::test {

TEST(Lang, Hash) {
  ASSERT_NE(core::Lang::Hash("hello"), core::Lang::Hash("world"));
}

TEST(Lang, AddDuplicated) {
  core::Lang lang;
  ASSERT_TRUE(lang.Add("text1", "hello world"));
  ASSERT_TRUE(lang.Add("text2", "hell world"));

  ASSERT_FALSE(lang.Add("text1", "hello"));
  ASSERT_FALSE(lang.Add("text2", "hell"));
}

TEST(Lang, Translate) {
  core::Lang lang;
  ASSERT_TRUE(lang.Add("text1", "hello world"));
  ASSERT_TRUE(lang.Add("text2", "hell world"));

  ASSERT_EQ(*lang.Translate(core::Lang::Hash("text1")), "hello world");
  ASSERT_EQ(*lang.Translate(core::Lang::Hash("text2")), "hell world");

  ASSERT_EQ(lang.Translate(core::Lang::Hash("text3")), nullptr);
}

TEST(Lang, TranslateFallback) {
  core::Lang back;
  ASSERT_TRUE(back.Add("text1", "hello world"));
  ASSERT_TRUE(back.Add("text2", "hell world"));

  core::Lang front(&back);
  ASSERT_TRUE(front.Add("text2", "awesome world"));

  ASSERT_EQ(*front.Translate(core::Lang::Hash("text1")), "hello world");
  ASSERT_EQ(*back .Translate(core::Lang::Hash("text2")), "hell world");
  ASSERT_EQ(*front.Translate(core::Lang::Hash("text2")), "awesome world");
}

TEST(Lang, HashTransition) {
  core::Lang lang;

  uint64_t hash;
  ASSERT_EQ(lang.hash(), 0);
  hash = lang.hash();

  ASSERT_TRUE(lang.Add("text1", "hello world"));
  ASSERT_NE(lang.hash(), hash);
  hash = lang.hash();

  ASSERT_TRUE(lang.Add("text2", "hell world"));
  ASSERT_NE(lang.hash(), hash);
  hash = lang.hash();
}

TEST(Lang_Text, Get) {
  core::Lang lang;
  ASSERT_TRUE(lang.Add("text1", "hello world"));

  core::Lang::Text text1(&lang, core::Lang::Hash("text1"));
  core::Lang::Text text2(&lang, core::Lang::Hash("text2"));

  ASSERT_EQ(*text1, "hello world");
  ASSERT_TRUE((*text2).size());

  lang.Clear();
  ASSERT_TRUE(lang.Add("text1", "goodbye world"));
  ASSERT_EQ(*text1, "goodbye world");

  ASSERT_TRUE(lang.Add("text2", "hell world"));
  ASSERT_EQ(*text2, "hell world");
}

}  // namespace mnian::test
