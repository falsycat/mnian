add_executable(mnian)
target_compile_options(mnian PRIVATE ${MNIAN_CXX_FLAGS})
target_include_directories(mnian PRIVATE "${PROJECT_SOURCE_DIR}")

target_sources(mnian
  PRIVATE
    app.cc
    app.h
    dir.h
    editor.cc
    editor.h
    file.cc
    file.h
    logger.h
    main.cc
    registry.h
    widget_project_view.h
    widget_project_view.cc
)
target_link_libraries(mnian
  PRIVATE
    mncore
    
    glew
    glfw
    imgui
    tracy
)
