"""
Class to handle Carla camera sensors
"""
from abc import abstractmethod

import math
import numpy as np

import rospy
import tf
from cv_bridge import CvBridge
from sensor_msgs.msg import CameraInfo

from carla_ros_bridge.sensor import Sensor


class Camera(Sensor):

    """
    Sensor implementation details for cameras
    """

    # global cv bridge to convert image between opencv and ros
    cv_bridge = CvBridge()

    @staticmethod
    def create_actor(carla_actor, parent):
        """
        Static factory method to create camera actors

        :param carla_actor: carla camera actor object
        :type carla_actor: carla.Camera
        :param parent: the parent of the new traffic actor
        :type parent: carla_ros_bridge.Parent
        :return: the created camera actor
        :rtype: carla_ros_bridge.Camera or derived type
        """
        if carla_actor.type_id.startswith("sensor.camera.rgb"):
            return RgbCamera(carla_actor=carla_actor, parent=parent)
        else:
            return Camera(carla_actor=carla_actor, parent=parent)

    def __init__(self, carla_actor, parent, topic_prefix=None):
        """
        Constructor

        :param carla_actor: carla actor object
        :type carla_actor: carla.Actor
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this actor
        :type topic_prefix: string
        """
        if topic_prefix is None:
            topic_prefix = 'camera'
        super(Camera, self).__init__(carla_actor=carla_actor,
                                     parent=parent,
                                     topic_prefix=topic_prefix)

        if self.__class__.__name__ == "Camera":
            rospy.logwarn("Created Unsupported Camera Actor"
                          "(id={}, parent_id={}, type={}, attributes={})".format(
                              self.get_id(), self.get_parent_id(),
                              self.carla_actor.type_id, self.carla_actor.attributes))
        else:
            self.build_camera_info()
            self.carla_actor.listen(self.update_camera_data)

    def destroy(self):
        """
        Function (override) to destroy this object.

        Stop listening to the carla.Sensor actor.
        Finally forward call to super class.

        :return:
        """
        rospy.logdebug("Destroy Camera(id={})".format(self.get_id()))
        if self.carla_actor.is_listening:
            self.carla_actor.stop()
        super(Camera, self).destroy()

    def build_camera_info(self):
        """
        Function to compute camera info

        camera info doesn't change over time
        """
        camera_info = CameraInfo()
        # store info without header
        camera_info.header = None
        camera_info.width = int(self.carla_actor.attributes['image_size_x'])
        camera_info.height = int(self.carla_actor.attributes['image_size_y'])
        camera_info.distortion_model = 'plumb_bob'
        cx = camera_info.width / 2.0
        cy = camera_info.height / 2.0
        fx = camera_info.width / (
            2.0 * math.tan(float(self.carla_actor.attributes['fov']) * math.pi / 360.0))
        fy = fx
        camera_info.K = [fx, 0, cx, 0, fy, cy, 0, 0, 1]
        camera_info.D = [0, 0, 0, 0, 0]
        camera_info.R = [1.0, 0, 0, 0, 1.0, 0, 0, 0, 1.0]
        camera_info.P = [fx, 0, cx, 0, 0, fy, cy, 0, 0, 0, 1.0, 0]
        self._camera_info = camera_info

    def update_camera_data(self, carla_image):
        """
        Function to transform the a received carla image into a ROS image message

        :param carla_image: carla image object
        :type carla_image: carla.Image
        """
        if ((carla_image.height != self._camera_info.height) or
                (carla_image.width != self._camera_info.width)):
            rospy.logerr(
                "Camera{} received image not matching configuration".format(self.topic_name()))

        image_data_array = self.get_carla_image_data_array(
            carla_image=carla_image)
        img_msg = Camera.cv_bridge.cv2_to_imgmsg(image_data_array)
        # the camera data is in respect to the camera's own frame
        img_msg.header = self.get_msg_header(use_parent_frame=False)

        cam_info = self._camera_info
        cam_info.header = img_msg.header

        self.publish_ros_message(self.topic_name() + '/camera_info', cam_info)
        self.publish_ros_message(
            self.topic_name() + '/image_color', img_msg)

    def update(self):
        """
        Function (override) to update this object.

        On update camera sends:
        - its own tf used by the camera data

        :return:
        """
        self.send_tf_msg()
        super(Camera, self).update()

    def send_tf_msg(self):
        """
        Function (override) to send tf messages of this camera.

        The camera transformation has to be altered to look at the same axis
        as the opencv projection in order to get easy depth cloud for RGBD camera

        :return:
        """
        tf_msg = self.get_tf_msg()
        rotation = tf_msg.transform.rotation
        quat = [rotation.x, rotation.y, rotation.z, rotation.w]
        quat_swap = tf.transformations.quaternion_from_matrix(
            [[0, 0, 1, 0],
             [-1, 0, 0, 0],
             [0, -1, 0, 0],
             [0, 0, 0, 1]])
        quat = tf.transformations.quaternion_multiply(quat, quat_swap)

        tf_msg.transform.rotation.x = quat[0]
        tf_msg.transform.rotation.y = quat[1]
        tf_msg.transform.rotation.z = quat[2]
        tf_msg.transform.rotation.w = quat[3]
        self.publish_ros_message('tf', tf_msg)

    @abstractmethod
    def get_carla_image_data_array(self, carla_image):
        """
        Virtual function to convert the carla image to a numpy data array
        as input for the cv_bridge.cv2_to_imgmsg() function

        :param carla_image: carla image object
        :type carla_image: carla.Image
        :return numpy data array containing the image information
        :rtype np.ndarray
        """
        raise NotImplementedError(
            "This function has to be re-implemented by derived classes")


class RgbCamera(Camera):

    """
    Camera implementation details for rgb cameras
    """

    def __init__(self, carla_actor, parent, topic_prefix=None):
        """
        Constructor

        :param carla_actor: carla actor object
        :type carla_actor: carla.Actor
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this actor
        :type topic_prefix: string
        """
        if topic_prefix is None:
            topic_prefix = 'camera/rgb'
        super(RgbCamera, self).__init__(carla_actor=carla_actor,
                                        parent=parent,
                                        topic_prefix=topic_prefix)

    def get_carla_image_data_array(self, carla_image):
        """
        Virtual function to convert the carla image to a numpy data array
        as input for the cv_bridge.cv2_to_imgmsg() function

        :param carla_image: carla image object
        :type carla_image: carla.Image
        :return numpy data array containing the image information
        :rtype np.ndarray
        """
        carla_image_data_array = np.ndarray(
            shape=(carla_image.height, carla_image.width, 4),
            dtype=np.uint8, buffer=carla_image.raw_data)

        return carla_image_data_array
