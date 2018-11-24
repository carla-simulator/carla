import numpy as np

import sys
sys.path.append('..')
from carla.util import print_over_same_line


def clip_throttle(throttle, curr_speed, target_speed):
    return np.clip(
        throttle - 0.1 * (curr_speed-target_speed),
        0.25,
        1.0
    )


def print_measurements(measurements):
    number_of_agents = len(measurements.non_player_agents)
    player_measurements = measurements.player_measurements
    message = 'Vehicle at ({pos_x:.1f}, {pos_y:.1f}), '
    message += '{speed:.0f} km/h, '
    message = message.format(
        pos_x=player_measurements.transform.location.x,
        pos_y=player_measurements.transform.location.y,
        speed=player_measurements.forward_speed * 3.6, # m/s -> km/h
    )
    print_over_same_line(message)
