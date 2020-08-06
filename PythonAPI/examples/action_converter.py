import carla

# TODO: Some parameters to be tuned

class ActionConverter:
    def __init__(self):
        self.action_type = 0
        self.control = carla.VehicleControl()
        self._steer_cache = 0.0

    def get_control(self, control, action):
        if self.action_type == 0:
            if action == 0: # forward TODO: let actions determine throttle value?
                self.control.throttle = min(control.throttle + 0.01, 1)
            else:
                self.control.throttle = 0.0
            if action == 1: # stop
                self.control.brake = min(control.brake + 0.2, 1)
            else:
                self.control.brake = 0
            steer_increment = 5e-4 * 500
            if action == 2: # steer left
                if self._steer_cache > 0:
                    self._steer_cache = 0
                else:
                    self._steer_cache -= steer_increment
            elif action == 3: # steer right
                if self._steer_cache < 0:
                    self._steer_cache = 0
                else:
                    self._steer_cache += steer_increment
            else:
                self._steer_cache = 0.0
            self._steer_cache = min(0.7, max(-0.7, self._steer_cache))
            self.control.steer = round(self._steer_cache, 1)
            
        # if self.action_type == 1:
        #     self.control.throttle = min(action.throttle, 1)
        #     self.control.brake = min(action.brake, 1)
        #     self.control.steer = action.steer
        #     if self.control.steer > 1:
        #         self.control.steer = 1
        #     elif self.control.steer < -1:
        #         self.control.steer = -1

        return self.control