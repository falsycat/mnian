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
    dir.cc
    dir.h
    file.cc
    file.h
    logger.cc
    logger.h
    serialize.cc
    serialize.h
)

add_test(NAME mntest COMMAND mntest)

include(GoogleTest)
gtest_discover_tests(mntest)
