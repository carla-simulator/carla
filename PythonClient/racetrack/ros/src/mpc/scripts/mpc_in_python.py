#!/usr/bin/env python
import rospy
from std_msgs.msg import Float32, Time
from visualization_msgs.msg import Marker
from _mpc_controller import MPCController


class MPCControllerNode:
    def __init__(self, mpc_controller):
        # To keep the previous state
        self.steer = None
        self.throttle = None

        # ROS-related
        self.points = None
        self.sub_spd = rospy.Subscriber('/centerline', Float32, self.centerline_cb, queue_size=1)

        self.pub_steer = rospy.Publisher('/steer', Float32, queue_size=1)
        self.pub_throttle = rospy.Publisher('/esc', Float32, queue_size=1)

        rospy.init_node('~mpc_controller')

        self.loop()

    def loop(self):
        rate = rospy.Rate(100)
        start_time = 0

        while not rospy.is_shutdown():
            elapsed = rospy.Time.now().to_sec() - start_time

            pts_2D = np.array([
                [-34.41720415,  60.01667464],
                [-38.59669945,  56.94990656],
                [-43.33353477,  54.53909335],
                [-48.72895121,  52.83095266]
            ])

            # Values for testing
            orientation = (0.7072, 0.7072)
            location = (-4.33, 25.15)
            curr_speed = 12

            steer, throttle = mpc_controller.control(pts_2D, curr_speed, location, orientation)

            self.pub_steer(steer)
            self.pub_throttle(throttle)

    ### Callbacks ###

    def centerline_cb(self, data):
        self.points = data.points




if __name__ == '__main__':
    target_speed = 10
    steps_ahead = 10
    dt = 0.1
    mpc_controller = MPCController(target_speed, steps_ahead, dt)

    try:
        MPCControllerNode(mpc_controller)
    except rospy.ROSInterruptException:
        rospy.logerr('MPCControllerNode failed')
