if (ENABLE_ALL_WARNINGS)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-Wall HAS_WALL)
      if (HAS_WALL)
        add_compile_options (-Wall)
      endif ()
    else ()
      check_cxx_compiler_flag(/Wall HAS_WALL)
      if (HAS_WALL)
        add_compile_options (/Wall)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-Wall HAS_WALL)
    if (HAS_WALL)
      add_compile_options (-Wall)
    endif ()
  endif ()
endif ()

if (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
  set (SUPPRESS_WARNING_DIRECTIVE_PREFIX -Wno-)
elseif (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
  set (SUPPRESS_WARNING_DIRECTIVE_PREFIX /wd)
endif ()

macro (carla_try_suppress_cxx_warning NAME FLAG)
  check_cxx_compiler_flag (
    ${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}
    HAS_${NAME}
  )
  if (HAS_${NAME})
    add_compile_options (
      $<$<COMPILE_LANGUAGE:CXX>:${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}>)
  endif ()
endmacro ()

macro (carla_try_suppress_c_warning NAME FLAG)
  check_c_compiler_flag (
    ${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}
    HAS_${NAME}
  )
  if (HAS_${NAME})
    add_compile_options (
      $<$<COMPILE_LANGUAGE:C>:${SUPPRESS_WARNING_DIRECTIVE_PREFIX}${FLAG}>)
  endif ()
endmacro ()

set (
  CARLA_C_SUPRESSED_WARNING_LIST
  macro-redefined 4005
  incompatible-pointer-types
)

set (
  CARLA_CXX_SUPRESSED_WARNING_LIST
  macro-redefined 4005
)

foreach (WARNING ${CARLA_C_SUPRESSED_WARNING_LIST})
  string (MAKE_C_IDENTIFIER "${WARNING}" WARNING_NAME)
  carla_try_suppress_c_warning (
    ${WARNING_NAME}
    ${WARNING}
  )
endforeach ()

foreach (WARNING ${CARLA_CXX_SUPRESSED_WARNING_LIST})
  string (MAKE_C_IDENTIFIER "${WARNING}" WARNING_NAME)
  carla_try_suppress_cxx_warning (
    ${WARNING_NAME}
    ${WARNING}
  )
endforeach ()
