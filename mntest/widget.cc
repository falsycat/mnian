// No copyright
#include "mncore/widget.h"

#include <gtest/gtest.h>


namespace mnian::test {

TEST(WidgetMap, Bind) {
  core::WidgetMap wmap;

  auto wa  = (core::iWidget*) 0x01;
  auto wb  = (core::iWidget*) 0x02;

  auto da  = (core::iDirItem*) 0x01;
  auto db  = (core::iDirItem*) 0x02;

  auto na  = (core::iNode*) 0x01;
  auto nb  = (core::iNode*) 0x02;

  wmap.Bind(wa, da);
  wmap.Bind(wa, na);

  wmap.Bind(wb, da);
  wmap.Bind(wb, nb);

  ASSERT_EQ(wmap.Find(da), (std::unordered_set<core::iWidget*> {wa, wb}));
  ASSERT_EQ(wmap.Find(na), std::unordered_set<core::iWidget*> {wa});
  ASSERT_EQ(wmap.Find(nb), std::unordered_set<core::iWidget*> {wb});
  ASSERT_EQ(wmap.Find(db), std::unordered_set<core::iWidget*> {});
}

TEST(WidgetMap, Forget) {
  core::WidgetMap wmap;

  auto wa  = (core::iWidget*) 0x01;
  auto wb  = (core::iWidget*) 0x02;

  auto da = (core::iDirItem*) 0x01;

  auto na  = (core::iNode*) 0x01;
  auto nb  = (core::iNode*) 0x02;

  wmap.Bind(wa, da);
  wmap.Bind(wa, na);

  wmap.Bind(wb, da);
  wmap.Bind(wb, nb);

  wmap.Forget(wb);

  ASSERT_EQ(wmap.Find(da), std::unordered_set<core::iWidget*> {wa});
  ASSERT_EQ(wmap.Find(na), std::unordered_set<core::iWidget*> {wa});
  ASSERT_EQ(wmap.Find(nb), std::unordered_set<core::iWidget*> {});
}

}  // namespace mnian::test
