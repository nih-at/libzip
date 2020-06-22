check_function_exists(getopt HAVE_GETOPT)
if(NOT HAVE_GETOPT)
  set(SRC_EXTRA_FILES ../src/getopt.c)
include_directories(${PROJECT_SOURCE_DIR}/examples
  ${PROJECT_SOURCE_DIR}/src)
endif(NOT HAVE_GETOPT)

add_executable(in-memory in-memory.c)
target_link_libraries(in-memory zip)

include_directories(${PROJECT_SOURCE_DIR}/lib
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_BINARY_DIR})
