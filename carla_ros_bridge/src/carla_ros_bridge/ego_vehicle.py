"""
Classes to handle Carla vehicles
"""
import rospy
import carla

import math
import numpy as np

from nav_msgs.msg import Odometry
from std_msgs.msg import ColorRGBA
from ackermann_msgs.msg import AckermannDrive

from carla_ros_bridge.vehicle import Vehicle
import carla_ros_bridge.transforms as transforms 

class EgoVehicle(Vehicle):
    
    """
    Generic Actor Implementation for the ego vehicle
    """
    def __init__(self, carla_actor, actor_parent):

        super(EgoVehicle, self).__init__(carla_actor = carla_actor, 
                                      actor_parent = actor_parent, 
                                      topic_prefix = "ego_vehicle")

        self.control_subscriber = rospy.Subscriber(
             self.topic_name() + "/ackermann_cmd", AckermannDrive, self.control_command_updated)
            
    def destroy(self):
        rospy.logdebug("Destroy EgoVehicle(id={})".format(self.id))
        self.control_subscriber = None
        super(EgoVehicle, self).destroy()

    def get_marker_color(self):
        color = ColorRGBA()
        color.r = 0
        color.g = 255
        color.b = 0
        return color;

    def send_object_msg(self):
        """
        Ego vehicle doesn't send its information as part of the object list
        It rather sends out Odometry message  
        """
        odometry = Odometry(header=self.get_msg_header())
        odometry.child_frame_id = "base_link"
        
        # Pose
        ros_transform = self.get_current_ros_transfrom()
        odometry.pose.pose = transforms.ros_transform_to_pose(ros_transform)
        
        self.publish_ros_message(self.topic_name() + "/odometry", odometry)


    def control_command_updated(self, ackermann_drive):
        """
        Convert a Ackerman drive msg into carla control msg
        
        This brigde is not responsible for any restrictions on velocity or steering.
        It's just forwarding the ROS input to CARLA

        :param ackermann_drive: AckermannDrive msg
        :return:
        """
        
        steering_angle_ctrl = ackermann_drive.steering_angle
        speed_ctrl = ackermann_drive.speed

        max_steering_angle = math.radians(
            500
        )  # 500 degrees is the max steering angle that I have on my car,
        #  would be nice if we could use the value provided by carla
        max_speed = 27  # just a value for me, 27 m/sec seems to be a reasonable max speed for now

        vehicle_control = carla.VehicleControl()
        vehicle_control.hand_brake = False
        
        if abs(steering_angle_ctrl) > max_steering_angle:
            rospy.logerr("Max steering angle reached, clipping value")
            steering_angle_ctrl = np.clip(
                steering_angle_ctrl, -max_steering_angle, max_steering_angle)

        if abs(speed_ctrl) > max_speed:
            rospy.logerr("Max speed reached, clipping value")
            speed_ctrl = np.clip(speed_ctrl, -max_speed, max_speed)

        if speed_ctrl == 0:
            vehicle_control.brake = True

        vehicle_control.steer = steering_angle_ctrl / max_steering_angle
        vehicle_control.throttle = abs(speed_ctrl / max_speed)
        vehicle_control.reverse = True if speed_ctrl < 0 else False

        self.carla_actor.apply_control(vehicle_control)
