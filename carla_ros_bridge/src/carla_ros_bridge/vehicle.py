"""
Classes to handle Carla vehicles
"""
import rospy

from std_msgs.msg import ColorRGBA
from derived_object_msgs.msg import Object
from shape_msgs.msg import SolidPrimitive
from visualization_msgs.msg import Marker

from carla_ros_bridge.actor import Actor
import carla_ros_bridge.transforms as transforms


class Vehicle(Actor):

    """
    Actor implementation details for vehicles
    """

    @staticmethod
    def create_actor(carla_actor, parent):
        """
        Static factory method to create vehicle actors

        :param carla_actor: carla vehicle actor object
        :type carla_actor: carla.Vehicle
        :param parent: the parent of the new traffic actor
        :type parent: carla_ros_bridge.Parent
        :return: the created vehicle actor
        :rtype: carla_ros_bridge.Vehicle or derived type
        """
        if (carla_actor.attributes.has_key('role_name') and
                carla_actor.attributes['role_name'] == parent.get_param('ego_vehicle_role_name')):
            return EgoVehicle(carla_actor=carla_actor, parent=parent)
        else:
            return Vehicle(carla_actor=carla_actor, parent=parent)

    def __init__(self, carla_actor, parent, topic_prefix=None, append_role_name_topic_postfix=True):
        """
        Constructor

        :param carla_actor: carla vehicle actor object
        :type carla_actor: carla.Vehicle
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        :param topic_prefix: the topic prefix to be used for this actor
        :type topic_prefix: string
        :param append_role_name_topic_postfix: if this flag is set True,
            the role_name of the actor is used as topic postfix
        :type append_role_name_topic_postfix: boolean
        """
        if topic_prefix is None:
            topic_prefix = "vehicle/{:03}".format(
                Actor.global_id_registry.get_id(carla_actor.id))

        super(Vehicle, self).__init__(carla_actor=carla_actor,
                                      parent=parent,
                                      topic_prefix=topic_prefix,
                                      append_role_name_topic_postfix=append_role_name_topic_postfix)

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
        """
        Function (override) to destroy this object.

        Finally forward call to super class.

        :return:
        """
        rospy.logdebug("Destroy Vehicle(id={})".format(self.get_id()))
        super(Vehicle, self).destroy()

    def update(self):
        """
        Function (override) to update this object.

        On update vehicles send:
        - tf global frame
        - object message
        - marker message

        :return:
        """
        self.send_tf_msg()
        self.send_object_msg()
        self.send_marker_msg()
        super(Vehicle, self).update()

    def get_marker_color(self):
        """
        Function (override) to return the color for marker messages.

        :return: the color used by a vehicle marker
        :rtpye : std_msgs.msg.ColorRGBA
        """
        color = ColorRGBA()
        color.r = 255
        color.g = 0
        color.b = 0
        return color

    def send_marker_msg(self):
        """
        Function to send marker messages of this vehicle.

        :return:
        """
        marker = self.get_marker(use_parent_frame=False)
        marker.type = Marker.CUBE

        marker.pose = transforms.carla_location_to_pose(
            self.carla_actor.bounding_box.location)
        marker.scale.x = self.carla_actor.bounding_box.extent.x * 2.0
        marker.scale.y = self.carla_actor.bounding_box.extent.y * 2.0
        marker.scale.z = self.carla_actor.bounding_box.extent.z * 2.0
        self.publish_ros_message('/carla/vehicle_marker', marker)

    def send_object_msg(self):
        """
        Function to send object messages of this vehicle.

        A derived_object_msgs.msg.Object is prepared to be published via '/carla/objects'

        :return:
        """
        vehicle_object = Object(header=self.get_msg_header())
        # ID
        vehicle_object.id = self.get_global_id()
        # Pose
        vehicle_object.pose = self.get_current_ros_pose()
        # Twist
        vehicle_object.twist = self.get_current_ros_twist()
        # Acceleration
        vehicle_object.accel = self.get_current_ros_accel()
        # Shape
        vehicle_object.shape.type = SolidPrimitive.BOX
        vehicle_object.shape.dimensions.extend([
            self.carla_actor.bounding_box.extent.x * 2.0,
            self.carla_actor.bounding_box.extent.y * 2.0,
            self.carla_actor.bounding_box.extent.z * 2.0])

        # Classification if available in attributes
        if self.classification != Object.CLASSIFICATION_UNKNOWN:
            vehicle_object.object_classified = True
            vehicle_object.classification = self.classification
            vehicle_object.classification_certainty = 1.0
            self.classification_age += 1
            vehicle_object.classification_age = self.classification_age

        self.publish_ros_message('/carla/objects', vehicle_object)

# this import has to be at the end to resolve cyclic dependency
from carla_ros_bridge.ego_vehicle import EgoVehicle
