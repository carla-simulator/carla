"""
Rosbridge class:

Class that handle communication between CARLA and ROS
"""
import rospy
import threading
import carla 

from rosgraph_msgs.msg import Clock
from tf2_msgs.msg import TFMessage
from derived_object_msgs.msg import ObjectArray

from carla_ros_bridge.actor_parent import ActorParent


class CarlaRosBridge(ActorParent):
    """
    Carla Ros bridge
    """
    
    def __init__(self, world, params):
        """
        :param params: dict of parameters, see settings.yaml
        """
        self.params = params
        super(CarlaRosBridge, self).__init__(id=0, carla_world = world)

        self.carla_world_timestamp = carla.Timestamp()
        self.tf_to_publish = []
        self.msgs_to_publish = []

        self.carla_world.on_tick( self.carla_time_tick )
        self.update_lock = threading.Lock()

        self.publishers = {}
        self.publishers['clock'] = rospy.Publisher(
            'clock', Clock, queue_size=10)

        self.publishers['tf'] = rospy.Publisher(
            'tf', TFMessage, queue_size=100)
        
        self.publishers['/carla/objects'] = rospy.Publisher(
            '/carla/objects', ObjectArray, queue_size=10)
        self.object_array = None

    def get_param(self, key):
        return self.params[key]

    def on_shutdown(self):
        rospy.loginfo("Shutdown requested")

    def get_current_ros_time(self):
        return self.ros_timestamp

    def publish_ros_message(self, topic, msg):
        """
        Function used to process message

        Here we create publisher if not yet created
        Store the message in a list (waiting for their publication) with their associated publisher

        Messages for /tf topics are handle differently in order to publish all transform in the same message
        :param topic: topic to publish the message on
        :param msg: ros message
        """
        if topic == 'tf':
            # transform are merged in same message
            self.tf_to_publish.append(msg)
        elif topic == '/carla/objects':
            # objects are collected in same message
            if self.object_array == None:
                self.object_array = ObjectArray(header = msg.header)
            self.object_array.objects.append(msg)
        else:
            if topic not in self.publishers:
                self.publishers[topic] = rospy.Publisher(topic, type(msg), queue_size=10)
            self.msgs_to_publish.append((self.publishers[topic], msg))

    def send_msgs(self):
        try:
            for publisher, msg in self.msgs_to_publish:
                publisher.publish(msg)
        except:
            rospy.logwarn("Failed to publish messages")
            pass
        self.msgs_to_publish = []

        tf_msg = TFMessage(self.tf_to_publish)
        self.publishers['tf'].publish(tf_msg)
        self.tf_to_publish = []

        if not self.object_array == None:
            self.publishers['/carla/objects'].publish(self.object_array)
            self.object_array = None

    def carla_time_tick(self, carla_timestamp):
        self.carla_world_timestamp = carla_timestamp
        self.perform_update()

    def update_clock(self):
        self.ros_timestamp = rospy.Time.from_sec(self.carla_world_timestamp.elapsed_seconds)
        self.publish_ros_message('clock', Clock(self.ros_timestamp))

    def topic_name(self):
        return "/carla"

    def perform_update(self):
        if not rospy.is_shutdown():
            if self.update_lock.acquire(False):
                self.update_clock()
                self.update_children()
                self.send_msgs()
                self.update_lock.release()



    def run(self):
        rospy.spin()
    
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self.update_lock.acquire():
            rospy.loginfo("Exiting Bridge")
            self.carla_world=None
            self.publishers.clear()
            super(CarlaRosBridge, self).__del__()
        return None

#    def add_sensor(self, name):
#        rospy.loginfo("Adding sensor {}".format(name))
        
        # creating handler to handle vehicles messages
#        self.player_handler = PlayerAgentHandler(
#            "player_vehicle", process_msg_fun=self.process_msg)
#        self.non_players_handler = NonPlayerAgentsHandler(
#            "vehicles", process_msg_fun=self.process_msg)

        # creating handler for sensors
 #       self.sensors = {}
 #       for name, _ in self.param_sensors.items():
 #           self.add_sensor(name)

        # creating input controller listener
#        self.input_controller = InputController()
#        sensor_type = self.param_sensors[name]['SensorType']
# #       params = self.param_sensors[name]['carla_settings']
#        sensor_handler = None
#        if sensor_type == 'LIDAR_RAY_CAST':
#            sensor_handler = LidarHandler
#        if sensor_type == 'CAMERA':
#            sensor_handler = CameraHandler
#        if sensor_handler:
#            self.sensors[name] = sensor_handler(
#                name,
#                params,
##                carla_settings=self.carla_settings,
#                process_msg_fun=self.process_msg)
#        else:
#            rospy.logerr(
#                "Unable to handle sensor {name} of type {sensor_type}".format(
#                    sensor_type=sensor_type, name=name))
