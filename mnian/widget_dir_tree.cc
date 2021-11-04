// No copyright
#include "mnian/widget_dir_tree.h"
#include "mnian/widget_dir_tree_command.h"

#include <fontawesome.h>
#include <imgui.h>

#include <cinttypes>
#include <string>
#include <vector>

#include <Tracy.hpp>

#include "mncore/dir.h"

#include "mnian/app.h"


namespace mnian {

static inline std::optional<std::vector<core::iDirItem*>> DeserializeItems(
    core::iDeserializer* des) {
  const auto n = des->size();
  if (!n) return std::nullopt;

  std::vector<core::iDirItem*> ret;
  for (size_t i = 0; i < *n; ++i) {
    core::iDeserializer::ScopeGuard dummy(des, i);

    auto item = core::iDirItem::DeserializeRef(des);
    if (!item) continue;
    ret.push_back(item);
  }
  return ret;
}


void DirTreeWidget::Register(core::DeserializerRegistry* reg) {
  reg->RegisterType<core::iWidget, DirTreeWidget>();
  reg->RegisterType<core::iCommand, DirCommand>();
  reg->RegisterType<core::iCommand, DirMoveCommand>();
}

std::string DirTreeWidget::FindUniqueNameForNewItem(
    core::Dir* dir, const std::string& prefix) {
  if (!dir->Find(prefix)) return prefix;

  const size_t pmax = prefix.size();

  // find an index of first char of suffix number
  size_t plen = pmax;
  for (size_t i = 0; i < pmax; ++i) {
    if (std::isdigit(prefix[i])) {
      if (plen == pmax) plen = i;
    } else {
      plen = pmax;
    }
  }

  const std::string::difference_type off =
      static_cast<std::string::difference_type>(plen);

  const std::string p(prefix.begin(), prefix.begin()+off);
  const std::string n(prefix.begin()+off, prefix.end());

  char* end;
  uintmax_t num = strtoumax(n.c_str(), &end, 0);
  if (num > UINT32_MAX) num = 0;

  // find unique name by incrementing suffix number
  std::string ret;
  for (size_t i = static_cast<size_t>(num)+1;; ++i) {
    ret = p + std::to_string(i);
    if (!dir->Find(ret)) return ret;
  }
}


DirTreeWidget::DirTreeWidget(core::iApp* app, ItemSet&& sel, ItemSet&& open) :
    ImGuiWidget(kType),
    app_(app),
    renamer_("renamer", _r("Enter new name: ")),
    selection_(std::move(sel)), open_(std::move(open)) {
  assert(app_);
}


void DirTreeWidget::Update() {
  ZoneScoped;

  // update popup
  renamer_.Update();

  // update tree nodes
  class UpdateChild : public core::iDirItemVisitor {
   public:
    UpdateChild(DirTreeWidget*     w,
                const std::string& path,
                core::iDirItem*    target) :
        w_(w), path_(path), target_(target) {
      target->Visit(this);
    }
    UpdateChild(DirTreeWidget* w, core::Dir* root) :
        w_(w), path_("/"), target_(root) {
      ImGui::Selectable(_("[ROOT]"));

      // popup menu
      if (ImGui::BeginPopupContextItem()) {
        MenuForChildAddition(root);
        MenuForModificationOfSelected();
        ImGui::EndPopup();
      }
      ItemDropTarget(root);

      for (auto& itr : root->items()) {
        auto& item = *itr.second;
        UpdateChild(w_, "/"+item.name(), &item);
      }
    }

    void VisitDir(core::Dir* dir) override {
      const auto n = dir->items().size();
      PushTreeItem(_("[D] %s"), n == 0? ImGuiTreeNodeFlags_Leaf: 0);

      // tooltip
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(
            _("path: %s\ntype: directory\nsize: %zu"), path_.c_str(), n);
      }

      // popup menu
      if (ImGui::BeginPopupContextItem()) {
        w_->SelectOnly(target_);
        MenuForChildAddition(dir);
        MenuForSiblingInsertion();
        MenuForModificationOfTarget();
        MenuForModificationOfSelected();
        MenuForSelection();
        ImGui::EndPopup();
      }
      ItemDropTarget(dir);

      // save open state
      if (open_ && n) {
        w_->open_.insert(target_);
      } else {
        w_->open_.erase(target_);
      }

      // update children
      if (open_) {
        for (auto& itr : dir->items()) {
          auto& item = *itr.second;
          UpdateChild(w_, path_+"/"+item.name(), &item);
        }
      }

      PopTreeItem();
    }

    void VisitNode(core::NodeRef*) override {
      PushTreeItem(_("[N] %s"), ImGuiTreeNodeFlags_Leaf);

      // tooltip
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("type: node"));
      }

