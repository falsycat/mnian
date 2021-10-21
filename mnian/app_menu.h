// No copyright
#pragma once
#include "mnian/app.h"

#include <imgui.h>

#include <cassert>

#include "mncore/lang.h"


namespace mnian {


class App::Menu final {
 public:
  using L = core::Lang;


  Menu() = delete;

# define _(name) name##_(&owner_->lang(), L::Hash("MENU_"#name))
  explicit Menu(App* owner) : owner_(owner),
      _(app),
      _(app_save),
      _(app_quit) {
    assert(owner_);
  }
# undef _

  Menu(const Menu&) = delete;
  Menu(Menu&&) = delete;

  Menu& operator=(const Menu&) = delete;
  Menu& operator=(Menu&&) = delete;


  void Update() {
    if (ImGui::BeginMainMenuBar()) {
      if (ImGui::BeginMenu(app_.c())) {
        if (ImGui::MenuItem(app_save_.c())) { owner_->Save(); }
        if (ImGui::MenuItem(app_quit_.c())) { owner_->Quit(); }
        ImGui::EndMenu();
      }
      ImGui::EndMainMenuBar();
    }
  }

 private:
  App* owner_;

  L::Text app_;
  L::Text app_save_;
  L::Text app_quit_;
};


}  // namespace mnian
