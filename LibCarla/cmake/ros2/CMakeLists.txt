cmake_minimum_required(VERSION 3.5.1)
project(libcarla_ros2)

# INCLUDED_DDS: comma-separated list of backends to compile (e.g. "fastdds,cyclonedds").
# Defaults to both backends if not specified.
if(NOT DEFINED INCLUDED_DDS OR INCLUDED_DDS STREQUAL "")
  set(INCLUDED_DDS "fastdds,cyclonedds")
endif()

string(REPLACE "," ";" INCLUDED_DDS_LIST "${INCLUDED_DDS}")
message(STATUS "ROS2 DDS backends: ${INCLUDED_DDS_LIST}")

# --- Shared headers (always installed) ---

file(GLOB libcarla_ros2_headers
  "${libcarla_source_path}/carla/ros2/*.h"
  "${libcarla_source_path}/carla/ros2/publishers/*.h"
  "${libcarla_source_path}/carla/ros2/subscribers/*.h"
  "${libcarla_source_path}/carla/ros2/listeners/*.h"
  "${libcarla_source_path}/carla/ros2/types/msg/*.h"
  )
install(FILES ${libcarla_ros2_headers} DESTINATION include/carla/ros2)

if("fastdds" IN_LIST INCLUDED_DDS_LIST)
  file(GLOB_RECURSE libcarla_fastdds_type_headers "${libcarla_source_path}/carla/ros2/types/fastdds/*.h")
  foreach(_hdr ${libcarla_fastdds_type_headers})
    file(RELATIVE_PATH _rel "${libcarla_source_path}/carla/ros2/types/fastdds" "${_hdr}")
    get_filename_component(_dir "${_rel}" DIRECTORY)
    install(FILES "${_hdr}" DESTINATION "include/carla/ros2/types/fastdds/${_dir}")
  endforeach()
endif()

file(GLOB libcarla_dds_headers
  "${libcarla_source_path}/carla/ros2/dds/*.h"
  )
install(FILES ${libcarla_dds_headers} DESTINATION include/carla/ros2/dds)

# --- Shared sources (always compiled) ---

file(GLOB libcarla_ros2_sources
  "${libcarla_source_path}/carla/ros2/ROS2.cpp"
  "${libcarla_source_path}/carla/ros2/publishers/*.cpp"
  "${libcarla_source_path}/carla/ros2/subscribers/*.cpp"
  "${libcarla_source_path}/carla/ros2/listeners/*.cpp"
  "${libcarla_source_path}/carla/ros2/dds/*.cpp")

if("fastdds" IN_LIST INCLUDED_DDS_LIST)
  file(GLOB_RECURSE libcarla_fastdds_type_sources "${libcarla_source_path}/carla/ros2/types/fastdds/*.cxx")
  list(APPEND libcarla_ros2_sources ${libcarla_fastdds_type_sources})
endif()

# --- Per-backend: accumulate defs, includes, native libs, headers ---

set(BACKEND_COMPILE_DEFS "")
set(BACKEND_INCLUDE_DIRS "")
set(CDDS_TYPE_SOURCES "")
set(HAS_CYCLONEDDS OFF)

