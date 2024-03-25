function (carla_message)
  message (STATUS "CARLA: " ${ARGN})
endfunction ()

macro (carla_warning)
  message (WARNING ${ARGN})
endmacro ()

macro (carla_error)
  message (FATAL_ERROR ${ARGN})
endmacro ()

macro (carla_string_option NAME DESCRIPTION VALUE)
  set (${NAME} "${VALUE}" CACHE STRING ${DESCRIPTION})
endmacro ()
