macro (carla_message)
  message (STATUS "CARLA: " ${ARGN})
endmacro ()



macro (carla_error)
  message (FATAL_ERROR ${ARGN})
endmacro ()
