function (carla_message)
  message (STATUS "CARLA: " ${ARGN})
endfunction ()

macro (carla_warning)
  message (WARNING ${ARGN})
endmacro ()

function (carla_error)
  message (FATAL_ERROR ${ARGN})
endfunction ()

macro (carla_string_option NAME DESCRIPTION VALUE)
  set (${NAME} "${VALUE}" CACHE STRING ${DESCRIPTION})
endmacro ()
