if (${CMAKE_VERSION} VERSION_GREATER 3.13.0)
    # UseSWIG generates standard target names
    cmake_policy(SET CMP0078 NEW)
endif()
if (${CMAKE_VERSION} VERSION_GREATER 3.14.0)
    # pass module name to swig compiler
    cmake_policy(SET CMP0086 NEW)
endif()
set(libsumostatic_STAT_SRCS
    Edge.cpp
    Edge.h
    Helper.cpp
    Helper.h
    InductionLoop.cpp
    InductionLoop.h
    Junction.cpp
    Junction.h
    Lane.cpp
    Lane.h
    LaneArea.cpp
    LaneArea.h
    MultiEntryExit.h
    MultiEntryExit.cpp
    Person.cpp
    Person.h
    POI.cpp
    POI.h
    Polygon.h
    Polygon.cpp
    Route.h
    Route.cpp
    Calibrator.h
    Calibrator.cpp
    BusStop.h
    BusStop.cpp
    ParkingArea.h
    ParkingArea.cpp
    ChargingStation.h
    ChargingStation.cpp
    RouteProbe.h
    RouteProbe.cpp
    Rerouter.h
    Rerouter.cpp
    VariableSpeedSign.h
    VariableSpeedSign.cpp
    MeanData.h
    MeanData.cpp
    OverheadWire.h
    OverheadWire.cpp
    Simulation.cpp
    Simulation.h
    Subscription.h
    TraCIConstants.h
    TraCIDefs.h
    TrafficLight.cpp
    TrafficLight.h
    Vehicle.cpp
    Vehicle.h
    VehicleType.cpp
    VehicleType.h
)

add_library(libsumostatic STATIC ${libsumostatic_STAT_SRCS})
add_dependencies(libsumostatic generate-version-h)
set_property(TARGET libsumostatic PROPERTY FOLDER "libsumo")

set(ENABLE_PYTHON_BINDINGS ON CACHE BOOL "Build Python Bindings")
set(ENABLE_JAVA_BINDINGS ON CACHE BOOL "Build Java Bindings")
set(DEFAULT_LIBSUMO_PYTHON ON CACHE BOOL "Include Python Bindings in the ALL target")

