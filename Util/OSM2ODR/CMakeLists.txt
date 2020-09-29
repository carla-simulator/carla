# set VERSION to empty string
cmake_policy(SET CMP0048 NEW)
# do not expand quoted variables in if statements
cmake_policy(SET CMP0054 NEW)
# Options
option(MULTITHREADED_BUILD "Use all available cores for building (applies to Visual Studio only)" true)
option(PROFILING "Enable output of profiling data (applies to gcc/clang builds only)" false)
set(BINARY_SUFFIX "" CACHE STRING "Append the suffix to every generated binary")
set(COMPILE_DEFINITIONS "" CACHE STRING "Macros or defines to add when compiling")

# Set a default build type if none was specified
# you may use -DCMAKE_BUILD_TYPE:STRING=Debug from the command line
set(default_build_type "Release")

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif ()

project(SUMO)
set(PACKAGE_VERSION "git")
cmake_minimum_required(VERSION 3.1)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake_modules/")

set(ENABLED_FEATURES "${CMAKE_SYSTEM} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} ${CMAKE_BUILD_TYPE}")

if (COMPILE_DEFINITIONS)
    add_compile_definitions(${COMPILE_DEFINITIONS})
endif ()

# compiler specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -pedantic -Wextra -fPIC")
elseif (MSVC)
    # enabling /WX is not possible due to warnings in external headers
    # /Wall brings MSVC 2013 to complete halt
    if (MULTITHREADED_BUILD)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    endif ()
    if (MSVC_VERSION GREATER 1914)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /experimental:external /external:W2")
        set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "/external:I ")
    endif ()
    # exporting symbols for shared libraries needs to enabled explicitly
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

# force Visual Studio to leave out the Release / Debug dirs
# Debug messages
message(STATUS "CMAKE_BINARY_DIR: " ${CMAKE_BINARY_DIR})
message(STATUS "CMAKE_SOURCE_DIR: " ${CMAKE_SOURCE_DIR})
message(STATUS "")
message(STATUS "Platform: ")
message(STATUS "    Host: " ${CMAKE_HOST_SYSTEM} " " ${CMAKE_HOST_SYSTEM_PROCESSOR})
message(STATUS "    Target: " ${CMAKE_SYSTEM} " " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "    CMake: " ${CMAKE_VERSION})
message(STATUS "    CMake generator: " ${CMAKE_GENERATOR})
message(STATUS "    CMake build tool: " ${CMAKE_BUILD_TOOL})
message(STATUS "    Compiler: " ${CMAKE_CXX_COMPILER_ID} " " ${CMAKE_CXX_COMPILER_VERSION})
if (CMAKE_GENERATOR MATCHES Xcode)
    message(STATUS "    Xcode: " ${XCODE_VERSION})
endif ()
message(STATUS "")

find_package(PythonInterp REQUIRED)
message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import setuptools" RESULT_VARIABLE SETUPTOOLS_MISSING)

#find_package(Proj)
#if (PROJ_FOUND)
    #include_directories(SYSTEM ${PROJ_INCLUDE_DIR})
    #set(ENABLED_FEATURES "${ENABLED_FEATURES} Proj")
#endif (PROJ_FOUND)

set(XERCES_PATH "../../Build/xerces-c-3.2.3-install")
set(XercesC_INCLUDE_DIR "${XERCES_PATH}/include")
set(XercesC_LIBRARY "${XERCES_PATH}/lib")
# Install xercesc
file(GLOB XERCES_LIBS "${XercesC_LIBRARY}/*.*")
install(FILES ${XERCES_LIBS} DESTINATION lib)
find_package(XercesC REQUIRED)
if (XercesC_FOUND)
    include_directories(SYSTEM ${XercesC_INCLUDE_DIRS})
endif (XercesC_FOUND)

find_package(X11)
if (X11_FOUND)
    link_directories(${X11_LIBRARY_DIR})
    include_directories(SYSTEM ${X11_INCLUDE_DIR})
endif (X11_FOUND)

find_package(ZLIB)
if (ZLIB_FOUND)
    set(HAVE_ZLIB 1)
    link_directories(${ZLIB_LIBRARY_DIR})
    include_directories(SYSTEM ${ZLIB_INCLUDE_DIR})
endif ()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

add_subdirectory(src)

message(STATUS "Enabled features: ${ENABLED_FEATURES}")
