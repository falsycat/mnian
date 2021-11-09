add_executable(mntest)
target_include_directories(mntest PRIVATE ${PROJECT_SOURCE_DIR})
target_compile_options(mntest PRIVATE ${MNIAN_CXX_FLAGS})

target_link_libraries(mntest
  PRIVATE
    gmock_main
    gtest_main

    mncore
)
target_sources(mntest
  PRIVATE
    action.cc
    action.h
    app.h
    command.cc
    command.h
    conv.cc
    dir.cc
    dir.h
    file.cc
    file.h
    history.cc
    logger.cc
    logger.h
    node.h
    serialize.cc
    serialize.h
    store.cc
    task.cc
    task.h
)

include(GoogleTest)
gtest_discover_tests(mntest PROPERTIES TIMEOUT 30)
