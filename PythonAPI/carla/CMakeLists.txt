cmake_minimum_required (
  VERSION 3.28
)

project (
  carla-python-api
  LANGUAGES CXX
  VERSION 0.9.15
  DESCRIPTION
    "Python API for communicating with the CARLA server."
)

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

include (F:/Carla/CMake/Util.cmake)

carla_message (
  "Python Interpreter ID: \"${Python3_INTERPRETER_ID}\""
)

set (
  PYTHON_API_SOURCES
  F:/Carla/PythonAPI/carla/src/PythonAPI.cpp
  F:/Carla/PythonAPI/carla/src/Actor.cpp
  F:/Carla/PythonAPI/carla/src/Blueprint.cpp
  F:/Carla/PythonAPI/carla/src/Client.cpp
  F:/Carla/PythonAPI/carla/src/Commands.cpp
  F:/Carla/PythonAPI/carla/src/Control.cpp
  F:/Carla/PythonAPI/carla/src/Exception.cpp
  F:/Carla/PythonAPI/carla/src/Geometry.cpp
  F:/Carla/PythonAPI/carla/src/LightManager.cpp
  F:/Carla/PythonAPI/carla/src/Map.cpp
  F:/Carla/PythonAPI/carla/src/Sensor.cpp
  F:/Carla/PythonAPI/carla/src/SensorData.cpp
  F:/Carla/PythonAPI/carla/src/Snapshot.cpp
  F:/Carla/PythonAPI/carla/src/TrafficManager.cpp
  F:/Carla/PythonAPI/carla/src/Weather.cpp
  F:/Carla/PythonAPI/carla/src/World.cpp
)
if (ENABLE_OSM2ODR)
  list (APPEND PYTHON_API_SOURCES F:/Carla/PythonAPI/carla/src/OSM2ODR.cpp)
endif ()
if (ENABLE_RSS)
  list (APPEND PYTHON_API_SOURCES F:/Carla/PythonAPI/carla/src/AdRss.cpp)
endif ()

Python3_add_library (
  carla-python-api
  MODULE
  WITH_SOABI
  ${PYTHON_API_SOURCES}
)

