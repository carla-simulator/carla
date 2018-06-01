"""
Classes to handle Agent object (player and non-player)
"""

from geometry_msgs.msg import TransformStamped
from std_msgs.msg import Header
from visualization_msgs.msg import MarkerArray, Marker

from carla.sensor import Transform as carla_Transform
from carla_ros_bridge.transforms import carla_transform_to_ros_transform, ros_transform_to_pose


class AgentObjectHandler(object):
    """
    Generic class to convert carla agent information to ros messages
    In ros messages are represented as Marker message (thus they are viewable in Rviz).
    """

    def __init__(self, name, process_msg_fun=None, world_link='map'):
        self.name = name
        self.world_link = world_link
        self.process_msg_fun = process_msg_fun
        self.lookup_table_marker_id = {}

    def process_msg(self, data, cur_time):
        """

        :param data: carla agent data
        :param cur_time: current ros simulation time
        :return:
        """
        raise NotImplemented

    def get_marker_id(self, agent_id):
        """
        Return a unique marker id for each agent

        ros marker id should be int32, carla/unrealengine seems to use int64
        A lookup table is used to remap agent_id to small number between 0 and max_int32

        :param agent_id:
        :return:
        """
        if agent_id not in self.lookup_table_marker_id:
            self.lookup_table_marker_id[agent_id] = len(
                self.lookup_table_marker_id)
        return self.lookup_table_marker_id[agent_id]


class PlayerAgentHandler(AgentObjectHandler):
    """
    Convert player agent into ros message (as marker)
    """

    def __init__(self, name, **kwargs):
        super(PlayerAgentHandler, self).__init__(name, **kwargs)

    def process_msg(self, data, cur_time):
        t = TransformStamped()
        t.header.stamp = cur_time
        t.header.frame_id = self.world_link
        t.child_frame_id = "base_link"
        t.transform = carla_transform_to_ros_transform(
            carla_Transform(data.transform))
        header = Header()
        header.stamp = cur_time
        header.frame_id = self.world_link
        marker = get_vehicle_marker(
            data, header=header, marker_id=0, is_player=True)
        self.process_msg_fun(self.name, marker)
        self.process_msg_fun('tf', t)


class NonPlayerAgentsHandler(AgentObjectHandler):
    """
    Convert non-player agents into ros messages
    """

    def __init__(self, name, **kwargs):
        super(NonPlayerAgentsHandler, self).__init__(name, **kwargs)

    def process_msg(self, data, cur_time):
        """
        :param data: list of carla_server_pb2.Agent return by carla API,
        with field 'id', 'vehicle', 'pedestrian', 'traffic_light', 'speed_limit_sign'

        :return:
        """
        vehicles = [(self.get_marker_id(agent.id), agent.vehicle)
                    for agent in data if agent.HasField('vehicle')]

        header = Header(stamp=cur_time, frame_id=self.world_link)
        if not (vehicles):
            return
        markers = [
            get_vehicle_marker(vehicle, header, agent_id)
            for agent_id, vehicle in vehicles
        ]
        marker_array = MarkerArray(markers)
        self.process_msg_fun('vehicles', marker_array)


def get_vehicle_marker(object, header, marker_id=0, is_player=False):
    """
    Return a marker msg

    :param object: carla agent object (pb2 object (vehicle, pedestrian or traffic light))
    :param header: ros header (stamp/frame_id)
    :param marker_id: a marker id (int32)
    :param is_player: True if player else False (usefull to change marker color)
    :return: a ros marker msg
    """
    marker = Marker(header=header)
    marker.color.a = 0.3
    if is_player:
        marker.color.g = 1
        marker.color.r = 0
        marker.color.b = 0
    else:
        marker.color.r = 1
        marker.color.g = 0
        marker.color.b = 0

    marker.id = marker_id
    marker.text = "id = {}".format(marker_id)
    update_marker_pose(object, marker)

    if not is_player:  # related to bug request #322
        marker.scale.x = marker.scale.x / 100.0
        marker.scale.y = marker.scale.y / 100.0
        marker.scale.z = marker.scale.z / 100.0

    # the box pose seems to require a little bump to be well aligned with camera depth
    marker.pose.position.z += marker.scale.z / 2.0

    return marker


def update_marker_pose(object, base_marker):
    """
    Update pose of a marker based on carla information

    :param object: pb2 carla object
    :param base_marker: marker to update pose
    """
    ros_transform = carla_transform_to_ros_transform(
        carla_Transform(object.transform))
    base_marker.pose = ros_transform_to_pose(ros_transform)

    base_marker.scale.x = object.bounding_box.extent.x * 2.0
    base_marker.scale.y = object.bounding_box.extent.y * 2.0
    base_marker.scale.z = object.bounding_box.extent.z * 2.0

    base_marker.type = Marker.CUBE
