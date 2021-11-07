// No copyright
#include "mncore/store.h"

#include <gtest/gtest.h>


namespace mnian::test {

TEST(ObjectStore, Add) {
  core::ObjectStore<const char> store;

  store.Add(core::ObjectId {0}, "hello");
  ASSERT_EQ(store.Find(core::ObjectId {0}), std::string("hello"));
  ASSERT_FALSE(store.Find(core::ObjectId {1}));
  ASSERT_FALSE(store.Find(core::ObjectId {2}));

  store.Add(core::ObjectId {1}, "world");
  ASSERT_EQ(store.Find(core::ObjectId {0}), std::string("hello"));
  ASSERT_EQ(store.Find(core::ObjectId {1}), std::string("world"));
  ASSERT_FALSE(store.Find(core::ObjectId {2}));

  store.Add(core::ObjectId {2}, "goodbye");
  ASSERT_EQ(store.Find(core::ObjectId {0}), std::string("hello"));
  ASSERT_EQ(store.Find(core::ObjectId {1}), std::string("world"));
  ASSERT_EQ(store.Find(core::ObjectId {2}), std::string("goodbye"));

  store.Clear();
}

TEST(ObjectStore, Remove) {
  core::ObjectStore<const char> store;

  store.Add(core::ObjectId {0}, "hello");
  store.Add(core::ObjectId {1}, "world");
  store.Add(core::ObjectId {2}, "goodbye");
  ASSERT_TRUE(store.Find(core::ObjectId {0}));
  ASSERT_TRUE(store.Find(core::ObjectId {1}));
  ASSERT_TRUE(store.Find(core::ObjectId {2}));

  store.Remove(core::ObjectId {0});
  ASSERT_FALSE(store.Find(core::ObjectId {0}));
  ASSERT_TRUE(store.Find(core::ObjectId {1}));
  ASSERT_TRUE(store.Find(core::ObjectId {2}));

  store.Remove(core::ObjectId {1});
  ASSERT_FALSE(store.Find(core::ObjectId {0}));
  ASSERT_FALSE(store.Find(core::ObjectId {1}));
  ASSERT_TRUE(store.Find(core::ObjectId {2}));

  store.Remove(core::ObjectId {2});
  ASSERT_FALSE(store.Find(core::ObjectId {0}));
  ASSERT_FALSE(store.Find(core::ObjectId {1}));
  ASSERT_FALSE(store.Find(core::ObjectId {2}));

  store.Clear();
}

TEST(ObjectStore, AllocateId) {
  core::ObjectStore<const char> store;

  ASSERT_EQ(store.AllocateId(), core::ObjectId {0});
  ASSERT_EQ(store.AllocateId(), core::ObjectId {1});
  ASSERT_EQ(store.AllocateId(), core::ObjectId {2});
}


TEST(ObjectStore_Tag, Lifetime) {
  core::ObjectStore<const char> store;
  {
    core::ObjectStore<const char>::Tag tag(&store, core::ObjectId {0});
    store.Add(core::ObjectId {0}, "hello");
    ASSERT_EQ(store.Find(core::ObjectId {0}), std::string("hello"));
  }
  ASSERT_FALSE(store.Find(core::ObjectId {0}));
}

}  // namespace mnian::test
