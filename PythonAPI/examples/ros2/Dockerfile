ARG ROS_DISTRO=humble
FROM osrf/ros:${ROS_DISTRO}-desktop

ARG ROS_DISTRO=humble
ARG RMW_IMPLEMENTATION=rmw_fastrtps_cpp

# Install the selected middleware package; FastDDS is already in the base image
RUN if [ "$RMW_IMPLEMENTATION" = "rmw_cyclonedds_cpp" ]; then \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-rmw-cyclonedds-cpp \
    && rm -rf /var/lib/apt/lists/*; \
    elif [ "$RMW_IMPLEMENTATION" = "rmw_zenoh_cpp" ]; then \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    ros-${ROS_DISTRO}-rmw-zenoh-cpp \
    && rm -rf /var/lib/apt/lists/*; \
    fi

ENV RMW_IMPLEMENTATION=${RMW_IMPLEMENTATION}
