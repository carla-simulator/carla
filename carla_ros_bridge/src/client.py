#!/usr/bin/env python
"""
Ros Bridge for carla

Using python2 for now, simpler for user on ubuntu to have python2
(default setup for kinetic and lunar)

# TODO: group tf in same message ?

"""

import math
import numpy as np
import random
import time
import copy

import rosbag
import rospy
import tf

from carla.client import make_carla_client
from carla.sensor import Camera, Lidar, LidarMeasurement, Image
from carla.sensor import Transform as carla_Transform
from carla.settings import CarlaSettings

from ackermann_msgs.msg import AckermannDrive
from cv_bridge import CvBridge
from geometry_msgs.msg import TransformStamped, Transform, Pose
from rosgraph_msgs.msg import Clock
from sensor_msgs.msg import Image as RosImage
from sensor_msgs.msg import PointCloud2, CameraInfo
from sensor_msgs.point_cloud2 import create_cloud_xyz32
from std_msgs.msg import Header
from tf2_msgs.msg import TFMessage
from visualization_msgs.msg import MarkerArray, Marker


def carla_transform_to_ros_transform(carla_transform):
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

def _ros_transform_to_pose(ros_transform):
    pose = Pose()
    pose.position.x, pose.position.y, pose.position.z = ros_transform.translation.x, \
                                                        ros_transform.translation.y, \
                                                        ros_transform.translation.z

    pose.orientation.x, pose.orientation.y, pose.orientation.z, pose.orientation.w = ros_transform.rotation.x, \
                                                                                     ros_transform.rotation.y, \
                                                                                     ros_transform.rotation.z,\
                                                                                     ros_transform.rotation.w
    return pose


def update_marker_pose(object, base_marker):
    ros_transform = carla_transform_to_ros_transform(carla_Transform(object.transform))
    base_marker.pose = _ros_transform_to_pose(ros_transform)

    base_marker.scale.x = object.box_extent.x * 2.0
    base_marker.scale.y = object.box_extent.y * 2.0
    base_marker.scale.z = object.box_extent.z * 2.0

    base_marker.type = Marker.CUBE

lookup_table_marker_id = {}  # <-- TODO: migrate this in a class
def get_vehicle_marker(object, header, agent_id=88, player=False):
    """
    :param pb2 object (vehicle, pedestrian or traffic light)
    :param base_marker: a marker to fill/update
    :return: a marker
    """
    marker = Marker(header=header)
    marker.color.a = 0.3
    if player:
        marker.color.g = 1
        marker.color.r = 0
        marker.color.b = 0
    else:
        marker.color.r = 1
        marker.color.g = 0
        marker.color.b = 0

    if agent_id not in lookup_table_marker_id:
        lookup_table_marker_id[agent_id] = len(lookup_table_marker_id)
    _id = lookup_table_marker_id[agent_id]

    marker.id = _id
    marker.text = "id = {}".format(_id)
    update_marker_pose(object, marker)


    if not player:  # related to bug request #322
        marker.scale.x = marker.scale.x / 100.0
        marker.scale.y = marker.scale.y / 100.0
        marker.scale.z = marker.scale.z / 100.0

    # the box pose seems to require a little bump to be well aligned with camera depth
    marker.pose.position.z += marker.scale.z / 2.0

    return marker


