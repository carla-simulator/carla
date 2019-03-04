import pygame
import sys

from abstract_controller import Controller


class PadController(Controller):
    def __init__(self):
        self.throttle = 0
        self.steer = 0
        pygame.init()
        pygame.joystick.init()
        try:
            self.joystick = pygame.joystick.Joystick(0)
            self.joystick.init()
        except:
            print('No joystic found')
            sys.exit(1)

    def control(self, pts_2D, measurements, depth_array):
        which_closest, _, _ = self._calc_closest_dists_and_location(
            measurements,
            pts_2D
        )

        for event in pygame.event.get(): # User did something
            # Possible joystick actions: JOYAXISMOTION JOYBALLMOTION JOYBUTTONDOWN JOYBUTTONUP JOYHATMOTION
            if event.type == pygame.JOYBUTTONDOWN:
                print("Joystick button pressed.")
            if event.type == pygame.JOYBUTTONUP:
                print("Joystick button released.")

        self.joystick.init()
        self.steer = self.joystick.get_axis(0)
        self.throttle = -self.joystick.get_axis(4)
        print('steer: {:.2f}, throttle: {:.2f}'.format(self.steer, self.throttle))

        one_log_dict = {
            'steer': self.steer,
            'throttle': self.throttle,
            'which_closest': which_closest
        }

        return one_log_dict
