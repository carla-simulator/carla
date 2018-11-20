"""
Classes to handle Carla vehicles
"""
import rospy
import carla

from std_msgs.msg import ColorRGBA
from derived_object_msgs.msg import Object
from shape_msgs.msg import SolidPrimitive
from geometry_msgs.msg import TransformStamped
from visualization_msgs.msg import Marker

from carla_ros_bridge.actor import Actor
import carla_ros_bridge.transforms as transforms 

class Vehicle(Actor):
    """
    Generic Actor Implementation for Vehicles
    """
    def __init__(self, carla_actor, actor_parent, topic_prefix = ''):

        super(Vehicle, self).__init__(carla_actor = carla_actor, 
                                      actor_parent = actor_parent,
                                      topic_prefix = topic_prefix)
        
        self.classification = Object.CLASSIFICATION_UNKNOWN
        if carla_actor.attributes.has_key('object_type'):
            if carla_actor.attributes['object_type'] == 'car':
                self.classification = Object.CLASSIFICATION_CAR
            elif carla_actor.attributes['object_type'] == 'bike':
                self.classification = Object.CLASSIFICATION_BIKE
            elif carla_actor.attributes['object_type'] == 'motorcycle':
                self.classification = Object.CLASSIFICATION_MOTORCYCLE
            elif carla_actor.attributes['object_type'] == 'truck':
                self.classification = Object.CLASSIFICATION_TRUCK
            elif carla_actor.attributes['object_type'] == 'other':
                self.classification = Object.CLASSIFICATION_OTHER_VEHICLE
        self.classification_age = 0

    def destroy(self):
        rospy.logdebug("Destroy Vehicle(id={})".format(self.id))
        super(Vehicle, self).destroy()

    def update(self):
        self.send_object_msg()
        self.send_marker_msg()
        super(Vehicle, self).update()

    def get_marker_color(self):
        color = ColorRGBA()
        color.r = 255
        color.g = 0
        color.b = 0
        return color;

    def send_marker_msg(self):
        # Transform
        t = TransformStamped()
        t.header = self.get_msg_header()
        t.child_frame_id = "base_link"
        t.transform = self.get_current_ros_transfrom()
        self.publish_ros_message('tf', t)
        # Actual marker
        marker = self.get_marker()
        marker.type = Marker.CUBE
        ros_transform = transforms.carla_location_to_ros_transform(self.carla_actor.bounding_box.location)
        ros_transform = transforms.concat_two_ros_transforms(ros_transform, self.get_current_ros_transfrom())
        marker.pose = transforms.ros_transform_to_pose(ros_transform)
        marker.scale.x = self.carla_actor.bounding_box.extent.x * 2.0
        marker.scale.y = self.carla_actor.bounding_box.extent.y * 2.0
        marker.scale.z = self.carla_actor.bounding_box.extent.z * 2.0
        self.publish_ros_message('/carla/vehicle_marker', marker)
    
    def send_object_msg(self):
        """
        Create an object msg from the current vehicle state and publish it 
        """
        object = Object(header=self.get_msg_header())
        # ID
        object.id = self.get_global_id()
        # Pose
        ros_transform = self.get_current_ros_transfrom()
        object.pose = transforms.ros_transform_to_pose(ros_transform)
        # Shape
        object.shape.type = SolidPrimitive.BOX
        object.shape.dimensions.extend([
            self.carla_actor.bounding_box.extent.x * 2.0, 
            self.carla_actor.bounding_box.extent.y * 2.0,
            self.carla_actor.bounding_box.extent.z * 2.0] )
        
        # Classification if available in attributes
        if not self.classification == Object.CLASSIFICATION_UNKNOWN:
            object.object_classified = True
            object.classification = self.classification
            object.classification_certainty = 1.0
            self.classification_age += 1
            object.classification_age = self.classification_age
    
        self.publish_ros_message('/carla/objects', object)
