add_subdirectory(${SRC_ALGORITHMS})

set(TARGET target)
add_executable(${TARGET})
target_sources(${TARGET} PRIVATE ${SRC}/main.cpp)
target_include_directories(${TARGET} PRIVATE ${INCLUDE})
target_link_libraries(${TARGET} PRIVATE algorithms)
