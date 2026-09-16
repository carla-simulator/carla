# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
Simple Trajectory Follower using BasicAgent's PID Controller

This provides maximum control for following NUREC trajectories that may deviate 
from lanes, with timing-based speed control.
"""

import logging
import carla
from typing import Optional, List, Tuple
import math


from utils import mat_to_carla_transform

logger = logging.getLogger(__name__)

class SimpleTrajectoryFollower:
    """
    A simplified trajectory follower that uses direct control calculations.
    Can follow trajectories that deviate from lanes and hits each waypoint at the expected time.
    """
    
    def __init__(self, nurec_actor, world: carla.World):
        """
        Initialize the simple trajectory follower.
        
        Args:
            nurec_actor: NurecActor instance containing the vehicle and track data
            world: CARLA World instance  
        """
        self.nurec_actor = nurec_actor
        self.world = world
        self.vehicle = nurec_actor.actor_inst
        self.track = nurec_actor.track
        
        # Control parameters
        self.max_throttle = 1.0
        self.max_brake = 1.0
        self.max_steering = 1.0
        
        # PID parameters for speed control
        self.speed_kf = 0.02
        self.speed_kp = 2.0
        self.speed_ki = 0.2
        self.speed_kd = 0.1
        self.speed_error_integral = 0.0
        self.speed_error_decay = 1
        self.last_speed_error = 0.0
        
        # Steering parameters
        self.steering_kp = 1.0
        self.max_steering_angle = 70.0  # degrees
        self.lookahead_time = 1.0  # seconds - lookahead time for steering
        
        # Trajectory state
        self.trajectory_points: List[Tuple[int, carla.Transform]] = []  # List of (timestamp_us, carla.Transform)
        self.trajectory_start_time: float = 0
        self.trajectory_scenario_start_time: int = 0
        self.current_target_index = 0
        self.trajectory_complete = False
        self.last_world_time = 0.0
        
        logger.debug(f"Initialized simple trajectory follower for vehicle {self.vehicle.id}")
        logger.debug(f"Using {self.lookahead_time}s lookahead for steering control")
    
    def set_trajectory_from_track(self, start_time: int, end_time: int, time_spacing: int = 50000) -> None:
        """
        Extract trajectory points directly from NUREC track data.
        
        Args:
            start_time: Start time in microseconds
            end_time: End time in microseconds
            time_spacing: Time spacing between samples in microseconds (50ms default)
        """
        self.trajectory_points = []
        
        current_time = start_time
        while current_time <= end_time:
            # Get pose from track
            pose_matrix = self.track.interpolate_pose_matrix(current_time)
            if pose_matrix is not None:
                # Convert to CARLA transform
                carla_transform = mat_to_carla_transform(pose_matrix)
                self.trajectory_points.append((current_time, carla_transform))
            
            current_time += time_spacing
        
        self.trajectory_scenario_start_time = start_time
        self.current_target_index = 0
        self.trajectory_complete = False
        self.speed_error_integral = 0.0
        self.last_speed_error = 0.0
        self.last_world_time = float(start_time)
        
        logger.debug(f"Set trajectory with {len(self.trajectory_points)} points "
                   f"from {start_time} to {end_time} microseconds")
    
    def start_following(self, world_start_time: float) -> None:
        """
        Start following the trajectory.
        
        Args:
            world_start_time: Current world time in seconds when starting
        """
        self.trajectory_start_time = world_start_time
        self.last_world_time = world_start_time
        logger.debug(f"Started trajectory following at world time {world_start_time}")
    
    def get_current_target_info(self, current_world_time: float) -> Optional[tuple]:
        """
        Get the current target point and speed based on timing.
        
        Args:
            current_world_time: Current world time in seconds
            
        Returns:
            Tuple of (target_transform, target_speed_ms, lookahead_transform) or None if trajectory is complete
        """
        if not self.trajectory_points or self.trajectory_start_time is None:
            return None
        
        # Calculate elapsed time since trajectory start
        elapsed_time_seconds = current_world_time - self.trajectory_start_time
        elapsed_time_microseconds = elapsed_time_seconds * 1_000_000
        
        # Calculate target time in scenario
        target_scenario_time = self.trajectory_scenario_start_time + elapsed_time_microseconds
        
        # Find the appropriate trajectory point for current position
        target_index = 0
        for i, (timestamp, _) in enumerate(self.trajectory_points):
            if timestamp <= target_scenario_time:
                target_index = i
            else:
                break
        
        # Check if we've completed the trajectory
        if target_index >= len(self.trajectory_points) - 1:
            self.trajectory_complete = True
            return None
        
        self.current_target_index = target_index
        
        # Get current and next waypoints for speed calculation
        current_timestamp, current_transform = self.trajectory_points[target_index]
        next_timestamp, next_transform = self.trajectory_points[target_index + 1]
        
        # Calculate distance and time to next waypoint for speed
        current_location = current_transform.location
        next_location = next_transform.location
        distance = current_location.distance(next_location)  # meters
        
        time_delta = (next_timestamp - current_timestamp) / 1_000_000  # seconds
        
        # Calculate required speed to hit next waypoint on time
        if time_delta > 0:
            target_speed_ms = distance / time_delta  # m/s
        else:
            target_speed_ms = 0.0
        
        # Get lookahead point for steering (2 seconds ahead)
        lookahead_transform = self.get_lookahead_point(target_scenario_time)
        if lookahead_transform is None:
            # If no lookahead available, use next waypoint
            lookahead_transform = next_transform
        
        return next_transform, target_speed_ms, lookahead_transform
    
    def get_lookahead_point(self, current_scenario_time: float) -> Optional[carla.Transform]:
        """
        Get the lookahead point for steering control.
        
        Args:
            current_scenario_time: Current time in scenario (microseconds)
            
        Returns:
            Transform of lookahead point or None if not available
        """
        lookahead_time_us = self.lookahead_time * 1_000_000  # Convert to microseconds
        lookahead_scenario_time = current_scenario_time + lookahead_time_us
        
        # Find the trajectory point closest to the lookahead time
        lookahead_index = None
        min_time_diff = float('inf')
        
        for i, (timestamp, _) in enumerate(self.trajectory_points):
            time_diff = abs(timestamp - lookahead_scenario_time)
            if time_diff < min_time_diff:
                min_time_diff = time_diff
                lookahead_index = i
            elif time_diff > min_time_diff:
                # We've passed the closest point
                break
        
        if lookahead_index is not None and lookahead_index < len(self.trajectory_points):
            return self.trajectory_points[lookahead_index][1]
        
        # If lookahead point is beyond trajectory, use the last point
        if len(self.trajectory_points) > 0:
            return self.trajectory_points[-1][1]
        
        return None
    
    def calculate_steering(self, target_transform: carla.Transform) -> tuple:
        """
        Calculate steering angle to reach target position using lookahead.
        
        Args:
            target_transform: Lookahead target transform to reach
            
        Returns:
            Tuple of (steering_value, steering_angle_degrees) 
        """
        # Get current vehicle transform
        vehicle_transform = self.vehicle.get_transform()
        vehicle_location = vehicle_transform.location
        vehicle_rotation = vehicle_transform.rotation
        
        # Calculate vector from vehicle to lookahead target
        target_location = target_transform.location
        dx = target_location.x - vehicle_location.x
        dy = target_location.y - vehicle_location.y
        
        # Convert to vehicle coordinate system
        vehicle_yaw_rad = math.radians(vehicle_rotation.yaw)
        local_x = dx * math.cos(vehicle_yaw_rad) + dy * math.sin(vehicle_yaw_rad)
        local_y = -dx * math.sin(vehicle_yaw_rad) + dy * math.cos(vehicle_yaw_rad)
        
        # Calculate desired steering angle using lookahead point
        if abs(local_x) < 0.1:  # Avoid division by zero
            steering_angle = 0.0
        else:
            steering_angle = math.degrees(math.atan2(local_y, local_x))
        
        # Apply proportional control
        steering_angle *= self.steering_kp
        
        # Clamp to max steering angle
        steering_angle = max(-self.max_steering_angle, min(self.max_steering_angle, steering_angle))
        
        # Convert to normalized steering (-1 to 1)
        normalized_steering = steering_angle / self.max_steering_angle
        
        return normalized_steering, steering_angle
    
    def calculate_throttle_brake(self, target_speed_ms: float, time_delta: float) -> tuple:
        """
        Calculate throttle and brake values based on target speed.
        
        Args:
            target_speed_ms: Target speed in m/s
            
        Returns:
            Tuple of (throttle, brake, p_term, i_term, d_term) values
        """
        # Get current vehicle speed
        velocity = self.vehicle.get_velocity()
        current_speed_ms = math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2)
        
        # Calculate speed error
        speed_error = target_speed_ms - current_speed_ms
        
        # PID control for speed
        self.speed_error_integral *= self.speed_error_decay
        self.speed_error_integral += speed_error * time_delta
        speed_error_derivative = (speed_error - self.last_speed_error) / time_delta
        
        # Calculate individual PID terms
        k_term = self.speed_kf * target_speed_ms
        p_term = self.speed_kp * speed_error
        i_term = self.speed_ki * self.speed_error_integral
        d_term = self.speed_kd * speed_error_derivative
        
        speed_control = k_term + p_term + i_term + d_term
        
        self.last_speed_error = speed_error
        
        # Convert to throttle/brake
        if speed_control > 0:
            # Need to accelerate
            throttle = min(speed_control, self.max_throttle)
            brake = 0.0
        else:
            # Need to brake
            throttle = 0.0
            brake = min(-speed_control, self.max_brake)
        
        return throttle, brake, k_term, p_term, i_term, d_term
    
    def update(self, current_world_time: float) -> carla.VehicleControl:
        """
        Update the trajectory follower and get vehicle control.
        
        Args:
            current_world_time: Current world time in seconds
            
        Returns:
            Vehicle control commands
        """
        if self.trajectory_complete:
            return carla.VehicleControl()
        
        time_delta = current_world_time - self.last_world_time
        if time_delta <= 0:
            time_delta = 0.01
        self.last_world_time = current_world_time
        
        # Get current target point, required speed, and lookahead point
        target_info = self.get_current_target_info(current_world_time)
        if target_info is None:
            return carla.VehicleControl()
        
        target_transform, target_speed_ms, lookahead_transform = target_info
        
        # Calculate control values using lookahead for steering
        steering, steering_angle = self.calculate_steering(lookahead_transform)
        throttle, brake, k_term, p_term, i_term, d_term = self.calculate_throttle_brake(target_speed_ms, time_delta)
        
        
        # Create control command
        control = carla.VehicleControl()
        control.throttle = throttle
        control.brake = brake
        control.steer = steering
        control.hand_brake = False
        control.reverse = False
        control.manual_gear_shift = False
        
        return control
    
    def is_complete(self) -> bool:
        """Check if trajectory following is complete."""
        return self.trajectory_complete
    
    def get_progress(self) -> float:
        """Get trajectory following progress as a percentage."""
        if not self.trajectory_points:
            return 0.0
        
        if self.trajectory_complete:
            return 100.0
        
        return (self.current_target_index / len(self.trajectory_points)) * 100.0
    