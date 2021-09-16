add_executable(mntest)
target_compile_options(mntest PRIVATE ${MNIAN_CXX_FLAGS})

target_link_libraries(mntest
  PRIVATE
    gtest_main
    mncore
)
target_sources(mntest
  PRIVATE
    test.cc
)

add_test(NAME mntest COMMAND mntest)

include(GoogleTest)
gtest_discover_tests(mntest)