target_include_directories (
  carla-python-api PRIVATE
  F:/Carla/PythonAPI/carla/include
  F:/Carla/LibCarla/source;F:/Carla/Build/_deps/boost-src/libs/asio/include;F:/Carla/Build/_deps/boost-src/libs/align/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/context/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/pool/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/coroutine/include;F:/Carla/Build/_deps/boost-src/libs/exception/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/date_time/include;F:/Carla/Build/_deps/boost-src/libs/algorithm/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/unordered/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/Build/_deps/boost-src/libs/tokenizer/include;F:/Carla/Build/_deps/boost-src/libs/geometry/include;F:/Carla/Build/_deps/boost-src/libs/any/include;F:/Carla/Build/_deps/boost-src/libs/type_index/include;F:/Carla/Build/_deps/boost-src/libs/endian/include;F:/Carla/Build/_deps/boost-src/libs/math/include;F:/Carla/Build/_deps/boost-src/libs/random/include;F:/Carla/Build/_deps/boost-src/libs/dynamic_bitset/include;F:/Carla/Build/_deps/boost-src/libs/multiprecision/include;F:/Carla/Build/_deps/boost-src/libs/polygon/include;F:/Carla/Build/_deps/boost-src/libs/qvm/include;F:/Carla/Build/_deps/boost-src/libs/rational/include;F:/Carla/Build/_deps/boost-src/libs/serialization/include;F:/Carla/Build/_deps/boost-src/libs/spirit/include;F:/Carla/Build/_deps/boost-src/libs/phoenix/include;F:/Carla/Build/_deps/boost-src/libs/proto/include;F:/Carla/Build/_deps/boost-src/libs/thread/include;F:/Carla/Build/_deps/boost-src/libs/atomic/include;F:/Carla/Build/_deps/boost-src/libs/chrono/include;F:/Carla/Build/_deps/boost-src/libs/ratio/include;F:/Carla/Build/_deps/boost-src/libs/variant/include;F:/Carla/Build/_deps/boost-src/libs/gil/include;F:/Carla/Build/_deps/boost-src/libs/filesystem/include;F:/Carla/Build/_deps/recastnavigation-src/Recast/Include;F:/Carla/Build/_deps/recastnavigation-src/Detour/Include;F:/Carla/Build/_deps/recastnavigation-src/DetourCrowd/Include;F:/Carla/Build/_deps/zlib-build;F:/Carla/Build/_deps/zlib-src;F:/Carla/Build/_deps/rpclib-src/include
  F:/Carla/Build/_deps/boost-src/libs/asio/include;F:/Carla/Build/_deps/boost-src/libs/align/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/context/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/pool/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/coroutine/include;F:/Carla/Build/_deps/boost-src/libs/exception/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/date_time/include;F:/Carla/Build/_deps/boost-src/libs/algorithm/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/unordered/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/Build/_deps/boost-src/libs/tokenizer/include
  F:/Carla/Build/_deps/boost-src/libs/python/include;F:/Carla/Build/_deps/boost-src/libs/align/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/foreach/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;C:/Users/Marce/AppData/Local/Programs/Python/Python312/include
  F:/Carla/Build/_deps/boost-src/libs/geometry/include;F:/Carla/Build/_deps/boost-src/libs/algorithm/include;F:/Carla/Build/_deps/boost-src/libs/array/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/exception/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/range/include;F:/Carla/Build/_deps/boost-src/libs/conversion/include;F:/Carla/Build/_deps/boost-src/libs/regex/include;F:/Carla/Build/_deps/boost-src/libs/unordered/include;F:/Carla/Build/_deps/boost-src/libs/any/include;F:/Carla/Build/_deps/boost-src/libs/type_index/include;F:/Carla/Build/_deps/boost-src/libs/container/include;F:/Carla/Build/_deps/boost-src/libs/intrusive/include;F:/Carla/Build/_deps/boost-src/libs/endian/include;F:/Carla/Build/_deps/boost-src/libs/integer/include;F:/Carla/Build/_deps/boost-src/libs/lexical_cast/include;F:/Carla/Build/_deps/boost-src/libs/numeric/conversion/include;F:/Carla/Build/_deps/boost-src/libs/math/include;F:/Carla/Build/_deps/boost-src/libs/random/include;F:/Carla/Build/_deps/boost-src/libs/dynamic_bitset/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/multiprecision/include;F:/Carla/Build/_deps/boost-src/libs/polygon/include;F:/Carla/Build/_deps/boost-src/libs/qvm/include;F:/Carla/Build/_deps/boost-src/libs/rational/include;F:/Carla/Build/_deps/boost-src/libs/serialization/include;F:/Carla/Build/_deps/boost-src/libs/spirit/include;F:/Carla/Build/_deps/boost-src/libs/phoenix/include;F:/Carla/Build/_deps/boost-src/libs/proto/include;F:/Carla/Build/_deps/boost-src/libs/pool/include;F:/Carla/Build/_deps/boost-src/libs/thread/include;F:/Carla/Build/_deps/boost-src/libs/atomic/include;F:/Carla/Build/_deps/boost-src/libs/chrono/include;F:/Carla/Build/_deps/boost-src/libs/ratio/include;F:/Carla/Build/_deps/boost-src/libs/date_time/include;F:/Carla/Build/_deps/boost-src/libs/tokenizer/include;F:/Carla/Build/_deps/boost-src/libs/variant/include
  F:/Carla/Build/_deps/boost-src/libs/gil/include;F:/Carla/Build/_deps/boost-src/libs/assert/include;F:/Carla/Build/_deps/boost-src/libs/config/include;F:/Carla/Build/_deps/boost-src/libs/concept_check/include;F:/Carla/Build/_deps/boost-src/libs/preprocessor/include;F:/Carla/Build/_deps/boost-src/libs/static_assert/include;F:/Carla/Build/_deps/boost-src/libs/type_traits/include;F:/Carla/Build/_deps/boost-src/libs/container_hash/include;F:/Carla/Build/_deps/boost-src/libs/describe/include;F:/Carla/Build/_deps/boost-src/libs/mp11/include;F:/Carla/Build/_deps/boost-src/libs/core/include;F:/Carla/Build/_deps/boost-src/libs/throw_exception/include;F:/Carla/Build/_deps/boost-src/libs/filesystem/include;F:/Carla/Build/_deps/boost-src/libs/detail/include;F:/Carla/Build/_deps/boost-src/libs/io/include;F:/Carla/Build/_deps/boost-src/libs/iterator/include;F:/Carla/Build/_deps/boost-src/libs/function_types/include;F:/Carla/Build/_deps/boost-src/libs/mpl/include;F:/Carla/Build/_deps/boost-src/libs/predef/include;F:/Carla/Build/_deps/boost-src/libs/utility/include;F:/Carla/Build/_deps/boost-src/libs/fusion/include;F:/Carla/Build/_deps/boost-src/libs/tuple/include;F:/Carla/Build/_deps/boost-src/libs/typeof/include;F:/Carla/Build/_deps/boost-src/libs/functional/include;F:/Carla/Build/_deps/boost-src/libs/function/include;F:/Carla/Build/_deps/boost-src/libs/bind/include;F:/Carla/Build/_deps/boost-src/libs/optional/include;F:/Carla/Build/_deps/boost-src/libs/move/include;F:/Carla/Build/_deps/boost-src/libs/smart_ptr/include;F:/Carla/Build/_deps/boost-src/libs/system/include;F:/Carla/Build/_deps/boost-src/libs/variant2/include;F:/Carla/Build/_deps/boost-src/libs/winapi/include;F:/Carla/Build/_deps/boost-src/libs/integer/include
  F:/Carla/Build/_deps/recastnavigation-src/Recast/Include
  F:/Carla/Build/_deps/recastnavigation-src/Detour/Include
  F:/Carla/Build/_deps/recastnavigation-src/DetourCrowd/Include;F:/Carla/Build/_deps/recastnavigation-src/Detour/Include
  F:/Carla/Build/_deps/libpng-src F:/Carla/Build/_deps/libpng-build 
  F:/Carla/Build/_deps/zlib-src F:/Carla/Build/_deps/zlib-build F:/Carla/Build/_deps/zlib-build;F:/Carla/Build/_deps/zlib-src
  F:/Carla/Build/_deps/rpclib-src/include
)

target_link_libraries (
  carla-python-api PRIVATE
  F:/Carla/Build/LibCarla/carla-client.lib
  F:/Carla/Build/_deps/recastnavigation-build/Recast/Recast.lib
  F:/Carla/Build/_deps/recastnavigation-build/Detour/Detour.lib
  F:/Carla/Build/_deps/recastnavigation-build/DetourCrowd/DetourCrowd.lib
  F:/Carla/Build/_deps/libpng-build/libpng16_static.lib
  F:/Carla/Build/_deps/zlib-build/zlibstatic.lib
  F:/Carla/Build/_deps/rpclib-build/rpc.lib
)

target_compile_definitions (
  carla-python-api PRIVATE
  LIBCARLA_IMAGE_SUPPORT_PNG=1;_WIN32_WINNT=0x0601;HAVE_SNPRINTF;_USE_MATH_DEFINES
  
  BOOST_ALL_NO_LIB
  LIBCARLA_WITH_PYTHON_SUPPORT
)

target_include_directories (
  carla-python-api PRIVATE
  F:/Carla/PythonAPI/carla/src
  
)

install (
  TARGETS carla-python-api
  DESTINATION PythonAPI
)
