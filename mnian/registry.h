// No copyright
#pragma once

#include <cassert>
#include <memory>

#include <Tracy.hpp>

#include "mncore/dir.h"

#include "mnian/widget_project_view.h"


namespace mnian {

void SetupDeserializerRegistry(core::DeserializerRegistry* reg) {
  ZoneScoped;

  // TODO(falsycat): replace with Deserialize method
  reg->RegisterType<core::iDirItem, core::Dir>();
  reg->RegisterType<core::iDirItem, core::FileRef>();
  reg->RegisterType<core::iDirItem, core::NodeRef>();

  reg->RegisterType<core::iWidget, ProjectViewWidget>();
}

}  // namespace mnian
