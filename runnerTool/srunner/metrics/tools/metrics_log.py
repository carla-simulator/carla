#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Support class of the MetricsManager to query the information available
to the metrics.

It also provides a series of functions to help the user querry
specific information
"""

import fnmatch
from srunner.metrics.tools.metrics_parser import MetricsParser

class MetricsLog(object):  # pylint: disable=too-many-public-methods
    """
    Utility class to query the log.
    """

    def __init__(self, recorder):
        """
        Initializes the log class and parses it to extract the dictionaries.
        """
        # Parse the information
        parser = MetricsParser(recorder)
        self._simulation, self._actors, self._frames = parser.parse_recorder_info()

    ### Functions used to get general info of the simulation ###
    def get_actor_collisions(self, actor_id):
        """
        Returns a dict where the keys are the frame number and the values,
        a list of actor ids the actor collided with.

        Args:
            actor_id (int): ID of the actor.
        """
        actor_collisions = {}

        for i, frame in enumerate(self._frames):
            collisions = frame["events"]["collisions"]

            if actor_id in collisions:
                actor_collisions.update({i: collisions[actor_id]})

        return actor_collisions

    def get_total_frame_count(self):
        """
        Returns an int with the total amount of frames the simulation lasted.
        """

        return self._simulation["total_frames"]

    def get_elapsed_time(self, frame):
        """
        Returns a float with the elapsed time of a specific frame.
        """

        return self._frames[frame]["frame"]["elapsed_time"]

    def get_delta_time(self, frame):
        """
        Returns a float with the delta time of a specific frame.
        """

        return self._frames[frame]["frame"]["delta_time"]

    def get_platform_time(self, frame):
        """
        Returns a float with the platform time time of a specific frame.
        """

        return self._frames[frame]["frame"]["platform_time"]

    ### Functions used to get info about the actors ###
    def get_ego_vehicle_id(self):
        """
        Returns the id of the ego vehicle.
        """
        return self.get_actor_ids_with_role_name("hero")[0]

    def get_actor_ids_with_role_name(self, role_name):
        """
        Returns a list of actor ids that match the given role_name.

        Args:
            role_name (str): string with the desired role_name to filter the actors.
        """
        actor_list = []

        for actor_id in self._actors:
            actor = self._actors[actor_id]
            if "role_name" in actor and actor["role_name"] == role_name:
                actor_list.append(actor_id)

        return actor_list

    def get_actor_ids_with_type_id(self, type_id):
        """
        Returns a list of actor ids that match the given type_id, matching fnmatch standard.

        Args:
            type_id (str): string with the desired type id to filter the actors.
        """
        actor_list = []

        for actor_id in self._actors:
            actor = self._actors[actor_id]
            if "type_id" in actor and fnmatch.fnmatch(actor["type_id"], type_id):
                actor_list.append(actor_id)

        return actor_list

    def get_actor_attributes(self, actor_id):
        """
        Returns a dictionary with all the attributes of an actor.

        Args:
            actor_id (int): ID of the actor.
        """
        if actor_id in self._actors:
            return self._actors[actor_id]

        return None

    def get_actor_bounding_box(self, actor_id):
        """
        Returns the bounding box of the specified actor

        Args:
            actor_id (int): Id of the actor
        """

        if actor_id in self._actors:
            if "bounding_box" in self._actors[actor_id]:
                return self._actors[actor_id]["bounding_box"]
            return None

        return None

    def get_traffic_light_trigger_volume(self, traffic_light_id):
        """
        Returns the trigger volume of the specified traffic light

        Args:
            actor_id (int): Id of the traffic light
        """

        if traffic_light_id in self._actors:
            if "trigger_volume" in self._actors[traffic_light_id]:
                return self._actors[traffic_light_id]["trigger_volume"]
            return None

        return None

    def get_actor_alive_frames(self, actor_id):
        """
        Returns a tuple with the first and last frame an actor was alive.
        It is important to note that frames start at 1, not 0.

        Args:
            actor_id (int): Id of the actor
        """

        if actor_id in self._actors:

            actor_info = self._actors[actor_id]
            first_frame = actor_info["created"]
            if "destroyed" in actor_info:
                last_frame = actor_info["destroyed"] - 1
            else:
                last_frame = self.get_total_frame_count()

            return first_frame, last_frame

        return None, None

    ### Functions used to get the actor states ###
    def _get_actor_state(self, actor_id, state, frame):
        """
        Given an actor id, returns the specific variable of that actor at a given frame.
        Returns None if the actor_id or the state are missing.

        Args:
            actor_id (int): Id of the actor to be checked.
            frame: (int): frame number of the simulation.
            attribute (str): name of the actor's attribute to be returned.
        """
        frame_state = self._frames[frame - 1]["actors"]

        # Check if the actor exists
        if actor_id in frame_state:

            # Check if the state exists
            if state not in frame_state[actor_id]:
                return None

            state_info = frame_state[actor_id][state]
            return state_info

        return None

    def _get_all_actor_states(self, actor_id, state, first_frame=None, last_frame=None):
        """
        Given an actor id, returns a list of the specific variable of that actor during
        a frame interval. Some elements might be None.

        By default, first_frame and last_frame are the start and end of the simulation, respectively.

        Args:
            actor_id (int): ID of the actor.
            attribute: name of the actor's attribute to be returned.
            first_frame (int): First frame checked. By default, 0.
            last_frame (int): Last frame checked. By default, max number of frames.
        """
        if first_frame is None:
            first_frame = 1
        if last_frame is None:
            last_frame = self.get_total_frame_count()

        state_list = []

        for frame_number in range(first_frame, last_frame + 1):
            state_info = self._get_actor_state(actor_id, state, frame_number)
            state_list.append(state_info)

        return state_list

    def _get_states_at_frame(self, frame, state, actor_list=None):
        """
        Returns a dict where the keys are the frame number, and the values are the
        carla.Transform of the actor at the given frame.

        By default, all actors will be considered.
        """
        states = {}
        actor_info = self._frames[frame]["actors"]

        for actor_id in actor_info:
            if not actor_list:
                _state = self._get_actor_state(actor_id, state, frame)
                if _state:
                    states.update({actor_id: _state})
            elif actor_id in actor_list:
                _state = self._get_actor_state(actor_id, state, frame)
                states.update({actor_id: _state})

        return states

    # Transforms
    def get_actor_transform(self, actor_id, frame):
        """
        Returns the transform of the actor at a given frame.
        """
        return self._get_actor_state(actor_id, "transform", frame)

    def get_all_actor_transforms(self, actor_id, first_frame=None, last_frame=None):
        """
        Returns a list with all the transforms of the actor at the frame interval.
        """
        return self._get_all_actor_states(actor_id, "transform", first_frame, last_frame)

    def get_actor_transforms_at_frame(self, frame, actor_list=None):
        """
        Returns a dictionary {int - carla.Transform} with the actor ID and transform
        at a given frame of all the actors at actor_list.
        """
        return self._get_states_at_frame(frame, "transform", actor_list)

    # Velocities
    def get_actor_velocity(self, actor_id, frame):
        """
        Returns the velocity of the actor at a given frame.
        """
        return self._get_actor_state(actor_id, "velocity", frame)

    def get_all_actor_velocities(self, actor_id, first_frame=None, last_frame=None):
        """
        Returns a list with all the velocities of the actor at the frame interval.
        """
        return self._get_all_actor_states(actor_id, "velocity", first_frame, last_frame)

    def get_actor_velocities_at_frame(self, frame, actor_list=None):
        """
        Returns a dictionary {int - carla.Vector3D} with the actor ID and velocity
        at a given frame of all the actors at actor_list.
        """
        return self._get_states_at_frame(frame, "velocity", actor_list)

    # Angular velocities
    def get_actor_angular_velocity(self, actor_id, frame):
        """
        Returns the angular velocity of the actor at a given frame.
        """
        return self._get_actor_state(actor_id, "angular_velocity", frame)

    def get_all_actor_angular_velocities(self, actor_id, first_frame=None, last_frame=None):
        """
        Returns a list with all the angular velocities of the actor at the frame interval.
        """
        return self._get_all_actor_states(actor_id, "angular_velocity", first_frame, last_frame)

    def get_actor_angular_velocities_at_frame(self, frame, actor_list=None):
        """
        Returns a dictionary {int - carla.Vector3D} with the actor ID and angular velocity
        at a given frame of all the actors at actor_list.
        """
        return self._get_states_at_frame(frame, "angular_velocity", actor_list)

    # Acceleration
    def get_actor_acceleration(self, actor_id, frame):
        """
        Returns the acceleration of the actor at a given frame.
        """
        return self._get_actor_state(actor_id, "acceleration", frame)

    def get_all_actor_accelerations(self, actor_id, first_frame=None, last_frame=None):
        """
        Returns a list with all the accelerations of the actor at the frame interval.
        """
        return self._get_all_actor_states(actor_id, "acceleration", first_frame, last_frame)

    def get_actor_accelerations_at_frame(self, frame, actor_list=None):
        """
        Returns a dictionary {int - carla.Vector3D} with the actor ID and angular velocity
        at a given frame of all the actors at actor_list.
        """
        return self._get_states_at_frame(frame, "acceleration", actor_list)

    # Controls
    def get_vehicle_control(self, vehicle_id, frame):
        """
        Returns the control of the vehicle at a given frame.
        """
        return self._get_actor_state(vehicle_id, "control", frame)

    def get_vehicle_physics_control(self, vehicle_id, frame):
        """
        Returns the carla.VehiclePhysicsControl of a vehicle at a given frame.
        Returns None if the id can't be found.
        """

        for i in range(frame - 1, -1, -1):  # Go backwards from the frame until 0
            physics_info = self._frames[i]["events"]["physics_control"]

            if vehicle_id in physics_info:
                return physics_info[vehicle_id]

        return None

    def get_walker_speed(self, walker_id, frame):
        """
        Returns the speed of the walker at a specific frame.
        """
        return self._get_actor_state(walker_id, "speed", frame)

    # Traffic lights
    def get_traffic_light_state(self, traffic_light_id, frame):
        """
        Returns the state of the traffic light at a specific frame.
        """
        return self._get_actor_state(traffic_light_id, "state", frame)

    def is_traffic_light_frozen(self, traffic_light_id, frame):
        """
        Returns whether or not the traffic light is frozen at a specific frame.
        """
        return self._get_actor_state(traffic_light_id, "frozen", frame)

    def get_traffic_light_elapsed_time(self, traffic_light_id, frame):
        """
        Returns the elapsed time of the traffic light at a specific frame.
        """
        return self._get_actor_state(traffic_light_id, "elapsed_time", frame)

    def get_traffic_light_state_time(self, traffic_light_id, state, frame):
        """
        Returns the state time of the traffic light at a specific frame.
        Returns None if the id can't be found.
        """

        for i in range(frame - 1, -1, -1):  # Go backwards from the frame until 0
            state_times_info = self._frames[i]["events"]["traffic_light_state_time"]

            if traffic_light_id in state_times_info:
                states = state_times_info[traffic_light_id]
                if state in states:
                    return state_times_info[traffic_light_id][state]

        return None

    # Vehicle lights
    def get_vehicle_lights(self, vehicle_id, frame):
        """
        Returns the vehicle lights of the vehicle at a specific frame.
        """
        return self._get_actor_state(vehicle_id, "lights", frame)

    def is_vehicle_light_active(self, light, vehicle_id, frame):
        """
        Returns the elapsed time of the traffic light at a specific frame.
        """
        lights = self.get_vehicle_lights(vehicle_id, frame)

        if light in lights:
            return True

        return False

    # Scene lights
    def get_scene_light_state(self, light_id, frame):
        """
        Returns the state of the scene light at a specific frame.
        Returns None if the id can't be found.
        """

        for i in range(frame - 1, -1, -1):  # Go backwards from the frame until 0
            scene_lights_info = self._frames[i]["events"]["scene_lights"]

            if light_id in scene_lights_info:
                return scene_lights_info[light_id]

        return None
