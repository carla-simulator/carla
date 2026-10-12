"""Convert an Alpamayo path into CARLA throttle, brake, and steering."""

from __future__ import annotations

from dataclasses import dataclass

import numpy as np


@dataclass(frozen=True)
class ControlCommand:
    throttle: float
    brake: float
    steer: float
    target_speed_mps: float
    target_index: int
    requested_steer: float = 0.0
    speed_index: int = 0
    speed_error_mps: float = 0.0
    longitudinal_effort: float = 0.0
    hold_active: bool = False


class TrajectoryTracker:
    def __init__(
        self,
        wheelbase_m: float = 2.8,
        max_wheel_angle_deg: float = 70.0,
        lookahead_m: float = 3.0,
        speed_kp: float = 0.35,
        speed_ki: float = 0.04,
        speed_kd: float = 0.02,
        speed_integral_limit: float = 5.0,
        stop_target_speed_mps: float = 0.25,
        hold_speed_mps: float = 0.15,
        minimum_stop_brake: float = 0.25,
        hold_brake: float = 1.0,
        steering_gain: float = 1.0,
        max_steer_rate_per_s: float = 1.0,
    ) -> None:
        self.wheelbase_m = wheelbase_m
        self.max_wheel_angle_rad = np.deg2rad(max_wheel_angle_deg)
        self.lookahead_m = lookahead_m
        self.speed_kp = speed_kp
        self.speed_ki = speed_ki
        self.speed_kd = speed_kd
        self.speed_integral_limit = speed_integral_limit
        self.stop_target_speed_mps = stop_target_speed_mps
        self.hold_speed_mps = hold_speed_mps
        self.minimum_stop_brake = minimum_stop_brake
        self.hold_brake = hold_brake
        self.steering_gain = steering_gain
        self.max_steer_rate_per_s = max_steer_rate_per_s
        self._previous_steer = 0.0
        self._speed_integral = 0.0
        self._previous_speed_mps: float | None = None

    def reset(self) -> None:
        """Reset steering-rate and longitudinal PID state."""
        self._previous_steer = 0.0
        self._speed_integral = 0.0
        self._previous_speed_mps = None

    def _longitudinal_command(
        self,
        target_speed_mps: float,
        current_speed_mps: float,
        time_step: float,
    ) -> tuple[float, float, float, float, bool]:
        """Return throttle, brake, error, signed PID effort, and hold state."""
        stop_requested = target_speed_mps <= self.stop_target_speed_mps
        if stop_requested:
            target_speed_mps = 0.0
            # Do not let positive integral accumulated while cruising oppose
            # a newly requested stop.
            self._speed_integral = min(self._speed_integral, 0.0)

        error = target_speed_mps - current_speed_mps
        speed_derivative = 0.0
        if self._previous_speed_mps is not None:
            # Derivative on measurement avoids a kick when a newly sampled
            # trajectory changes the requested speed at a replan boundary.
            speed_derivative = -(
                current_speed_mps - self._previous_speed_mps
            ) / time_step
        self._previous_speed_mps = current_speed_mps

        candidate_integral = float(
            np.clip(
                self._speed_integral + error * time_step,
                -self.speed_integral_limit,
                self.speed_integral_limit,
            )
        )
        candidate_effort = (
            self.speed_kp * error
            + self.speed_ki * candidate_integral
            + self.speed_kd * speed_derivative
        )
        # Conditional integration: do not wind the integral farther into a
        # saturated pedal command.
        if not (
            (candidate_effort > 1.0 and error > 0.0)
            or (candidate_effort < -1.0 and error < 0.0)
        ):
            self._speed_integral = candidate_integral
        effort = float(
            np.clip(
                self.speed_kp * error
                + self.speed_ki * self._speed_integral
                + self.speed_kd * speed_derivative,
                -1.0,
                1.0,
            )
        )

        if stop_requested:
            if current_speed_mps <= self.hold_speed_mps:
                return 0.0, self.hold_brake, error, -self.hold_brake, True
            brake = max(-effort, self.minimum_stop_brake)
            return 0.0, float(np.clip(brake, 0.0, 1.0)), error, -brake, False
        if effort >= 0.0:
            return effort, 0.0, error, effort, False
        return 0.0, -effort, error, effort, False

    def command(
        self,
        path_carla_local: np.ndarray,
        current_position_local: np.ndarray,
        current_yaw_local_rad: float,
        current_speed_mps: float,
        time_step: float = 0.1,
        trajectory_time_index: int = 0,
    ) -> ControlCommand:
        path = np.asarray(path_carla_local, dtype=np.float64)
        position = np.asarray(current_position_local, dtype=np.float64)
        if path.ndim != 2 or path.shape[1] < 3 or not len(path):
            raise ValueError("path must have shape [T, >=3]")
        distances = np.linalg.norm(path[:, :2] - position[:2], axis=1)
        nearest = int(np.argmin(distances))
        candidates = np.flatnonzero(distances[nearest:] >= self.lookahead_m)
        target_index = nearest + int(candidates[0]) if len(candidates) else len(path) - 1

        delta = path[target_index, :2] - position[:2]
        cosine, sine = np.cos(current_yaw_local_rad), np.sin(current_yaw_local_rad)
        target_vehicle = np.array(
            [cosine * delta[0] + sine * delta[1], -sine * delta[0] + cosine * delta[1]]
        )
        distance_sq = max(float(target_vehicle @ target_vehicle), 1e-6)
        curvature = 2.0 * target_vehicle[1] / distance_sq
        wheel_angle = np.arctan(self.wheelbase_m * curvature)
        requested_steer = float(
            np.clip(
                self.steering_gain * wheel_angle / self.max_wheel_angle_rad,
                -1.0,
                1.0,
            )
        )
        max_steer_delta = self.max_steer_rate_per_s * time_step
        steer = float(
            np.clip(
                requested_steer,
                self._previous_steer - max_steer_delta,
                self._previous_steer + max_steer_delta,
            )
        )
        self._previous_steer = steer

        # Alpamayo waypoints are sampled at 10 Hz beginning at t0 + 0.1 s.
        # Longitudinal control therefore follows trajectory time, independent
        # of the spatial pure-pursuit point selected above for steering.
        deltas = np.diff(np.vstack([np.zeros((1, 3)), path[:, :3]]), axis=0)
        speeds = np.linalg.norm(deltas[:, :2], axis=1) / time_step
        speed_index = min(max(int(trajectory_time_index), 0), len(speeds) - 1)
        target_speed = float(speeds[speed_index])
        throttle, brake, speed_error, effort, hold_active = (
            self._longitudinal_command(target_speed, current_speed_mps, time_step)
        )
        if target_speed <= self.stop_target_speed_mps:
            target_speed = 0.0
        return ControlCommand(
            throttle,
            brake,
            steer,
            target_speed,
            target_index,
            requested_steer,
            speed_index,
            speed_error,
            effort,
            hold_active,
        )
