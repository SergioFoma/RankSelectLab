include(GoogleTest)

add_executable(unit_tests)
target_compile_options(unit_tests PRIVATE -Wall -O2)
target_sources(unit_tests PRIVATE unit_tests.cpp)
target_link_libraries(unit_tests PRIVATE GTest::gtest_main integer_keys_set)
gtest_discover_tests(unit_tests)

set(end_to_end_tests)
add_executable(end_to_end_tests)
target_sources(end_to_end_tests PRIVATE end_to_end.cpp)
target_link_libraries(end_to_end_tests PRIVATE
  prog_error
  integer_keys_set
  parser
  fmt::fmt
  CLI11::CLI11
)
