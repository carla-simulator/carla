"""
Tool functions to convert transforms from carla to ros coordinate system
"""
from geometry_msgs.msg import Vector3, Quaternion, Transform, Pose
import tf
import numpy
import carla 

def carla_location_to_ros_translation(carla_location):
    """
    Convert a carla transform to a ros transform
    :param carla_transform:
    :return: a ros transform
    """
    ros_translation = Vector3()
    # remember that we go from left-handed system (unreal) to right-handed system (ros)
    ros_translation.x = carla_location.x
    ros_translation.y = -carla_location.y
    ros_translation.z = carla_location.z

    return ros_translation

def carla_rotation_to_ros_rotation(carla_rotation):

    roll = -carla_rotation.roll
    pitch = carla_rotation.pitch
    yaw = -carla_rotation.yaw

    ros_quaternion = Quaternion()

    quat = tf.transformations.quaternion_from_euler(roll, pitch, yaw)
    ros_quaternion.x = quat[0]
    ros_quaternion.y = quat[1]
    ros_quaternion.z = quat[2]
    ros_quaternion.w = quat[3]
    
    return ros_quaternion

def carla_transform_to_ros_transform(carla_transform):
    """
    Convert a carla transform to a ros transform
    :param carla_transform:
    :return: a ros transform
    """
    ros_transform = Transform()

    ros_transform.translation = carla_location_to_ros_translation(carla_transform.location)
    ros_transform.rotation = carla_rotation_to_ros_rotation(carla_transform.rotation)

    return ros_transform

def carla_location_to_ros_transform(carla_location):
    """
    Convert a carla transform to a ros transform
    :param carla_transform:
    :return: a ros transform
    """
    ros_transform = Transform()

    ros_transform.translation = carla_location_to_ros_translation(carla_location)
    
    ros_transform.rotation = carla_rotation_to_ros_rotation( carla.Rotation() )

    return ros_transform

def ros_transform_to_matrix(ros_transform):
    trans_mat = tf.transformations.translation_matrix((ros_transform.translation.x, ros_transform.translation.y, ros_transform.translation.z ))
    rot_mat   = tf.transformations.quaternion_matrix( (ros_transform.rotation.x, ros_transform.rotation.y, ros_transform.rotation.z, ros_transform.rotation.w) )
    mat = numpy.dot(trans_mat, rot_mat)
    return mat

def concat_two_ros_transforms(ros_transform_left, ros_transform_right):
    mat_left = ros_transform_to_matrix(ros_transform_left)
    mat_right = ros_transform_to_matrix(ros_transform_right)
    
    mat_result = numpy.dot(mat_left, mat_right)
    ros_transform_result = Transform()
    trans = tf.transformations.translation_from_matrix(mat_result)
    ros_transform_result.translation.x = trans[0]
    ros_transform_result.translation.y = trans[1]
    ros_transform_result.translation.z = trans[2]
    quat = tf.transformations.quaternion_from_matrix(mat_result)
    ros_transform_result.rotation.x = quat[0]
    ros_transform_result.rotation.y = quat[1]
    ros_transform_result.rotation.z = quat[2]
    ros_transform_result.rotation.w = quat[3]
    
    return ros_transform_result

def ros_transform_to_pose(ros_transform):
    """
    Util function to convert a ros transform into a ros pose

    :param ros_transform:
    :return: a ros pose msg
    """
    pose = Pose()
    pose.position.x = ros_transform.translation.x
    pose.position.y = ros_transform.translation.y
    pose.position.z = ros_transform.translation.z

    pose.orientation = ros_transform.rotation
    return pose
