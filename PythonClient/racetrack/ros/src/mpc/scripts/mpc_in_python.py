#!/usr/bin/env python
import numpy as np
from scipy.linalg import expm

import rospy
from rospy.numpy_msg import numpy_msg
from tf.transformations import (
    euler_from_quaternion,
    unit_vector,
    quaternion_multiply,
    quaternion_conjugate,
)

# Message types
from std_msgs.msg import Float32, Time
from nav_msgs.msg import Odometry
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Point
from rospy_tutorials.msg import Floats

from _mpc_controller import MPCController
import config


class MPCControllerNode:
    def __init__(self, mpc_controller, common_queue_size=None, which_speed='VESC'):
        self.which_speed = which_speed

        self.points = None
        self.speed_from_pf = None
        self.speed_from_vesc = None
        self.position = None
        self.psi = None
        self.orient_euler = None

        queue_size = common_queue_size or 1

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
        #  This gives us the speed (from VESC)
        self.subscribers['/odom'] = rospy.Subscriber(
            '/odom',
            Odometry,
            self.odom_cb,
            queue_size=queue_size,
        )

        # Publishers
        self.publishers = {}
        #  For publishing the steering angle
        self.publishers['/mpc/angle'] = rospy.Publisher(
            '/mpc/angle',
            Float32,
            queue_size=queue_size,
        )
        #  For publishing the throttle
        self.publishers['/mpc/throttle'] = rospy.Publisher(
            '/mpc/throttle',
            Float32,
            queue_size=queue_size,
        )
        if config.DEBUG:
            #  For publishing predicted next positions
            self.publishers['/mpc/next_pos'] = rospy.Publisher(
                '/mpc/next_pos',
                Marker,
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
                mpc_results = mpc_controller.control(self.points, self.speed, self.position, self.psi)
                self.publishers['/mpc/angle'].publish(mpc_results['steer'])
                self.publishers['/mpc/throttle'].publish(mpc_results['throttle'])

                if config.DEBUG and mpc_results['next_pos'] is not None:
                    next_pos = mpc_results['next_pos']
                    rot_fn = lambda v: self.rot_euler(v, self.orient_euler)
                    rotated_next_pos = np.apply_along_axis(rot_fn, 1, next_pos)
                    marker = self.get_marker(rotated_next_pos + self.position)
                    self.publishers['/mpc/next_pos'].publish(marker)

                elapsed = rospy.Time.now().to_sec() - start_time
                rospy.loginfo('This took {:.4f}s'.format(elapsed))
                start_time = rospy.Time.now().to_sec()

    @staticmethod
    def get_marker(next_pos):
        # TODO: an almost identical function is in markers_mode.py
        marker = Marker()
        marker.header.frame_id = '/map'
        marker.type = marker.LINE_STRIP
        marker.action = marker.ADD

        # marker scale
        marker.scale.x = 0.1
        marker.scale.y = 0.1
        marker.scale.z = 0.1

        # marker color
        marker.color.a = 0.7
        marker.color.r = 1.0
        marker.color.g = 0.0
        marker.color.b = 0.0

        # marker orientaiton
        marker.pose.orientation.x = 0.0
        marker.pose.orientation.y = 0.0
        marker.pose.orientation.z = 0.0
        marker.pose.orientation.w = 1.0

        # marker position
        marker.pose.position.x = 0.0
        marker.pose.position.y = 0.0
        marker.pose.position.z = 0.0

        # marker line points
        marker.points = []

        for i in range(next_pos.shape[0]):
            p = Point()
            p.x = next_pos[i, 0]
            p.y = next_pos[i, 1]
            p.z = 0.0
            marker.points.append(p)

        return marker

    @staticmethod
    def rot_euler(v, xyz, dim=2):
        '''Rotate vector v (or array of vectors) by the euler angles xyz '''
        # https://stackoverflow.com/questions/6802577/python-rotation-of-3d-vector
        v = np.r_[v, 0]
        for theta, axis in zip(xyz, np.eye(3)):
            v = np.dot(np.array(v), expm(np.cross(np.eye(3), axis*-theta)))
        return v[:2]

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
        self.orient_euler = euler_from_quaternion([orient.x, orient.y, orient.z, orient.w])
        self.psi = self.orient_euler[-1]  # The last, third Euler angle is psi

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
