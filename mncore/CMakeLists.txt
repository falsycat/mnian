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
    node.h
    serialize.h
    store.h
    task.h
    widget.h
  PRIVATE
    app.cc
    command.cc
    dir.cc
    file.cc
    history.cc
    node.cc
    serialize.cc
    serialize_json.cc
    widget.cc

    $<$<PLATFORM_ID:Linux,Darwin>:file_unix.cc>
    $<$<PLATFORM_ID:Windows>:file_win.cc>
)

target_link_libraries(mncore
  PRIVATE
    rapidjson
)

target_compile_definitions(mncore
  PRIVATE
    RAPIDJSON_HAS_STDSTRING
)
