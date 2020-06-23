check_function_exists(getopt HAVE_GETOPT)
if(NOT HAVE_GETOPT)
  set(SRC_EXTRA_FILES ../src/getopt.c)
endif(NOT HAVE_GETOPT)

add_executable(in-memory in-memory.c)
target_link_libraries(in-memory zip)
target_include_directories(in-memory PRIVATE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR})
