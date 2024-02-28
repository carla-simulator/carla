cmake_minimum_required(VERSION 3.5.1)
project(libcarla-server)

install(DIRECTORY "${libcarla_source_path}/compiler" DESTINATION include)

foreach(dir "" 
            "actor/"
            "geom/"
            "opendrive/" "opendrive/parser/"
            "profiler/"
            "road/" "road/element/" "road/general/" "road/object/" "road/signal/"
            "rpc/"
            "sensor/" "sensor/data/" "sensor/s11n/"
            "streaming/" "streaming/detail/" "streaming/detail/tcp/" "streaming/low_level/"
            "multigpu/")

  file(GLOB headers "${libcarla_source_path}/carla/${dir}*.h")
  install(FILES ${headers} DESTINATION include/carla/${dir})

endforeach()


if (LIBCARLA_USE_ROS)
  # only install the required public interface headers
  foreach(dir "" "types/" )
    file(GLOB libcarla_carla_ros2_public_headers
      "${libcarla_source_path}/carla/ros2/${dir}*.h"
    )
    install(FILES ${libcarla_carla_ros2_public_headers} DESTINATION include/carla/ros2/${dir})
  endforeach()

  file(GLOB subdirs RELATIVE "${libcarla_source_path}/carla/ros2/ros_types" "${libcarla_source_path}/carla/ros2/ros_types/*")
  foreach(dir ${subdirs})
    if(IS_DIRECTORY "${libcarla_source_path}/carla/ros2/ros_types/${dir}")
      file(GLOB libcarla_carla_ros2_types_${dir}_headers
            "${libcarla_source_path}/carla/ros2/ros_types/${dir}/msg/*.h"
      )
      install(FILES ${libcarla_carla_ros2_types_${dir}_headers} DESTINATION include/carla/ros2/ros_types/${dir}/msg/)
    endif()
  endforeach()

  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3" CACHE STRING "" FORCE)
  endif()
endif()

install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

if(WIN32)
  file(GLOB boostlibs
      "${BOOST_LIB_PATH}/libboost_date_time-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_system-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_filesystem-*-mt-*.lib")
  install(FILES ${boostlibs} DESTINATION lib)
endif()

# carla_server library.
# first the sources which are picked from some directories
set(libcarla_server_sources
  "${libcarla_source_path}/carla/Buffer.cpp"
  "${libcarla_source_path}/carla/Exception.cpp"
  "${libcarla_source_path}/carla/StringUtil.cpp"
  # other cpp files only define Deserialize used in client
  "${libcarla_source_path}/carla/sensor/s11n/SensorHeaderSerializer.cpp"
  )
# then the globs on the folders to add as a whole
foreach(dir 
      "actor"
      "geom"
      "opendrive" "opendrive/parser" 
      "profiler"
      "road" "road/element" "road/general" "road/object" "road/signal"
      "rpc"
      "sensor" "sensor/data"
      "streaming" "streaming/detail" "streaming/detail/tcp" "streaming/low_level"
      "multigpu")

  file(GLOB sources "${libcarla_source_path}/carla/${dir}/*.cpp")
  list(APPEND libcarla_server_sources ${sources})

endforeach()

foreach(thirdparty_dir
        "odrSpiral"
        "moodycamel"
        "pugixml")

  file(GLOB sources "${libcarla_source_thirdparty_path}/${thirdparty_dir}/*.cpp")
  list(APPEND libcarla_server_sources ${sources})

endforeach()

if (LIBCARLA_USE_ROS)
  foreach(ros2_dir
          "/"
          "impl/fastdds/"
          "publishers/"
          "services/"
          "subscribers/"
          "types/")

    file(GLOB sources "${libcarla_source_path}/carla/ros2/${ros2_dir}*.cpp")
    list(APPEND libcarla_server_sources ${sources})
  endforeach()

  file(GLOB sources "${libcarla_source_path}/carla/ros2/ros_types/*/msg/*.cxx")
  list(APPEND libcarla_server_sources ${sources})

  file(GLOB sources "${libcarla_source_path}/carla/ros2/ros_types/*/srv/*.cxx")
  list(APPEND libcarla_server_sources ${sources})

endif()

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)

  add_library(carla_server STATIC
    ${libcarla_server_sources}
  )
  target_compile_definitions(carla_server PUBLIC -DCARLA_SERVER_BUILD)

  target_include_directories(carla_server SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_server DESTINATION lib OPTIONAL)

  set_target_properties(carla_server PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

  if (LIBCARLA_USE_ROS)
    target_include_directories(carla_server SYSTEM PRIVATE
      "${FASTDDS_INCLUDE_PATH}"
      "${libcarla_source_path}/carla/ros2/ros_types")
    target_link_directories(carla_server PRIVATE ${FASTDDS_LIB_PATH})
    target_link_libraries(carla_server fastrtps fastcdr)
    target_compile_definitions(carla_server PUBLIC -DWITH_ROS2)
  endif()
endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_server_debug STATIC
    ${libcarla_server_sources}
  )
  target_compile_definitions(carla_server_debug PUBLIC -DCARLA_SERVER_BUILD)

  target_include_directories(carla_server_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_server_debug DESTINATION lib OPTIONAL)

  set_target_properties(carla_server_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_server_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

  if (LIBCARLA_USE_ROS)
    target_include_directories(carla_server_debug SYSTEM PRIVATE
     "${FASTDDS_INCLUDE_PATH}"
     "${libcarla_source_path}/carla/ros2/ros_types")
     target_link_directories(carla_server_debug PRIVATE ${FASTDDS_LIB_PATH})
     target_link_libraries(carla_server_debug fastrtps fastcdr)
     target_compile_definitions(carla_server_debug PUBLIC -DWITH_ROS2)
     endif()
endif()
