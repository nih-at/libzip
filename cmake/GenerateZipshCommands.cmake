# create zip_err_str.c from zip.h and zipint.h
set(zipsh_commands_str [=[
/*
  This file was generated automatically by CMake
  from C files in src/zipsh\; make changes there.
*/

#include "zipsh.h"

]=])

set(command_table [=[
command_t *commands[] = {
]=])

file(GLOB FILES ${PROJECT_SOURCE_DIR}/src/zipsh/*.c)
foreach(source_file ${FILES})
  file(READ ${source_file} source_content)
  string(REGEX MATCHALL "COMMAND[_A-Z]*\\(([^,]+), ([^\\)]+)" command_matches ${source_content})
  foreach(match ${command_matches})
    string(APPEND zipsh_commands_str "extern command_t command_${CMAKE_MATCH_1}\;\n")
    string(APPEND command_table "    &command_${CMAKE_MATCH_1},\n")
  endforeach()
endforeach()

string(APPEND zipsh_commands_str ${command_table})
string(APPEND zipsh_commands_str [=[
}\;
size_t n_commands = sizeof(commands) / sizeof(commands[0])\;
]=])

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/zipsh_commands.c ${zipsh_commands_str})
