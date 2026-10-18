include(GoogleTest)

set(UNIT_TESTS unit_tests)
add_executable(${UNIT_TESTS})
target_sources(${UNIT_TESTS} PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/unit_tests.cpp
)
target_link_libraries(${UNIT_TESTS} PRIVATE GTest::gtest_main algorithms)

enable_testing()

set(MODULE_FILES
  ${MODULES}/integer_keys_set.cppm
  ${MODULES}/parser.cppm
)

set(END_TO_END_TESTS end_to_end_tests)
add_executable(${END_TO_END_TESTS})
target_sources(${END_TO_END_TESTS}
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${MODULES}
    FILES
      ${MODULE_FILES}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/end_to_end.cpp
)
target_link_libraries(${END_TO_END_TESTS} PRIVATE algorithms)
