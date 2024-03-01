# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

""" This module contains PID controllers to perform lateral and longitudinal control. """

from collections import deque
import math
import numpy as np
import carla
from agents.tools.misc import get_speed

STEERING_UPDATE_SPEED = 0.1
ERROR_BUFFER_LENGTH = 10

class VehiclePIDController():
    """
    VehiclePIDController is the combination of two PID controllers
    (lateral and longitudinal) to perform the
    low level control a vehicle from client side
    """

    def __init__(self, vehicle: carla.Vehicle, config):
        """
        Constructor method.

        :param vehicle: actor to apply to local planner logic onto
        :param config (BasicAgentSettings): configuration parameters for the agent (must contain a planner attribute).
        
        config.planner.lateral_control_dict: dictionary of arguments to set the lateral PID controller
        using the following semantics:
            K_P -- Proportional term
            K_D -- Differential term
            K_I -- Integral term
        
        config.planner.longitudinal_control_dict: dictionary of arguments to set the longitudinal
        PID controller using the following semantics:
            K_P -- Proportional term
            K_D -- Differential term
            K_I -- Integral term
        
        config.planner.offset: If different than zero, the vehicle will drive displaced from the center line.
        Positive values imply a right offset while negative ones mean a left one. Numbers high enough
        to cause the vehicle to drive through other lanes might break the controller.
        """
        
        self.config = config

        self._vehicle = vehicle
        self._world = self._vehicle.get_world()
        self.past_steering = self._vehicle.get_control().steer
        self._lon_controller = PIDLongitudinalController(vehicle, config)
        self._lat_controller = PIDLateralController(vehicle, config)

    def run_step(self, waypoint):
        """
        Execute one step of control invoking both lateral and longitudinal
        PID controllers to reach a target waypoint
        at a given target_speed.

            :param waypoint: target location encoded as a waypoint
            :return: distance (in meters) to the waypoint
        """
        
        acceleration = self._lon_controller.run_step()
        current_steering = self._lat_controller.run_step(waypoint)
        control = carla.VehicleControl()
        if acceleration >= 0.0:
            control.throttle = min(acceleration, self.config.controls.max_throttle)
            control.brake = 0.0
        else:
            control.throttle = 0.0
            control.brake = min(abs(acceleration), self.config.controls.max_brake)

        # Steering regulation: changes cannot happen abruptly, can't steer too much.

        if current_steering > self.past_steering + STEERING_UPDATE_SPEED:
            current_steering = self.past_steering + STEERING_UPDATE_SPEED
        elif current_steering < self.past_steering - STEERING_UPDATE_SPEED:
            current_steering = self.past_steering - STEERING_UPDATE_SPEED

        if current_steering >= 0:
            steering = min(self.config.controls.max_steering, current_steering)
        else:
            steering = max(-self.config.controls.max_steering, current_steering)

        control.steer = steering
        control.hand_brake = False
        control.manual_gear_shift = False
        self.past_steering = steering

        return control

    def change_longitudinal_PID(self, args_longitudinal):
        """Changes the parameters of the PIDLongitudinalController"""
        self._lon_controller.change_parameters(**args_longitudinal)

    def change_lateral_PID(self, args_lateral):
        """Changes the parameters of the PIDLateralController"""
        self._lat_controller.change_parameters(**args_lateral)

    def set_offset(self, offset):
        """Changes the offset"""
        self._lat_controller.set_offset(offset)


