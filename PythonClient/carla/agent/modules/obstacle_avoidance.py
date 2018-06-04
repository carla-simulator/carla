import math
import numpy as np

class ObstacleAvoidance(object):


    def __init__(self):
        self.coast_factor = 2
        # Distance Threshold Traffic Light
        self.tl_dist_thres = 1500
        # Angle Threshold Traffic Light
        self.tl_angle_thres = 0.5
        # Distance Threshold Pedestrian
        self.p_dist_thres = 1000
        # Angle Threshold Pedestrian
        self.p_angle_thres = 0.3
        # Distance Threshold Vehicle
        self.v_dist_thres = 1500
        # Angle Threshold Vehicle
        self.v_angle_thres = 0.35
        # Select WP Number
        self.wp_num_steer = 0.8  # Select WP - Reverse Order: 1 - closest, 0 - furthest
        self.wp_num_speed = 0.5  # Select WP - Reverse Order: 1 - closest, 0 - furthest

        # Stop for traffic lights
        self.stop4TL = False
        # Stop for pedestrians
        self.stop4P = True
        # Stop for vehicles
        self.stop4V = True



    def stop_traffic_light(self, location, agent, wp_vector, wp_angle, speed_factor_tl):

        speed_factor_tl_temp = 1

        if agent.traffic_light.state != 0:  # Not green
            x_agent = agent.traffic_light.transform.location.x
            y_agent = agent.traffic_light.transform.location.y
            tl_vector, tl_dist = self.get_vec_dist(x_agent, y_agent, location.x, location.y)
            # tl_angle = self.get_angle(tl_vector,[ori_x_player,ori_y_player])
            tl_angle = self.get_angle(tl_vector, wp_vector)
            # print ('Traffic Light: ', tl_vector, tl_dist, tl_angle)
            if (
                    0 < tl_angle < self.tl_angle_thres / self.coast_factor and tl_dist < self.tl_dist_thres * self.coast_factor) or (
                    0 < tl_angle < self.tl_angle_thres and tl_dist < self.tl_dist_thres) and math.fabs(
                wp_angle) < 0.2:
                speed_factor_tl_temp = tl_dist / (self.coast_factor * self.tl_dist_thres)
            if (
                    0 < tl_angle < self.tl_angle_thres * self.coast_factor and tl_dist < self.tl_dist_thres / self.coast_factor) and math.fabs(
                wp_angle) < 0.2:
                speed_factor_tl_temp = 0

            if (speed_factor_tl_temp < speed_factor_tl):
                speed_factor_tl = speed_factor_tl_temp

    def stop_pedestrian(self, location, agent, wp_vector, speed_factor_p):

        speed_factor_p_temp = 1

        x_agent = agent.pedestrian.transform.location.x
        y_agent = agent.pedestrian.transform.location.y
        p_vector, p_dist = self.get_vec_dist(x_agent, y_agent, location.x, location.y)
        # p_angle = self.get_angle(p_vector,[ori_x_player,ori_y_player])
        p_angle = self.get_angle(p_vector, wp_vector)
        # print ('Pedestrian: ', p_vector, p_dist, p_angle)
        if (math.fabs(
                p_angle) < self.p_angle_thres / self.coast_factor and p_dist < self.p_dist_thres * self.coast_factor) or (
                0 < p_angle < self.p_angle_thres and p_dist < self.p_dist_thres):
            speed_factor_p_temp = p_dist / (self.coast_factor * self.p_dist_thres)
        if (math.fabs(
                p_angle) < self.p_angle_thres * self.coast_factor and p_dist < self.p_dist_thres / self.coast_factor):
            speed_factor_p_temp = 0

        if (speed_factor_p_temp < speed_factor_p):
            speed_factor_p = speed_factor_p_temp

        return speed_factor_p

    def stop_vehicle(self, location, agent, wp_vector, speed_factor_v):
        speed_factor_v_temp = 1
        x_agent = agent.vehicle.transform.location.x
        y_agent = agent.vehicle.transform.location.y
        v_vector, v_dist = self.get_vec_dist(x_agent, y_agent, location.x, location.y)
        # v_angle = self.get_angle(v_vector,[ori_x_player,ori_y_player])
        v_angle = self.get_angle(v_vector, wp_vector)
        # print ('Vehicle: ', v_vector, v_dist, v_angle)
        # print (v_angle, self.v_angle_thres, self.coast_factor)
        if (
                -0.5 * self.v_angle_thres / self.coast_factor < v_angle < self.v_angle_thres / self.coast_factor and v_dist < self.v_dist_thres * self.coast_factor) or (
                -0.5 * self.v_angle_thres / self.coast_factor < v_angle < self.v_angle_thres and v_dist < self.v_dist_thres):
            speed_factor_v_temp = v_dist / (self.coast_factor * self.v_dist_thres)
        if (
                -0.5 * self.v_angle_thres * self.coast_factor < v_angle < self.v_angle_thres * self.coast_factor and v_dist < self.v_dist_thres / self.coast_factor):
            speed_factor_v_temp = 0

        if (speed_factor_v_temp < speed_factor_v):
            speed_factor_v = speed_factor_v_temp

        return speed_factor_v

    def stop_for_agents(self, location, agents):

        speed_factor = 1
        speed_factor_tl = 1
        speed_factor_p = 1
        speed_factor_v = 1

        for agent in agents:

            if agent.HasField('traffic_light') and self.stop4TL:
                speed_factor_tl = self.stop_traffic_light(location, agent, speed_factor_tl)
            if agent.HasField('pedestrian') and self.stop4P:
                speed_factor_p = self.stop_pedestrian(location, agent, speed_factor_p)
            if agent.HasField('vehicle') and self.stop4V:
                speed_factor_v = self.stop_vehicle(location, agent, speed_factor_v)


            speed_factor = min(speed_factor_tl, speed_factor_p, speed_factor_v)

        return speed_factor