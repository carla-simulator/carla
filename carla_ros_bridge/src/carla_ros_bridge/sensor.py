"""
Classes to handle Carla sensors
"""
import rospy
import carla 

from carla_ros_bridge.actor import Actor

class Sensor(Actor):
    """
    Generic Actor Implementation for Sensors
    """
    def __init__(self, carla_actor, actor_parent, topic_prefix):
        super(Sensor, self).__init__(carla_actor = carla_actor,
                                     actor_parent = actor_parent, 
                                     topic_prefix = topic_prefix)

class Camera(Sensor):
    """
    Generic Actor Implementation for Camera Sensors
    """
    def __init__(self, carla_actor, actor_parent, topic_prefix):
        super(Camera, self).__init__(carla_actor = carla_actor,
                                     actor_parent = actor_parent, 
                                     topic_prefix = topic_prefix)

    def update(self):
        rospy.logdebug("Camera: update")

    def __del__(self):
        rospy.logdebug("Deleting Camera(id={})".format(self.id))
        super(Camera, self).__del__()


            # handle sensors
#            for name, data in sensor_data.items():
#                self.sensors[name].process_sensor_data(data, self.cur_time)






