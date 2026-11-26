# Adds the headers, static libraries and defines that Carla::carla-client needs but the installed package omits.
# Set CARLA_BUILD_DIR to the CARLA build tree (Build/<preset>) that produced the installed libcarla-client.a.

if (NOT CARLA_BUILD_DIR OR NOT EXISTS "${CARLA_BUILD_DIR}/_deps/boost-src")
  message (FATAL_ERROR "Set -DCARLA_BUILD_DIR=/path/to/carla/Build/<preset> (it must hold _deps/boost-src).")
endif ()

set (_D "${CARLA_BUILD_DIR}/_deps")

file (GLOB _BOOST_INCLUDES LIST_DIRECTORIES true "${_D}/boost-src/libs/*/include" "${_D}/boost-src/libs/numeric/*/include")

set (_CARLA_DEP_LIBS)
foreach (_lib context graph random serialization thread date_time container atomic chrono filesystem)
  list (APPEND _CARLA_DEP_LIBS "${_D}/boost-build/libs/${_lib}/libboost_${_lib}.a")
endforeach ()
list (APPEND _CARLA_DEP_LIBS
  "${_D}/recastnavigation-build/Recast/libRecast.a"
  "${_D}/recastnavigation-build/DetourCrowd/libDetourCrowd.a"
  "${_D}/recastnavigation-build/Detour/libDetour.a"
  "${_D}/libpng-build/libpng16.a"
  "${_D}/zlib-build/libz.a"
  "${_D}/rpclib-build/librpc.a")
foreach (_lib IN LISTS _CARLA_DEP_LIBS)
  if (NOT EXISTS "${_lib}")
    message (FATAL_ERROR "${_lib} is missing: build the carla-client target in ${CARLA_BUILD_DIR} first.")
  endif ()
endforeach ()

set_property (TARGET Carla::carla-client APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES
  ${_BOOST_INCLUDES}
  "${_D}/rpclib-src/include"
  "${_D}/recastnavigation-src/Recast/Include"
  "${_D}/recastnavigation-src/Detour/Include"
  "${_D}/recastnavigation-src/DetourCrowd/Include"
  "${_D}/libpng-src" "${_D}/libpng-build"
  "${_D}/zlib-src" "${_D}/zlib-build")

set_property (TARGET Carla::carla-client APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${_CARLA_DEP_LIBS} m)

set_property (TARGET Carla::carla-client APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS
  BOOST_ALL_NO_LIB BOOST_FILESYSTEM_STATIC_LINK=1 BOOST_THREAD_USE_LIB
  LIBCARLA_IMAGE_SUPPORT_PNG=1 RPCLIB_LINUX RPCLIB_MSGPACK=clmdep_msgpack)
