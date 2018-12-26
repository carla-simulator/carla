import inputs
import numpy as np

from abstract_controller import Controller

from config import STEER_BOUND, THROTTLE_BOUND


MAX_READING_FROM_PAD = 32768.


def scaling_fun(value):
    return np.sign(value) * np.abs(value)**0.1 / MAX_READING_FROM_PAD**0.1


class PadController(Controller):
    def __init__(self):
        self.throttle = 0
        self.steer = 0

    def control(self, pts_2D, measurements, depth_array):
        steer_not_read = True
        throttle_not_read = True

        events = inputs.get_gamepad()
        for event in events:
            if steer_not_read and event.code == 'ABS_X':
                self.steer = STEER_BOUND * scaling_fun(event.state)
                steer_not_read = False
            elif throttle_not_read and event.code == 'ABS_RY':
                self.throttle = -THROTTLE_BOUND * scaling_fun(event.state)
                throttle_not_read = False

        print('steer: {:.2f}, throttle: {:.2f}'.format(self.steer, self.throttle))

        one_log_dict = {
            'steer': self.steer,
            'throttle': self.throttle,
        }

        return one_log_dict, None
