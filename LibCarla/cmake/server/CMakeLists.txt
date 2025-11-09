cmake_minimum_required(VERSION 3.5.1)
project(libcarla-server)

# Install libc++ shared libraries.
file(GLOB LibCXX_Libraries "${LLVM_LIB_PATH}/libc++*")
install(FILES ${LibCXX_Libraries} DESTINATION lib)

# Install rpclib.
install(DIRECTORY "${RPCLIB_INCLUDE_PATH}/rpc" DESTINATION include)
file(GLOB libcarla_carla_rpclib "${RPCLIB_LIB_PATH}/*.*")
install(FILES ${libcarla_carla_rpclib} DESTINATION lib)

# Install headers.

install(DIRECTORY "${libcarla_source_path}/compiler" DESTINATION include)

foreach(dir "" 
            "actors/"
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

  file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.a")
  install(FILES ${fast_dds_dependencies} DESTINATION lib)

  if(NOT WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3" CACHE STRING "" FORCE)
  endif()
endif()

# Install boost headers and libraries.
install(DIRECTORY "${BOOST_INCLUDE_PATH}/boost" DESTINATION include)

if(WIN32)
  file(GLOB boostlibs
      "${BOOST_LIB_PATH}/libboost_date_time-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_system-*-mt-*.lib"
      "${BOOST_LIB_PATH}/libboost_filesystem-*-mt-*.lib")
  install(FILES ${boostlibs} DESTINATION lib)
endif()

# carla_server library.
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
      "actors"
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
          ""
          "fastdds/carla/ros2/impl/"
          "publishers/"
          "services/"
          "subscribers/"
          "types/")

    file(GLOB sources "${libcarla_source_path}/carla/ros2/${ros2_dir}*.cpp")
    list(APPEND libcarla_server_sources ${sources})
  endforeach()

  file(GLOB msg_sources "${libcarla_source_path}/carla/ros2/fastdds/*/msg/*.cxx")
  list(APPEND libcarla_server_sources ${msg_sources})

  file(GLOB srv_sources "${libcarla_source_path}/carla/ros2/fastdds/*/srv/*.cxx")
  list(APPEND libcarla_server_sources ${srv_sources})

endif()

# ==============================================================================
# Create targets for debug and release in the same build type.
# ==============================================================================

if (LIBCARLA_BUILD_RELEASE)

  add_library(carla_server STATIC ${libcarla_server_sources})

  target_include_directories(carla_server SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_server DESTINATION lib OPTIONAL)

  set_target_properties(carla_server PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_RELEASE}")

  if (LIBCARLA_USE_ROS)
    target_include_directories(carla_server SYSTEM PRIVATE
     # first the fastdds local folder allowing potential overrides of header files
     "${libcarla_source_path}/carla/ros2/fastdds"
     "${FASTDDS_INCLUDE_PATH}")
    target_link_directories(carla_server PRIVATE ${FASTDDS_LIB_PATH})
    target_link_libraries(carla_server fastrtps fastcdr)
    target_compile_definitions(carla_server PUBLIC -DWITH_ROS2)
    target_compile_options(carla_server PRIVATE -fexceptions)
  endif()

endif()

if (LIBCARLA_BUILD_DEBUG)

  add_library(carla_server_debug STATIC ${libcarla_server_sources})

  target_include_directories(carla_server_debug SYSTEM PRIVATE
      "${BOOST_INCLUDE_PATH}"
      "${RPCLIB_INCLUDE_PATH}")

  install(TARGETS carla_server_debug DESTINATION lib OPTIONAL)

  set_target_properties(carla_server_debug PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
  target_compile_definitions(carla_server_debug PUBLIC -DBOOST_ASIO_ENABLE_BUFFER_DEBUGGING)

  if (LIBCARLA_USE_ROS)
    target_include_directories(carla_server_debug SYSTEM PRIVATE
     # first the fastdds local folder allowing potential overrides of header files
     "${libcarla_source_path}/carla/ros2/fastdds"
     "${FASTDDS_INCLUDE_PATH}")
     target_link_directories(carla_server_debug PRIVATE ${FASTDDS_LIB_PATH})
     target_link_libraries(carla_server_debug fastrtps fastcdr)
     target_compile_definitions(carla_server_debug PUBLIC -DWITH_ROS2)
    target_compile_options(carla_server_debug PRIVATE -fexceptions)
  endif()

endif()
