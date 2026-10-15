# create zip_err_str.c from zip.h and zipint.h
set(commands_str [=[
/*
  This file was generated automatically by CMake
  from ${SOURCES}\; make changes there.
*/

#include "zipsh.h"

extern command_t command_help\;

]=])

set(command_table [=[
command_t *commands[] = {
    &command_help,
]=])

foreach(source_file ${SOURCES})
  file(READ ${source_file} source_content)
  string(REGEX MATCHALL "COMMAND[_A-Z]*\\(([^,]+), ([^\\)]+)" command_matches ${source_content})
  foreach(match ${command_matches})
    string(APPEND commands_str "extern command_t command_${CMAKE_MATCH_1}\;\n")
    string(APPEND command_table "    &command_${CMAKE_MATCH_1},\n")
  endforeach()
endforeach()

string(APPEND commands_str ${command_table})
string(APPEND commands_str [=[
}\;
size_t n_commands = sizeof(commands) / sizeof(commands[0])\;
]=])

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} ${commands_str})
