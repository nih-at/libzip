# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.

#[=======================================================================[.rst:
Dist
-------

Provide ``dist`` and ``distcheck`` targets similar to
autoconf/automake functionality.

The ``dist`` target creates tarballs of the project in ``.tar.gz`` and
``.tar.xz`` formats.

The ``distcheck`` target extracts one of created tarballs, builds the
software using its defaults, and runs the tests.

Both targets use Unix shell commands.

The Dist target takes one argument, the file name (before the extension).

The ``distcheck`` target creates (and removes) ``${ARCHIVE_NAME}-build``
and ``${ARCHIVE_NAME}-dest``.

#]=======================================================================]
function(Dist ARCHIVE_NAME)
  if(NOT TARGET dist AND NOT TARGET distcheck)
    add_custom_target(dist
      COMMAND git config tar.tar.xz.command "xz -c"
      COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.gz HEAD
      COMMAND git archive --prefix=${ARCHIVE_NAME}/ -o ${ARCHIVE_NAME}.tar.xz HEAD
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    add_custom_target(distcheck
      COMMAND chmod -R u+w ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest 2>/dev/null || true
      COMMAND rm -rf ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
      COMMAND ${CMAKE_COMMAND} -E tar xf ${ARCHIVE_NAME}.tar.gz
      COMMAND chmod -R u-w ${ARCHIVE_NAME}
      COMMAND mkdir ${ARCHIVE_NAME}-build
      COMMAND mkdir ${ARCHIVE_NAME}-dest
      COMMAND ${CMAKE_COMMAND} -DCMAKE_INSTALL_PREFIX=${ARCHIVE_NAME}-dest ${ARCHIVE_NAME} -B ${ARCHIVE_NAME}-build
      COMMAND make -C ${ARCHIVE_NAME}-build -j4
      COMMAND make -C ${ARCHIVE_NAME}-build test
      COMMAND make -C ${ARCHIVE_NAME}-build install
      #  COMMAND make -C ${ARCHIVE_NAME}-build uninstall
      #  COMMAND if [ `find ${ARCHIVE_NAME}-dest ! -type d | wc -l` -ne 0 ]; then echo leftover files in ${ARCHIVE_NAME}-dest; false; fi
      COMMAND make -C ${ARCHIVE_NAME}-build clean
      COMMAND chmod -R u+w ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
      COMMAND rm -rf ${ARCHIVE_NAME} ${ARCHIVE_NAME}-build ${ARCHIVE_NAME}-dest
      COMMAND echo "${ARCHIVE_NAME}.tar.gz is ready for distribution."
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
      )
    add_dependencies(distcheck dist)
  endif()
endfunction()
