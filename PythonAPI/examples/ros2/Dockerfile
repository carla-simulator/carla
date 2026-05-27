ARG ROS_DISTRO=humble
FROM osrf/ros:${ROS_DISTRO}-desktop

# FastDDS (rmw_fastrtps_cpp) ships in the base image; select it explicitly.
ENV RMW_IMPLEMENTATION=rmw_fastrtps_cpp
