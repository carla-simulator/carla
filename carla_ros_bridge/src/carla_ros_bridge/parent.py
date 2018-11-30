"""
Parent factory class for carla.Actor lifecycle handling
"""

from abc import abstractmethod

import rospy
from std_msgs.msg import Header


class Parent(object):

    """
    Factory class to create actors and manage lifecylce of the children objects
    """

    def __init__(self, carla_id, carla_world, frame_id):
        """
        Constructor

        :param carla_id: unique carla_id of this parent object
            carla_id > 0: carla actor ids (see also carla.Actor)
            carla_id == 0: used by the (root) bridge object
            carla_id == -1: used by the map object
        :type carla_id: int64
        :param carla_world: carla world object
        :type carla_world: carla.World
        :param frame_id: the ROS tf frame id of this object
        :type frame_id: string
        """
        self.carla_id = carla_id
        self.carla_world = carla_world
        self.frame_id = frame_id
        self.child_actors = {}

    def destroy(self):
        """
        Function (virtual) to destroy this object.

        Recursively calls destroy() of all children.
        Remove the reference to the carla.World object.
        Finally remove the references to the children object.

        :return:
        """
        for actor_id, actor in self.child_actors.iteritems():
            actor.destroy()
        self.child_actors.clear()
        self.carla_world = None

    def get_frame_id(self):
        """
        Getter for the frame id of this.

        :return: the ROS tf frame id of this object
        :rtype: string
        """
        return self.frame_id

    def get_id(self):
        """
        Getter for the carla_id of this.

        :return: unique carla_id of this parent object
        :rtype: int64
        """
        return self.carla_id

    def get_carla_world(self):
        """
        Getter for the carla world object of this.

        :return: the carla world
        :rtype: carla.World
        """
        return self.carla_world

    def _create_new_children(self):
        """
        Private function to create the actors in the carla world
        which are children actors of this parent.

        :return:
        """
        for actor in self.carla_world.get_actors():
            if ((actor.parent and actor.parent.id == self.carla_id)
                    or (actor.parent is None and self.carla_id == 0)):
                if actor.id not in self.child_actors:
                    if actor.type_id.startswith('traffic'):
                        self.child_actors[actor.id] = Traffic.create_actor(
                            carla_actor=actor, parent=self)
                    elif actor.type_id.startswith("vehicle"):
                        self.child_actors[actor.id] = Vehicle.create_actor(
                            carla_actor=actor, parent=self)
                    elif actor.type_id.startswith("sensor"):
                        self.child_actors[actor.id] = Sensor.create_actor(
                            carla_actor=actor, parent=self)
                    else:
                        self.child_actors[actor.id] = Actor(
                            carla_actor=actor, parent=self)

    def _destroy_dead_children(self):
        """
        Private function to detect and remove non existing children actors

        :return:
        """
        actors_to_delete = []
        for child_actor_id, child_actor in self.child_actors.iteritems():
            if not child_actor.carla_actor.is_alive:
                rospy.loginfo(
                    "Detected non alive child Actor(id={})".format(child_actor_id))
                actors_to_delete.append(child_actor_id)
            else:
                found_actor = False
                for actor in self.carla_world.get_actors():
                    if actor.id == child_actor_id:
                        found_actor = True
                        break
                if not found_actor:
                    rospy.loginfo(
                        "Detected not existing child Actor(id={})".format(child_actor_id))
                    actors_to_delete.append(child_actor_id)

        for actor_id in actors_to_delete:
            self.child_actors[actor_id].destroy()
            del self.child_actors[actor_id]

    def update(self):
        """
        Virtual (non abstract) function to update this object.

        Override this function if the derived class has to perform
        some additional update tasks. But don't forget to forward the update
        call to the super class, ensuring that this concrete function is called.

        The update part of the parent class consists
        of updating the children of this by:

        create new child actors
        destroy dead children
        update the exising children

        :return:
        """
        self._create_new_children()
        self._destroy_dead_children()
        for actor_id, actor in self.child_actors.iteritems():
            actor.update()

    def get_msg_header(self):
        """
        Helper function to create a ROS message Header

        :return: prefilled Header object
        """
        header = Header()
        header.stamp = self.get_current_ros_time()
        header.frame_id = self.get_frame_id()
        return header

    @abstractmethod
    def get_current_ros_time(self):
        """
        Pure virtual function to query the current ROS time from
        the carla_ros_bridge.CarlaRosBridge parent root.

        Is intended to be implemented by the directly derived classes:
        carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge.
        Be aware: Its not intended that classes further down in the class hierarchy override this!

        :return: The latest received ROS time of the bridge
        :rtype: rospy.Time
        """
        raise NotImplementedError(
            "This function is re-implemented by"
            "carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge"
            "If this error becomes visible the class hierarchy is somehow broken")

    @abstractmethod
    def publish_ros_message(self, topic, msg):
        """
        Pure virtual function to publish ROS messages via
        the carla_ros_bridge.CarlaRosBridge parent root.

        Is intended to be implemented by the directly derived classes:
        carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge.
        Be aware: Its not intended that classes further down in the class hierarchy override this!

        :param topic: ROS topic to publish the message on
        :type topic: string
        :param msg: the ROS message to be published
        :type msg: a valid ROS message type
        :return:
        """
        raise NotImplementedError(
            "This function is re-implemented by"
            "carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge"
            "If this error becomes visible the class hierarchy is somehow broken")

    @abstractmethod
    def get_param(self, key):
        """
        Pure virtual function to query global parameters passed from the outside.

        Is intended to be implemented by the directly derived classes:
        carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge.
        Be aware: Its not intended that classes further down in the class hierarchy override this!

        :param key: the key of the parameter
        :type key: string
        :return: the parameter string
        :rtype: string
        """
        raise NotImplementedError(
            "This function is re-implemented by"
            "carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge"
            "If this error becomes visible the class hierarchy is somehow broken")

    @abstractmethod
    def topic_name(self):
        """
        Pure virtual function to get the topic name of the current entity.

        Is intended to be implemented by the directly derived classes:
        carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge.
        Be aware: Its not intended that classes further down in the class hierarchy override this!

        :return: the final topic name of this
        :rtype: string
        """
        raise NotImplementedError(
            "This function is re-implemented by"
            "carla_ros_bridge.Child and carla_ros_bridge.CarlaRosBridge"
            "If this error becomes visible the class hierarchy is somehow broken")

# these imports have to be at the end to resolve cyclic dependency
from carla_ros_bridge.actor import Actor
from carla_ros_bridge.sensor import Sensor
from carla_ros_bridge.traffic import Traffic
from carla_ros_bridge.vehicle import Vehicle
