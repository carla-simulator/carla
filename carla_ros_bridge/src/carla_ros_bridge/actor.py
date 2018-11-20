"""
Base Classes to handle Actor objects
"""
import rospy


from std_msgs.msg import Header
from visualization_msgs.msg import Marker

from carla_ros_bridge.actor_parent import ActorParent
from carla_ros_bridge.actor_id_registry import ActorIdRegistry
from carla_ros_bridge.transforms import carla_transform_to_ros_transform

class Actor(ActorParent):
    """
    Generic base class for all carla actors
    """

    global_id_registry = ActorIdRegistry()
        

    def __init__(self, carla_actor, actor_parent, topic_prefix = '', world_link = 'map'):
        super(Actor, self).__init__(id = carla_actor.id, carla_world = carla_actor.get_world())
        self.carla_actor = carla_actor
        self.parent = actor_parent
        self.topic_prefix = topic_prefix.replace(".", "/")
        self.world_link = world_link
        rospy.loginfo("Created {}(id={}, parent_id={}, type={}, topic_name={}, attributes={}".format(self.__class__.__name__, self.carla_actor.id, self.getParentId(), self.carla_actor.type_id, self.topic_name(), self.carla_actor.attributes))

    def destroy(self):
        rospy.logdebug("Destroying {}-Actor(id={})".format(self.__class__.__name__, self.id))
        self.parent=None
        self.carla_actor=None
        super(Actor, self).destroy_children()

    def __del__(self):
        rospy.logdebug("Deleting {}-Actor(id={})".format(self.__class__.__name__, self.id))

    def getParentId(self):
        if self.carla_actor.parent == None:
            return 0
        else:
            return self.carla_actor.parent.id

    def update(self):
        self.update_children()

    def topic_name(self):
        if len(self.topic_prefix) > 0:
            return self.parent.topic_name() + "/" + self.topic_prefix
        else:
            return self.parent.topic_name()
    
    def get_current_ros_time(self):
        return self.parent.get_current_ros_time()
    
    def get_param(self, key):
        return self.parent.get_param(key)

    def get_marker_color(self):
        raise NotImplementedError("This function has to be implemented by the derived classes if markers should be sent")

    def get_marker(self):
        marker = Marker(header=self.get_msg_header())
        marker.color = self.get_marker_color()
        marker.color.a = 0.3
        marker.id = self.get_global_id()
        marker.text = "id = {}".format(marker.id)
        return marker

    def get_msg_header(self):
        header = Header()
        header.stamp = self.get_current_ros_time()
        header.frame_id = self.world_link
        return header

    def get_current_ros_transfrom(self):
        return carla_transform_to_ros_transform(
            self.carla_actor.get_transform())

    def publish_ros_message(self, topic, msg):
        self.parent.publish_ros_message(topic, msg)
    
    def get_global_id(self):
        """
        Return a unique global id for the actor used for markers, object ids, etc. 

        ros marker id should be int32, carla/unrealengine seems to use int64
        A lookup table is used to remap actor_id to small number between 0 and max_int32

        :param actor_id:
        :return:
        """
        return Actor.global_id_registry.get_id(self.carla_actor.id)
