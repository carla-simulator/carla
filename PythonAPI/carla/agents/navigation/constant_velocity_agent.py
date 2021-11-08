# Copyright (c) # Copyright (c) 2018-2020 CVC.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module implements an agent that roams around a track following random
waypoints and avoiding other vehicles. The agent also responds to traffic lights.
It can also make use of the global route planner to follow a specifed route
"""

import carla
import weakref

from agents.navigation.basic_agent import BasicAgent

class ConstantVelocityAgent(BasicAgent):
    """
    ConstantVelocityAgent implements an agent that navigates the scene at a fixed velocity.
    This agent will fail if asked to perform turns that are impossible are the desired speed.
    This includes lane changes. When a collision is detected, the constant velocity will stop,
    and normal behavior.
    """

    def __init__(self, vehicle, target_speed=20, opt_dict={}):
        """
        Initialization the agent parameters, the local and the global planner.

            :param vehicle: actor to apply to agent logic onto
            :param target_speed: speed (in Km/h) at which the vehicle will move
            :param opt_dict: dictionary in case some of its parameters want to be changed.
                This also applies to parameters related to the LocalPlanner.
        """
        super(ConstantVelocityAgent, self).__init__(vehicle, target_speed, opt_dict=opt_dict)

        self._restart_time = 10  # Time after collision before the constant velocity behavior starts again
        self._use_basic_behavior = True  # Whether or not to use the BasicAgent behavior when the constant velcoity is down
        self._traffic_light_detection_dist = 1
        self._vehicle_detection_dist = 10

        if 'restart_time' in opt_dict:
            self._restart_time = opt_dict['restart_time']
        if 'use_basic_behavior' in opt_dict:
            self._use_basic_behavior = opt_dict['use_basic_behavior']
        if 'traffic_light_detection_dist' in opt_dict:
            self._traffic_light_detection_dist = opt_dict['traffic_light_detection_dist']
        if 'vehicle_detection_dist' in opt_dict:
            self._vehicle_detection_dist = opt_dict['vehicle_detection_dist']

        self._is_vehicle_stopped = False
        self._constant_velocity_stop_time = None

        self._set_collision_sensor()
        self._set_constant_velocity()

    def set_target_speed(self, speed):
        """Changes the target speed of the agent"""
        super(ConstantVelocityAgent, self).set_target_speed(speed)
        self._set_constant_velocity()

    def _set_constant_velocity(self):
        """Forces the agent to drive at the specified speed"""
        self._vehicle.enable_constant_velocity(carla.Vector3D(self._target_speed / 3.6, 0, 0))
        self.is_constant_velocity_active = True
        self._is_vehicle_stopped = False

    def _stop_vehicle(self):
        """Stops the vehicle"""
        self._vehicle.enable_constant_velocity(carla.Vector3D(0, 0, 0))
        self._is_vehicle_stopped = True

    def _stop_constant_velocity(self):
        """Stops the constant velocity behavior"""
        self._vehicle.disable_constant_velocity()

    def restart_constant_velocity(self):
        """Public method to restart the constant velocity"""
        self._set_constant_velocity()

    def run_step(self):
        """Execute one step of navigation."""
        if not self.is_constant_velocity_active:
            # Check if it time to restart it again. If not, do something else
            if self._world.get_snapshot().timestamp.elapsed_seconds - self._constant_velocity_stop_time > self._restart_time:
                self.restart_constant_velocity()
                self.is_constant_velocity_active = True
            elif self._use_basic_behavior:
                return super(ConstantVelocityAgent, self).run_step()
            else:
                return carla.VehicleControl()

        vehicle_location = self._vehicle.get_location()
        vehicle_wp = self._map.get_waypoint(vehicle_location)
        if abs(vehicle_wp.transform.location.z - vehicle_location.z) > 5:
            self._stop_constant_velocity()

        hazard_detected = False

        # Retrieve all relevant actors
        actor_list = self._world.get_actors()
        vehicle_list = actor_list.filter("*vehicle*")
        lights_list = actor_list.filter("*traffic_light*")

        # Check for possible vehicle obstacles
        affected_by_vehicle, _, _ = self._vehicle_obstacle_detected(vehicle_list)
        if affected_by_vehicle:
            hazard_detected = True

        # Check if the vehicle is affected by a red traffic light
        affected_by_tlight, _ = self._affected_by_traffic_light(lights_list, self._traffic_light_detection_dist)
        if affected_by_tlight:
            hazard_detected = True

        # The longitudinal PID is overwritten by the constant velocity, so set them to 0 (though it really isn't needed)
        control = self._local_planner.run_step()
        control.throttle = 0
        control.brake = 0

        if hazard_detected:
            self._stop_vehicle()
        elif self._is_vehicle_stopped:  # First frame of not detecting hazards, reactivate the movement
            self._set_constant_velocity()

        return control

    def _set_collision_sensor(self):
        blueprint = self._world.get_blueprint_library().find('sensor.other.collision')
        self._collision_sensor = self._world.spawn_actor(blueprint, carla.Transform(), attach_to=self._vehicle)
        self._collision_sensor.listen(lambda event: self._on_collision(weakref.ref(self), event))

    @staticmethod
    def _on_collision(weak_self, event):
        """Disables the constant velocity to avoid weird behaviors"""
        self = weak_self()
        self.is_constant_velocity_active = False
        self._stop_constant_velocity()
        self._constant_velocity_stop_time =  self._world.get_snapshot().timestamp.elapsed_seconds