      // popup menu
      if (ImGui::BeginPopupContextItem()) {
        w_->SelectOnly(target_);
        MenuForSiblingInsertion();
        MenuForModificationOfTarget();
        MenuForModificationOfSelected();
        MenuForSelection();
        ImGui::EndPopup();
      }

      PopTreeItem();
    }

    void VisitFile(core::FileRef*) override {
      PushTreeItem(_("[F] %s"), ImGuiTreeNodeFlags_Leaf);

      // tooltip
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(_("type: file"));
      }

      // popup menu
      if (ImGui::BeginPopupContextItem()) {
        w_->SelectOnly(target_);
        MenuForSiblingInsertion();
        MenuForModificationOfTarget();
        MenuForModificationOfSelected();
        MenuForSelection();
        ImGui::EndPopup();
      }

      PopTreeItem();
    }

   private:
    void Add(core::Dir*                        dir,
             const std::string&                name,
             std::unique_ptr<core::iDirItem>&& item) {
      w_->app_->ExecCommand(
          std::make_unique<DirCommand>(w_, dir, name, std::move(item)));
    }

    void Clone() {
      auto parent = &target_->parent();

      const auto name = FindUniqueNameForNewItem(parent, target_->name());
      Add(&target_->parent(), name, target_->Clone());
    }

    void Rename() {
      auto dir      = &target_->parent();
      auto old_name = target_->name();

      w_->renamer_.Open(
          dir, target_,
          [w = w_, dir, old_name] (const std::string& new_name) {
            w->app_->ExecCommand(
                std::make_unique<DirMoveCommand>(dir, old_name, dir, new_name));
          });
    }
    void Move(core::iDirItem* item, core::Dir* dir) {
      if (dir->Find(item->name())) {
        return;
      }
      w_->app_->ExecCommand(
          std::make_unique<DirMoveCommand>(
              &item->parent(), item->name(), dir, item->name()));
    }

    void Remove() {
      auto dir = &target_->parent();

      w_->app_->ExecCommand(
          std::make_unique<DirCommand>(w_, dir, target_->name()));
    }
    void RemoveSelection() {
      for (auto item : w_->selection_) {
        bool skip = false;

        // skips removal if the parent will also be removed
        auto itr = item;
        while (!itr->isRoot()) {
          itr = &itr->parent();
          if (w_->selection_.contains(itr)) {
            skip = true;
            break;
          }
        }
        if (skip) continue;

        w_->app_->ExecCommand(
            std::make_unique<DirCommand>(w_, &item->parent(), item->name()));
      }
    }


    void PushTreeItem(const char* str, ImGuiTreeNodeFlags flags = 0) {
      flags |=
          ImGuiTreeNodeFlags_SpanFullWidth |
          ImGuiTreeNodeFlags_NoTreePushOnOpen;

      // check if selected
      if (w_->selection_.contains(target_)) {
        flags |= ImGuiTreeNodeFlags_Selected;
      }

      // set open state
      ImGui::SetNextItemOpen(w_->open_.contains(target_));

      // push tree node
      open_ = ImGui::TreeNodeEx(target_, flags, str, target_->name().c_str());
      ImGui::TreePush(target_);

      rc_min_ = ImGui::GetItemRectMin();
      rc_max_ = ImGui::GetItemRectMax();

      // scroll to the featured item
      if (featured()) {
        ImGui::SetScrollHereY(.5f);
      }

      // handle selection
      if (ImGui::IsItemClicked()) {
        if (!ImGui::GetIO().KeyCtrl) {
          w_->app_->mainQ().Exec([w = w_]() { w->DeselectAll(); });
        }
        w_->app_->mainQ().Exec(
            [w = w_, item = target_]() { w->ToggleSelection(item); });
      }

      // dragdrop source
      static constexpr auto kDragDropFlags =
          ImGuiDragDropFlags_SourceAutoExpirePayload;
      if (ImGui::BeginDragDropSource(kDragDropFlags)) {
        const uintptr_t ptr = reinterpret_cast<uintptr_t>(target_);
        ImGui::SetDragDropPayload("mnian::core::iDirItem", &ptr, sizeof(ptr));
        ImGui::Text(_("DirItem (%s)"), path_.c_str());
        ImGui::EndDragDropSource();
      }
    }
    void PopTreeItem() {
      ImGui::TreePop();

      if (featured()) {
        w_->featured_ = nullptr;
      }
    }

    void ItemDropTarget(core::Dir* dir) {
      if (ImGui::BeginDragDropTarget()) {
        static constexpr auto kDragDropFlags =
            ImGuiDragDropFlags_AcceptBeforeDelivery;
        const auto payload = ImGui::AcceptDragDropPayload(
            "mnian::core::iDirItem",
            kDragDropFlags);
        if (payload) {
          assert(payload->DataSize == sizeof(core::iDirItem*));

          auto item = reinterpret_cast<core::iDirItem*>(
              *reinterpret_cast<uintptr_t*>(payload->Data));

          if (item->IsAncestorOf(*dir)) {
            ImGui::Text(_("isn't your parent you?"));
          } else if (&item->parent() == dir) {
            ImGui::Text(_("changes nothing"));
          } else if (dir->Find(item->name())) {
            ImGui::Text(_("name duplication"));
          } else {
            ImGui::Text(_("move to here?"), path_.c_str());
            if (payload->IsDelivery()) Move(item, dir);
          }
        }
        ImGui::EndDragDropTarget();
      }
    }

    void MenuForSiblingInsertion() {
      if (ImGui::BeginMenu(_("Insert"))) {
        SubMenuForAddition(&target_->parent());
        ImGui::EndMenu();
      }
    }
    void MenuForChildAddition(core::Dir* dir) {
      if (ImGui::BeginMenu(_("Add"))) {
        SubMenuForAddition(dir);
        ImGui::EndMenu();
      }
    }
    void SubMenuForAddition(core::Dir* target) {
      if (ImGui::Selectable(_("new directory"))) {
        const auto name = FindUniqueNameForNewItem(target);
        Add(target, name, std::make_unique<core::Dir>());
      }
      if (ImGui::Selectable(_("new node"))) {
        // TODO(falsycat)
      }
      if (ImGui::BeginMenu(_("new file"))) {
        if (ImGui::Selectable(_("from existing native file"))) {
          // TODO(falsycat)
        }
        if (ImGui::Selectable(_("from new native file"))) {
          // TODO(falsycat)
        }
        if (ImGui::Selectable(_("from URL"))) {
          // TODO(falsycat)
        }
        ImGui::EndMenu();
      }
    }
    void MenuForModificationOfTarget() {
      if (ImGui::Selectable(_("Rename"))) {
        Rename();
      }
      if (ImGui::Selectable(_("Clone"))) {
        Clone();
      }
      if (ImGui::Selectable(_("Remove"))) {
        Remove();
      }
    }
    void MenuForModificationOfSelected() {
      if (w_->selection_.size() == 0) return;

      ImGui::Separator();
      if (ImGui::Selectable(_("Remove selected items"))) {
        RemoveSelection();
      }
    }
    void MenuForSelection() {
      ImGui::Separator();
      if (selected()) {
        if (ImGui::Selectable(_("Deselect"))) {
          w_->app_->mainQ().Exec(
              [w = w_, item = target_]() { w->Deselect(item); });
        }
      } else {
        if (ImGui::Selectable(_("Select"))) {
          w_->app_->mainQ().Exec(
              [w = w_, item = target_]() { w->Select(item); });
        }
      }
      if (w_->selection_.size()) {
        if (ImGui::Selectable(_("Deselect all"))) {
          w_->app_->mainQ().Exec([w = w_]() { w->DeselectAll(); });
        }
      }
      if (!target_->isRoot() && target_->parent().items().size() >= 2) {
        if (ImGui::Selectable(_("Select all siblings"))) {
          w_->app_->mainQ().Exec(
              [w = w_, item = target_]() { w->SelectSiblings(item); });
        }
      }
    }


    bool featured() const {
      return w_->featured_ == target_;
    }
    bool selected() const {
      return w_->selection_.contains(target_);
    }


    DirTreeWidget* w_;

    std::string path_;

    core::iDirItem* target_;

    bool open_;

    ImVec2 rc_min_;
    ImVec2 rc_max_;
  };

  if (ImGui::Begin(strId().c_str())) {
    UpdateChild(this, &app_->project().root());
  }
  ImGui::End();
}


