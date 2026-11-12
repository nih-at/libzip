# Create command table from source files containing COMMAND definitions.
# This is the implementation script used by the generate_command_table() function.
# Parameters (passed via -D):
#   SOURCES - semicolon-separated list of source file paths
#   OUTPUT_FILE - output filename (written to CMAKE_CURRENT_BINARY_DIR)

set(commands_str [=[
/*
  This file was generated automatically by CMake.
  Do not edit; make changes in the source files instead.
*/

#include "zipsh.h"

]=])

set(command_table [=[
command_t *commands[] = {
]=])

set(COMMANDS help)

foreach(source_file IN LISTS SOURCES)
  file(READ "${source_file}" source_content)
  string(REGEX MATCHALL "COMMAND[_A-Z]*\\(([^,]+), ([^\\)]+)" command_matches "${source_content}")
  foreach(match ${command_matches})
    list(APPEND COMMANDS ${CMAKE_MATCH_1})
  endforeach()
endforeach()

list(SORT COMMANDS)
foreach(command ${COMMANDS})
  string(APPEND commands_str "extern command_t command_${command}\;\n")
  string(APPEND command_table "    &command_${command},\n")
endforeach()

string(APPEND commands_str ${command_table})
string(APPEND commands_str [=[
}\;
size_t n_commands = sizeof(commands) / sizeof(commands[0])\;
]=])

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${OUTPUT_FILE} ${commands_str})
