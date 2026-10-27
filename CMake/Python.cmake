#[[
  Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
  de Barcelona (UAB). Licensed under the MIT license.
]]

# Boost uses FindPython; the bindings use FindPython3. Select one interpreter
# before configuring either consumer so their versioned targets agree.
macro (carla_select_python)
  if (Python_EXECUTABLE AND NOT Python3_EXECUTABLE)
    set (Python3_EXECUTABLE "${Python_EXECUTABLE}")
  endif ()
  if (Python_ROOT_DIR AND NOT Python3_ROOT_DIR)
    set (Python3_ROOT_DIR "${Python_ROOT_DIR}")
  endif ()

  # Discover the interpreter first, then validate both modules' cached
  # development artifacts against that exact ABI version on reconfigure.
  find_package (Python3 REQUIRED COMPONENTS Interpreter)
  set (_carla_python_version "${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}")
  find_package (
    Python3 ${_carla_python_version} EXACT
    COMPONENTS
      Interpreter
      Development.Module
      Development.Embed
      Development.SABIModule
    REQUIRED
  )

  if (Python_EXECUTABLE AND
      NOT "${Python_EXECUTABLE}" STREQUAL "${Python3_EXECUTABLE}")
    string (
      CONCAT
      _carla_python_abi_query
      "import struct,sys,sysconfig; "
      "print((sys.implementation.name, sys.version_info[:2], "
      "sysconfig.get_config_var('SOABI'), struct.calcsize('P'), "
      "sysconfig.get_config_var('Py_DEBUG'), "
      "sysconfig.get_config_var('Py_GIL_DISABLED')))"
    )
    execute_process (
      COMMAND "${Python_EXECUTABLE}" -I -S -c "${_carla_python_abi_query}"
      RESULT_VARIABLE _carla_legacy_python_status
      OUTPUT_VARIABLE _carla_legacy_python_abi
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process (
      COMMAND "${Python3_EXECUTABLE}" -I -S -c "${_carla_python_abi_query}"
      RESULT_VARIABLE _carla_selected_python_status
      OUTPUT_VARIABLE _carla_selected_python_abi
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (_carla_legacy_python_status OR _carla_selected_python_status OR
        NOT "${_carla_legacy_python_abi}" STREQUAL "${_carla_selected_python_abi}")
      message (
        FATAL_ERROR
        "Python_EXECUTABLE and Python3_EXECUTABLE select incompatible Python ABIs. "
        "Specify one interpreter, or use compatible interpreter hints for both."
      )
    endif ()
  endif ()

  # A normal variable also overrides an earlier cached hint in child projects.
  set (Python_EXECUTABLE "${Python3_EXECUTABLE}")
  find_package (
    Python ${_carla_python_version} EXACT
    REQUIRED COMPONENTS Interpreter Development
  )
  unset (_carla_python_version)
  unset (_carla_python_abi_query)
  unset (_carla_legacy_python_status)
  unset (_carla_legacy_python_abi)
  unset (_carla_selected_python_status)
  unset (_carla_selected_python_abi)
endmacro ()
