cmake_minimum_required (
  VERSION
  3.28.0
)

project (
  ${SKBUILD_PROJECT_NAME}
  VERSION 0.9.15
  LANGUAGES CXX
)

cmake_policy (SET CMP0097 NEW)
cmake_policy (SET CMP0091 NEW)
cmake_policy (SET CMP0074 NEW)
cmake_policy (SET CMP0077 NEW)
cmake_policy (SET CMP0117 NEW)
if (${CMAKE_MINOR_VERSION} GREATER_EQUAL 24)
  cmake_policy (SET CMP0135 NEW)
endif ()

include (F:/Carla/CMake/Util.cmake)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_C_STANDARD 11)
set (CMAKE_C_STANDARD_REQUIRED ON)

find_package (
  Python3
  COMPONENTS
    Interpreter
    Development.Module
    Development.Embed
    Development.SABIModule
  REQUIRED
)

if (WIN32)
  set (PYTHON_MODULE_EXT .pyd)
else ()
  set (PYTHON_MODULE_EXT .so)
endif ()

set (
  CARLA_PYTHON_API_CARLA_PATH
  ${CMAKE_CURRENT_SOURCE_DIR}
)

carla_message (
  "Selected Python executable: ${Python3_EXECUTABLE} (Interpreter ID: ${Python3_INTERPRETER_ID})"
)

set (
  CARLA_PYTHON_API_DEPENDENCIES
)

set (
  CARLA_PYTHON_API_INCLUDE_PATH
  ${CARLA_PYTHON_API_CARLA_PATH}/include
)

set (
  CARLA_PYTHON_API_SOURCE_PATH
  ${CARLA_PYTHON_API_CARLA_PATH}/src
)

set (
  PYTHON_API_HEADERS
  ${CARLA_PYTHON_API_INCLUDE_PATH}/PythonAPI.h
)

set (
  PYTHON_API_SOURCES
  ${CARLA_PYTHON_API_SOURCE_PATH}/PythonAPI.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Actor.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Blueprint.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Client.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Commands.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Control.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Exception.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Geometry.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/LightManager.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Map.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Sensor.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/SensorData.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Snapshot.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/TrafficManager.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/Weather.cpp
  ${CARLA_PYTHON_API_SOURCE_PATH}/World.cpp
)

if (ENABLE_OSM2ODR)
  list (
    APPEND
    PYTHON_API_SOURCES
    ${CARLA_PYTHON_API_SOURCE_PATH}/OSM2ODR.cpp
  )
endif ()

if (ENABLE_RSS)
  list (
    APPEND
    PYTHON_API_SOURCES
    ${CARLA_PYTHON_API_SOURCE_PATH}/AdRss.cpp
  )
endif ()

set (
  USE_LIMITED_API
  OFF
)

set (
  PYTHON_API_ADD_LIBRARY_ARGS
)

if (USE_LIMITED_API)
  list (
    APPEND
    PYTHON_API_ADD_LIBRARY_ARGS
    USE_SABI ${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}
  )
endif ()

list (
  APPEND
  PYTHON_API_ADD_LIBRARY_ARGS
  WITH_SOABI
)

Python3_add_library (
  ${SKBUILD_PROJECT_NAME}
  MODULE
  ${PYTHON_API_ADD_LIBRARY_ARGS}
  ${PYTHON_API_HEADERS}
  ${PYTHON_API_SOURCES}
)

install (
  TARGETS ${SKBUILD_PROJECT_NAME}
  DESTINATION ${SKBUILD_PROJECT_NAME}
)

