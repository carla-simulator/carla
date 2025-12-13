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

macro (carla_add_compile_option FLAG)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endmacro ()

macro (carla_add_link_option FLAG)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAG}")
endmacro ()

macro (carla_try_add_compile_options)
  foreach (NAME ${ARGN})
    string (REPLACE "/" "__" NAME2 ${NAME})
    string (MAKE_C_IDENTIFIER "CO_${NAME2}" RESULT)
    check_c_compiler_flag(${NAME} ${RESULT})
    if (${VARIABLE_IDENTIFIER})
      carla_add_compile_option (${NAME})
    endif ()
  endforeach ()
endmacro ()

macro (carla_try_add_link_options)
  foreach (NAME ${ARGN})
    string (REPLACE "/" "__" NAME2 ${NAME})
    string (MAKE_C_IDENTIFIER "LO_${NAME2}" RESULT)
    check_linker_flag(C ${NAME} ${RESULT})
    if (${RESULT})
      carla_add_link_option (${NAME})
    endif ()
  endforeach ()
endmacro ()

if (WIN32)
  add_compile_definitions (_CRT_SECURE_NO_WARNINGS)
  carla_try_add_compile_options (/utf-8)
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
carla_try_add_compile_options (${EXCEPTIONS_FLAG})

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

if (ENABLE_RTTI)
  carla_try_add_compile_options (/GR)
  carla_try_add_compile_options (-frtti)
else ()
  carla_try_add_compile_options (/GR-)
  carla_try_add_compile_options (-fno-rtti)
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
    carla_try_add_compile_options (/W4)
  else ()
    carla_try_add_compile_options (-Wall)
    carla_try_add_compile_options (-Wextra)
  endif ()
endif ()

function (carla_try_suppress_warning WARNING_ID)
  if (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
    carla_try_add_compile_options (-Wno-${WARNING_ID})
  elseif (CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    carla_try_add_compile_options (/wd${WARNING_ID})
  endif ()
endfunction ()

set (
  CARLA_SUPRESSED_WARNING_LIST
  macro-redefined 4005
  incompatible-pointer-types
)

foreach (WARNING ${CARLA_SUPRESSED_WARNING_LIST})
  carla_try_suppress_warning (${WARNING})
endforeach ()

# ================================
#   WError Config
# ================================

if (ENABLE_WARNINGS_TO_ERRORS)
  if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    carla_try_add_compile_options (/WX)
  else ()
    carla_try_add_compile_options (-Werror)
  endif ()
endif ()
