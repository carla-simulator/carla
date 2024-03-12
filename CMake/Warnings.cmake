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

macro (carla_try_suppress_cxx_warning NAME FLAG)
  check_cxx_compiler_flag (
    -Wno-${FLAG}
    HAS_${NAME}
  )
  if (HAS_${NAME})
    add_compile_options (
      $<$<COMPILE_LANGUAGE:CXX>:-Wno-${FLAG}>
    )
  endif ()
endmacro ()

set (
  CARLA_SUPRESSED_WARNING_LIST
  macro-redefined
  incompatible-pointer-types
)

foreach (WARNING ${CARLA_SUPRESSED_WARNING_LIST})
  string (MAKE_C_IDENTIFIER "${WARNING}" WARNING_NAME)
  carla_try_suppress_cxx_warning (
    ${WARNING_NAME}
    ${WARNING}
  )
endforeach ()
