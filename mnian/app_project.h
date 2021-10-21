// No copyright
#pragma once


namespace mnian {

static constexpr const char* kInitialProject = R"({
  "project": {
    "root": {
      "type" : "GenericDir",
      "param": {},
    },
    "nstore": [],
    "wstore": [
      {
        "id": 0,
        "entity": {
          "type" : "ProjectView",
          "param": {}
        },
      },
    ],
    "history": {
    },
  },
})";

}  // namespace mnian
