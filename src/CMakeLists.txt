add_subdirectory(prog_error)
add_subdirectory(integer_keys_set)
add_subdirectory(parser)

add_executable(main)
add_compile_options(main PRIVATE -O2 -Wall)
target_sources(main PRIVATE main.cpp)
target_link_libraries(main PRIVATE prog_error integer_keys_set parser CLI11::CLI11)
