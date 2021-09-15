cmake_minimum_required(VERSION 3.20)
project(mnian CXX)

enable_testing()


# this project requires C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# this grotesque line constructs compiler flags to enable warnings
set(MNIAN_CXX_FLAGS
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall -Werror -pedantic-errors -Wextra -Wconversion -Wsign-conversion>
  $<$<CXX_COMPILER_ID:MSVC>:
    /W4 /WX>)


# add subprojects
add_subdirectory(thirdparty EXCLUDE_FROM_ALL)
add_subdirectory(mncore)
add_subdirectory(mnian)
add_subdirectory(mntest)
