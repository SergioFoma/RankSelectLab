add_library(prog_error STATIC)
target_compile_options(prog_error PUBLIC -Wall -O2)
target_sources(prog_error
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
      prog_error.cppm
)
