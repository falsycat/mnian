// No copyright
#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>

#include "mncore/app.h"
#include "mncore/dir.h"

#include "mnian/popup_input.h"
#include "mnian/widget.h"


namespace mnian {

class DirTreeWidget : public ImGuiWidget {
 public:
  static constexpr const char* kType = "mnian::DirTreeWidget";


  using ItemSet = std::unordered_set<core::iDirItem*>;

  class DirAddCommand;
  class DirRemoveCommand;
  class DirMoveCommand;


  static std::unique_ptr<DirTreeWidget> DeserializeParam(
      core::iDeserializer*);

  static void Register(core::DeserializerRegistry* reg);

  static std::string FindUniqueNameForNewItem(
      core::Dir*, const std::string& prefix = "NewItem1");


  DirTreeWidget() = delete;
  explicit DirTreeWidget(
      core::iApp* app, ItemSet&& sel = {}, ItemSet&& open = {});

  DirTreeWidget(const DirTreeWidget&) = delete;
  DirTreeWidget(DirTreeWidget&&) = delete;

  DirTreeWidget& operator=(const DirTreeWidget&) = delete;
  DirTreeWidget& operator=(DirTreeWidget&&) = delete;


  void Update() override;

  void SerializeParam(core::iSerializer*) const override;

 private:
  void Open(core::iDirItem*);
  void Feature(core::iDirItem*);

  void ToggleSelection(core::iDirItem*);
  void Select(core::iDirItem*);
  void SelectOnly(core::iDirItem*);
  void SelectSiblings(core::iDirItem*);
  void Deselect(core::iDirItem*);
  void DeselectAll();


  core::iApp* app_;

  core::iDirItem* featured_;

  // popup
  DirItemNameInputPopup renamer_;

  // parameters that will be saved permanently
  ItemSet selection_;
  ItemSet open_;
};

}  // namespace mnian
