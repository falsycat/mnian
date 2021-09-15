# ---- fetch googletest ----
include(FetchContent)
FetchContent_Declare(
  googletest
  URL "https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip"
)

set(BUILD_GMOCK   OFF CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

if (WIN32)
  # Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)
include(GoogleTest)


# ---- add tests ----
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
gtest_discover_tests(mntest)
