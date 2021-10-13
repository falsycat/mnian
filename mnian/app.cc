// No copyright
#include "mnian/app.h"

#include <sstream>

#include <Tracy.hpp>

#include "mncore/serialize.h"


namespace mnian {

static constexpr const char* kInitialProject = R"({
  "editor": {
    "type" : "ImGUI",
    "param": {
      "widgets": [
        {
          "type" : "ProjectView",
          "param": {
          }
        },
      ],
    },
  },
  "root": {
    "type" : "GenericDir",
    "param": {
    },
  },
  "history": {
  },
})";


void App::Load(const std::string&) {
  assert(false);
}

void App::Save() {
  assert(false);
}


void App::Reset() {
  ZoneScoped;

  std::stringstream st(kInitialProject);
  auto des = core::iDeserializer::CreateJson(this, &logger(), &registry(), &st);
  project().Deserialize(des.get());
}


void App::Update() {
  ZoneScoped;

  project().editor().Update();
}

}  // namespace mnian
