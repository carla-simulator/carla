import pandas as pd
import numpy as np

from utils import clip_throttle
from abstract_controller import Controller


class PDController(Controller):
    def __init__(self, target_speed, kp=0.6, kd=7.0):
        self.target_speed = target_speed
        self.prev_speed = np.nan
        self.prev_prev_speed = np.nan

        self.prev_prop = np.nan
        self.prev_prev_prop = np.nan
        self.curr_prop = np.nan
        self.deriv_list = []
        self.deriv_len = 5

        self.steer = 0
        self.throttle = 0.5

        self.kp = kp
        self.kd = kd

    def control(self, pts_2D, measurements, depth_array):
        location = self._extract_location(measurements)
        which_closest, dists = self._find_closest(pts_2D, location)

        closest, next = pts_2D[which_closest], pts_2D[which_closest+1]
        road_direction = next - closest
        perpendicular = np.array([
            -road_direction[1],
            road_direction[0],
        ])
        steer_direction = (location-closest).dot(perpendicular)
        self.prev_prev_prop = self.prev_prop
        self.prev_prop = self.curr_prop
        self.curr_prop = np.sign(steer_direction) * dists[which_closest]

        if any(pd.isnull([self.prev_prev_prop, self.prev_prop, self.curr_prop])):
            deriv = 0
        else:
            deriv = 0.5 * (self.curr_prop - self.prev_prev_prop)
            self.deriv_list.append(deriv)
            if len(self.deriv_list) > self.deriv_len:
                self.deriv_list = self.deriv_list[-self.deriv_len:]
            deriv = np.mean(self.deriv_list)

        curr_speed = measurements.player_measurements.forward_speed * 3.6
        self.throttle = clip_throttle(
            self.throttle,
            curr_speed,
            self.target_speed
        )

        self.steer = -self.kp * self.curr_prop - self.kd * deriv

        one_log_dict = {
            'steer': self.steer,
            'throttle': self.throttle,
            'speed': curr_speed,
            'deriv': deriv,
            'cte': self.curr_prop,
        }

        return one_log_dict, which_closest
