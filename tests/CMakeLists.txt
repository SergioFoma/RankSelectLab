include(GoogleTest)

set(UNIT_TESTS unit_tests)
add_executable(${UNIT_TESTS})
target_sources(${UNIT_TESTS} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp
)
target_link_libraries(${UNIT_TESTS} PRIVATE GTest::gtest_main algorithms)

enable_testing()
