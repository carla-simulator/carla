cmake_minimum_required(VERSION 3.5.1)
project(libcarla_fastdds)

# Install headers.

file(GLOB libcarla_carla_ros2_headers      "${libcarla_source_path}/carla/ros2/*.h")
file(GLOB libcarla_carla_publishers_headers "${libcarla_source_path}/carla/ros2/publishers/*.h")
file(GLOB libcarla_carla_subscribers_headers "${libcarla_source_path}/carla/ros2/subscribers/*.h")
file(GLOB libcarla_carla_listeners_headers  "${libcarla_source_path}/carla/ros2/listeners/*.h")
file(GLOB libcarla_carla_types_headers      "${libcarla_source_path}/carla/ros2/types/*.h")
file(GLOB libcarla_carla_types_msg_headers  "${libcarla_source_path}/carla/ros2/types/msg/*.h")
file(GLOB libcarla_carla_middleware_headers        "${libcarla_source_path}/carla/ros2/middleware/*.h")
file(GLOB libcarla_carla_middleware_fastdds_headers "${libcarla_source_path}/carla/ros2/middleware/fastdds/*.h")

install(FILES ${libcarla_carla_ros2_headers}       DESTINATION include/carla/ros2)
install(FILES ${libcarla_carla_publishers_headers}  DESTINATION include/carla/ros2/publishers)
install(FILES ${libcarla_carla_subscribers_headers} DESTINATION include/carla/ros2/subscribers)
install(FILES ${libcarla_carla_listeners_headers}   DESTINATION include/carla/ros2/listeners)
install(FILES ${libcarla_carla_types_headers}       DESTINATION include/carla/ros2/types)
install(FILES ${libcarla_carla_types_msg_headers}   DESTINATION include/carla/ros2/types/msg)
install(FILES ${libcarla_carla_middleware_headers}         DESTINATION include/carla/ros2/middleware)
install(FILES ${libcarla_carla_middleware_fastdds_headers} DESTINATION include/carla/ros2/middleware/fastdds)

file(GLOB fast_dds_dependencies "${FASTDDS_LIB_PATH}/*.a")
install(FILES ${fast_dds_dependencies} DESTINATION lib)
install(DIRECTORY "${FASTDDS_INCLUDE_PATH}/fastcdr"   DESTINATION include)
install(DIRECTORY "${FASTDDS_INCLUDE_PATH}/fastdds"   DESTINATION include)
install(DIRECTORY "${FASTDDS_INCLUDE_PATH}/fastrtps"  DESTINATION include)

# Note: middleware/fastdds/ headers are installed here.  The one .cpp source
# (FastDDSSharedParticipant.cpp) is compiled via cmake/ros2/ (carla_ros2),
# together with all other ros2 sources, once with both
# CARLA_ROS2_MIDDLEWARE_FASTDDS and CARLA_ROS2_MIDDLEWARE_CYCLONEDDS defined.
