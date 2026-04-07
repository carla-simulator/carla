# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""This module contains PID controllers to perform lateral and longitudinal control."""

from __future__ import annotations

import math
from collections import deque
from typing import TYPE_CHECKING

import numpy as np

import carla
from agents.tools.misc import get_speed

if TYPE_CHECKING:
    from carla import Vehicle, Waypoint


class VehiclePIDController:
    """Combination of lateral and longitudinal PID controllers for low-level vehicle control."""

    def __init__(
        self,
        vehicle: Vehicle,
        args_lateral: dict[str, float],
        args_longitudinal: dict[str, float],
        offset: float = 0.0,
        max_throttle: float = 0.75,
        max_brake: float = 0.3,
        max_steering: float = 0.8,
    ) -> None:
        """Constructor.

        Args:
            vehicle: Actor to apply local planner logic onto.
            args_lateral: Dict with K_P, K_D, K_I for lateral PID.
            args_longitudinal: Dict with K_P, K_D, K_I for longitudinal PID.
            offset: Lateral offset from center line (positive=right, negative=left).
            max_throttle: Maximum throttle value.
            max_brake: Maximum brake value.
            max_steering: Maximum steering value.

        Raises:
            ValueError: If vehicle is None or invalid
        """
        if vehicle is None:
            raise ValueError('Vehicle cannot be None')

        self.max_brake = max_brake
        self.max_throt = max_throttle
        self.max_steer = max_steering

        self._vehicle = vehicle
        self._world = self._vehicle.get_world()

        if self._world is None:
            raise ValueError('Vehicle world is None - vehicle may be destroyed')

        self.past_steering = self._vehicle.get_control().steer
        self._lon_controller = PIDLongitudinalController(self._vehicle, **args_longitudinal)
        self._lat_controller = PIDLateralController(self._vehicle, offset, **args_lateral)

    def run_step(self, target_speed: float, waypoint: Waypoint) -> carla.VehicleControl:
        """Execute one control step invoking both PID controllers.

        Args:
            target_speed: Desired vehicle speed in km/h.
            waypoint: Target location encoded as a waypoint.

        Returns:
            VehicleControl with throttle, brake, and steering.
        """
        acceleration = self._lon_controller.run_step(target_speed)
        current_steering = self._lat_controller.run_step(waypoint)
        control = carla.VehicleControl()
        if acceleration >= 0.0:
            control.throttle = min(acceleration, self.max_throt)
            control.brake = 0.0
        else:
            control.throttle = 0.0
            control.brake = min(abs(acceleration), self.max_brake)

        if current_steering > self.past_steering + 0.1:
            current_steering = self.past_steering + 0.1
        elif current_steering < self.past_steering - 0.1:
            current_steering = self.past_steering - 0.1

        if current_steering >= 0:
            steering = min(self.max_steer, current_steering)
        else:
            steering = max(-self.max_steer, current_steering)

        control.steer = steering
        control.hand_brake = False
        control.manual_gear_shift = False
        self.past_steering = steering

        return control

    def change_longitudinal_pid(self, args_longitudinal: dict[str, float]) -> None:
        """Change the parameters of the longitudinal PID controller."""
        self._lon_controller.change_parameters(**args_longitudinal)

    def change_lateral_pid(self, args_lateral: dict[str, float]) -> None:
        """Change the parameters of the lateral PID controller."""
        self._lat_controller.change_parameters(**args_lateral)

    def set_offset(self, offset: float) -> None:
        """Change the lateral offset."""
        self._lat_controller.set_offset(offset)


