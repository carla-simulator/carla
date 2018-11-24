from abc import ABCMeta, abstractmethod
import numpy as np


class Controller(metaclass=ABCMeta):
    @abstractmethod
    def control(self, pts_2D, measurements, depth_array):
        pass

    def _find_closest(self, pts_2D, location):
        dists = np.linalg.norm(pts_2D - location, axis=1)
        return np.argmin(dists), dists

    def _extract_location(self, measurements):
        return np.array([
            measurements.player_measurements.transform.location.x,
            measurements.player_measurements.transform.location.y,
        ])
