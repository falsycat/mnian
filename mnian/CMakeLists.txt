add_executable(mnian)
target_compile_options(mnian PRIVATE ${MNIAN_CXX_FLAGS})
target_include_directories(mnian PRIVATE "${PROJECT_SOURCE_DIR}")

target_sources(mnian
  PRIVATE
    app.cc
    app.h
    app_lang.cc
    app_menu.h
    app_panic_popup.h
    app_project.h
    dir.h
    file.cc
    file.h
    logger.h
    main.cc
    registry.h
    widget.h
    widget_project_view.h
    widget_project_view.cc
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
    tracy

    $<$<PLATFORM_ID:Linux,Darwin>:pthread>
)
