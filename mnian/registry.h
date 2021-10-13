// No copyright
#pragma once

#include <cassert>

#include "mnian/editor.h"

namespace mnian {

void SetupDeserializerRegistry(core::DeserializerRegistry* reg) {
  reg->RegisterType<core::iEditor, Editor>();
}

}  // namespace mnian