if(SWIG_FOUND)
    include(${SWIG_USE_FILE})
    set_source_files_properties(libsumo.i PROPERTIES CPLUSPLUS ON)
    file(GLOB LIBSUMO_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    if(ENABLE_JAVA_BINDINGS)
        find_package(Java)
        find_package(JNI)
        if(JAVA_FOUND AND JNI_FOUND)
            include(UseJava)
            include_directories(${JNI_INCLUDE_DIRS})
            set(CMAKE_SWIG_FLAGS -package org.eclipse.sumo.libsumo)
            set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/java)
            set(SWIG_MODULE_libsumojni_EXTRA_DEPS ${LIBSUMO_HEADERS})
            if (${CMAKE_VERSION} VERSION_GREATER 3.8.0)
                SWIG_ADD_LIBRARY(libsumojni LANGUAGE java SOURCES libsumo.i)
            else()
                SWIG_ADD_MODULE(libsumojni java libsumo.i)
            endif()
            if (MSVC OR APPLE)
                swig_link_libraries(libsumojni libsumostatic ${sumolibs})
            else()
                set_source_files_properties(${swig_generated_file_fullname} PROPERTIES COMPILE_FLAGS "-Wno-strict-aliasing")
                swig_link_libraries(libsumojni -Wl,--whole-archive libsumostatic ${sumolibs} -Wl,--no-whole-archive)
            endif()
            FILE(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/classes")
            ADD_CUSTOM_COMMAND(TARGET libsumojni POST_BUILD
                COMMAND "${Java_JAVAC_EXECUTABLE}" -d classes java/*.java
                COMMAND "${Java_JAR_EXECUTABLE}" -cfM ${CMAKE_SOURCE_DIR}/bin/libsumojni.jar -C classes .
                COMMAND "${Java_JAR_EXECUTABLE}" -cfM ${CMAKE_SOURCE_DIR}/bin/libsumojni_src.jar -C java .
            )
            set_property(TARGET libsumojni PROPERTY FOLDER "libsumo")
        else()
            message(WARNING "ENABLE_JAVA_BINDINGS is set but Java or JNI was not found.")
        endif()
    endif()

    find_package(PythonLibs)
    if(ENABLE_PYTHON_BINDINGS)
        if (MSVC)
            # recheck that the platform of the generator and python matches
            execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "import sys; print(sys.maxsize > 2**32)"
                            OUTPUT_VARIABLE IS_PYTHON64 OUTPUT_STRIP_TRAILING_WHITESPACE)
            if (${CMAKE_MODULE_LINKER_FLAGS} STREQUAL "/machine:x64")
                if (${IS_PYTHON64} STREQUAL "False")
                    message(STATUS "Did not find Python 64 bit. Please set PYTHON_EXECUTABLE, PYTHON_INCLUDE_DIR and PYTHON_LIBRARY manually.")
                    set(PYTHONLIBS_FOUND OFF)
                endif()
            else()
                if (${IS_PYTHON64} STREQUAL "True")
                    message(STATUS "Did not find Python 32 bit. Please set PYTHON_EXECUTABLE, PYTHON_INCLUDE_DIR and PYTHON_LIBRARY manually.")
                    set(PYTHONLIBS_FOUND OFF)
                endif()
            endif()
        endif()
        if(PYTHONLIBS_FOUND)
            if (PYTHON_DEBUG_LIBRARY OR NOT "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
                include_directories(${PYTHON_INCLUDE_PATH})
#                set(CMAKE_SWIG_FLAGS "")
                set(CMAKE_SWIG_FLAGS "-keyword") # is necessary for using keyword args, but generates lots of warnings
# other options which should be evaluated: -builtin, -modern, -extranative
                set(CMAKE_SWIG_OUTDIR ${CMAKE_SOURCE_DIR}/tools/libsumo)
                set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/libsumo)
                set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/tools/libsumo)
                set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/tools/libsumo)
                set(SWIG_MODULE_libsumo_EXTRA_DEPS ${LIBSUMO_HEADERS})
                if (${CMAKE_VERSION} VERSION_GREATER 3.8.0)
                    SWIG_ADD_LIBRARY(libsumo LANGUAGE python SOURCES libsumo.i)
                else()
                    SWIG_ADD_MODULE(libsumo python libsumo.i)
                endif()
                if (NOT DEFAULT_LIBSUMO_PYTHON)
                    # needed to selectively build the python module on the build server
                    set_property(TARGET ${SWIG_MODULE_libsumo_REAL_NAME} PROPERTY EXCLUDE_FROM_ALL TRUE)
                endif()
                if (MSVC OR APPLE)
                    # disable python module for the debug build because virtually no one has a python debug dll to link against
#                    set_property(TARGET ${SWIG_MODULE_libsumo_REAL_NAME} PROPERTY EXCLUDE_FROM_DEFAULT_BUILD_DEBUG TRUE)
                    set_property(TARGET ${SWIG_MODULE_libsumo_REAL_NAME} PROPERTY FOLDER "libsumo")
                    swig_link_libraries(libsumo libsumostatic ${sumolibs} ${PYTHON_LIBRARIES})
                else()
                    swig_link_libraries(libsumo -Wl,--whole-archive libsumostatic ${sumolibs} -Wl,--no-whole-archive)
                endif()
                if (NOT ${SETUPTOOLS_MISSING})
                    install(
                        CODE "execute_process(COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/build/setup-libsumo.py clean --all install --root=\$ENV{DESTDIR}/ --prefix=${CMAKE_INSTALL_PREFIX} --optimize=1)"
                        COMPONENT pylibsumo)
                endif()
            endif()
        else()
            message(WARNING "ENABLE_PYTHON_BINDINGS is set but Python libraries were not found.")
        endif()
    endif()

endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin)
add_library(libsumocpp SHARED ${libsumostatic_STAT_SRCS})
set_property(TARGET libsumocpp PROPERTY FOLDER "libsumo")
set_target_properties(libsumocpp PROPERTIES PREFIX "")
install(TARGETS libsumocpp DESTINATION ${CMAKE_INSTALL_LIBDIR})
if (MSVC OR APPLE)
    target_link_libraries(libsumocpp PRIVATE ${sumolibs})
else ()
    target_link_libraries(libsumocpp PRIVATE -Wl,--whole-archive ${sumolibs} -Wl,--no-whole-archive)
endif()
