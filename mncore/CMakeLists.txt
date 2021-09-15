add_library(mncore)
target_compile_options(mncore PRIVATE ${MNIAN_CXX_FLAGS})

target_sources(mncore
  PUBLIC
    version.cc
)
