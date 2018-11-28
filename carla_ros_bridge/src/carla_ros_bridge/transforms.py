"""
Tool functions to convert transforms from carla to ROS coordinate system
"""

import math

import tf
from geometry_msgs.msg import Vector3, Quaternion, Transform, Pose, Point, Twist, Accel


def carla_location_to_ros_vector3(carla_location):
    """
    Convert a carla location to a ROS vector3

    Considers the conversion from left-handed system (unreal) to right-handed
    system (ROS)

    :param carla_location: the carla location
    :type carla_location: carla.Location
    :return: a ROS vector3
    :rtype: geometry_msgs.msg.Vector3
    """
    ros_translation = Vector3()
    ros_translation.x = carla_location.x
    ros_translation.y = -carla_location.y
    ros_translation.z = carla_location.z

    return ros_translation


def carla_location_to_ros_point(carla_location):
    """
    Convert a carla location to a ROS point

    Considers the conversion from left-handed system (unreal) to right-handed
    system (ROS)

    :param carla_location: the carla location
    :type carla_location: carla.Location
    :return: a ROS point
    :rtype: geometry_msgs.msg.Point
    """
    ros_point = Point()
    ros_point.x = carla_location.x
    ros_point.y = -carla_location.y
    ros_point.z = carla_location.z

    return ros_point


def carla_rotation_to_ros_quaternion(carla_rotation):
    """
    Convert a carla rotation to a ROS quaternion

    Considers the conversion from left-handed system (unreal) to right-handed
    system (ROS).
    Considers the conversion from degrees (carla) to radians (ROS).

    :param carla_rotation: the carla rotation
    :type carla_rotation: carla.Rotation
    :return: a ROS quaternion
    :rtype: geometry_msgs.msg.Quaternion
    """

    roll = -math.radians(carla_rotation.roll)
    pitch = math.radians(carla_rotation.pitch)
    yaw = -math.radians(carla_rotation.yaw)

    ros_quaternion = Quaternion()

    quat = tf.transformations.quaternion_from_euler(roll, pitch, yaw)
    ros_quaternion.x = quat[0]
    ros_quaternion.y = quat[1]
    ros_quaternion.z = quat[2]
    ros_quaternion.w = quat[3]

    return ros_quaternion


def carla_velocity_to_ros_twist(carla_velocity):
    """
    Convert a carla velocity to a ROS twist

    Considers the conversion from left-handed system (unreal) to right-handed
    system (ROS)
    The angular velocities remain zero.

    :param carla_velocity: the carla velocity
    :type carla_velocity: carla.Vector3D
    :return: a ROS twist
    :rtype: geometry_msgs.msg.Twist
    """
    ros_twist = Twist()
    ros_twist.linear.x = carla_velocity.x
    ros_twist.linear.y = -carla_velocity.y
    ros_twist.linear.z = carla_velocity.z

    return ros_twist


def carla_acceleration_to_ros_accel(carla_acceleration):
    """
    Convert a carla acceleration to a ROS accel

    Considers the conversion from left-handed system (unreal) to right-handed
    system (ROS)
    The angular accelerations remain zero.

    :param carla_acceleration: the carla acceleration
    :type carla_acceleration: carla.Vector3D
    :return: a ROS accel
    :rtype: geometry_msgs.msg.Accel
    """
    ros_accel = Accel()
    ros_accel.linear.x = carla_acceleration.x
    ros_accel.linear.y = -carla_acceleration.y
    ros_accel.linear.z = carla_acceleration.z

    return ros_accel


def carla_transform_to_ros_transform(carla_transform):
    """
    Convert a carla transform to a ROS transform

    See carla_location_to_ros_vector3() and carla_rotation_to_ros_quaternion() for details

    :param carla_transform: the carla transform
    :type carla_transform: carla.Transform
    :return: a ROS transform
    :rtype: geometry_msgs.msg.Transform
    """
    ros_transform = Transform()

    ros_transform.translation = carla_location_to_ros_vector3(
        carla_transform.location)
    ros_transform.rotation = carla_rotation_to_ros_quaternion(
        carla_transform.rotation)

    return ros_transform


def carla_transform_to_ros_pose(carla_transform):
    """
    Convert a carla transform to a ROS pose

    See carla_location_to_ros_point() and carla_rotation_to_ros_quaternion() for details

    :param carla_transform: the carla transform
    :type carla_transform: carla.Transform
    :return: a ROS pose
    :rtype: geometry_msgs.msg.Pose
    """
    ros_pose = Pose()

    ros_pose.position = carla_location_to_ros_point(
        carla_transform.location)
    ros_pose.orientation = carla_rotation_to_ros_quaternion(
        carla_transform.rotation)

    return ros_pose


def carla_location_to_pose(carla_location):
    """
    Convert a carla location to a ROS pose

    See carla_location_to_ros_point() for details.
    pose quaternion remains zero.

    :param carla_location: the carla location
    :type carla_location: carla.Location
    :return: a ROS pose
    :rtype: geometry_msgs.msg.Pose
    """
    ros_pose = Pose()
    ros_pose.position = carla_location_to_ros_point(carla_location)
    return ros_pose
