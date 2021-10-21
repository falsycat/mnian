// No copyright
#pragma once

#include <cassert>
#include <memory>

#include <Tracy.hpp>

#include "mnian/dir.h"
#include "mnian/widget_project_view.h"


namespace mnian {

void SetupDeserializerRegistry(core::DeserializerRegistry* reg) {
  ZoneScoped;

  // TODO(falsycat): replace with Deserialize method
  reg->RegisterFactory<core::iDirItem>(
      GenericDir::kType,
      [](auto) { return std::make_unique<GenericDir>(); });

  reg->RegisterType<core::iWidget, ProjectViewWidget>();
}

}  // namespace mnian