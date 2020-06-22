include_directories(BEFORE ${PROJECT_SOURCE_DIR}/lib ${PROJECT_BINARY_DIR})

check_function_exists(getopt HAVE_GETOPT)
if(NOT HAVE_GETOPT)
  set(SRC_EXTRA_FILES getopt.c)
endif(NOT HAVE_GETOPT)

add_executable(zipcmp zipcmp.c ${SRC_EXTRA_FILES})
target_link_libraries(zipcmp zip ${FTS_LIB} ${ZLIB_LIBRARIES})
if(LIBZIP_DO_INSTALL)
  install(TARGETS zipcmp EXPORT ${PROJECT_NAME}-targets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(zipmerge zipmerge.c ${SRC_EXTRA_FILES})
target_link_libraries(zipmerge zip)
if(LIBZIP_DO_INSTALL)
  install(TARGETS zipmerge EXPORT ${PROJECT_NAME}-targets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

add_executable(ziptool ziptool.c ${SRC_EXTRA_FILES})
target_link_libraries(ziptool zip)
if(LIBZIP_DO_INSTALL)
  install(TARGETS ziptool EXPORT ${PROJECT_NAME}-targets RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
