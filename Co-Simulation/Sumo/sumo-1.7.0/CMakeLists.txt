# set VERSION to empty string
cmake_policy(SET CMP0048 NEW)
# do not expand quoted variables in if statements
cmake_policy(SET CMP0054 NEW)
# Options
option(CHECK_OPTIONAL_LIBS "Try to download / enable all optional libraries (use only EPL clean libraries, if set to false)" true)
option(MULTITHREADED_BUILD "Use all available cores for building (applies to Visual Studio only)" true)
option(PROFILING "Enable output of profiling data (applies to gcc/clang builds only)" false)
option(COVERAGE "Enable output of coverage data (applies to gcc/clang builds only)" false)
option(SUMO_UTILS "Enable generation of a shared library for the utility functions for option handling, XML parsing etc." false)
option(FMI "Enable generation of an FMI library for SUMO" false)
set(BINARY_SUFFIX "" CACHE STRING "Append the suffix to every generated binary")
set(COMPILE_DEFINITIONS "" CACHE STRING "Macros or defines to add when compiling")
set(CCACHE_SUPPORT ON CACHE BOOL "Enable ccache support")

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
set(PACKAGE_VERSION "1.7.0")
cmake_minimum_required(VERSION 3.1)

find_program(CCACHE_FOUND "ccache")
if (CCACHE_FOUND AND CCACHE_SUPPORT)
  message(STATUS "Enabling ccache")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "ccache")
endif()

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/build/cmake_modules/")

set(ENABLED_FEATURES "${CMAKE_SYSTEM} ${CMAKE_SYSTEM_PROCESSOR} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION} ${CMAKE_BUILD_TYPE}")

if (COMPILE_DEFINITIONS)
    add_compile_definitions(${COMPILE_DEFINITIONS})
endif ()

