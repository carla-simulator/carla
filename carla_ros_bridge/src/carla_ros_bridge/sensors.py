"""
Classes to handle Carla sensors
"""
import math
import numpy as np
import tf

from cv_bridge import CvBridge
from geometry_msgs.msg import TransformStamped
from sensor_msgs.msg import CameraInfo
from sensor_msgs.point_cloud2 import create_cloud_xyz32
from std_msgs.msg import Header

from carla.sensor import Camera, Lidar, Sensor
from carla_ros_bridge.transforms import carla_transform_to_ros_transform

cv_bridge = CvBridge(
)  # global cv bridge to convert image between opencv and ros


class SensorHandler(object):
    """
    Generic Sensor Handler

    A sensor handler compute the associated ros message to a carla sensor data, and the transform asssociated to the
    sensor.
    These messages are passed to a *process_msg_fun* which will take care of publishing them.
    """

    def __init__(self,
                 name,
                 params=None,
                 carla_sensor_class=Sensor,
                 carla_settings=None,
                 process_msg_fun=None):
        """
        :param name: sensor name
        :param params: params used to set the sensor in carla
        :param carla_sensor_class: type of sensor
        :param carla_settings: carla_settings object
        :param process_msg_fun: function to call on each new computed message
        """
        self.process_msg_fun = process_msg_fun
        self.carla_object = carla_sensor_class(name, **params)
        carla_settings.add_sensor(self.carla_object)
        self.name = name

    def process_sensor_data(self, data, cur_time):
        """
        process a carla sensor data object

        Generate sensor message and transform message

        :param data: carla sensor data
        :param cur_time: current ros simulation time
        :return:
        """
        self._compute_sensor_msg(data, cur_time)
        self._compute_transform(data, cur_time)

    def _compute_sensor_msg(self, data, cur_time):
        """
        Compute the ros msg associated to carla data
        :param data: SensorData object
        :param cur_time: current ros simulation time
        """
        raise NotImplemented

    def _compute_transform(self, data, cur_time):
        """
        Compute the tf msg associated to carla data

        :param data: SensorData object
        :param cur_time: current ros simulation time
        """
        raise NotImplemented


class LidarHandler(SensorHandler):
    """
    Class to handle Lidar sensors
    """

    def __init__(self, name, params, **kwargs):
        super(LidarHandler, self).__init__(
            name, carla_sensor_class=Lidar, params=params, **kwargs)

    def _compute_sensor_msg(self, sensor_data, cur_time):
        header = Header()
        header.frame_id = self.name
        header.stamp = cur_time
        # we take the oposite of y axis (as lidar point are express in left handed coordinate system, and ros need right handed)
        # we need a copy here, because the data are read only in carla numpy array
        new_sensor_data = sensor_data.data.copy()
        new_sensor_data = -new_sensor_data
        # we also need to permute x and y
        new_sensor_data = new_sensor_data[..., [1, 0, 2]]
        point_cloud_msg = create_cloud_xyz32(header, new_sensor_data)
        topic = self.name
        self.process_msg_fun(topic, point_cloud_msg)

    def _compute_transform(self, sensor_data, cur_time):
        parent_frame_id = "base_link"
        child_frame_id = self.name

        t = TransformStamped()
        t.header.stamp = self.cur_time
        t.header.frame_id = parent_frame_id
        t.child_frame_id = child_frame_id
        t.transform = carla_transform_to_ros_transform(
            self.carla_object.get_transform())

        self.process_msg_fun('tf', t)


class CameraHandler(SensorHandler):
    """
    Class to handle Camera sensors
    """

    def __init__(self, name, params, **kwargs):
        super(CameraHandler, self).__init__(
            name, carla_sensor_class=Camera, params=params, **kwargs)

        self.topic_image = '/'.join([name, 'image_raw'])
        self.topic_cam_info = '/'.join([name, 'camera_info'])
        self.build_camera_info()

    def build_camera_info(self):
        """
        computing camera info

        camera info doesn't change over time
        """
        camera_info = CameraInfo()
        camera_info.header.frame_id = self.name
        camera_info.width = self.carla_object.ImageSizeX
        camera_info.height = self.carla_object.ImageSizeY
        camera_info.distortion_model = 'plumb_bob'
        cx = self.carla_object.ImageSizeX / 2.0
        cy = self.carla_object.ImageSizeY / 2.0
        fx = self.carla_object.ImageSizeX / (
            2.0 * math.tan(self.carla_object.FOV * math.pi / 360.0))
        fy = fx
        camera_info.K = [fx, 0, cx, 0, fy, cy, 0, 0, 1]
        camera_info.D = [0, 0, 0, 0, 0]
        camera_info.R = [1.0, 0, 0, 0, 1.0, 0, 0, 0, 1.0]
        camera_info.P = [fx, 0, cx, 0, 0, fy, cy, 0, 0, 0, 1.0, 0]
        self._camera_info = camera_info

    def _compute_sensor_msg(self, sensor_data, cur_time):
        if sensor_data.type == 'Depth':
            # ROS PEP 0118 : Depth images are published as sensor_msgs/Image encoded as 32-bit float.
            # Each pixel is a depth (along the camera Z axis) in meters.
            data = np.float32(sensor_data.data * 1000.0)  # in carla 1.0 = 1km
            encoding = 'passthrough'
        elif sensor_data.type == 'SemanticSegmentation':
            encoding = 'mono16'  # for semantic segmentation we use mono16 in order to be able to limit range in rviz
            data = np.uint16(sensor_data.data)
        else:
            encoding = 'rgb8'
            data = sensor_data.data
        img_msg = cv_bridge.cv2_to_imgmsg(data, encoding=encoding)
        img_msg.header.frame_id = self.name
        img_msg.header.stamp = cur_time

        cam_info = self._camera_info
        cam_info.header = img_msg.header

        self.process_msg_fun(self.topic_cam_info, cam_info)
        self.process_msg_fun(self.topic_image, img_msg)

    def _compute_transform(self, sensor_data, cur_time):
        parent_frame_id = "base_link"
        child_frame_id = self.name

        t = TransformStamped()
        t.header.stamp = cur_time
        t.header.frame_id = parent_frame_id
        t.child_frame_id = child_frame_id

        # for camera we reorient it to look at the same axis as the opencv projection
        # in order to get easy depth cloud for RGBD camera
        t.transform = carla_transform_to_ros_transform(
            self.carla_object.get_transform())

        rotation = t.transform.rotation
        quat = [rotation.x, rotation.y, rotation.z, rotation.w]
        roll, pitch, yaw = tf.transformations.euler_from_quaternion(quat)

        roll -= math.pi / 2.0
        yaw -= math.pi / 2.0

        quat = tf.transformations.quaternion_from_euler(roll, pitch, yaw)

        t.transform.rotation.x = quat[0]
        t.transform.rotation.y = quat[1]
        t.transform.rotation.z = quat[2]
        t.transform.rotation.w = quat[3]

        self.process_msg_fun('tf', t)
