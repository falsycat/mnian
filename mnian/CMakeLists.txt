add_executable(mnian)
target_include_directories(mnian PRIVATE "${PROJECT_SOURCE_DIR}")

target_compile_options(mnian
  PRIVATE
    ${MNIAN_CXX_FLAGS}

    # prevent a compiler to warn when gettext is used as format string
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
      -Wno-format-security>
)

target_sources(mnian
  PRIVATE
    app.cc
    app.h
    app_project.h
    command.h
    file.cc
    file.h
    lang.cc
    lang.h
    logger.h
    main.cc
    popup_input.cc
    popup_input.h
    registry.h
    widget.h
    widget_dir_tree.cc
    widget_dir_tree.h
    widget_dir_tree_command.h
    worker.cc
    worker.h
)
target_link_libraries(mnian
  PRIVATE
    mncore
    mnres
    
    fontawesome
    glew
    glfw
    imgui
    libmulan
    linalg.h
    tracy

    $<$<PLATFORM_ID:Linux,Darwin>:pthread>
)
