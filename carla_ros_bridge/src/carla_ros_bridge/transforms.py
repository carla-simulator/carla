"""
Tool functions to convert transforms from carla to ros coordinate system
"""
from geometry_msgs.msg import Transform, Pose
import tf


def carla_transform_to_ros_transform(carla_transform):
    """
    Convert a carla transform to a ros transform
    :param carla_transform:
    :return: a ros transform
    """
    transform_matrix = carla_transform.matrix

    x, y, z = tf.transformations.translation_from_matrix(transform_matrix)
    quat = tf.transformations.quaternion_from_matrix(transform_matrix)

    ros_transform = Transform()
    # remember that we go from left-handed system (unreal) to right-handed system (ros)
    ros_transform.translation.x = x
    ros_transform.translation.y = -y
    ros_transform.translation.z = z

    roll, pitch, yaw = tf.transformations.euler_from_quaternion(quat)
    roll = -roll
    pitch = pitch
    yaw = -yaw

    quat = tf.transformations.quaternion_from_euler(roll, pitch, yaw)
    ros_transform.rotation.x = quat[0]
    ros_transform.rotation.y = quat[1]
    ros_transform.rotation.z = quat[2]
    ros_transform.rotation.w = quat[3]

    return ros_transform


def carla_transform_to_ros_pose(carla_transform):
    """
    convert a carla transform to ros pose msg
    :param carla_transform:
    :return: a ros pose msg
    """
    transform_matrix = Transform(carla_transform).matrix

    x, y, z = tf.transformations.translation_from_matrix(transform_matrix)
    quat = tf.transformations.quaternion_from_matrix(transform_matrix)

    ros_transform = Transform()
    ros_transform.translation.x = x
    ros_transform.translation.y = y
    ros_transform.translation.z = z

    ros_transform.rotation.x = quat[0]
    ros_transform.rotation.y = quat[1]
    ros_transform.rotation.z = quat[2]
    ros_transform.rotation.w = quat[3]

    return ros_transform


def ros_transform_to_pose(ros_transform):
    """
    Util function to convert a ros transform into a ros pose

    :param ros_transform:
    :return: a ros pose msg
    """
    pose = Pose()
    pose.position.x, pose.position.y, pose.position.z = ros_transform.translation.x, \
                                                        ros_transform.translation.y, \
                                                        ros_transform.translation.z

    pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w = ros_transform.rotation.x, \
                                                                                     ros_transform.rotation.y, \
                                                                                     ros_transform.rotation.z, \
                                                                                     ros_transform.rotation.w
    return pose
