# message wrapper for normal messages.
function (carla_message)
  message (STATUS "CARLA: " ${ARGN})
endfunction ()

# message wrapper for normal messages.
function (carla_message_verbose)
  if (VERBOSE_CONFIGURE)
    message (STATUS "CARLA: " ${ARGN})
  endif ()
endfunction ()



# message() wrapper for warnings.
function (carla_warning)
  message (WARNING ${ARGN})
endfunction ()



# message() wrapper for errors.
function (carla_error)
  message (FATAL_ERROR ${ARGN})
endfunction ()



function (carla_get_option_docs OUT_VAR)
  get_property (DOCS GLOBAL PROPERTY CARLA_OPTION_DOCS)
  set (${OUT_VAR} ${DOCS})
  return (PROPAGATE ${OUT_VAR})
endfunction ()

macro (carla_option NAME DESCRIPTION VALUE)
  option (${NAME} ${DESCRIPTION} ${VALUE})
  carla_message_verbose ("(option) ${NAME} : ${${NAME}}")
  get_property (DOCS GLOBAL PROPERTY CARLA_OPTION_DOCS)
  string (
    APPEND
    DOCS
    "- ${NAME}\n"
    "\t- Description: ${DESCRIPTION}\n"
    "\t- Default: ${VALUE}\n"
  )
  set_property (GLOBAL PROPERTY CARLA_OPTION_DOCS ${DOCS})
endmacro ()



macro (carla_string_option NAME DESCRIPTION VALUE)
  set (${NAME} "${VALUE}")
  carla_message_verbose ("(option) ${NAME} : \"${${NAME}}\"")
  get_property (DOCS GLOBAL PROPERTY CARLA_OPTION_DOCS)
  string (
    APPEND
    DOCS
    "- ${NAME}\n"
    "\t- Description: ${DESCRIPTION}\n"
    "\t- Default: \"${VALUE}\"\n"
  )
  set_property (GLOBAL PROPERTY CARLA_OPTION_DOCS ${DOCS})
endmacro ()



# Similar to configure_file, but also expands variables
# that are set at generate time, like generator expressions.
function (carla_two_step_configure_file DESTINATION SOURCE)
  carla_message_verbose ("Configuring file ${DESTINATION}")
  # Configure-time step; evaluate variables:
  configure_file (${SOURCE} ${DESTINATION} @ONLY)
  # Generate-time step; evaluate generator expressions:
  file (GENERATE OUTPUT ${DESTINATION} INPUT ${DESTINATION})
endfunction ()



# If, for some reason, CARLA is configured with CMake<3.5, this is necessary:
if (${CMAKE_VERSION} VERSION_LESS 3.5)
  include (CMakeParseArguments)
endif ()

function (carla_add_target_docs)
  set (OPTIONS)
  set (ONE_VAL_ARGS NAME TYPE DOCS_BRIEF)
  set (MULTI_VAL_ARGS)
  cmake_parse_arguments (
    ARG
    "${OPTIONS}"
    "${ONE_VAL_ARGS}"
    "${MULTI_VAL_ARGS}"
    ${ARGN}
  )
  get_property (DOCS GLOBAL PROPERTY CARLA_TARGET_DOCS)
  string (APPEND DOCS "- ${ARG_NAME}\n")
  if (NOT ${ARG_TYPE} STREQUAL "")
    string (APPEND DOCS "\t- Type: ${ARG_TYPE}\n")
  endif ()
  if (NOT ${ARG_DOCS_BRIEF} STREQUAL "")
    string (APPEND DOCS "\t- Description: ${ARG_DOCS_BRIEF}\n")
  endif ()
  set_property (GLOBAL PROPERTY CARLA_TARGET_DOCS ${DOCS})
endfunction ()



function (carla_get_target_docs OUT_VAR)
  get_property (DOCS GLOBAL PROPERTY CARLA_TARGET_DOCS)
  set (${OUT_VAR} ${DOCS})
  return (PROPAGATE ${OUT_VAR})
endfunction ()



function (carla_get_targets OUT_VAR)
  get_property (TARGETS GLOBAL PROPERTY CARLA_TARGETS)
  set (${OUT_VAR} ${TARGETS})
  return (PROPAGATE ${OUT_VAR})
endfunction ()




function (carla_add_library NAME DESCRIPTION)
  carla_add_target_docs (
    NAME ${NAME}
    TYPE Library
    DOCS_BRIEF ${DESCRIPTION}
  )
  add_library (${NAME} ${ARGN})
  get_property (TARGETS GLOBAL PROPERTY CARLA_TARGETS)
  list (APPEND TARGETS ${NAME})
  set_property (GLOBAL PROPERTY CARLA_TARGETS ${TARGETS})
