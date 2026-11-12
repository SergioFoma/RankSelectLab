add_library(parser STATIC)
target_compile_options(parser PUBLIC -Wall -O2)
target_sources(parser
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
      parser.cppm
)
target_link_libraries(parser PUBLIC prog_error integer_keys_set)
