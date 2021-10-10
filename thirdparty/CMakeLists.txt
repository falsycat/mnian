include(FetchContent)

if (MNIAN_STATIC)
  set(BUILD_SHARED_LIBS OFF)
else()
  set(BUILD_SHARED_LIBS ON)
endif()


# ---- googletest ----
# repository: https://github.com/google/googletest
# license   : BSD-3-Clause

FetchContent_Declare(
  googletest
  URL "https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip"
)

set(BUILD_GMOCK   ON  CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

if (WIN32)
  # Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

if (MNIAN_BUILD_TEST)
  FetchContent_MakeAvailable(googletest)
endif()


# ---- GLEW ----
# repository: https://github.com/Perlmint/glew-cmake
# license   :  Modified BSD License, the Mesa 3-D License (MIT) and the Khronos License (MIT).

FetchContent_Declare(
  glew
  URL "https://github.com/Perlmint/glew-cmake/archive/refs/tags/glew-cmake-2.2.0.zip"
)
FetchContent_MakeAvailable(glew)

if (MNIAN_STATIC)
  add_library(glew ALIAS libglew_static)
else()
  add_library(glew ALIAS libglew_shared)
endif()


# ---- GLFW ----
# repository: https://github.com/glfw/glfw
# license   : zlib

FetchContent_Declare(
  glfw
  URL "https://github.com/glfw/glfw/archive/refs/tags/3.3.4.zip"
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)


# ---- ImGUI ----
# repository: https://github.com/ocornut/imgui/
# license   : MIT

FetchContent_Declare(
  imgui
  URL "https://github.com/ocornut/imgui/archive/refs/tags/v1.84.2.zip"
)
FetchContent_Populate(imgui)

add_library(imgui)
target_sources(imgui
  PRIVATE
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_internal.h"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
  PUBLIC
    "${imgui_SOURCE_DIR}/imgui.h"
    "${imgui_SOURCE_DIR}/imstb_rectpack.h"
    "${imgui_SOURCE_DIR}/imstb_textedit.h"
    "${imgui_SOURCE_DIR}/imstb_truetype.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
)
target_include_directories(imgui SYSTEM
  PUBLIC
    "${imgui_SOURCE_DIR}"
    "${imgui_SOURCE_DIR}/backends"
)
target_link_libraries(imgui
  PRIVATE glfw
)


# ---- RapidJSON ----
# repository: https://github.com/Tencent/rapidjson
# license   : MIT

FetchContent_Declare(
  rapidjson
  URL "https://github.com/Tencent/rapidjson/archive/2e8f5d897d9d461a7273b4b812b0127f321b1dcf.zip"
)

FetchContent_Populate(rapidjson)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson
  INTERFACE "${rapidjson_SOURCE_DIR}/include"
)


# ---- Tracy ----
# repository: https://github.com/wolfpld/tracy
# license   : BSD-3-Clause

FetchContent_Declare(
  tracy
  URL "https://github.com/wolfpld/tracy/archive/refs/tags/v0.7.8.zip"
)

FetchContent_Populate(tracy)

add_library(tracy)
target_sources(tracy
  PRIVATE
    "${tracy_SOURCE_DIR}/TracyClient.cpp"
  PUBLIC
    "${tracy_SOURCE_DIR}/Tracy.hpp"
)
target_include_directories(tracy SYSTEM
  PUBLIC "${tracy_SOURCE_DIR}"
)

if (MNIAN_USE_TRACY)
  target_compile_definitions(tracy
    PUBLIC TRACY_ENABLE
  )
endif()