if("fastdds" IN_LIST INCLUDED_DDS_LIST)
  list(APPEND BACKEND_COMPILE_DEFS "CARLA_ROS2_DDS_FASTDDS")
  list(APPEND BACKEND_INCLUDE_DIRS "${FASTDDS_INCLUDE_PATH}")

  # Install FastDDS backend headers
  file(GLOB libcarla_fastdds_backend_headers
    "${libcarla_source_path}/carla/ros2/dds/fastdds/*.h"
    )
  install(FILES ${libcarla_fastdds_backend_headers} DESTINATION include/carla/ros2/dds/fastdds)

  # Install FastDDS native libraries (*.a)
  file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.a")
  install(FILES ${fast_dds_dependencies} DESTINATION lib)

  # --- IDL -> FastDDS C++ code generation via fastddsgen ---
  find_program(FASTDDSGEN_EXECUTABLE fastddsgen
    PATHS "${FASTDDS_INCLUDE_PATH}/../bin"
    NO_DEFAULT_PATH)

  set(FASTDDS_IDL_ROOT "${libcarla_source_path}/carla/ros2/types/idl")
  set(FASTDDS_OUTPUT_ROOT "${libcarla_source_path}/carla/ros2/types/fastdds")
  file(GLOB_RECURSE FASTDDS_IDL_FILES "${FASTDDS_IDL_ROOT}/*.idl")

  if(FASTDDSGEN_EXECUTABLE)
    message(STATUS "fastddsgen found: ${FASTDDSGEN_EXECUTABLE}")
    foreach(idl ${FASTDDS_IDL_FILES})
      file(RELATIVE_PATH idl_rel "${FASTDDS_IDL_ROOT}" "${idl}")
      get_filename_component(idl_subdir "${idl_rel}" DIRECTORY)
      get_filename_component(idl_name "${idl_rel}" NAME_WE)
      set(out_dir "${FASTDDS_OUTPUT_ROOT}/${idl_subdir}")
      set(out_h "${out_dir}/${idl_name}.h")
      set(out_cxx "${out_dir}/${idl_name}.cxx")
      set(out_psh "${out_dir}/${idl_name}PubSubTypes.h")
      set(out_psc "${out_dir}/${idl_name}PubSubTypes.cxx")

      # Parse direct #include dependencies for this IDL
      file(READ "${idl}" _fdds_content)
      string(REGEX MATCHALL "#include \"[^\"]+\\.idl\"" _fdds_incs "${_fdds_content}")
      set(_fdds_deps "")
      foreach(_inc ${_fdds_incs})
        string(REGEX REPLACE "#include \"(.+)\"" "\\1" _path "${_inc}")
        if(EXISTS "${FASTDDS_IDL_ROOT}/${_path}")
          list(APPEND _fdds_deps "${FASTDDS_IDL_ROOT}/${_path}")
        endif()
      endforeach()

      add_custom_command(
        OUTPUT "${out_h}" "${out_cxx}" "${out_psh}" "${out_psc}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${out_dir}"
        COMMAND "${FASTDDSGEN_EXECUTABLE}"
          -replace
          -d "${out_dir}"
          -I "${FASTDDS_IDL_ROOT}"
          "${idl}"
        DEPENDS "${idl}" ${_fdds_deps}
        COMMENT "fastddsgen: ${idl_rel}"
        VERBATIM)
    endforeach()
  else()
    # Stale-file check: warn if any IDL is newer than its generated output
    foreach(idl ${FASTDDS_IDL_FILES})
      file(RELATIVE_PATH idl_rel "${FASTDDS_IDL_ROOT}" "${idl}")
      get_filename_component(idl_subdir "${idl_rel}" DIRECTORY)
      get_filename_component(idl_name "${idl_rel}" NAME_WE)
      set(out_h "${FASTDDS_OUTPUT_ROOT}/${idl_subdir}/${idl_name}.h")
      if(EXISTS "${out_h}")
        if("${idl}" IS_NEWER_THAN "${out_h}")
          message(WARNING
            "FastDDS generated file ${idl_subdir}/${idl_name}.h is older than ${idl_rel}. "
            "Install fastddsgen (requires JDK) and re-run cmake, or manually "
            "regenerate with: Util/BuildTools/regenerate_fastdds_types.sh")
        endif()
      endif()
    endforeach()
  endif()
endif()

