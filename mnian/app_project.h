// No copyright
#pragma once

#include "mnian/app.h"

#include <sstream>

#include <Tracy.hpp>

#include "mncore/serialize.h"


namespace mnian {

static constexpr const char* kInitialProject = R"({
  "project": {
    "root": {
      "type" : "mnian::core::Dir",
      "param": {
        "id"   : 0,
        "items": [],
      },
    },
    "nstore": [],
    "wstore": [
      {
        "id": 0,
        "entity": {
          "type" : "mnian::DirTreeWidget",
          "param": {}
        },
      },
      {
        "id": 1,
        "entity": {
          "type" : "mnian::HistoryTreeWidget",
          "param": {}
        },
      },
    ],
    "history": {
      "commands": [],
      "origin"  : [],
      "head"    : [],
    },
  },
})";

void App::LoadInitialProject() {
  ZoneScopedN("load initial project");

  std::stringstream st(kInitialProject);

  auto des =
      core::iDeserializer::CreateJson(this, &logger(), &registry(), &st);
  assert(des);

  if (!Deserialize(des.get())) {
    assert(false);
  }
}

}  // namespace mnian
