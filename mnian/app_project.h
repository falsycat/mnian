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
      "param": {},
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
    ],
    "history": {
      "items": [
        {
          "id"       : 0,
          "createdAt": 0,
          "marked"   : false,
          "command"  : {"type": "mnian::NullCommand", "param": "ORIGIN"},
          "branch"   : [],
        },
      ],
      "root": 0,
      "head": 0,
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
