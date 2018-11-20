"""
Factory class to create actors having a given parent_id
"""
import rospy

class ActorParent(object):
    """
    Factory class to create actors having a given parent_id  
    """
    def __init__(self, id, carla_world):
        """
        initialize this class
        """
        self.id = id
        self.carla_world = carla_world
        self.child_actors = {}

    def __del__(self):
        rospy.logdebug("Deleting ActorParent(id={} destroying children {})".format(self.id, len(self.child_actors)))
        self.destroy_children()

    def destroy_children(self):
        for actor_id, actor in self.child_actors.iteritems():
            actor.destroy()
        self.child_actors.clear()

    def create_new_children(self):
        """
        check the actors in the carla world for new children actors of this
        """
        for actor in self.carla_world.get_actors():
            if (actor.parent and actor.parent.id == self.id) or (actor.parent==None and 0 == self.id):
                if actor.id not in self.child_actors:
                    if actor.type_id == "traffic.traffic_light":
                       self.child_actors[actor.id] = TrafficLight(carla_actor = actor, actor_parent = self, topic_prefix = "traffic_light")  
                    elif actor.type_id.startswith("vehicle."):
                        if actor.attributes.has_key('role_name') and actor.attributes['role_name'] == self.get_param('ego_vehicle_role_name'):
                            self.child_actors[actor.id] = EgoVehicle(carla_actor = actor, actor_parent = self)
                        else:
                            self.child_actors[actor.id] = Vehicle(carla_actor = actor, actor_parent = self)
                    elif actor.type_id.startswith("sensor.camera"):
                       self.child_actors[actor.id] = Camera(carla_actor = actor, actor_parent = self, topic_prefix = actor.type_id)
                    elif actor.type_id.startswith("sensor"):
                       self.child_actors[actor.id] = Sensor(carla_actor = actor, actor_parent = self, topic_prefix = actor.type_id)

    def destroy_dead_children(self):
        """
        remove non-active children
        """
        actors_to_delete = []
        for actor_id, actor in self.child_actors.iteritems():
            if not actor.carla_actor.is_alive:
                rospy.loginfo("Detected non alive child Actor(id={})".format(actor_id))
                actors_to_delete.append(actor_id)
            else:
                actor_found = False;
                for actor in self.carla_world.get_actors():
                    if actor.id == actor_id:
                        actor_found = True
                        break
                if not actor_found:
                    rospy.loginfo("Detected not existing child Actor(id={})".format(actor_id))
                    actors_to_delete.append(actor_id)
                        
        for actor_id in actors_to_delete:
            del self.child_actors[actor_id]

    def update_children(self):
        """
        Function to update the children of this
        - create new child actors
        - destroy dead children
        - update the children 
        """
        self.create_new_children()
        self.destroy_dead_children()
        for actor_id, actor in self.child_actors.iteritems():
            actor.update()

    def get_current_ros_time(self):
        raise NotImplementedError("This function has to be implemented by the derived classes")

    def publish_ros_message(self, topic, msg):
        raise NotImplementedError("This function has to be implemented by the derived classes")

    def get_global_marker_id(self, actor_id):
        raise NotImplementedError("This function has to be implemented by the derived classes")

from carla_ros_bridge.sensor import Sensor, Camera
from carla_ros_bridge.traffic import TrafficLight
from carla_ros_bridge.vehicle import Vehicle
from carla_ros_bridge.ego_vehicle import EgoVehicle