class CarlaRosBridge(object):
    """
    Carla Ros bridge
    """
    def __init__(self, client, params, rate=15):
        """

        :param params: dict of parameters, see settings.yaml
        :param rate: rate to query data from carla in Hz
        """
        self.message_to_publish = []
        self.param_sensors = params['sensors']
        self.client = client
        self.carla_settings = CarlaSettings()
        self.carla_settings.set(
            SendNonPlayerAgentsInfo=True,
            NumberOfVehicles=20,
            NumberOfPedestrians=40,
            WeatherId=random.choice([1, 3, 7, 8, 14]),
            SynchronousMode=params['SynchronousMode'],
            QualityLevel=params['QualityLevel'])
        self.carla_settings.randomize_seeds()

        self.cv_bridge = CvBridge()

        self.cur_time = rospy.Time.from_sec(0)  # at the beginning of simulation
        self.carla_game_stamp = 0
        self.carla_platform_stamp = 0
        self.rate = rospy.Rate(rate)
        self.publishers = {}
        self._camera_infos = {}
        self.processes = []
        self.publishers['tf'] = rospy.Publisher("/tf", TFMessage, queue_size=100)
        self.publishers['vehicles'] = rospy.Publisher("/vehicles", MarkerArray, queue_size=10)
        self.publishers['vehicles_text'] = rospy.Publisher("/vehicles_text", MarkerArray, queue_size=10)
        self.publishers['player_vehicle'] = rospy.Publisher("/player_vehicle", Marker, queue_size=10)
        self.publishers['pedestrians'] = rospy.Publisher("/pedestrians", MarkerArray, queue_size=10)
        self.publishers['traffic_lights'] = rospy.Publisher("/traffic_lights", MarkerArray, queue_size=10)

        # default control command
        self.cur_control = {'steer': 0.0, 'throttle': 0.0, 'brake': 0.0, 'hand_brake': False, 'reverse': False}
        self.cmd_vel_subscriber = rospy.Subscriber('/ackermann_cmd', AckermannDrive, self.set_new_control_callback)
        self.world_link = 'map'
        self.sensors = {}
        self.tf_to_publish = []

    def set_new_control_callback(self, data):
        """
        Convert a Ackerman drive msg into carla control msg

        Right now the control is really simple and don't enforce acceleration and jerk command, nor the steering acceleration too
        :param data: AckermannDrive msg
        :return:
        """
        steering_angle_ctrl = data.steering_angle
        speed_ctrl = data.speed

        max_steering_angle = math.radians(500)  # 500 degrees is the max steering angle that I have on my car,
                                                #  would be nice if we could use the value provided by carla
        max_speed = 27  # just a value for me, 27 m/sec seems to be a reasonable max speed for now

        control = {}

        if abs(steering_angle_ctrl) > max_steering_angle:
            rospy.logerr("Max steering angle reached, clipping value")
            steering_angle_ctrl = np.clip(steering_angle_ctrl, -max_steering_angle, max_steering_angle)

        if abs(speed_ctrl) > max_speed:
            rospy.logerr("Max speed reached, clipping value")
            speed_ctrl = np.clip(speed_ctrl, -max_speed, max_speed)

        if speed_ctrl == 0:
            control['brake'] = True

        control['steer'] = steering_angle_ctrl / max_steering_angle
        control['throttle'] = abs(speed_ctrl / max_speed)
        control['reverse'] = True if speed_ctrl < 0 else False

        self.cur_control = control

    def send_msgs(self):
        """
        Publish all message store then clean the list of message to publish

        :return:
        """
        for name, message in self.message_to_publish:
            self.publishers[name].publish(message)
        self.message_to_publish = []

    def add_publishers(self):
        self.publishers['clock'] = rospy.Publisher("clock", Clock, queue_size=10)
        for name, _ in self.param_sensors.items():
            self.add_sensor(name)

    def compute_cur_time_msg(self):
        self.message_to_publish.append(('clock', Clock(self.cur_time)))

    def compute_sensor_msg(self, name, sensor_data):
        if isinstance(sensor_data, Image):
            self.compute_camera_transform(name, sensor_data)
            self.compute_camera_sensor_msg(name, sensor_data)
        elif isinstance(sensor_data, LidarMeasurement):
            self.compute_lidar_transform(name, sensor_data)
            self.compute_lidar_sensor_msg(name, sensor_data)
        else:
            rospy.logerr("{}, {} is not handled yet".format(name, sensor_data))


    def compute_camera_sensor_msg(self, name, sensor):
        if sensor.type == 'Depth':
            # ROS PEP 0118 : Depth images are published as sensor_msgs/Image encoded as 32-bit float. Each pixel is a depth (along the camera Z axis) in meters.
            data = np.float32(sensor.data * 1000.0)  # in carla 1.0 = 1km
            encoding = 'passthrough'
        elif sensor.type == 'SemanticSegmentation':
            encoding = 'mono16'  # for semantic segmentation we use mono16 in order to be able to limit range in rviz
            data = np.uint16(sensor.data)
        else:
            encoding = 'rgb8'
            data = sensor.data
        img_msg = self.cv_bridge.cv2_to_imgmsg(data, encoding=encoding)
        img_msg.header.frame_id = name
        img_msg.header.stamp = self.cur_time
        self.message_to_publish.append((name + '/image_raw', img_msg))

        cam_info = self._camera_infos[name]
        cam_info.header = img_msg.header
        self.message_to_publish.append((name + '/camera_info', cam_info))


    def compute_lidar_sensor_msg(self, name, sensor):
        header = Header()
        header.frame_id = name
        header.stamp = self.cur_time
        # we take the oposite of y axis (as lidar point are express in left handed coordinate system, and ros need right handed)
        # we need a copy here, because the data are read only in carla numpy array
        new_sensor_data = sensor.data.copy()
        new_sensor_data = -new_sensor_data
        # we also need to permute x and y , todo find why
        new_sensor_data = new_sensor_data[..., [1, 0, 2]]
        point_cloud_msg = create_cloud_xyz32(header, new_sensor_data)
        self.message_to_publish.append((name, point_cloud_msg))


    def compute_player_pose_msg(self, player_measurement):
        #print("Player measurement is {}".format(player_measurement))
        t = TransformStamped()
        t.header.stamp = self.cur_time
        t.header.frame_id = self.world_link
        t.child_frame_id = "base_link"
        t.transform = carla_transform_to_ros_transform(carla_Transform(player_measurement.transform))
        header = Header()
        header.stamp = self.cur_time
        header.frame_id = self.world_link
        marker = get_vehicle_marker(player_measurement, header=header, agent_id=0, player=True)
        self.message_to_publish.append(('player_vehicle', marker))
        self.tf_to_publish.append(t)

    def compute_non_player_agent_msg(self, non_player_agents):
        """

        :param non_player_agents: list of carla_server_pb2.Agent return by carla API,
        with field 'id', 'vehicle', 'pedestrian', 'traffic_light', 'speed_limit_sign'

        :return:
        """
        vehicles = [(agent.id, agent.vehicle) for agent in non_player_agents if agent.HasField('vehicle')]
        pedestrians = [(agent.id, agent.pedestrian) for agent in non_player_agents if agent.HasField('pedestrian')]
        traffic_lights = [(agent.id, agent.traffic_light) for agent in non_player_agents if agent.HasField('traffic_light')]

        # TODO: add traffic signs
        #traffic_lights = [(agent.id, agent.traffic_light) for agent in non_player_agents if agent.HasField('traffic_light')]

        header = Header(stamp=self.cur_time, frame_id=self.world_link)

        self.compute_vehicle_msgs(vehicles, header)
        #self.compute_pedestrian_msgs(pedestrians)
        #self.compute_traffic_light_msgs(traffic_lights)


    def compute_vehicle_msgs(self, vehicles, header, agent_id=8):
        """
        Add MarkerArray msg for vehicle to the list of message to be publish

        :param vehicles: list of carla pb2 vehicle
        """
        if not(vehicles):
            return

        markers = [get_vehicle_marker(vehicle, header, agent_id) for agent_id, vehicle in vehicles]
        marker_array = MarkerArray(markers)
        self.message_to_publish.append(('vehicles', marker_array))

        # adding text in rviz (TODO: refactor)
        markers_text = [copy.copy(marker) for marker in markers]
        for marker in markers_text:
            marker.type = Marker.TEXT_VIEW_FACING

        marker_array = MarkerArray(markers_text)
        self.message_to_publish.append(('vehicles_text', marker_array))


    def run(self):
        # creating ros publishers, and adding sensors to carla settings
        self.add_publishers()

        # load settings into the server
        scene = self.client.load_settings(self.carla_settings)
        # Choose one player start at random.
        number_of_player_starts = len(scene.player_start_spots)
        player_start = random.randint(0, max(0, number_of_player_starts - 1))
        # start
        self.client.start_episode(player_start)
        while not rospy.is_shutdown():
            measurements, sensor_data = self.client.read_data()
            self.carla_game_stamp = measurements.game_timestamp
            self.carla_platform_stamp = measurements.platform_timestamp
            self.cur_time = rospy.Time.from_sec(self.carla_game_stamp * 1000.0)
            self.compute_cur_time_msg()
            self.compute_player_pose_msg(measurements.player_measurements)
            self.compute_non_player_agent_msg(measurements.non_player_agents)

            self.send_msgs()

            for name, sensor in sensor_data.items():
                self.compute_sensor_msg(name, sensor)

            tf_msg = TFMessage(self.tf_to_publish)
            self.publishers['tf'].publish(tf_msg)
            self.tf_to_publish = []

            if rospy.get_param('carla_autopilot', True):
                control = measurements.player_measurements.autopilot_control
                self.client.send_control(control)
            else:
                control = self.cur_control
                self.client.send_control(**control)


            #self.rate.sleep() # <-- no need of sleep the read call should already be blocking


    def add_sensor(self, name):
        """
        Add sensor base on sensor name in settings
        :param name:
        :return:
        """
        sensor_type = self.param_sensors[name]['SensorType']
        params = self.param_sensors[name]['carla_settings']
        rospy.loginfo("Adding publisher for sensor {}".format(name))

        if sensor_type == 'LIDAR_RAY_CAST':
            self.add_lidar_sensor(name, params)
        elif sensor_type == 'CAMERA':
            self.add_camera_sensor(name, params)
        else:
            rospy.logerr("{sensor_type} not implemented".format(sensor_type=sensor_type))


    def compute_camera_transform(self, name, sensor_data):
        parent_frame_id = "base_link"
        child_frame_id = name

        t = TransformStamped()
        t.header.stamp = self.cur_time
        t.header.frame_id = parent_frame_id
        t.child_frame_id = child_frame_id

        # for camera we reorient it to look at the same axis as the opencv projection
        # in order to get easy depth cloud for rgbd camera
        t.transform = carla_transform_to_ros_transform(self.sensors[name].get_transform())

        rotation = t.transform.rotation
        quat = [rotation.x, rotation.y, rotation.z, rotation.w]
        roll, pitch, yaw = tf.transformations.euler_from_quaternion(quat)

        roll -= math.pi/2.0
        yaw -= math.pi/2.0

        quat = tf.transformations.quaternion_from_euler(roll, pitch, yaw)

        t.transform.rotation.x = quat[0]
        t.transform.rotation.y = quat[1]
        t.transform.rotation.z = quat[2]
        t.transform.rotation.w = quat[3]

        self.tf_to_publish.append(t)

    def compute_lidar_transform(self, name, sensor_data):
        parent_frame_id = "base_link"
        child_frame_id = name

        t = TransformStamped()
        t.header.stamp = self.cur_time
        t.header.frame_id = parent_frame_id
        t.child_frame_id = child_frame_id
        t.transform = carla_transform_to_ros_transform(self.sensors[name].get_transform())

        self.tf_to_publish.append(t)


    def add_camera_sensor(self, name, params):
        """

        :param name:
        :param params:
        :return:
        """
        # The default camera captures RGB images of the scene.
        cam = Camera(name, **params)

        self.carla_settings.add_sensor(cam)
        self.sensors[name] = cam  # we also add the sensor to our lookup
        topic_image = name + '/image_raw'
        topic_camera = name + '/camera_info'
        self.publishers[topic_image] = rospy.Publisher(topic_image, RosImage, queue_size=10)
        self.publishers[topic_camera] = rospy.Publisher(topic_camera, CameraInfo, queue_size=10)

        # computing camera info, when publishing update the stamp
        camera_info = CameraInfo()
        camera_info.header.frame_id = name
        camera_info.width = cam.ImageSizeX
        camera_info.height = cam.ImageSizeY
        camera_info.distortion_model = 'plumb_bob'
        cx = cam.ImageSizeX / 2.0
        cy = cam.ImageSizeY / 2.0
        fx = cam.ImageSizeX / (2.0 * math.tan(cam.FOV * math.pi / 360.0))
        fy = fx

        camera_info.K = [fx, 0, cx,
                         0,  fy, cy,
                         0, 0, 1]

        camera_info.D = [0, 0, 0, 0, 0]

        camera_info.R = [1.0, 0, 0,
                         0, 1.0, 0,
                         0, 0, 1.0]

        camera_info.P = [fx, 0, cx, 0,
                         0, fy, cy, 0,
                         0, 0, 1.0, 0]

        self._camera_infos[name] = camera_info


    def add_lidar_sensor(self, name, params):
        """

        :param name:
        :param params:
        :return:
        """
        lidar = Lidar(name, **params)
        self.carla_settings.add_sensor(lidar)
        self.sensors[name] = lidar  # we also add the sensor to our lookup
        self.publishers[name] = rospy.Publisher(name, PointCloud2, queue_size=10)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        rospy.loginfo("Exiting Bridge")
        return None


