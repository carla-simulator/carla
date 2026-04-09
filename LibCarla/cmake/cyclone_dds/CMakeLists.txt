cmake_minimum_required(VERSION 3.5.1)
project(libcarla_cyclonedds)

# Install headers.

file(GLOB libcarla_carla_ros2_headers        "${libcarla_source_path}/carla/ros2/*.h")
file(GLOB libcarla_carla_publishers_headers  "${libcarla_source_path}/carla/ros2/publishers/*.h")
file(GLOB libcarla_carla_subscribers_headers "${libcarla_source_path}/carla/ros2/subscribers/*.h")
file(GLOB libcarla_carla_listeners_headers   "${libcarla_source_path}/carla/ros2/listeners/*.h")
file(GLOB libcarla_carla_types_headers       "${libcarla_source_path}/carla/ros2/types/*.h")
file(GLOB libcarla_carla_types_msg_headers   "${libcarla_source_path}/carla/ros2/types/msg/*.h")
file(GLOB libcarla_carla_dds_headers         "${libcarla_source_path}/carla/ros2/dds/*.h")
file(GLOB libcarla_carla_dds_cyclonedds_headers "${libcarla_source_path}/carla/ros2/dds/cyclonedds/*.h")

install(FILES ${libcarla_carla_ros2_headers}          DESTINATION include/carla/ros2)
install(FILES ${libcarla_carla_publishers_headers}    DESTINATION include/carla/ros2/publishers)
install(FILES ${libcarla_carla_subscribers_headers}   DESTINATION include/carla/ros2/subscribers)
install(FILES ${libcarla_carla_listeners_headers}     DESTINATION include/carla/ros2/listeners)
install(FILES ${libcarla_carla_types_headers}         DESTINATION include/carla/ros2/types)
install(FILES ${libcarla_carla_types_msg_headers}     DESTINATION include/carla/ros2/types/msg)
install(FILES ${libcarla_carla_dds_headers}           DESTINATION include/carla/ros2/dds)
install(FILES ${libcarla_carla_dds_cyclonedds_headers} DESTINATION include/carla/ros2/dds/cyclonedds)

file(GLOB cyclonedds_dependencies "${CYCLONEDDS_LIB_PATH}/*.a")
install(FILES ${cyclonedds_dependencies} DESTINATION lib)
install(DIRECTORY "${CYCLONEDDS_INCLUDE_PATH}/dds"   DESTINATION include)
install(DIRECTORY "${CYCLONEDDS_INCLUDE_PATH}/ddsc"  DESTINATION include)

# Note: dds/cyclonedds/ compiled sources (CycloneDDSSertype.cpp) are included
# in cmake/ros2/ (carla_ros2), compiled once with both middleware macros defined.
