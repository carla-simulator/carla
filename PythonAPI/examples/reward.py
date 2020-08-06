import math
import numpy as np

# TODO: Some parameters to be tuned

class Reward:
    '''
        Reward used for training in the original CARLA paper (http://proceedings.mlr.press/v78/dosovitskiy17a/dosovitskiy17a.pdf)

        From the paper:
        "The reward is a weighted sum of five terms: distance traveled towards the goal d in km, speed v in
        km/h, collision damage c, intersection with the sidewalk s (between 0 and 1), and intersection with
        the opposite lane o (between 0 and 1)"

    '''
    def __init__(self):
        # Reward is calculated based on differences between timesteps, so need to
        # save the current state for the next reward.
        self.state = None

    def get_reward(self, obs, target):
        # Distance to reach goal
        d_x = obs.transform.location.x
        d_y = obs.transform.location.y
        d_z = obs.transform.location.z
        player_location = np.array([d_x, d_y, d_z])
        goal_location = np.array([target.x,
                                    target.y,
                                    target.z])
        d = np.linalg.norm(player_location - goal_location) # / 1000
        success = False
        if d < 1.0:
            success = True

        # Speed
        v = obs.speed * 3.6

        # Collision damage  
        # collision = [colhist[x + self.frame - 200] for x in range(0, 200)]
        max_col = 0
        if len(obs.colhist) > 0:
            max_col = max(1.0, max(obs.colhist))
        c = max_col
        # c_v = measurements.collision_vehicles
        # c_p = measurements.collision_pedestrians
        # c_o = measurements.collision_other
        # c = c_v + c_p + c_o

        # # Intersection with sidewalk
        # s = measurements.intersection_offroad

        # # Intersection with opposite lane
        # o = measurements.intersection_otherlane

        # Compute reward
        r = 0.0
        if self.state is not None:
            r += 1000 * (self.state['d'] - d)
            r += 0.05 * (v - self.state['v'])
            r -= 0.00002 * (c - self.state['c'])
            # r -= -0.1 * float(s > 0.001)
            # r -= 2 * (o - self.state['o'])
        
        # TODO: out of lane, timeout, landcrossing, overspeed
        
        # Update state
        new_state = {'d': d, 'v': v, 'c': c, # 's': s, 'o': o,
                    #  'c_v': c_v, 'c_p': c_p, 'c_o': c_o,
                     'd_x': d_x, 'd_y': d_y, 'd_z': d_z}
        self.state = new_state

        return r, success 

    def reset_reward(self):
        self.state = None