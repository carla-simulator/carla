import math

from pid_controller.pid import PID

# PID based controller, we can have different ones


class Controller(object):
    # TODO: this controller can become more general

    def __init__(self):
        # Default Throttle
        self.throttle = 0.5
        # Default Brake
        self.brake = 0
        # Gain on computed steering angle
        self.steer_gain = 0.7

        # Strength for applying brake - Value between 0 and 1
        self.brake_strength = 1
        # Factor to control coasting
        self.coast_factor = 2
        # PID speed controller
        self.pid = PID(p=0.25, i=0.08, d=0)
        # Target speed - could be controlled by speed limit
        self.target_speed = 35
        # Maximum throttle
        self.throttle_max = 0.75
        # Flag to decide if PID should be used
        self.usePID = True  # If off car will be a bit more aggressive



    def get_control(self, wp_angle, wp_mag, wp_angle_speed, speed_factor, current_speed):
        control = Control()

        steer = self.steer_gain * wp_angle
        if steer > 0:
            control.steer = min(steer, 1)
        else:
            control.steer = max(steer, -1)

        if (self.usePID):
            # Don't go to fast around corners
            if math.fabs(wp_angle_speed) < 0.1:
                target_speed_adjusted = self.target_speed * speed_factor
            elif math.fabs(wp_angle_speed) < 0.5:
                target_speed_adjusted = 25 * speed_factor
            else:
                target_speed_adjusted = 20 * speed_factor

            self.pid.target = target_speed_adjusted
            pid_gain = self.pid(feedback=current_speed)
            # print ('Target: ', self.pid.target, 'Error: ', self.pid.error, 'Gain: ', pid_gain)
            # print ('Target Speed: ', target_speed_adjusted, 'Current Speed: ', current_speed, 'Speed Factor: ',
            #       speed_factor)

            self.throttle = min(max(self.throttle - 0.25 * pid_gain, 0), self.throttle_max)

            if pid_gain > 0.5:
                self.brake = min(0.25 * pid_gain * self.brake_strength, 1)
            else:
                self.brake = 0
        else:
            throttle_adjusted = self.throttle_max
            brake_adjusted = 0

            # Don't go to fast around corners
            if (math.fabs(wp_angle_speed) < 0.1):
                throttle_adjusted = speed_factor * self.throttle_max
            elif (math.fabs(wp_angle_speed) < 0.5) and current_speed > 20:
                throttle_adjusted = 0.8 * speed_factor * self.throttle_max
                brake_adjusted = (1 - 0.8) * (1 - speed_factor) * self.brake_strength
            elif current_speed > 15:
                throttle_adjusted = 0.6 * speed_factor * self.throttle_max
                brake_adjusted = (1 - 0.6) * (1 - speed_factor) * self.brake_strength

            if (current_speed > self.target_speed):
                throttle_adjusted = (1 / (
                        current_speed + 2 - self.target_speed) * self.throttle_max + 2 * self.throttle_max / 3) * speed_factor
            if (speed_factor < 1 and current_speed > 5):
                brake_adjusted = 0.5 * (1 - speed_factor) * self.brake_strength

            self.throttle = min(max(throttle_adjusted, 0), self.throttle_max)
            self.brake = min(max(brake_adjusted, 0), 1)

        control.throttle = max(self.throttle, 0.01)  # Prevent N by putting at least 0.01
        control.brake = self.brake

        # print ('Throttle: ', control.throttle, 'Brake: ', control.brake, 'Steering Angle: ', control.steer)

        return control