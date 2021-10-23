find_program(XXD xxd REQUIRED)

add_library(mnres)

target_sources(mnres
  PUBLIC  all.h
  PRIVATE font.cc lang.cc
)
target_include_directories(mnres
  PRIVATE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}"
)


# create mulan file
add_mulan(lang_english lang_english.po)


set(files
  font_fontawesome_solid_900.ttf
  font_mpluscode_regular.ttf
  lang_english
)
foreach (file ${files})
  set(src "${CMAKE_CURRENT_SOURCE_DIR}/${file}")
  set(dst "${CMAKE_CURRENT_BINARY_DIR}/${file}.h")

  if (NOT EXISTS "${src}")
    set(src "${CMAKE_CURRENT_BINARY_DIR}/${file}")
  endif()

  add_custom_command(
    OUTPUT  "${dst}"
    DEPENDS "${src}"
    COMMAND cat "${src}" | ${XXD} -i > "${dst}"

    COMMENT "converting ${file} into C header..."
    VERBATIM
  )
  target_sources(mnres PRIVATE "${dst}")
endforeach()