class PIDLongitudinalController:
    """Implements longitudinal control using a PID."""

    def __init__(
        self,
        vehicle: Vehicle,
        K_P: float = 1.0,
        K_I: float = 0.0,
        K_D: float = 0.0,
        dt: float = 0.03,
    ) -> None:
        """Constructor.

        Args:
            vehicle: Actor to apply control onto.
            K_P: Proportional term.
            K_I: Integral term.
            K_D: Differential term.
            dt: Time differential in seconds.

        Raises:
            ValueError: If vehicle is None
        """
        if vehicle is None:
            raise ValueError('Vehicle cannot be None')

        self._vehicle = vehicle
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt
        self._error_buffer: deque[float] = deque(maxlen=10)

    def run_step(self, target_speed: float, debug: bool = False) -> float:
        """Execute one step of longitudinal control.

        Args:
            target_speed: Target speed in km/h.
            debug: Whether to print debug info.

        Returns:
            Throttle/brake control value in [-1, 1].

        Raises:
            ValueError: If target_speed is invalid
        """
        if target_speed < 0:
            raise ValueError('Target speed cannot be negative')

        current_speed = get_speed(self._vehicle)

        if debug:
            print(f'Current speed = {current_speed}')

        return self._pid_control(target_speed, current_speed)

    def _pid_control(self, target_speed: float, current_speed: float) -> float:
        """Estimate throttle/brake based on PID equations.

        Args:
            target_speed: Target speed in km/h.
            current_speed: Current speed in km/h.

        Returns:
            Control value in [-1, 1] (positive=throttle, negative=brake).
        """
        error = target_speed - current_speed
        self._error_buffer.append(error)

        if len(self._error_buffer) >= 2:
            _de = (self._error_buffer[-1] - self._error_buffer[-2]) / self._dt
            _ie = sum(self._error_buffer) * self._dt
        else:
            _de = 0.0
            _ie = 0.0

        return np.clip((self._k_p * error) + (self._k_d * _de) + (self._k_i * _ie), -1.0, 1.0)

    def change_parameters(self, K_P: float, K_I: float, K_D: float, dt: float) -> None:
        """Change the PID parameters."""
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt


class PIDLateralController:
    """Implements lateral control using a PID."""

    def __init__(
        self,
        vehicle: Vehicle,
        offset: float = 0.0,
        K_P: float = 1.0,
        K_I: float = 0.0,
        K_D: float = 0.0,
        dt: float = 0.03,
    ) -> None:
        """Constructor.

        Args:
            vehicle: Actor to apply control onto.
            offset: Distance to center line.
            K_P: Proportional term.
            K_I: Integral term.
            K_D: Differential term.
            dt: Time differential in seconds.

        Raises:
            ValueError: If vehicle is None
        """
        if vehicle is None:
            raise ValueError('Vehicle cannot be None')

        self._vehicle = vehicle
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt
        self._offset = offset
        self._e_buffer: deque[float] = deque(maxlen=10)

    def run_step(self, waypoint: Waypoint) -> float:
        """Execute one step of lateral control.

        Args:
            waypoint: Target waypoint.

        Returns:
            Steering control in [-1, 1] (-1=max left, +1=max right).

        Raises:
            ValueError: If waypoint is None
        """
        if waypoint is None:
            raise ValueError('Waypoint cannot be None')

        return self._pid_control(waypoint, self._vehicle.get_transform())

    def set_offset(self, offset: float) -> None:
        """Change the offset."""
        self._offset = offset

    def _pid_control(self, waypoint: Waypoint, vehicle_transform: carla.Transform) -> float:
        """Estimate steering angle based on PID equations.

        Args:
            waypoint: Target waypoint.
            vehicle_transform: Current transform of the vehicle.

        Returns:
            Steering control in [-1, 1].
        """
        ego_loc = vehicle_transform.location
        v_vec = vehicle_transform.get_forward_vector()
        v_vec = np.array([v_vec.x, v_vec.y, 0.0])

        if self._offset != 0:
            w_tran = waypoint.transform
            r_vec = w_tran.get_right_vector()
            w_loc = w_tran.location + carla.Location(x=self._offset * r_vec.x, y=self._offset * r_vec.y)
        else:
            w_loc = waypoint.transform.location

        w_vec = np.array([w_loc.x - ego_loc.x, w_loc.y - ego_loc.y, 0.0])

        wv_linalg = np.linalg.norm(w_vec) * np.linalg.norm(v_vec)
        _dot = 1.0 if wv_linalg == 0 else math.acos(np.clip(np.dot(w_vec, v_vec) / wv_linalg, -1.0, 1.0))
        _cross = np.cross(v_vec, w_vec)
        if _cross[2] < 0:
            _dot *= -1.0

        self._e_buffer.append(_dot)
        if len(self._e_buffer) >= 2:
            _de = (self._e_buffer[-1] - self._e_buffer[-2]) / self._dt
            _ie = sum(self._e_buffer) * self._dt
        else:
            _de = 0.0
            _ie = 0.0

        return np.clip((self._k_p * _dot) + (self._k_d * _de) + (self._k_i * _ie), -1.0, 1.0)

    def change_parameters(self, K_P: float, K_I: float, K_D: float, dt: float) -> None:
        """Change the PID parameters."""
        self._k_p = K_P
        self._k_i = K_I
        self._k_d = K_D
        self._dt = dt
