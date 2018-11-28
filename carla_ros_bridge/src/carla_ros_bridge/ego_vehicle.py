"""
Classes to handle Carla vehicles
"""
import math
import numpy as np

import rospy

from nav_msgs.msg import Odometry
from std_msgs.msg import ColorRGBA
from ackermann_msgs.msg import AckermannDrive

from carla import VehicleControl
from carla_ros_bridge.vehicle import Vehicle


class EgoVehicle(Vehicle):

    """
    Vehicle implementation details for the ego vehicle
    """

    def __init__(self, carla_actor, parent):
        """
        Constructor

        :param carla_actor: carla actor object
        :type carla_actor: carla.Actor
        :param parent: the parent of this
        :type parent: carla_ros_bridge.Parent
        """

        super(EgoVehicle, self).__init__(carla_actor=carla_actor,
                                         parent=parent,
                                         topic_prefix="ego_vehicle",
                                         append_role_name_topic_postfix=False)

        self.control_subscriber = rospy.Subscriber(
            self.topic_name() + "/ackermann_cmd", AckermannDrive, self.control_command_updated)

    def destroy(self):
        """
        Function (override) to destroy this object.

        Terminate ROS subscription on AckermannDrive control commands.
        Finally forward call to super class.

        :return:
        """
        rospy.logdebug("Destroy EgoVehicle(id={})".format(self.get_id()))
        self.control_subscriber = None
        super(EgoVehicle, self).destroy()

    def get_marker_color(self):
        """
        Function (override) to return the color for marker messages.

        The ego vehicle uses a different marker color than other vehicles.

        :return: the color used by a ego vehicle marker
        :rtpye : std_msgs.msg.ColorRGBA
        """
        color = ColorRGBA()
        color.r = 0
        color.g = 255
        color.b = 0
        return color

    def send_object_msg(self):
        """
        Function (override) to send odometry message of the ego vehicle
        instead of an object message.

        The ego vehicle doesn't send its information as part of the object list.
        A nav_msgs.msg.Odometry is prepared to be published via '/carla/ego_vehicle'

        :return:
        """
        odometry = Odometry(header=self.get_msg_header())
        odometry.child_frame_id = self.get_frame_id()

        odometry.pose.pose = self.get_current_ros_pose()
        odometry.twist.twist = self.get_current_ros_twist()

        self.publish_ros_message(self.topic_name() + "/odometry", odometry)

    def control_command_updated(self, ackermann_drive):
        """
        Convert a Ackerman drive msg into carla control msg

        This function gets called whenever a ROS message is received via
        '/carla/ego_vehicle/ackermann_cmd' topic.
        The received ROS message is converted into carla.Actor control commands.
        This brigde is not responsible for any restrictions on velocity or steering.
        It's just forwarding the ROS input to CARLA

        :param ackermann_drive:
        :type ackermann_drive: ackermann_msgs.msg.AckermannDrive
        :return:
        """

        steering_angle_ctrl = ackermann_drive.steering_angle
        speed_ctrl = ackermann_drive.speed

        max_steering_angle = math.radians(
            80
        )  # 80 degrees is the approximate max steering angle of a car
        #  would be nice if we could use the value provided by carla
        max_speed = 27  # just a value for me, 27 m/sec seems to be a reasonable max speed for now

        vehicle_control = VehicleControl()
        vehicle_control.hand_brake = False

        if abs(steering_angle_ctrl) > max_steering_angle:
            rospy.logerr("Max steering angle reached, clipping value")
            steering_angle_ctrl = np.clip(
                steering_angle_ctrl, -max_steering_angle, max_steering_angle)

        # todo:
        # This will not work when slowing down/braking. In this case we need to use vehicle_control.brake.
        # The problem is that when slowing down, first vehicle_control.throttle will get smaller (eventually zero),
        # and then, vehicle_control.brake has to be set.
        # Maybe we can make use of the acceleration part in the ackermann msgs to find out,
        # when a reduction of throttle is sufficient, and when we have to use
        # brake...
        if abs(speed_ctrl) > max_speed:
            rospy.logerr("Max speed reached, clipping value")
            speed_ctrl = np.clip(speed_ctrl, -max_speed, max_speed)

        if speed_ctrl == 0:
            vehicle_control.brake = True

        vehicle_control.steer = steering_angle_ctrl / max_steering_angle
        vehicle_control.throttle = abs(speed_ctrl / max_speed)
        vehicle_control.reverse = True if speed_ctrl < 0 else False

        self.carla_actor.apply_control(vehicle_control)