if("cyclonedds" IN_LIST INCLUDED_DDS_LIST)
  set(HAS_CYCLONEDDS ON)
  list(APPEND BACKEND_COMPILE_DEFS "CARLA_ROS2_DDS_CYCLONEDDS")
  list(APPEND BACKEND_INCLUDE_DIRS "${CYCLONEDDS_INCLUDE_PATH}")

  # Install CycloneDDS backend headers
  file(GLOB libcarla_cyclonedds_backend_headers
    "${libcarla_source_path}/carla/ros2/dds/cyclonedds/*.h"
    )
  install(FILES ${libcarla_cyclonedds_backend_headers} DESTINATION include/carla/ros2/dds/cyclonedds)

  # Install CycloneDDS native libraries (*.a)
  file(GLOB cyclonedds_dependencies "${CYCLONEDDS_LIB_PATH}/*.a")
  install(FILES ${cyclonedds_dependencies} DESTINATION lib)

  # --- IDL -> C code generation via idlc ---
  get_filename_component(CYCLONEDDS_INSTALL_DIR "${CYCLONEDDS_INCLUDE_PATH}" DIRECTORY)
  find_program(IDLC_EXECUTABLE idlc
    PATHS "${CYCLONEDDS_INSTALL_DIR}/bin"
    NO_DEFAULT_PATH)
  if(NOT IDLC_EXECUTABLE)
    message(FATAL_ERROR
      "idlc not found at ${CYCLONEDDS_INSTALL_DIR}/bin/idlc. "
      "Run 'make setup' with --included-dds=cyclonedds first.")
  endif()

  set(IDL_ROOT "${libcarla_source_path}/carla/ros2/types/idl")
  set(CDDS_OUTPUT_ROOT "${libcarla_source_path}/carla/ros2/types/cyclonedds")
  file(GLOB_RECURSE ALL_IDL_FILES "${IDL_ROOT}/*.idl")

  # Parse direct #include dependencies
  foreach(idl ${ALL_IDL_FILES})
    file(READ "${idl}" _content)
    string(REGEX MATCHALL "#include \"[^\"]+\\.idl\"" _incs "${_content}")
    set(_deps "")
    foreach(_inc ${_incs})
      string(REGEX REPLACE "#include \"(.+)\"" "\\1" _path "${_inc}")
      if(EXISTS "${IDL_ROOT}/${_path}")
        list(APPEND _deps "${IDL_ROOT}/${_path}")
      endif()
    endforeach()
    set(IDL_DIRECT_DEPS_${idl} "${_deps}")
  endforeach()

  # Transitive closure (max depth ~3 for ROS2 types)
  foreach(idl ${ALL_IDL_FILES})
    set(_all ${IDL_DIRECT_DEPS_${idl}})
    set(_frontier ${_all})
    while(_frontier)
      set(_next "")
      foreach(_dep ${_frontier})
        foreach(_tdep ${IDL_DIRECT_DEPS_${_dep}})
          list(FIND _all "${_tdep}" _idx)
          if(_idx EQUAL -1)
            list(APPEND _all "${_tdep}")
            list(APPEND _next "${_tdep}")
          endif()
        endforeach()
      endforeach()
      set(_frontier ${_next})
    endwhile()
    set(IDL_ALL_DEPS_${idl} "${_all}")
  endforeach()

  # Generate per-IDL custom commands
  set(CDDS_TYPE_SOURCES "")
  foreach(idl ${ALL_IDL_FILES})
    file(RELATIVE_PATH idl_rel "${IDL_ROOT}" "${idl}")
    get_filename_component(idl_subdir "${idl_rel}" DIRECTORY)
    get_filename_component(idl_name "${idl_rel}" NAME_WE)
    set(out_dir "${CDDS_OUTPUT_ROOT}/${idl_subdir}")
    set(out_c "${out_dir}/${idl_name}.c")
    set(out_h "${out_dir}/${idl_name}.h")
    add_custom_command(
      OUTPUT "${out_c}" "${out_h}"
      COMMAND ${CMAKE_COMMAND} -E make_directory "${out_dir}"
      COMMAND "${IDLC_EXECUTABLE}" -l c -I "${IDL_ROOT}" -o "${out_dir}" "${idl}"
      DEPENDS "${idl}" ${IDL_ALL_DEPS_${idl}}
      COMMENT "idlc: ${idl_rel}"
      VERBATIM)
    list(APPEND CDDS_TYPE_SOURCES "${out_c}")
  endforeach()
endif()

# Future: if("opendds" IN_LIST INCLUDED_DDS_LIST)
#   list(APPEND BACKEND_COMPILE_DEFS "CARLA_ROS2_DDS_OPENDDS")
#   ...
# endif()

