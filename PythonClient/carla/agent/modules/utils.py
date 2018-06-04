import math
import numpy as np

def get_vec_dist(self, x_dst, y_dst, x_src, y_src):
    vec = np.array([x_dst, y_dst] - np.array([x_src, y_src]))
    dist = math.sqrt(vec[0] ** 2 + vec[1] ** 2)
    return vec / dist, dist


def get_angle(self, vec_dst, vec_src):
    angle = math.atan2(vec_dst[1], vec_dst[0]) - math.atan2(vec_src[1], vec_src[0])
    if angle > math.pi:
        angle -= 2 * math.pi
    elif angle < -math.pi:
        angle += 2 * math.pi
    return angle