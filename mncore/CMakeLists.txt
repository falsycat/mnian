add_library(mncore)
target_compile_options(mncore PRIVATE ${MNIAN_CXX_FLAGS})
target_include_directories(mncore PRIVATE "${PROJECT_SOURCE_DIR}")

target_sources(mncore
  PUBLIC
    action.h
    app.h
    clock.h
    command.h
    conv.h
    dir.h
    file.h
    logger.h
    serialize.h
  PRIVATE
    command.cc
    dir.cc
    file.cc
    serialize.cc
)
