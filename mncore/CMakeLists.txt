add_library(mncore)
target_compile_options(mncore PRIVATE ${MNIAN_CXX_FLAGS})
target_include_directories(mncore PRIVATE "${PROJECT_SOURCE_DIR}")

target_sources(mncore
  PUBLIC
    action.h
    dir.h
    file.h
    logger.h
    serialize.h
  PRIVATE
    dir.cc
    file.cc
    serialize.cc
)