# compiler specific flags
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -Wall -pedantic -Wextra")
    if (PROFILING)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(ENABLED_FEATURES "${ENABLED_FEATURES} Profiling")
        set(BINARY_SUFFIX "${BINARY_SUFFIX}P")
    endif ()
    if (COVERAGE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
        set(ENABLED_FEATURES "${ENABLED_FEATURES} Coverage")
    endif ()
elseif (MSVC)
    # enabling /WX is not possible due to warnings in external headers
    # /Wall brings MSVC 2013 to complete halt
    if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        # this is a workaround to disable lots of warnings for replacing /W3 with /W4 in VS 2019
        # there is a policy for that starting with CMake 3.15
        string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else ()
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /W4")
    endif ()
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

# special debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=undefined,address,integer,unsigned-integer-overflow -fno-omit-frame-pointer -fsanitize-blacklist=${CMAKE_SOURCE_DIR}/build/clang_sanitize_blacklist.txt")
endif ()

# we need to build position independent code when generating a shared library
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
if (SUMO_UTILS)
    set(ENABLED_FEATURES "${ENABLED_FEATURES} SumoUtilsLibrary")
endif ()
if (FMI)
    set(ENABLED_FEATURES "${ENABLED_FEATURES} FMI")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
# force Visual Studio to leave out the Release / Debug dirs
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
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

#set(Python_ADDITIONAL_VERSIONS 3.7 3.6 3.5 3.4 3.3 2.7) # it may help in finding the correct python for libsumo
find_package(PythonInterp REQUIRED)
message(STATUS "Found Python: " ${PYTHON_EXECUTABLE})
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import setuptools" RESULT_VARIABLE SETUPTOOLS_MISSING)

#http://stackoverflow.com/questions/1487752/how-do-i-instruct-cmake-to-look-for-libraries-installed-by-macports
if (APPLE)
    link_directories(/opt/X11/lib)
    include_directories(SYSTEM /opt/X11/include)
    # Detect if the "port" command is valid on this system; if so, return full path
    execute_process(COMMAND which port RESULT_VARIABLE DETECT_MACPORTS OUTPUT_VARIABLE MACPORTS_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${DETECT_MACPORTS} EQUAL 0)
        # MACPORTS_PREFIX contains now something like "/opt/local/bin/port", so we get the parent directory
        get_filename_component(MACPORTS_PREFIX ${MACPORTS_PREFIX} DIRECTORY)
        # "/opt/local" is where MacPorts lives, add `/lib` suffix and link
        link_directories(${MACPORTS_PREFIX}/../lib)
        include_directories(${MACPORTS_PREFIX}/../include)
        message(STATUS "Macports detected: ${MACPORTS_PREFIX}")
    endif ()
    # Recommendation, also add a "brew --prefix" custom command to detect a homebrew build environment
    execute_process(COMMAND brew --prefix RESULT_VARIABLE DETECT_BREW OUTPUT_VARIABLE BREW_PREFIX ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if (${DETECT_BREW} EQUAL 0)
        link_directories(${BREW_PREFIX}/lib)
        include_directories(${BREW_PREFIX}/include)
        message(STATUS "Brew detected: ${BREW_PREFIX}")
    endif ()
endif (APPLE)

#  Specifically define variable WIN32 for compilations under windows (due an error in Shawn)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND WIN32)
    option(USE_MINGW_64BITS "Use 64 bits libraries for the compilation with MinGW" true)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWIN32 -D_WIN32_WINNT=0x0501")
    set(MINGW32 1)
endif ()

# Check if libraries have to be found, depending on SUMO_LIBRARIES
set(SUMO_LIBRARIES "$ENV{SUMO_LIBRARIES}" CACHE PATH "Location of SUMOLibraries dependencies")

# some user place SUMOLibraries in the same SUMO folder
if (NOT SUMO_LIBRARIES AND EXISTS "${CMAKE_SOURCE_DIR}/../SUMOLibraries")
    set(SUMO_LIBRARIES "${CMAKE_SOURCE_DIR}/../SUMOLibraries")
endif ()

# check if SUMOLibraries was found (Only in Windows)
if (SUMO_LIBRARIES AND WIN32)
    # set option for install debug and release runtime dlls
    option(INSTALL_DLL_RUNTIMES "Copy debug and release runtimes for MSVC" true)
    # Special option for MinGW32
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND WIN32)
        if (USE_MINGW_64BITS)
            message(STATUS "Using 64 bit libraries from SUMO_LIBRARIES placed in " ${SUMO_LIBRARIES} " for MinGW compilation")
        else()
            message(STATUS "Using 32 bit libraries from SUMO_LIBRARIES placed in " ${SUMO_LIBRARIES} " for MinGW compilation")
            # Use 32 bits libraries
            set(LIB_VERSION "/32bits")
            # disable optional libraries
            set(CHECK_OPTIONAL_LIBS false)
        endif ()
    elseif ("${CMAKE_MODULE_LINKER_FLAGS}" STREQUAL "/machine:x64")
        message(STATUS "Using 64 bit libraries from SUMO_LIBRARIES placed in " ${SUMO_LIBRARIES})
    else()
        message(STATUS "Using 32 bit libraries from SUMO_LIBRARIES placed in " ${SUMO_LIBRARIES})
        # Use 32 bits libraries
        set(LIB_VERSION "/32bits")
        # disable optional libraries
        set(CHECK_OPTIONAL_LIBS false)
    endif ()
    # set Xerces
    if ("${MSVC_VERSION}" STREQUAL "1800")
        # msvc2012 requieres a special version of xerces 
        file(GLOB XERCES_PATH "${SUMO_LIBRARIES}/msvc12${LIB_VERSION}/xerces-c-3.?.?")
    else()
        file(GLOB XERCES_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/xerces-c-3.?.?")
    endif ()
    # set zlib
    file(GLOB ZLIB_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/zlib-?.?.??")
    # set PROJ and GDAL
    if (EXISTS "${SUMO_LIBRARIES}${LIB_VERSION}/proj_gdal-1911")
        # "proj_gdal-1911" is used by old versions of SUMOLibraries
        file(GLOB PROJ_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/proj_gdal-1911")
        file(GLOB GDAL_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/proj_gdal-1911")
        set(PROJ_6 0)
    elseif ("${MSVC_VERSION}" STREQUAL "1800")
        # msvc2012 requieres a special version of proj_gdal 
        file(GLOB PROJ_PATH "${SUMO_LIBRARIES}/msvc12${LIB_VERSION}/proj_gdal-1911")
        file(GLOB GDAL_PATH "${SUMO_LIBRARIES}/msvc12${LIB_VERSION}/proj_gdal-1911")
        set(PROJ_6 0)
    else()
        # use proj 6 and gdal 3
        file(GLOB PROJ_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/proj-?.?.?")
        file(GLOB GDAL_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/gdal-?.?.?")
        set(PROJ_6 1)
    endif ()
    # set fox
    file(GLOB FOX_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/fox-?.?.??")
    # set eigen (no version in directory name as it is a submodule)
    file(GLOB EIGEN3_PATH "${SUMO_LIBRARIES}/eigen")
    # set all paths in prefix
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${EIGEN3_PATH};${XERCES_PATH};${ZLIB_PATH};${ZLIB_PATH}/Release;${ZLIB_PATH}/Debug;${ZLIB_PATH}/bin;${ZLIB_PATH}/lib;${PROJ_PATH};${GDAL_PATH};${FOX_PATH}")
    # save in variable SUMO_LIBRARIES_DLL all paths to dll files
    file(GLOB SUMO_LIBRARIES_DLL "${XERCES_PATH}/bin/*.dll"
                                 "${PROJ_PATH}/bin/*.dll"
                                 "${FOX_PATH}/lib/*.dll" # used by old SUMOLibraries versions
                                 "${FOX_PATH}/bin/*.dll"
                                 "${ZLIB_PATH}/bin/*.dll"
                                 "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/bzip2-?.?.?/*.dll" # used by old versions of SUMOLibraries
                                 "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/bzip2-?.?.?/bin/*.dll"
                                 "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/libpng-?.?.??/bin/*.dll"
                                 "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/tiff-?.?.?/bin/*.dll")
    # set MSVC dlls
    if ("${MSVC_VERSION}" STREQUAL "1800")
        file(GLOB MSVC_DLLS "${SUMO_LIBRARIES}${LIB_VERSION}/runtimes/*.dll" "${SUMO_LIBRARIES}/msvc12${LIB_VERSION}/runtimes/*.dll")
    else()
        file(GLOB MSVC_DLLS "${SUMO_LIBRARIES}${LIB_VERSION}/runtimes/*.dll")
        file(GLOB MORE_SUMO_LIBRARIES_DLL "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/zlib-?.?.??/bin/*.dll"
                                          "${SUMO_LIBRARIES}${LIB_VERSION}/3rdPartyLibs/sqlite-?.?.?/*.dll")
        list(APPEND SUMO_LIBRARIES_DLL ${MORE_SUMO_LIBRARIES_DLL})
    endif ()
    # set google test root
    file(GLOB GTEST_ROOT "${SUMO_LIBRARIES}/googletest-?.?.?/googletest")
    # set GoogleTest only if compiler isn't mingw32
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND WIN32 AND NOT USE_MINGW_64BITS)
        message(STATUS "Disabled Google Test in Mingw32")
    else()
        # Prevent GoogleTest from overriding our compiler/linker options
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory("${GTEST_ROOT}" "${CMAKE_BINARY_DIR}/googletest-build")
        include_directories("${GTEST_ROOT}/include")
        set(GTEST_BOTH_LIBRARIES "gtest" "gtest_main")
        set(GTEST_FOUND true)
    endif ()
    file(GLOB FFMPEG_PATH "${SUMO_LIBRARIES}/FFMPEG-?.?.?")
    file(GLOB OSG_PATH "${SUMO_LIBRARIES}/OSG-?.?.?")
    if ("${MSVC_VERSION}" STREQUAL "1800")
        file(GLOB GL2PS_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/msvc12/gl2ps-?.?.?")
    else()
        file(GLOB GL2PS_PATH "${SUMO_LIBRARIES}${LIB_VERSION}/gl2ps-?.?.?")
    endif ()
    set(CMAKE_LIBRARY_PATH "${CMAKE_LIBRARY_PATH};${GL2PS_PATH}/Release;${GL2PS_PATH}/lib") # "Release" used by old versions of SUMOLibraries
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${FFMPEG_PATH};${OSG_PATH};${GL2PS_PATH}")
    file(GLOB SUMO_OPTIONAL_LIBRARIES_DLL "${GDAL_PATH}/bin/*.dll" "${FFMPEG_PATH}/bin/*.dll" "${GL2PS_PATH}/Release/*.dll" "${GL2PS_PATH}/bin/*.dll")  # "Release" used by old versions of SUMOLibraries
    # set SWIG
    set(SWIG_EXECUTABLE "${SUMO_LIBRARIES}/swigwin-3.0.12/swig.exe")
    # set protobuf
    set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${SUMO_LIBRARIES}/3rdPartyLibs/protobuf-3.9.1")
    set(Protobuf_PROTOC_EXECUTABLE "${SUMO_LIBRARIES}/3rdPartyLibs/protobuf-3.9.1/bin/protoc.exe")
    set(Protobuf_INCLUDE_DIR "${SUMO_LIBRARIES}/3rdPartyLibs/protobuf-3.9.1/include")
    set(Protobuf_LIBRARY_DEBUG "${CMAKE_LIBRARY_PATH};${SUMO_LIBRARIES}/protobuf-3.9.1/lib/libprotobufd.lib")
    set(Protobuf_LIBRARY "${CMAKE_LIBRARY_PATH};${SUMO_LIBRARIES}/protobuf-3.9.1/lib/libprotobuf.lib")
    # set apache maven
    set(MVN_EXECUTABLE "${SUMO_LIBRARIES}/apache-maven-3.6.1/bin/mvn.cmd")
else ()
    # for Linux and Mac only
    find_package(GTest)
endif ()

find_package(XercesC REQUIRED)
if (XercesC_FOUND)
    include_directories(SYSTEM ${XercesC_INCLUDE_DIRS})
endif (XercesC_FOUND)

find_package(Proj)
if (PROJ_FOUND)
    include_directories(SYSTEM ${PROJ_INCLUDE_DIR})
    set(ENABLED_FEATURES "${ENABLED_FEATURES} Proj")
endif (PROJ_FOUND)

find_package(FOX)
if (FOX_FOUND)
    include_directories(SYSTEM ${FOX_INCLUDE_DIR})
    add_definitions(${FOX_CXX_FLAGS})
    add_definitions(-DFLOAT_MATH_FUNCTIONS)
    set(HAVE_FOX 1)
    set(ENABLED_FEATURES "${ENABLED_FEATURES} GUI")
endif (FOX_FOUND)

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

find_package(SWIG 3.0)
if (SWIG_FOUND)
    set(ENABLED_FEATURES "${ENABLED_FEATURES} SWIG")
endif ()

if (CHECK_OPTIONAL_LIBS)
    find_package(GDAL)
    if (GDAL_FOUND)
        include_directories(SYSTEM ${GDAL_INCLUDE_DIR})
        set(HAVE_GDAL 1)
        set(ENABLED_FEATURES "${ENABLED_FEATURES} GDAL")
    else (GDAL_FOUND)
        set(GDAL_LIBRARY "")
    endif (GDAL_FOUND)

    find_package(FFMPEG)
    if (FFMPEG_FOUND)
        include_directories(SYSTEM ${FFMPEG_INCLUDE_DIR})
        set(HAVE_FFMPEG 1)
        set(ENABLED_FEATURES "${ENABLED_FEATURES} FFmpeg")
    endif ()

    find_package(OpenSceneGraph 3.4.0 COMPONENTS osgGA osgViewer osgUtil osgDB osgText)
    if (OPENSCENEGRAPH_FOUND)
        include_directories(SYSTEM ${OPENSCENEGRAPH_INCLUDE_DIRS})
        set(HAVE_OSG 1)
        set(ENABLED_FEATURES "${ENABLED_FEATURES} OSG")
    endif ()

    find_package(GL2PS)
    if (GL2PS_FOUND)
        include_directories(SYSTEM ${GL2PS_INCLUDE_DIR})
        set(HAVE_GL2PS 1)
        set(ENABLED_FEATURES "${ENABLED_FEATURES} GL2PS")
    endif (GL2PS_FOUND)

    # Eigen (for overhead wire / electric circuit simulation)
    # TODO: Probably may be used in 32bit build as well.
    find_package(Eigen3 3.2 MODULE)
    if (EIGEN3_FOUND)
        include_directories(SYSTEM ${EIGEN3_INCLUDE_DIR})
        set(HAVE_EIGEN 1)  # see config.h.cmake for #cmakedefine
        set(ENABLED_FEATURES "${ENABLED_FEATURES} Eigen")
    endif (EIGEN3_FOUND)

#    set(JPS_FOUND 1)
    if (JPS_FOUND)
        add_library(jps SHARED IMPORTED) # or STATIC instead of SHARED
        set_target_properties(jps PROPERTIES IMPORTED_LOCATION "/tmp/libjps.so")
        include_directories(SYSTEM "/tmp/include")
        set(HAVE_JPS 1)
    endif (JPS_FOUND)

    list(APPEND SUMO_LIBRARIES_DLL ${SUMO_OPTIONAL_LIBRARIES_DLL})
endif (CHECK_OPTIONAL_LIBS)

include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set(commonlibs
        utils_distribution utils_shapes utils_options
        utils_xml utils_geom utils_common utils_importio utils_iodevices utils_traction_wire foreign_tcpip
        ${XercesC_LIBRARIES} ${ZLIB_LIBRARIES} ${PROJ_LIBRARY})
if (MSVC)
    set(commonlibs ${commonlibs} ws2_32)
endif ()
set(commonvehiclelibs
        utils_emissions foreign_phemlight utils_vehicle ${commonlibs} ${FOX_LIBRARY})

# set custom name and folder for ALL_BUILD and ZERO_CHECK in visual studio solutions
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")

# installation
include(GNUInstallDirs)
install(DIRECTORY bin/ DESTINATION bin FILES_MATCHING PATTERN "*.dll")
install(DIRECTORY data/ DESTINATION share/sumo/data)
install(DIRECTORY tools/ DESTINATION share/sumo/tools
        USE_SOURCE_PERMISSIONS
        PATTERN "calibration" EXCLUDE
        PATTERN "lisum-core" EXCLUDE
        PATTERN "lisum-gui" EXCLUDE
        PATTERN "sumolib4matlab/src" EXCLUDE
        PATTERN "traas" EXCLUDE
        PATTERN "traci4matlab/src" EXCLUDE
        PATTERN "__pycache__" EXCLUDE
        PATTERN "*.pyc" EXCLUDE
        PATTERN ".git" EXCLUDE)
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ../../bin \$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/sumo/bin)")

if (NOT ${SETUPTOOLS_MISSING})
    install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/build/setup-sumolib.py clean --all install --root=\$ENV{DESTDIR}/ --prefix=${CMAKE_INSTALL_PREFIX} --optimize=1)"
            COMPONENT pysumolib)
    install(CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/build/setup-traci.py clean --all install --root=\$ENV{DESTDIR}/ --prefix=${CMAKE_INSTALL_PREFIX} --optimize=1)"
            COMPONENT pytraci)
endif ()

if (SUMO_LIBRARIES AND WIN32)
    # check if MSVC dll runtimes have to be copied
    if (INSTALL_DLL_RUNTIMES)
        list(APPEND SUMO_LIBRARIES_DLL ${MSVC_DLLS})
    endif(INSTALL_DLL_RUNTIMES)
    # copy common dlls
    foreach(DLL ${SUMO_LIBRARIES_DLL})
        get_filename_component(DLL_BASENAME ${DLL} NAME)
        set(TARGET_DLL ${CMAKE_SOURCE_DIR}/bin/${DLL_BASENAME})
        list(FIND TARGET_DLLS ${TARGET_DLL} DLL_KNOWN)
        if (DLL_KNOWN EQUAL -1)
            add_custom_command(OUTPUT ${TARGET_DLL}
                               COMMAND ${CMAKE_COMMAND} -E copy ${DLL} ${TARGET_DLL}
                               DEPENDS ${DLL})
            list(APPEND TARGET_DLLS ${TARGET_DLL})
        endif()
    endforeach(DLL)
    # copy OSG dlls
    if (HAVE_OSG)
        set(OSG_TARGET ${CMAKE_SOURCE_DIR}/bin/osg.dll)
        add_custom_command(OUTPUT ${OSG_TARGET}
                           COMMAND ${CMAKE_COMMAND} -E copy_directory ${OSG_PATH}/bin ${CMAKE_SOURCE_DIR}/bin
                           DEPENDS ${OSG_PATH})
    endif (HAVE_OSG)
    # proj 6 needs to copy "share/proj" folder in SUMO_HOME/data
    if (PROJ_6)
        set(PROJ_DATA ${CMAKE_SOURCE_DIR}/data/proj/proj.db)
        add_custom_command(OUTPUT ${PROJ_DATA}
                           COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJ_PATH}/share/proj ${CMAKE_SOURCE_DIR}/data/proj
                           DEPENDS ${PROJ_PATH})
    endif (PROJ_6)
    # set install_dll target
    add_custom_target(install_dll ALL DEPENDS ${TARGET_DLLS} ${OSG_TARGET} ${PROJ_DATA})
    set_property(TARGET install_dll PROPERTY FOLDER "CMake")
endif ()

# java targets
find_program(MVN_EXECUTABLE mvn)
find_package(Java COMPONENTS Development)
if (MVN_EXECUTABLE AND Java_FOUND)
    if (NOT DEFINED ENV{JAVA_HOME})
        get_filename_component(JAVA_BIN ${Java_JAVAC_EXECUTABLE} DIRECTORY)
        get_filename_component(JAVA_HOME ${JAVA_BIN} DIRECTORY)
        set(MVN_COMMAND_PREFIX ${CMAKE_COMMAND} -E env JAVA_HOME=${JAVA_HOME})
    endif ()
    add_custom_target(traas
        COMMAND ${MVN_COMMAND_PREFIX} ${MVN_EXECUTABLE} --batch-mode -f tools/contributed/traas/pom.xml clean install
        COMMAND ${CMAKE_COMMAND} -E copy tools/contributed/traas/target/traas-1.0-jar-with-dependencies.jar bin/TraaS.jar
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_custom_target(lisum-core
        COMMAND ${MVN_COMMAND_PREFIX} ${MVN_EXECUTABLE} --batch-mode -f tools/contributed/lisum-core/pom.xml clean install
        COMMAND ${CMAKE_COMMAND} -E copy tools/contributed/lisum-core/target/lisum-core-1.0.1-SNAPSHOT-jar-with-dependencies.jar bin/lisum-core.jar
        COMMAND ${MVN_EXECUTABLE} install:install-file -Dfile=tools/contributed/lisum-core/target/lisum-core-1.0.1-SNAPSHOT.jar -DgroupId=de.dlr.ts -DartifactId=lisum-core -Dversion=1.0.1 -Dpackaging=jar
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_dependencies(lisum-core traas)
    add_custom_target(lisum-gui
        COMMAND ${MVN_COMMAND_PREFIX} ${MVN_EXECUTABLE} --batch-mode -f tools/contributed/lisum-gui/pom_new.xml clean install
        COMMAND ${CMAKE_COMMAND} -E copy tools/contributed/lisum-gui/target/lisum-gui-1.0-jar-with-dependencies.jar bin/lisum-gui.jar
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_dependencies(lisum-gui lisum-core)
    add_custom_target(lisum)
    add_dependencies(lisum lisum-gui lisum-core)
    set(JAVA_TARGETS traas lisum-core lisum-gui)
    foreach (JAVAT ${JAVA_TARGETS})
        set_property(TARGET ${JAVAT} PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
        set_property(TARGET ${JAVAT} PROPERTY FOLDER "java")
    endforeach ()
endif ()

# doc targets
add_custom_target(doxygen
    COMMAND rm -rf docs/doxygen
    COMMAND mkdir docs/doxygen
    COMMAND doxygen sumo.doxyconf &> doxygen.log
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
set_property(TARGET doxygen PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_property(TARGET doxygen PROPERTY FOLDER "doc")

add_custom_target(userdoc
    COMMAND ../../tools/build/buildPyDoc.py -p ../pydoc -c
    COMMAND mkdocs build -d ../userdoc
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/docs/web)
set_property(TARGET userdoc PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_property(TARGET userdoc PROPERTY FOLDER "doc")

if (MVN_EXECUTABLE)
    add_custom_target(javadoc
        COMMAND rm -rf docs/javadoc
        COMMAND mkdir docs/javadoc
        COMMAND ${MVN_EXECUTABLE} --batch-mode -f tools/contributed/traas/pom.xml javadoc:javadoc
        COMMAND mv tools/contributed/traas/target/site/apidocs docs/javadoc/traas
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_property(TARGET javadoc PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    set_property(TARGET javadoc PROPERTY FOLDER "doc")
    set(JAVADOC_TARGET javadoc)
endif ()

find_program(HELPMAN_EXECUTABLE help2man)
if (HELPMAN_EXECUTABLE)
    add_custom_target(man
        COMMAND rm -rf docs/man
        COMMAND mkdir docs/man
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "A microscopic, multi-modal traffic simulation" bin/sumo > docs/man/sumo.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "GUI version of the simulation SUMO" bin/sumo-gui > docs/man/sumo-gui.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Builds vehicle routes for SUMO using detector values" bin/dfrouter > docs/man/dfrouter.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Shortest path router and DUE computer for the microscopic traffic simulation SUMO" bin/duarouter > docs/man/duarouter.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Router for the microscopic traffic simulation SUMO based on junction turning ratios" bin/jtrrouter > docs/man/jtrrouter.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Import O/D-matrices and trips using macroscopic traffic assignment for SUMO" bin/marouter > docs/man/marouter.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Generates routes of persons throughout a day for the microscopic traffic simulation SUMO" bin/activitygen > docs/man/activitygen.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Importer of O/D-matrices for the traffic simulation SUMO" bin/od2trips > docs/man/od2trips.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Road network importer / builder for the traffic simulation SUMO" bin/netconvert > docs/man/netconvert.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Road network editor for the traffic simulation SUMO" bin/netedit > docs/man/netedit.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Road network generator for the microscopic traffic simulation SUMO" bin/netgenerate > docs/man/netgenerate.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Importer of polygons and POIs for the traffic simulation SUMO" bin/polyconvert > docs/man/polyconvert.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "TraCITestClient for the traffic simulation SUMO" bin/TraCITestClient > docs/man/TraCITestClient.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Builds and writes an emissions map for SUMO\\\'s emission models" bin/emissionsMap > docs/man/emissionsMap.1
        COMMAND ${HELPMAN_EXECUTABLE} -N -n "Computes emissions by driving a time line using SUMO\\\'s emission models" bin/emissionsDrivingCycle > docs/man/emissionsDrivingCycle.1
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    set_property(TARGET man PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
    set_property(TARGET man PROPERTY FOLDER "doc")
    set(MAN_TARGET man)
endif ()

add_custom_target(doc)
add_dependencies(doc doxygen userdoc ${JAVADOC_TARGET} ${MAN_TARGET})
set_property(TARGET doc PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_property(TARGET doc PROPERTY FOLDER "doc")

# coverage targets
if (COVERAGE)
    find_program(LCOV_EXECUTABLE lcov)
    if (LCOV_EXECUTABLE)
        add_custom_target(lcov
            COMMAND rm -rf docs/lcov
            COMMAND mkdir docs/lcov
            COMMAND ${LCOV_EXECUTABLE} -d . --capture --output-file docs/lcov/lcov.info
            COMMAND ${LCOV_EXECUTABLE} --remove docs/lcov/lcov.info '/usr/*' --output-file docs/lcov/lcov.info
            COMMAND genhtml -o docs/lcov/html docs/lcov/lcov.info
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
        add_custom_target(lcov-reset
            COMMAND ${LCOV_EXECUTABLE} -d . -z
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    else ()
        message(WARNING "COVERAGE is enabled but lcov was not found.")
    endif ()
endif ()

# testing
enable_testing()
add_subdirectory(src)
if (GTEST_FOUND)
    add_subdirectory(unittest)
endif ()

# example and dist targets
add_custom_target(examples
    COMMAND ${PYTHON_EXECUTABLE} tools/extractTest.py -x -f tests/examples.txt -p docs/examples/runAll.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
add_dependencies(examples sumo netconvert dfrouter duarouter jtrrouter)
set_property(TARGET examples PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
set_property(TARGET examples PROPERTY FOLDER "doc")

add_test(exampletest ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/docs/examples/runAll.py)
find_package(Git)
if (GIT_FOUND)
    add_custom_target(dist
        COMMAND rm -rf sumo-${PACKAGE_VERSION} sumo-${PACKAGE_VERSION}.zip sumo-src-${PACKAGE_VERSION}.tar.gz sumo-src-${PACKAGE_VERSION}.zip sumo-all-${PACKAGE_VERSION}.tar.gz sumo-all-${PACKAGE_VERSION}.zip sumo-gui-macos-${PACKAGE_VERSION}.zip
        COMMAND ${GIT_EXECUTABLE} archive --prefix sumo-${PACKAGE_VERSION}/ -o sumo-${PACKAGE_VERSION}.zip HEAD
        COMMAND unzip -q sumo-${PACKAGE_VERSION}.zip
        COMMAND cp -a docs/tutorial docs/examples sumo-${PACKAGE_VERSION}/docs
        COMMAND find tools/contributed/saga/ tools/contributed/traci4matlab -type f | grep -v .git | xargs cp --parents --target-dir sumo-${PACKAGE_VERSION}
        COMMAND mkdir sumo-${PACKAGE_VERSION}/include
        COMMAND cp ${CMAKE_BINARY_DIR}/src/version.h sumo-${PACKAGE_VERSION}/include
        COMMAND zip -rq sumo-src-${PACKAGE_VERSION}.zip sumo-${PACKAGE_VERSION}
        COMMAND tar -czf sumo-src-${PACKAGE_VERSION}.tar.gz sumo-${PACKAGE_VERSION}
        COMMAND cp -a docs/userdoc docs/pydoc docs/javadoc docs/man sumo-${PACKAGE_VERSION}/docs
        COMMAND tar -czf sumo-debian-src-${PACKAGE_VERSION}.tar.gz --exclude "*.jar" sumo-${PACKAGE_VERSION}
        COMMAND cp bin/*.jar sumo-${PACKAGE_VERSION}/bin
        COMMAND cp -a tests sumo-${PACKAGE_VERSION}
        COMMAND zip -rq sumo-all-${PACKAGE_VERSION}.zip sumo-${PACKAGE_VERSION}
        COMMAND tar -czf sumo-all-${PACKAGE_VERSION}.tar.gz sumo-${PACKAGE_VERSION}
        COMMAND mkdir sumo-${PACKAGE_VERSION}/sumo-gui-macos
        COMMAND cp -a build/macos/*/*.app sumo-${PACKAGE_VERSION}/sumo-gui-macos
        COMMAND zip -rq sumo-gui-macos-${PACKAGE_VERSION}.zip sumo-${PACKAGE_VERSION}/sumo-gui-macos/*.app
        COMMAND rm -rf sumo-${PACKAGE_VERSION} sumo-${PACKAGE_VERSION}.zip
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_dependencies(dist examples doc ${JAVA_TARGETS})
    set_property(TARGET dist PROPERTY FOLDER "CMake")
    set_property(TARGET dist PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)

    add_custom_target(distcheck
        COMMAND rm -rf sumo-${PACKAGE_VERSION}
        COMMAND unzip -q sumo-src-${PACKAGE_VERSION}.zip
        COMMAND cd sumo-${PACKAGE_VERSION} && mkdir _cmake_build _cmake_install && cd _cmake_build
                && cmake -DCMAKE_INSTALL_PREFIX=../_cmake_install .. || (echo "ERROR: the cmake configuration failed." && false)
                && make -j8 || (echo "ERROR: the compilation failed." && false)
                && make test || (echo "ERROR: the test suite failed." && false)
                && make install || (echo "ERROR: the install target failed." && false)
        COMMAND rm -rf sumo-${PACKAGE_VERSION}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    add_dependencies(distcheck dist)
    set_property(TARGET distcheck PROPERTY FOLDER "CMake")
    set_property(TARGET distcheck PROPERTY EXCLUDE_FROM_DEFAULT_BUILD TRUE)
endif ()
message(STATUS "Enabled features: ${ENABLED_FEATURES}")
