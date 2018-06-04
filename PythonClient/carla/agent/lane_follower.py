


from carla.agent.agent import Agent
from carla.client import VehicleControl


"""
waypoints = self.waypointer.get_next_waypoints(
    (player.transform.location.x, player.transform.location.y, 22) \
    , (
        player.transform.orientation.x, player.transform.orientation.y,
        player.transform.orientation.z) \
    , (target.location.x, target.location.y, target.location.z) \
    , (target.orientation.x, target.orientation.y, target.orientation.z))
if waypoints == []:
    waypoints = [[player.transform.location.x, player.transform.location.y, 22]]
"""


class LaneFollower(Agent):
    """
    Simple derivation of Agent Class,
    A trivial agent agent that goes straight
    """
    def __init__(self):
        pass



    def run_step(self, measurements, sensor_data, directions, target):
        # Reset speed factor
        self.speed_factor = 1
        speed_factor_tl = 1
        speed_factor_tl_temp = 1
        speed_factor_p = 1
        speed_factor_p_temp = 1
        speed_factor_v = 1
        speed_factor_v_temp = 1
        player = measurements.player_measurements
        agents = measurements.non_player_agents
        # print ' it has  ',len(agents),' agents'
        loc_x_player = player.transform.location.x
        loc_y_player = player.transform.location.y
        ori_x_player = player.transform.orientation.x
        ori_y_player = player.transform.orientation.y

        waypoints = directions

        self.wp = [waypoints[int(self.wp_num_steer * len(waypoints))][0],
                   waypoints[int(self.wp_num_steer * len(waypoints))][1]]
        wp_vector, wp_mag = self.get_vec_dist(self.wp[0], self.wp[1], loc_x_player, loc_y_player)

        if wp_mag > 0:
            wp_angle = self.get_angle(wp_vector, [ori_x_player, ori_y_player])
        else:
            wp_angle = 0

        # WP Look Ahead for steering
        self.wp_speed = [waypoints[int(self.wp_num_speed * len(waypoints))][0],
                         waypoints[int(self.wp_num_speed * len(waypoints))][1]]
        wp_vector_speed, wp_mag_speed = self.get_vec_dist(self.wp_speed[0], self.wp_speed[1],
                                                          loc_x_player,
                                                          loc_y_player)
        wp_angle_speed = self.get_angle(wp_vector_speed, [ori_x_player, ori_y_player])

        # print ('Next Waypoint (Steer): ', waypoints[self.wp_num_steer][0], waypoints[self.wp_num_steer][1])
        # print ('Car Position: ', player.transform.location.x, player.transform.location.y)
        # print ('Waypoint Vector: ', wp_vector[0]/wp_mag, wp_vector[1]/wp_mag)
        # print ('Car Vector: ', player.transform.orientation.x, player.transform.orientation.y)
        # print ('Waypoint Angle: ', wp_angle, ' Magnitude: ', wp_mag)



        self.current_speed = player.forward_speed
        # We should run some state machine around here
        control = self.controller(wp_angle, wp_mag, wp_angle_speed)

        return control