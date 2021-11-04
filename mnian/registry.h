// No copyright
#pragma once

#include <cassert>
#include <memory>

#include <Tracy.hpp>

#include "mncore/dir.h"

#include "mnian/command.h"
#include "mnian/widget_dir_tree.h"
#include "mnian/widget_history_tree.h"


namespace mnian {

void SetupDeserializerRegistry(core::DeserializerRegistry* reg) {
  ZoneScoped;

  // commands
  reg->RegisterType<core::iCommand, OriginCommand>();

  // iDirItem
  reg->RegisterType<core::iDirItem, core::Dir>();
  reg->RegisterType<core::iDirItem, core::FileRef>();
  reg->RegisterType<core::iDirItem, core::NodeRef>();

  // iWidget
  DirTreeWidget::Register(reg);
  HistoryTreeWidget::Register(reg);
}

}  // namespace mnian