class CarlaRosBridgeWithBag(CarlaRosBridge):
    def __init__(self, *args, **kwargs):
        super(CarlaRosBridgeWithBag, self).__init__(*args, **kwargs)
        timestr = time.strftime("%Y%m%d-%H%M%S")
        self.bag = rosbag.Bag('/tmp/output_{}.bag'.format(timestr), mode='w')

    def send_msgs(self):
        for name, msg in self.message_to_publish:
            self.bag.write(name, msg, self.cur_time)

        super(CarlaRosBridgeWithBag, self).send_msgs()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        rospy.loginfo("Closing the bag file")
        self.bag.close()
        super(CarlaRosBridgeWithBag, self).__exit__(exc_type, exc_value, traceback)



def main():
    rospy.init_node("carla_client", anonymous=True)


    params = rospy.get_param('carla')
    host = params['host']
    port = params['port']

    rospy.loginfo("Trying to connect to {host}:{port}".format(host=host, port=port))

    with make_carla_client(host, port) as client:
        rospy.loginfo("Connection is ok")

        bridge_cls = CarlaRosBridgeWithBag if rospy.get_param('enable_rosbag') else CarlaRosBridge
        with bridge_cls(client=client, params=params) as carla_ros_bridge:
            carla_ros_bridge.run()


if __name__ == "__main__":
    main()



