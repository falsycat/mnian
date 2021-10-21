// No copyright
#include "mnian/app.h"

#include <fontawesome.h>

#include <Tracy.hpp>


namespace mnian {

void App::BuildDefaultLang() {
  ZoneScopedN("build default language");
# define _(key, value) lang().Add(core::Lang::Hash(key), value)

  _("ERR_FILE_OPEN",   "Failed to open file.");
  _("ERR_PARSE_JSON",  "Failed to parse JSON.");
  _("ERR_DESERIALIZE", "Failed to deserialize an object.");

  _("PANIC_TITLE", ICON_FA_SKULL_CROSSBONES);
  _("PANIC_ABORT", "ABORT");

  _("MENU_APP",      ICON_FA_STREAM       " App");
  _("MENU_APP_SAVE", ICON_FA_SAVE         " Save");
  _("MENU_APP_QUIT", ICON_FA_TIMES_CIRCLE " Quit");

# undef _
}

}  // namespace mnian
