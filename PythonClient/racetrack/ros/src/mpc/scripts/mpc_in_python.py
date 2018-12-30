#!/usr/bin/env python
import numpy as np

import rospy
from rospy.numpy_msg import numpy_msg
from tf.transformations import euler_from_quaternion

# Message types
from std_msgs.msg import Float32, Time
from nav_msgs.msg import Odometry
from visualization_msgs.msg import Marker
from rospy_tutorials.msg import Floats

from _mpc_controller import MPCController
import config


class MPCControllerNode:
    def __init__(self, mpc_controller, common_queue_size=None, which_speed='VESC'):
        queue_size = common_queue_size or 1
        self.which_speed = which_speed

        self.points = None
        self.speed_from_pf = None
        self.speed_from_vesc = None
        self.position = None
        self.psi = None

        # Subscribers
        self.subscribers = {}
        #  This gives us the waypoints
        self.subscribers['/centerline_numpy'] = rospy.Subscriber(
            '/centerline_numpy',
            numpy_msg(Floats),
            self.centerline_cb,
            queue_size=queue_size,
        )
        #  This gives us the position and orientation
        self.subscribers['/pf/pose/odom'] = rospy.Subscriber(
            '/pf/pose/odom',
            Odometry,
            self.pf_pose_odom_cb,
            queue_size=queue_size,
        )
        # This gives us the speed (from VESC)
        self.subscribers['/odom'] = rospy.Subscriber(
            '/odom',
            Odometry,
            self.odom_cb,
            queue_size=queue_size,
        )

        # Publishers
        self.publishers = {}
        self.publishers['/angle'] = rospy.Publisher(
            '/angle',
            Float32,
            queue_size=queue_size,
        )
        self.publishers['/throttle'] = rospy.Publisher(
            '/throttle',
            Float32,
            queue_size=queue_size,
        )

        rospy.init_node('~mpc_controller')

        self.loop()

    def loop(self):
        rate = rospy.Rate(100)
        start_time = 0

        while not rospy.is_shutdown():
            self.speed = {
                'VESC': self.speed_from_vesc,
                'PF': self.speed_from_pf,
            }[self.which_speed]

            points_OK = self.points is not None
            speed_OK = self.speed is not None
            position_OK = self.position is not None
            psi_OK = self.psi is not None

            if points_OK and speed_OK and position_OK and psi_OK:
                steer, throttle, cost = mpc_controller.control(self.points, self.speed, self.position, self.psi)
                self.publishers['/angle'].publish(steer)
                self.publishers['/throttle'].publish(throttle)

                elapsed = rospy.Time.now().to_sec() - start_time
                rospy.loginfo('This took {:.4f}s'.format(elapsed))
                start_time = rospy.Time.now().to_sec()




    ### Callbacks ###
    def centerline_cb(self, data):
        # There are: x, y, yaw, speed in self.points -- I'm only interested in
        #  the first two
        self.points = data.data.reshape(-1, 4)[:, :2]  # TODO: magic number 4

    def pf_pose_odom_cb(self, data):
        position = data.pose.pose.position
        self.position = np.array([position.x, position.y])

        # Psi is a bit more complicated
        orient = data.pose.pose.orientation
        euler = euler_from_quaternion([orient.x, orient.y, orient.z, orient.w])
        self.psi = euler[-1]  # The last, third Euler angle is psi

        # FIXME
        self.speed_from_pf = 0.5  # TODO: consult with Karol how to determine speed

    def odom_cb(self, data):
        self.speed_from_vesc = data.twist.twist.linear.x



if __name__ == '__main__':
    target_speed = config.TARGET_SPEED
    steps_ahead = config.STEPS_AHEAD
    dt = config.TIME_STEP
    mpc_controller = MPCController(target_speed, steps_ahead, dt)

    try:
        MPCControllerNode(mpc_controller)
    except rospy.ROSInterruptException:
        rospy.logerr('MPCControllerNode failed')
