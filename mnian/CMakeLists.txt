add_executable(mnian)
target_compile_options(mnian PRIVATE ${MNIAN_CXX_FLAGS})

target_sources(mnian
  PRIVATE
    main.cc
)
target_link_libraries(mnian
  PRIVATE
    mncore
    
    glew
    glfw
    imgui
)