# --- Generate backends manifest for Carla.Build.cs ---

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/included_dds_backends.txt" "${INCLUDED_DDS}")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/included_dds_backends.txt" DESTINATION ".")

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)

  # If CycloneDDS is included, compile idlc-generated C files as a separate
  # pure-C library (avoids passing C++ flags to C source files).
  if(HAS_CYCLONEDDS AND CDDS_TYPE_SOURCES)
    add_library(carla_ros2_cdds_types STATIC ${CDDS_TYPE_SOURCES})
    target_include_directories(carla_ros2_cdds_types PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
    target_include_directories(carla_ros2_cdds_types PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
    set_target_properties(carla_ros2_cdds_types PROPERTIES COMPILE_FLAGS "-std=c11" POSITION_INDEPENDENT_CODE ON)
    install(TARGETS carla_ros2_cdds_types DESTINATION lib)
  endif()

  add_library(carla_ros2 STATIC ${libcarla_ros2_sources})

  target_compile_options(carla_ros2 PRIVATE -fexceptions)
  target_compile_definitions(carla_ros2 PRIVATE ${BACKEND_COMPILE_DEFS})

  target_include_directories(carla_ros2 SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_ros2 PRIVATE
      "${libcarla_source_path}/carla/ros2"
      "${libcarla_source_path}")

  if("fastdds" IN_LIST INCLUDED_DDS_LIST)
    target_include_directories(carla_ros2 PRIVATE
        "${libcarla_source_path}/carla/ros2/types/fastdds"
        "${FASTDDS_INCLUDE_PATH}")
  endif()

  foreach(inc_dir ${BACKEND_INCLUDE_DIRS})
    target_include_directories(carla_ros2 PRIVATE "${inc_dir}")
  endforeach()

  if(HAS_CYCLONEDDS)
    target_include_directories(carla_ros2 PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
    if(CDDS_TYPE_SOURCES)
      target_link_libraries(carla_ros2 ddsc carla_ros2_cdds_types)
    endif()
  endif()

  install(TARGETS carla_ros2 DESTINATION lib)
  set_target_properties(carla_ros2 PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

endif()

if (LIBCARLA_BUILD_DEBUG)

  if(HAS_CYCLONEDDS AND CDDS_TYPE_SOURCES)
    add_library(carla_ros2_cdds_types_debug STATIC ${CDDS_TYPE_SOURCES})
    target_include_directories(carla_ros2_cdds_types_debug PRIVATE "${CYCLONEDDS_INCLUDE_PATH}")
    target_include_directories(carla_ros2_cdds_types_debug PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
    set_target_properties(carla_ros2_cdds_types_debug PROPERTIES COMPILE_FLAGS "-std=c11" POSITION_INDEPENDENT_CODE ON)
    install(TARGETS carla_ros2_cdds_types_debug DESTINATION lib)
  endif()

  add_library(carla_ros2_debug STATIC ${libcarla_ros2_sources})

  target_compile_options(carla_ros2_debug PRIVATE -fexceptions)
  target_compile_definitions(carla_ros2_debug PRIVATE ${BACKEND_COMPILE_DEFS})

  target_include_directories(carla_ros2_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  target_include_directories(carla_ros2_debug PRIVATE
      "${libcarla_source_path}/carla/ros2"
      "${libcarla_source_path}")

  if("fastdds" IN_LIST INCLUDED_DDS_LIST)
    target_include_directories(carla_ros2_debug PRIVATE
        "${libcarla_source_path}/carla/ros2/types/fastdds"
        "${FASTDDS_INCLUDE_PATH}")
  endif()

  foreach(inc_dir ${BACKEND_INCLUDE_DIRS})
    target_include_directories(carla_ros2_debug PRIVATE "${inc_dir}")
  endforeach()

  if(HAS_CYCLONEDDS)
    target_include_directories(carla_ros2_debug PRIVATE "${libcarla_source_path}/carla/ros2/types/cyclonedds")
    if(CDDS_TYPE_SOURCES)
      target_link_libraries(carla_ros2_debug ddsc carla_ros2_cdds_types_debug)
    endif()
  endif()

  install(TARGETS carla_ros2_debug DESTINATION lib)
  set_target_properties(carla_ros2_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_ros2_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

endif()
