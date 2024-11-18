# ================================
#   Common
# ================================

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

set (CMAKE_POSITION_INDEPENDENT_CODE ON)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  carla_warning (
    "CARLA is set to be built in Debug mode. This may cause issues when building CarlaUnrealEditor."
  )
endif ()

if (LINUX)
  check_linker_flag (CXX -lpthread HAS_PTHREAD)
  if (HAS_PTHREAD)
    add_link_options (-lpthread)
  endif ()
endif ()

if (WIN32)
  if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set (CARLA_DEBUG_AFFIX d)
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
  else ()
    set (CARLA_DEBUG_AFFIX )
    set (CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
  endif ()
endif ()

if (CMAKE_TOOLCHAIN_FILE)
  cmake_path (
    ABSOLUTE_PATH
      CMAKE_TOOLCHAIN_FILE
    BASE_DIRECTORY
      ${CARLA_WORKSPACE_PATH}
    NORMALIZE
    OUTPUT_VARIABLE
      TOOLCHAIN_FILE
  )
  set (CMAKE_TOOLCHAIN_FILE ${TOOLCHAIN_FILE})
endif ()

# ================================
#   Common Definitions
# ================================

if (WIN32)
  add_compile_definitions (_CRT_SECURE_NO_WARNINGS)
  check_cxx_compiler_flag (/utf-8 HAS_MSVC_UTF8)
  if (HAS_MSVC_UTF8)
    # @TODO This causes warnings with MASM. A better approach should be looked into.
    add_compile_options ($<$<COMPILE_LANGUAGE:CXX>:/utf-8>)
  endif ()
endif ()

set (CARLA_COMMON_DEFINITIONS)

foreach (FORMAT ${LIBCARLA_IMAGE_SUPPORTED_FORMATS})
  carla_message_verbose ("Enabling CARLA image support for \"${FORMAT}\".")
  string (TOUPPER "${FORMAT}" FORMAT_UPPERCASE)
  list (APPEND CARLA_COMMON_DEFINITIONS LIBCARLA_IMAGE_SUPPORT_${FORMAT_UPPERCASE}=1)
endforeach ()

if (WIN32)
  # Documentation: https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
  list (APPEND CARLA_COMMON_DEFINITIONS _WIN32_WINNT=0x0601) # <- Windows 10
  list (APPEND CARLA_COMMON_DEFINITIONS HAVE_SNPRINTF)
  list (APPEND CARLA_COMMON_DEFINITIONS _USE_MATH_DEFINES)
endif ()

if (WIN32)
  set (EXE_EXT .exe)
  set (UE_SYSTEM_NAME Win64)
elseif (LINUX)
  set (EXE_EXT)
  set (UE_SYSTEM_NAME Linux)
elseif (APPLE)
  set (EXE_EXT)
  set (UE_SYSTEM_NAME Mac)
else ()
  carla_error ("Unknown target system.")
endif ()

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

carla_message ("Checking for ${EXCEPTIONS_FLAG} support")
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

carla_message ("Checking for ${RTTI_FLAG} support")
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
