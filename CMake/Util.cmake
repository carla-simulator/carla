# Similar to configure_file, but also expands variables
# that are set at generate time, like generator expressions.
macro (carla_two_step_configure_file DESTINATION SOURCE)
  carla_message ("Configuring file ${DESTINATION}")
  # Configure-time step; evaluate variables:
  configure_file (${SOURCE} ${DESTINATION})
  # Generate-time step; evaluate generator expressions:
  file (GENERATE OUTPUT ${DESTINATION} INPUT ${DESTINATION})
endmacro ()



# message wrapper for normal messages.
function (carla_message)
  message (STATUS "CARLA: " ${ARGN})
endfunction ()



# message wrapper for warnings.
macro (carla_warning)
  message (WARNING ${ARGN})
endmacro ()



# message wrapper for errors.
function (carla_error)
  message (FATAL_ERROR ${ARGN})
endfunction ()



macro (carla_option NAME DESCRIPTION VALUE)
  option (${NAME} ${DESCRIPTION} ${VALUE})
  carla_message ("(option) ${NAME} : ${VALUE}")
endmacro ()

macro (carla_string_option NAME DESCRIPTION VALUE)
  set (${NAME} "${VALUE}")
  carla_message ("(option) ${NAME} : \"${VALUE}\"")
endmacro ()
