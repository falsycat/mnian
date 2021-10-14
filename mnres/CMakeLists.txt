find_program(XXD xxd REQUIRED)

add_library(mnres)

target_sources(mnres
  PUBLIC  all.h
  PRIVATE font.cc
)
target_include_directories(mnres
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
)

set(files
  font_fontawesome_solid_900.ttf
  font_mpluscode_regular.ttf
)
foreach (file ${files})
  set(pwd "${CMAKE_CURRENT_SOURCE_DIR}")
  set(src "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  set(dst "${CMAKE_CURRENT_BINARY_DIR}/${file}.h")

  add_custom_command(
    OUTPUT  "${dst}"
    DEPENDS "${src}"
    COMMAND cat "${file}" | ${XXD} -i > "${dst}"

    WORKING_DIRECTORY "${pwd}"
    COMMENT           "converting ${file} into C header..."
    VERBATIM
  )
  target_sources(mnres PRIVATE "${dst}")
endforeach()