class PIDLongitudinalController():
    """
    PIDLongitudinalController implements longitudinal control using a PID.
    """

    def __init__(self, vehicle, config):
        """
        Constructor method.

            :param vehicle: actor to apply to local planner logic onto
            :param config (BasicAgentSettings): configuration parameters for the agent.
        """
        self._vehicle = vehicle
        self.config = config
        self._error_buffer = deque(maxlen=ERROR_BUFFER_LENGTH)
        
    def run_step(self, debug=False):
        """
        Execute one step of longitudinal control to reach a given target speed.

        Assumes the config.live_info was updated with the current speed before 
        calling this function.

            :param debug: boolean for debugging. Prints the current speed.
            :return: throttle control
        """

        if debug:
            print('Current speed = {}'.format(self.config.live_info.current_speed))

        return self._pid_control()

    def _pid_control(self):
        """
        Estimate the throttle/brake of the vehicle based on the PID equations

            :return: throttle/brake control
        """

        target_speed = self.config.speed.target_speed
        error = target_speed - self.config.live_info.current_speed
        self._error_buffer.append(error)

        if len(self._error_buffer) >= 2:
            _de = (self._error_buffer[-1] - self._error_buffer[-2]) / self.config.planner.dt
            _ie = sum(self._error_buffer) * self.config.planner.dt
        else:
            _de = 0.0
            _ie = 0.0

        return np.clip((self.config.planner.longitudinal_control_dict.K_P * error) + (self.config.planner.longitudinal_control_dict.K_D * _de) + (self.config.planner.longitudinal_control_dict.K_I * _ie), -1.0, 1.0)

    def change_parameters(self, K_P, K_I, K_D, dt):
        """Changes the PID parameters"""
        self.config.planner.longitudinal_control_dict.K_P = K_P
        self.config.planner.longitudinal_control_dict.K_I = K_I
        self.config.planner.longitudinal_control_dict.K_D = K_D
        self.config.planner.dt = dt


class PIDLateralController():
    """
    PIDLateralController implements lateral control using a PID.
    """

    def __init__(self, vehicle, config):
        """
        Constructor method.

            :param vehicle: actor to apply to local planner logic onto
            :param offset: distance to the center line. If might cause issues if the value
                is large enough to make the vehicle invade other lanes.
            :param K_P: Proportional term
            :param K_D: Differential term
            :param K_I: Integral term
            :param dt: time differential in seconds
        """
        self._vehicle = vehicle
        self.config = config
        self._e_buffer = deque(maxlen=ERROR_BUFFER_LENGTH)

    def run_step(self, waypoint):
        """
        Execute one step of lateral control to steer
        the vehicle towards a certain waypoint.

            :param waypoint: target waypoint
            :return: steering control in the range [-1, 1] where:
            -1 maximum steering to left
            +1 maximum steering to right
        """
        return self._pid_control(waypoint, self._vehicle.get_transform())

    def set_offset(self, offset: float):
        """Changes the offset"""
        self.config.planner.offset = offset

    def _pid_control(self, waypoint, vehicle_transform):
        """
        Estimate the steering angle of the vehicle based on the PID equations

            :param waypoint: target waypoint
            :param vehicle_transform: current transform of the vehicle
            :return: steering control in the range [-1, 1]
        """
        # Get the ego's location and forward vector
        ego_loc = vehicle_transform.location
        v_vec = vehicle_transform.get_forward_vector()
        v_vec = np.array([v_vec.x, v_vec.y, 0.0])

        # Get the vector vehicle-target_wp
        if self.config.planner.offset != 0:
            # Displace the wp to the side
            w_tran = waypoint.transform
            r_vec = w_tran.get_right_vector()
            w_loc = w_tran.location + carla.Location(x=self.config.planner.offset*r_vec.x,
                                                     y=self.config.planner.offset*r_vec.y)
        else:
            w_loc = waypoint.transform.location

        w_vec = np.array([w_loc.x - ego_loc.x,
                          w_loc.y - ego_loc.y,
                          0.0])

        wv_linalg = np.linalg.norm(w_vec) * np.linalg.norm(v_vec)
        if wv_linalg == 0:
            _dot = 1
        else:
            _dot = math.acos(np.clip(np.dot(w_vec, v_vec) / (wv_linalg), -1.0, 1.0))
        _cross = np.cross(v_vec, w_vec)
        if _cross[2] < 0:
            _dot *= -1.0

        self._e_buffer.append(_dot)
        if len(self._e_buffer) >= 2:
            _de = (self._e_buffer[-1] - self._e_buffer[-2]) / self.config.planner.dt
            _ie = sum(self._e_buffer) * self.config.planner.dt
        else:
            _de = 0.0
            _ie = 0.0

        return np.clip((self.config.planner.lateral_control_dict.K_P * _dot) + (self.config.planner.lateral_control_dict.K_D * _de) + (self.config.planner.lateral_control_dict.K_I * _ie), -1.0, 1.0)

    def change_parameters(self, K_P, K_I, K_D, dt):
        """Changes the PID parameters"""
        self.config.planner.lateral_control_dict.K_P = K_P
        self.config.planner.lateral_control_dict.K_I = K_I
        self.config.planner.lateral_control_dict.K_D = K_D
        self.config.planner.dt = dt
