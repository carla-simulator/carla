cmake_minimum_required(VERSION 3.5.1)
project(libcarla_fastdds)

# Install the required public interface headers
foreach(dir "" "types/" )
  file(GLOB libcarla_carla_ros2_public_headers
    "${libcarla_source_path}/carla/ros2/${dir}*.h"
  )
  install(FILES ${libcarla_carla_ros2_public_headers} DESTINATION include/carla/ros2/${dir})
endforeach()

file(GLOB subdirs RELATIVE "${libcarla_source_path}/carla/ros2/fastdds" "${libcarla_source_path}/carla/ros2/fastdds/*")
foreach(typedir "msg" "srv")
  foreach(dir ${subdirs})
    if(IS_DIRECTORY "${libcarla_source_path}/carla/ros2/fastdds/${dir}/${typedir}")
      file(GLOB libcarla_carla_ros2_types_${dir}_headers
      "${libcarla_source_path}/carla/ros2/fastdds/${dir}/${typedir}/*.h"
      "${libcarla_source_path}/carla/ros2/fastdds/${dir}/${typedir}/*.hpp"
      "${libcarla_source_path}/carla/ros2/fastdds/${dir}/${typedir}/*.ipp"
      )
      install(FILES ${libcarla_carla_ros2_types_${dir}_headers} DESTINATION include/carla/ros2/ros_types/${dir}/${typedir}/)
    endif()
  endforeach()
endforeach()

# Install dependencies for UE4 build
file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.a")
install(FILES ${fast_dds_dependencies} DESTINATION lib)
file(GLOB fast_dds_include_dirs "${FASTDDS_INCLUDE_PATH}/*")
foreach(fast_dds_include_dir ${fast_dds_include_dirs})
  install(DIRECTORY "${fast_dds_include_dir}" DESTINATION include)
endforeach()

# Collect the sources
foreach(ros2_dir
        "/"
        "fastdds/carla/ros2/impl/"
        "publishers/"
        "services/"
        "subscribers/"
        "types/")

  file(GLOB sources "${libcarla_source_path}/carla/ros2/${ros2_dir}*.cpp")
  list(APPEND libcarla_fastdds_sources ${sources})
endforeach()

file(GLOB msg_sources "${libcarla_source_path}/carla/ros2/fastdds/*/msg/*.cxx")
list(APPEND libcarla_fastdds_sources ${msg_sources})

file(GLOB srv_sources "${libcarla_source_path}/carla/ros2/fastdds/*/srv/*.cxx")
list(APPEND libcarla_fastdds_sources ${srv_sources})

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================
set(libcarla_fastdds_include_directories
    # first the fastdds local folder allowing potential overrides of header files
    "${libcarla_source_path}/carla/ros2/fastdds"
    "${BOOST_INCLUDE_PATH}"
    "${RPCLIB_INCLUDE_PATH}"
    "${FASTDDS_INCLUDE_PATH}"
    "${libcarla_source_path}/carla/ros2"
)

if (LIBCARLA_BUILD_RELEASE)
  add_library(carla_fastdds STATIC
    ${libcarla_fastdds_sources}
  )
  target_include_directories(carla_fastdds SYSTEM PRIVATE
    ${libcarla_fastdds_include_directories}
  )
  target_link_directories(carla_fastdds PRIVATE 
    ${FASTDDS_LIB_PATH}
  )
  install(TARGETS carla_fastdds DESTINATION lib)
  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${CMAKE_CXX_FLAGS_RELEASE}" CACHE STRING "" FORCE)
  endif()
  set_target_properties(carla_fastdds PROPERTIES COMPILE_FLAGS
    "-fexceptions ${CMAKE_CXX_FLAGS_RELEASE}")
  target_compile_definitions(carla_fastdds PUBLIC 
    WITH_ROS2 CARLA_SERVER_BUILD)
endif()

if (LIBCARLA_BUILD_DEBUG)
  add_library(carla_fastdds_debug STATIC 
    ${libcarla_fastdds_sources}
  )
  target_include_directories(carla_fastdds_debug SYSTEM PRIVATE
      "${libcarla_fastdds_include_directories}"
  )
  install(TARGETS carla_fastdds_debug DESTINATION lib)
  set_target_properties(carla_fastdds_debug PROPERTIES COMPILE_FLAGS
    "-fexceptions ${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_fastdds_debug PUBLIC 
    WITH_ROS2 CARLA_SERVER_BUILD BOOST_ASIO_ENABLE_BUFFER_DEBUGGING)
endif()