target_include_directories (
  ${SKBUILD_PROJECT_NAME} PRIVATE
  ${CARLA_PYTHON_API_INCLUDE_PATH}
  F:/Carla/LibCarla/source;F:/Carla/Build/_deps/boost-src/libs/asio/include;F:/Carla/Build/_deps/boost-src/libs/align/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/context/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/pool/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/coroutine/include;F:/Carla/Build/_deps/boost-src/libs/exception/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/date_time/include;F:/Carla/Build/_deps/boost-src/libs/algorithm/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/unordered/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/Build/_deps/boost-src/libs/tokenizer/include;F:/Carla/Build/_deps/boost-src/libs/python/include;F:/Carla/Build/_deps/boost-src/libs/foreach/include;C:/Users/Marce/AppData/Local/Programs/Python/Python312/include;F:/Carla/Build/_deps/boost-src/libs/geometry/include;F:/Carla/Build/_deps/boost-src/libs/any/include;F:/Carla/Build/_deps/boost-src/libs/type_index/include;F:/Carla/Build/_deps/boost-src/libs/endian/include;F:/Carla/Build/_deps/boost-src/libs/math/include;F:/Carla/Build/_deps/boost-src/libs/random/include;F:/Carla/Build/_deps/boost-src/libs/dynamic_bitset/include;F:/Carla/Build/_deps/boost-src/libs/multiprecision/include;F:/Carla/Build/_deps/boost-src/libs/polygon/include;F:/Carla/Build/_deps/boost-src/libs/qvm/include;F:/Carla/Build/_deps/boost-src/libs/rational/include;F:/Carla/Build/_deps/boost-src/libs/serialization/include;F:/Carla/Build/_deps/boost-src/libs/spirit/include;F:/Carla/Build/_deps/boost-src/libs/phoenix/include;F:/Carla/Build/_deps/boost-src/libs/proto/include;F:/Carla/Build/_deps/boost-src/libs/thread/include;F:/Carla/Build/_deps/boost-src/libs/atomic/include;F:/Carla/Build/_deps/boost-src/libs/chrono/include;F:/Carla/Build/_deps/boost-src/libs/ratio/include;F:/Carla/Build/_deps/boost-src/libs/variant/include;F:/Carla/Build/_deps/boost-src/libs/gil/include;F:/Carla/Build/_deps/boost-src/libs/filesystem/include;F:/Carla/Build/_deps/recastnavigation-src/Recast/Include;F:/Carla/Build/_deps/recastnavigation-src/Detour/Include;F:/Carla/Build/_deps/recastnavigation-src/DetourCrowd/Include;F:/Carla/Build/_deps/zlib-build;F:/Carla/Build/_deps/zlib-src;F:/Carla/Build/_deps/rpclib-src/include
  F:/Carla/Build/_deps/boost-src/libs/asio/include;F:/Carla/Build/_deps/boost-src/libs/align/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/context/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/pool/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/coroutine/include;F:/Carla/Build/_deps/boost-src/libs/exception/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/date_time/include;F:/Carla/Build/_deps/boost-src/libs/algorithm/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/unordered/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/Build/_deps/boost-src/libs/tokenizer/include
  F:/Carla/Build/_deps/boost-src/libs/python/include;F:/Carla/Build/_deps/boost-src/libs/align/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/foreach/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;C:/Users/Marce/AppData/Local/Programs/Python/Python312/include
  F:/Carla/Build/_deps/boost-src/libs/geometry/include;F:/Carla/Build/_deps/boost-src/libs/algorithm/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/exception/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/unordered/include;F:/Carla/Build/_deps/boost-src/libs/any/include;F:/Carla/Build/_deps/boost-src/libs/type_index/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/endian/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/Build/_deps/boost-src/libs/math/include;F:/Carla/Build/_deps/boost-src/libs/random/include;F:/Carla/Build/_deps/boost-src/libs/dynamic_bitset/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/multiprecision/include;F:/Carla/Build/_deps/boost-src/libs/polygon/include;F:/Carla/Build/_deps/boost-src/libs/qvm/include;F:/Carla/Build/_deps/boost-src/libs/rational/include;F:/Carla/Build/_deps/boost-src/libs/serialization/include;F:/Carla/Build/_deps/boost-src/libs/spirit/include;F:/Carla/Build/_deps/boost-src/libs/phoenix/include;F:/Carla/Build/_deps/boost-src/libs/proto/include;F:/Carla/Build/_deps/boost-src/libs/pool/include;F:/Carla/Build/_deps/boost-src/libs/thread/include;F:/Carla/Build/_deps/boost-src/libs/atomic/include;F:/Carla/Build/_deps/boost-src/libs/chrono/include;F:/Carla/Build/_deps/boost-src/libs/ratio/include;F:/Carla/Build/_deps/boost-src/libs/date_time/include;F:/Carla/Build/_deps/boost-src/libs/tokenizer/include;F:/Carla/Build/_deps/boost-src/libs/variant/include
  F:/Carla/Build/_deps/boost-src/libs/gil/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/filesystem/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/integer/include
  F:/Carla/Build/_deps/recastnavigation-src/Recast/Include
  F:/Carla/Build/_deps/recastnavigation-src/Detour/Include
  F:/Carla/Build/_deps/recastnavigation-src/DetourCrowd/Include;F:/Carla/Build/_deps/recastnavigation-src/Detour/Include
  
  F:/Carla/Build/_deps/zlib-build;F:/Carla/Build/_deps/zlib-src
  F:/Carla/Build/_deps/rpclib-src/include
  F:/Carla/Build/_deps/libpng-src F:/Carla/Build/_deps/libpng-build
  F:/Carla/Build/_deps/zlib-src F:/Carla/Build/_deps/zlib-build
)

target_compile_definitions (
  ${SKBUILD_PROJECT_NAME} PRIVATE
  BOOST_ALL_NO_LIB
  BOOST_PYTHON_STATIC_LIB
  LIBCARLA_WITH_PYTHON_SUPPORT
  LIBCARLA_IMAGE_SUPPORT_PNG=1;_WIN32_WINNT=0x0601;HAVE_SNPRINTF;_USE_MATH_DEFINES
  ASIO_NO_EXCEPTIONS;BOOST_NO_EXCEPTIONS;LIBCARLA_NO_EXCEPTIONS;PUGIXML_NO_EXCEPTIONS
  BOOST_TYPE_INDEX_FORCE_NO_RTTI_COMPATIBILITY
  
  
  
  
  
  
  
  
  
  
  
)

target_link_libraries (
  ${SKBUILD_PROJECT_NAME} PUBLIC
  F:/Carla/Build/LibCarla/carla-client.lib
  F:/Carla/Build/_deps/boost-build/libs/python/libboost_python312-vc143-mt-x64-1_84.lib
  F:/Carla/Build/_deps/recastnavigation-build/Recast/Recast.lib
  F:/Carla/Build/_deps/recastnavigation-build/Detour/Detour.lib
  F:/Carla/Build/_deps/recastnavigation-build/DetourCrowd/DetourCrowd.lib
  F:/Carla/Build/_deps/libpng-build/libpng16_static.lib
  F:/Carla/Build/_deps/zlib-build/zlibstatic.lib
  F:/Carla/Build/_deps/rpclib-build/rpc.lib
)
