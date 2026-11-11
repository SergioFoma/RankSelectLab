add_library(integer_keys_set STATIC)
target_compile_options(integer_keys_set PUBLIC -Wall -O2)
target_sources(integer_keys_set
  PUBLIC
    FILE_SET CXX_MODULES
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
    FILES
      std_set/std_set_impl.cppm
      sorted_vector/sorted_vector_impl.cppm
      avl_tree/avl_tree_impl.cppm
      b_tree/b_tree_impl.cppm
      integer_keys_set.cppm
  PRIVATE
     std_set/std_set_impl.cpp
     sorted_vector/sorted_vector_impl.cpp
     avl_tree/avl_tree_impl.cpp
     b_tree/b_tree_impl.cpp
)
target_link_libraries(integer_keys_set PUBLIC prog_error TBB::tbb)
