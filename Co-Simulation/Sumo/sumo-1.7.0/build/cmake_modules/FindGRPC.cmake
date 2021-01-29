find_package(PkgConfig)
pkg_check_modules(PC_GRPC grpc++)
set(GRPC_DEFINITIONS ${PC_GRPC_CFLAGS_OTHER})

find_path(GRPC_INCLUDE_DIR grpc++/grpc++.h
        HINTS ${PC_GRPC_INCLUDEDIR} ${PC_GRPC_INCLUDE_DIRS}
        PATHS_SUFIXES grpc++)

find_library(GRPC_LIBRARY NAMES grpc++ libgrpc++
        HINTS ${PC_GRPC_LIBDIR} ${PC_GRPC_LIBRARY_DIRS})

set(GRPC_LIBRARIES ${GRPC_LIBRARY})
set(GRPC_INCLUDE_DIRS ${GRPC_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GRPC DEFAULT_MSG
        GRPC_LIBRARY GRPC_INCLUDE_DIR)
mark_as_advanced(GRPC_INCLUDE_DIR GRPC_LIBRARY)

find_program(GRPC_CPP_PLUGIN grpc_cpp_plugin) # Get full path to plugin

function(PROTOBUF_GENERATE_GRPC_CPP SRCS HDRS)
    if(NOT ARGN)
        message(SEND_ERROR "Error: PROTOBUF_GENERATE_GRPC_CPP() called without any proto files")
        return()
    endif()

    if(PROTOBUF_GENERATE_CPP_APPEND_PATH) # This variable is common for all types of output.
        # Create an include path for each file specified
        foreach(FIL ${ARGN})
            get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
            get_filename_component(ABS_PATH ${ABS_FIL} PATH)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if(${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif()
        endforeach()
    else()
        set(_protobuf_include_path -I ${CMAKE_CURRENT_SOURCE_DIR})
    endif()

    if(DEFINED PROTOBUF_IMPORT_DIRS)
        foreach(DIR ${PROTOBUF_IMPORT_DIRS})
            get_filename_component(ABS_PATH ${DIR} ABSOLUTE)
            list(FIND _protobuf_include_path ${ABS_PATH} _contains_already)
            if(${_contains_already} EQUAL -1)
                list(APPEND _protobuf_include_path -I ${ABS_PATH})
            endif()
        endforeach()
    endif()

    set(${SRCS})
    set(${HDRS})
    foreach(FIL ${ARGN})
        get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
        get_filename_component(FIL_WE ${FIL} NAME_WE)

        list(APPEND ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.cc")
        list(APPEND ${HDRS} "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.h")

        add_custom_command(
                OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.cc"
                "${CMAKE_CURRENT_BINARY_DIR}/${FIL_WE}.grpc.pb.h"
                COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
                ARGS --grpc_out=${CMAKE_CURRENT_BINARY_DIR}
                --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
                ${_protobuf_include_path} ${ABS_FIL}
                DEPENDS ${ABS_FIL} ${PROTOBUF_PROTOC_EXECUTABLE}
                COMMENT "Running gRPC C++ protocol buffer compiler on ${FIL}"
                VERBATIM)
    endforeach()

    set_source_files_properties(${${SRCS}} ${${HDRS}} PROPERTIES GENERATED TRUE)
    set(${SRCS} ${${SRCS}} PARENT_SCOPE)
    set(${HDRS} ${${HDRS}} PARENT_SCOPE)
endfunction()
