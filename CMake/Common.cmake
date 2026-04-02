
# ================================
#   Exception Definitions
# ================================

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" AND
  NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (ENABLE_EXCEPTIONS)
    set (EXCEPTIONS_FLAG /EHsc)
  else ()
    set (EXCEPTIONS_FLAG /EHs-c-)
  endif ()
else ()
  if (ENABLE_EXCEPTIONS)
    set (EXCEPTIONS_FLAG -fexceptions)
  else ()
    set (EXCEPTIONS_FLAG -fno-exceptions)
  endif ()
endif ()

message (STATUS "Checking for ${EXCEPTIONS_FLAG} support")
check_cxx_compiler_flag (${EXCEPTIONS_FLAG} HAS_EXCEPTIONS_FLAG)
if (HAS_EXCEPTIONS_FLAG)
  add_compile_options ($<$<COMPILE_LANGUAGE:CXX>:${EXCEPTIONS_FLAG}>)
endif ()

set (CARLA_EXCEPTION_DEFINITIONS)
if (ENABLE_EXCEPTIONS)
  # Nothing
else ()
  list (APPEND CARLA_EXCEPTION_DEFINITIONS _HAS_EXCEPTIONS=0)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS ASIO_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS BOOST_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS LIBCARLA_NO_EXCEPTIONS)
  list (APPEND CARLA_EXCEPTION_DEFINITIONS PUGIXML_NO_EXCEPTIONS)
endif ()

# ================================
#   RTTI Definitions
# ================================

if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC" AND
    NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  if (ENABLE_RTTI)
    set (RTTI_FLAG /GR)
  else ()
    set (RTTI_FLAG /GR-)
  endif ()
else ()
  if (ENABLE_RTTI)
    set (RTTI_FLAG -frtti)
  else ()
    set (RTTI_FLAG -fno-rtti)
  endif ()
endif ()

message (STATUS "Checking for ${RTTI_FLAG} support")
check_cxx_compiler_flag (${RTTI_FLAG} HAS_RTTI_FLAG)
if (HAS_RTTI_FLAG)
  add_compile_options ($<$<COMPILE_LANGUAGE:CXX>:${RTTI_FLAG}>)
endif ()

set (CARLA_RTTI_DEFINITIONS)

if (ENABLE_RTTI)
  # Nothing
else ()
  list (APPEND CARLA_RTTI_DEFINITIONS BOOST_NO_RTTI)
  list (APPEND CARLA_RTTI_DEFINITIONS BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY)
endif ()

# ================================
#   WAll Config
# ================================

if (ENABLE_ALL_WARNINGS)
  check_cxx_compiler_flag(-Wall HAS_WALL_GNU)
  if (HAS_WALL_GNU)
    set (CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
  endif ()
  check_cxx_compiler_flag(-Wextra HAS_WEXTRA_GNU)
  if (HAS_WEXTRA_GNU)
    set (CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
  endif ()
  check_cxx_compiler_flag(/Wall HAS_WALL_MSVC)
  if (HAS_WALL_MSVC)
    set (CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
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

# ================================
#   WError Config
# ================================

if (ENABLE_WARNINGS_TO_ERRORS)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      check_cxx_compiler_flag(-Werror HAS_WALL)
      if (HAS_WALL)
        add_compile_options (-Werror)
      endif ()
    else ()
      check_cxx_compiler_flag(/WX HAS_WALL)
      if (HAS_WALL)
        add_compile_options (/WX)
      endif ()
    endif ()
  else ()
    check_cxx_compiler_flag(-Werror HAS_WALL)
    if (HAS_WALL)
      add_compile_options (-Werror)
    endif ()
  endif ()
endif ()