endfunction ()



function (carla_add_executable NAME DESCRIPTION)
  carla_add_target_docs (
    NAME ${NAME}
    TYPE Executable
    DOCS_BRIEF ${DESCRIPTION}
  )
  add_executable (${NAME} ${ARGN})
  get_property (TARGETS GLOBAL PROPERTY CARLA_TARGETS)
  list (APPEND TARGETS ${NAME})
  set_property (GLOBAL PROPERTY CARLA_TARGETS ${TARGETS})
endfunction ()



function (carla_add_custom_target NAME DESCRIPTION)
  carla_add_target_docs (
    NAME ${NAME}
    TYPE CustomTarget
    DOCS_BRIEF ${DESCRIPTION}
  )
  add_custom_target (${NAME} ${ARGN})
  get_property (TARGETS GLOBAL PROPERTY CARLA_TARGETS)
  list (APPEND TARGETS ${NAME})
  set_property (GLOBAL PROPERTY CARLA_TARGETS ${TARGETS})
endfunction ()



function (carla_add_library_alias NAME ORIGINAL_TARGET)
  carla_add_target_docs (
    NAME ${NAME}
    TYPE LibraryAlias
    DOCS_BRIEF "Alias for \"${ORIGINAL_TARGET}\"."
  )
  add_library (${NAME} ALIAS ${ORIGINAL_TARGET} ${ARGN})
endfunction ()



function (carla_add_executable_alias NAME ORIGINAL_TARGET)
  carla_add_target_docs (
    NAME ${NAME}
    TYPE ExecutableAlias
    DOCS_BRIEF "Alias for \"${ORIGINAL_TARGET}\"."
  )
  add_executable (${NAME} ALIAS ${ORIGINAL_TARGET} ${ARGN})
endfunction ()



function (carla_add_custom_target_alias NAME ORIGINAL_TARGET)
  carla_add_target_docs (
    NAME ${NAME}
    TYPE CustomTargetAlias
    DOCS_BRIEF "Alias for \"${ORIGINAL_TARGET}\"."
  )
  add_custom_target (${NAME} DEPENDS ${ORIGINAL_TARGET})
endfunction ()



function (carla_create_symlink ${DESTINATION} ${SOURCE})
  execute_process (
    COMMAND
      ${CMAKE_COMMAND}
        -E create_symlink
        ${SOURCE}
        ${DESTINATION}
    RESULT_VARIABLE
      CREATE_SYMLINK_RESULT
  )
  if (${CREATE_SYMLINK_RESULT})
    carla_error ("Failed to create symlink from ${SOURCE} to ${DESTINATION}")
  endif ()
endfunction ()



if (VERBOSE_CONFIGURE)
  macro (carla_print_cmake_variable NAME)
    carla_message ("${NAME}: \'${${NAME}}\'")
  endmacro ()

  carla_print_cmake_variable (CMAKE_C_COMPILER)
  carla_print_cmake_variable (CMAKE_CXX_COMPILER)
  carla_print_cmake_variable (CMAKE_ASM_COMPILER)
  carla_print_cmake_variable (CMAKE_AR)
  carla_print_cmake_variable (CMAKE_C_COMPILER_AR)
  carla_print_cmake_variable (CMAKE_CXX_COMPILER_AR)
  carla_print_cmake_variable (CMAKE_OBJCOPY)
  carla_print_cmake_variable (CMAKE_ADDR2LINE)
  carla_print_cmake_variable (CMAKE_C_COMPILER_RANLIB)
  carla_print_cmake_variable (CMAKE_CXX_COMPILER_RANLIB)
  carla_print_cmake_variable (CMAKE_LINKER)
  carla_print_cmake_variable (CMAKE_NM)
  carla_print_cmake_variable (CMAKE_OBJDUMP)
  carla_print_cmake_variable (CMAKE_RANLIB)
  carla_print_cmake_variable (CMAKE_READELF)
  carla_print_cmake_variable (CMAKE_STRIP)
  carla_print_cmake_variable (COVERAGE_COMMAND)
  carla_print_cmake_variable (CMAKE_CXX_STANDARD_LIBRARIES)
  carla_print_cmake_variable (CMAKE_CXX_STANDARD_INCLUDE_DIRECTORIES)
endif ()



carla_add_target_docs (
  NAME clean
  TYPE Builtin
  DOCS_BRIEF "Removes all build directories and files."
)
