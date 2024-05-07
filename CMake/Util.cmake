# Similar to configure_file, but also expands variables
# that are set at generate time, like generator expressions.
function (carla_two_step_configure_file DESTINATION SOURCE)
  carla_message ("Configuring file ${DESTINATION}")
  # Configure-time step; evaluate variables:
  configure_file (${SOURCE} ${DESTINATION} @ONLY)
  # Generate-time step; evaluate generator expressions:
  file (GENERATE OUTPUT ${DESTINATION} INPUT ${DESTINATION})
endfunction ()



# message wrapper for normal messages.
function (carla_message)
  message (STATUS "CARLA: " ${ARGN})
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
  carla_message ("(option) ${NAME} : ${VALUE}")
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
  carla_message ("(option) ${NAME} : \"${VALUE}\"")
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



function (carla_get_target_docs OUT_VAR)
  get_property (DOCS GLOBAL PROPERTY CARLA_TARGET_DOCS)
  set (${OUT_VAR} ${DOCS})
  return (PROPAGATE ${OUT_VAR})
endfunction ()


function (carla_add_library NAME DESCRIPTION)
  get_property (DOCS GLOBAL PROPERTY CARLA_TARGET_DOCS)
  string (
    APPEND
    DOCS
    "- ${NAME}\n"
    "\t- Type: Library\n"
    "\t- Description: ${DESCRIPTION}\n"
  )
  add_library (${NAME} ${ARGN})
  set_property (GLOBAL PROPERTY CARLA_TARGET_DOCS ${DOCS})
endfunction ()



function (carla_add_executable NAME DESCRIPTION)
  get_property (DOCS GLOBAL PROPERTY CARLA_TARGET_DOCS)
  string (
    APPEND
    DOCS
    "- ${NAME}\n"
    "\t- Type: Executable\n"
    "\t- Description: ${DESCRIPTION}\n"
  )
  add_executable (${NAME} ${ARGN})
  set_property (GLOBAL PROPERTY CARLA_TARGET_DOCS ${DOCS})
endfunction ()



function (carla_add_custom_target NAME DESCRIPTION)
  get_property (DOCS GLOBAL PROPERTY CARLA_TARGET_DOCS)
  string (
    APPEND
    DOCS
    "- ${NAME}\n"
    "\t- Type: CustomTarget\n"
    "\t- Description: ${DESCRIPTION}\n"
  )
  add_custom_target (${NAME} ${ARGN})
  set_property (GLOBAL PROPERTY CARLA_TARGET_DOCS ${DOCS})
endfunction ()
