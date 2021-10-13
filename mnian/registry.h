// No copyright
#pragma once

#include <cassert>
#include <memory>

#include "mnian/dir.h"
#include "mnian/editor.h"
#include "mnian/widget_project_view.h"


namespace mnian {

void SetupDeserializerRegistry(core::DeserializerRegistry* reg) {
  reg->RegisterType<core::iEditor, Editor>();

  // TODO(falsycat): replace with Deserialize method
  reg->RegisterFactory<core::iDirItem>(
      GenericDir::kType,
      [](auto) { return std::make_unique<GenericDir>(); });

  reg->RegisterType<core::iWidget, ProjectViewWidget>();
}

}  // namespace mnian
