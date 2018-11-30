"""
Rosbridge class:

Class that handle communication between CARLA and ROS
"""
import threading
import rospy

from rosgraph_msgs.msg import Clock
from tf2_msgs.msg import TFMessage
from derived_object_msgs.msg import ObjectArray

from carla_ros_bridge.parent import Parent
from carla_ros_bridge.map import Map


class CarlaRosBridge(Parent):

    """
    Carla Ros bridge
    """

    def __init__(self, carla_world, params):
        """
        Constructor

        :param carla_world: carla world object
        :type carla_world: carla.World
        :param params: dict of parameters, see settings.yaml
        :type params: dict
        """
        self.params = params
        super(CarlaRosBridge, self).__init__(
            carla_id=0, carla_world=carla_world, frame_id='/map')

        self.ros_timestamp = rospy.Time()
        self.tf_to_publish = []
        self.msgs_to_publish = []

        self.carla_world.on_tick(self._carla_time_tick)
        self.update_lock = threading.Lock()

        self.publishers = {}
        self.publishers['clock'] = rospy.Publisher(
            'clock', Clock, queue_size=10)

        self.publishers['tf'] = rospy.Publisher(
            'tf', TFMessage, queue_size=100)

        self.publishers['/carla/objects'] = rospy.Publisher(
            '/carla/objects', ObjectArray, queue_size=10)
        self.object_array = ObjectArray()

        self.map = Map(carla_world=self.carla_world, parent=self, topic='/map')

    def destroy(self):
        """
        Function (virtual) to destroy this object.

        Lock the update mutex.
        Remove all publisher.
        Finally forward call to super class.

        :return:
        """
        if self.update_lock.acquire():
            rospy.loginfo("Exiting Bridge")
            self.publishers.clear()
            super(CarlaRosBridge, self).destroy()

    def get_current_ros_time(self):
        """
        Function (override) to return the current ROS time.

        :return: The latest received ROS time of the bridge
        :rtype: rospy.Time
        """
        return self.ros_timestamp

    def publish_ros_message(self, topic, msg):
        """
        Function (override) to publish ROS messages.

        Store the message in a list (waiting for their publication)
        with their associated publisher.
        If required corresponding publishers are created automatically.

        Messages for /tf topics and /carla/objects are handle differently
        in order to publish all transforms, objects in the same message

        :param topic: ROS topic to publish the message on
        :type topic: string
        :param msg: the ROS message to be published
        :type msg: a valid ROS message type
        :return:
        """
        if topic == 'tf':
            # transform are merged in same message
            self.tf_to_publish.append(msg)
        elif topic == '/carla/objects':
            # objects are collected in same message
            self.object_array.objects.append(msg)
        else:
            if topic not in self.publishers:
                self.publishers[topic] = rospy.Publisher(
                    topic, type(msg), queue_size=10)
            self.msgs_to_publish.append((self.publishers[topic], msg))

    def get_param(self, key):
        """
        Function (override) to query global parameters passed from the outside.

        :param key: the key of the parameter
        :type key: string
        :return: the parameter string
        :rtype: string
        """
        return self.params[key]

    def topic_name(self):
        """
        Function (override) to get the topic name of this root entity.

        The topic root '/carla' is returned by this bridge class.

        :return: the final topic name of this
        :rtype: string
        """
        return "/carla"

    def _carla_time_tick(self, carla_timestamp):
        """
        Private callback registered at carla.World.on_tick()
        to trigger cyclic updates.

        After successful locking the update mutex
        (only perform trylock to respect bridge processing time)
        the clock and the children are updated.
        Finally the ROS messages collected to be published are sent out.

        :param carla_timestamp: the current carla time
        :type carla_timestamp: carla.Timestamp
        :return:
        """
        if not rospy.is_shutdown():
            if self.update_lock.acquire(False):
                self._update_clock(carla_timestamp)
                self.update()
                self._prepare_msgs()
                self.send_msgs()
                self.update_lock.release()

    def _update_clock(self, carla_timestamp):
        """
        Private function to perform the update of the clock

        :param carla_timestamp: the current carla time
        :type carla_timestamp: carla.Timestamp
        :return:
        """
        self.ros_timestamp = rospy.Time.from_sec(
            carla_timestamp.elapsed_seconds)
        self.publish_ros_message('clock', Clock(self.ros_timestamp))

    def _prepare_msgs(self):
        """
        Private function to prepare tf and object message to be sent out

        :return:
        """
        tf_msg = TFMessage(self.tf_to_publish)
        self.msgs_to_publish.append((self.publishers['tf'], tf_msg))
        self.tf_to_publish = []

        self.object_array.header = self.get_msg_header()
        self.msgs_to_publish.append(
            (self.publishers['/carla/objects'], self.object_array))
        self.object_array = ObjectArray()

    def send_msgs(self):
        """
        Function to actually send the collected ROS messages out via the publisher

        :return:
        """
        try:
            for publisher, msg in self.msgs_to_publish:
                publisher.publish(msg)
        except:
            rospy.logwarn("Failed to publish messages")
        self.msgs_to_publish = []

    def run(self):
        """
        Run the bridge functionality.

        Registers on shutdown callback at rospy and spins ROS.

        :return:
        """
        rospy.on_shutdown(self.on_shutdown)
        rospy.spin()

    def on_shutdown(self):
        """
        Function to be called on shutdown.

        This function is registered at rospy as shutdown handler.

        """
        rospy.loginfo("Shutdown requested")
        self.destroy()