void DirTreeWidget::Open(core::iDirItem* itr) {
  for (;;) {
    open_.insert(itr);
    if (itr->isRoot()) break;
    itr = &itr->parent();
  }
}

void DirTreeWidget::Feature(core::iDirItem* item) {
  if (item->isRoot()) return;

  featured_ = item;
  Open(&item->parent());

  selection_.clear();
  selection_.insert(item);
}


void DirTreeWidget::ToggleSelection(core::iDirItem* item) {
  auto itr = selection_.find(item);
  if (itr != selection_.end()) {
    selection_.erase(itr);
  } else {
    selection_.insert(item);
  }
}

void DirTreeWidget::Select(core::iDirItem* item) {
  selection_.insert(item);
}

void DirTreeWidget::SelectOnly(core::iDirItem* item) {
  selection_ = {item};
}

void DirTreeWidget::SelectSiblings(core::iDirItem* item) {
  for (auto& itr : item->parent().items()) {
    selection_.insert(itr.second.get());
  }
}

void DirTreeWidget::Deselect(core::iDirItem* item) {
  selection_.erase(item);
}

void DirTreeWidget::DeselectAll() {
  selection_.clear();
}


std::unique_ptr<DirTreeWidget> DirTreeWidget::DeserializeParam(
    core::iDeserializer* des) {
  assert(des);

  // recover open
  des->Enter(std::string("open"));
  auto open = DeserializeItems(des);
  des->Leave();
  if (!open) open = std::vector<core::iDirItem*>{};

  // recover open
  des->Enter(std::string("selection"));
  auto sel = DeserializeItems(des);
  des->Leave();
  if (!sel) sel = std::vector<core::iDirItem*>{};

  return std::make_unique<DirTreeWidget>(
      &des->app(),
      ItemSet(sel->begin(), sel->end()),
      ItemSet(open->begin(), open->end()));
}

void DirTreeWidget::SerializeParam(core::iSerializer* serial) const {
  assert(serial);

  serial->SerializeMap(2);

  serial->SerializeKey("open");
  serial->SerializeArray(open_.size());

  for (auto item : open_) {
    const auto path = item->GeneratePath();
    serial->SerializeArray(path.size());
    for (const auto& term : path) {
      serial->SerializeValue(term);
    }
  }

  serial->SerializeKey("selection");
  serial->SerializeArray(selection_.size());

  for (auto item : selection_) {
    const auto path = item->GeneratePath();
    serial->SerializeArray(path.size());
    for (const auto& term : path) {
      serial->SerializeValue(term);
    }
  }
}

}  // namespace mnian
