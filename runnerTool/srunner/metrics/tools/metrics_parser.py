#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Support class of the MetricsManager to parse the information of
the CARLA recorder into a readable dictionary
"""

import carla


def parse_actor(info):
    """Returns a dictionary with the basic actor information"""
    actor = {
        "type_id": info[2],
        "location": carla.Location(
            x=float(info[5][1:-1]) / 100,
            y=float(info[6][:-1]) / 100,
            z=float(info[7][:-1]) / 100
        )
    }
    return actor

def parse_transform(info):
    """Parses a list into a carla.Transform"""
    transform = carla.Transform(
        carla.Location(
            x=float(info[3][1:-1]) / 100,
            y=float(info[4][:-1]) / 100,
            z=float(info[5][:-1]) / 100,
        ),
        carla.Rotation(
            roll=float(info[7][1:-1]),
            pitch=float(info[8][:-1]), 
            yaw=float(info[9][:-1])
        )
    )
    return transform

def parse_control(info):
    """Parses a list into a carla.VehicleControl"""
    control = carla.VehicleControl(
        throttle=float(info[5]),
        steer=float(info[3]),
        brake=float(info[7]),
        hand_brake=bool(int(info[9])),
        reverse=int(info[11]) < 0,
        manual_gear_shift=False,
        gear=int(info[11]),
    )
    return control

def parse_vehicle_lights(info):
    """Parses a list into a carla.VehicleLightState"""
    srt_to_vlight = {
        "None": carla.VehicleLightState.NONE,
        "Position": carla.VehicleLightState.Position,
        "LowBeam": carla.VehicleLightState.LowBeam,
        "HighBeam": carla.VehicleLightState.HighBeam,
        "Brake": carla.VehicleLightState.Brake,
        "RightBlinker": carla.VehicleLightState.RightBlinker,
        "LeftBlinker": carla.VehicleLightState.LeftBlinker,
        "Reverse": carla.VehicleLightState.Reverse,
        "Fog": carla.VehicleLightState.Fog,
        "Interior": carla.VehicleLightState.Interior,
        "Special1": carla.VehicleLightState.Special1,
        "Special2": carla.VehicleLightState.Special2,
    }

    lights = []
    for i in range(2, len(info)):
        lights.append(srt_to_vlight[info[i]])

    return lights

def parse_traffic_light(info):
    """Parses a list into a dictionary with all the traffic light's information"""
    number_to_state = {
        "0": carla.TrafficLightState.Red,
        "1": carla.TrafficLightState.Yellow,
        "2": carla.TrafficLightState.Green,
        "3": carla.TrafficLightState.Off,
        "4": carla.TrafficLightState.Unknown,
    }
    traffic_light = {
        "state": number_to_state[info[3]],
        "frozen": bool(int(info[5])),
        "elapsed_time": float(info[7]),
    }
    return traffic_light

def parse_velocity(info):
    """Parses a list into a carla.Vector3D with the velocity"""
    velocity = carla.Vector3D(
        x=float(info[3][1:-1]),
        y=float(info[4][:-1]),
        z=float(info[5][:-1])
    )
    return velocity

def parse_angular_velocity(info):
    """Parses a list into a carla.Vector3D with the angular velocity"""
    velocity = carla.Vector3D(
        x=float(info[7][1:-1]),
        y=float(info[8][:-1]),
        z=float(info[9][:-1])
    )
    return velocity

def parse_scene_lights(info):
    """Parses a list into a carla.VehicleLightState"""

    red = int(float(info[7][1:-1]) * 255)
    green = int(float(info[8][:-1]) * 255)
    blue = int(float(info[9][:-1]) * 255)

    scene_light = carla.LightState(
        intensity=int(float(info[5])),
        color=carla.Color(red, green, blue),
        group=carla.LightGroup.NONE,
        active=bool(info[3])
    )
    return scene_light

def parse_bounding_box(info):
    """
    Parses a list into a carla.BoundingBox.
    Some actors like sensors might have 'nan' location and 'inf' extent, so filter those.
    """
    if 'nan' in info[3]:
        location = carla.Location()
    else:
        location = carla.Location(
            float(info[3][1:-1])/100,
            float(info[4][:-1])/100,
            float(info[5][:-1])/100,
        )

    if 'inf' in info[7]:
        extent = carla.Vector3D()
    else:
        extent = carla.Vector3D(
            float(info[7][1:-1])/100,
            float(info[8][:-1])/100,
            float(info[9][:-1])/100,
        )

    bbox = carla.BoundingBox(location, extent)

    return bbox

def parse_state_times(info):
    """Parses a list into a dict containing the state times of the traffic lights"""
    state_times = {
        carla.TrafficLightState.Green: float(info[3]),
        carla.TrafficLightState.Yellow: float(info[5]),
        carla.TrafficLightState.Red: float(info[7]),
    }
    return state_times

def parse_vector_list(info):
    """Parses a list of string into a list of Vector2D"""
    vector_list = []
    for i in range(0, len(info), 2):
        vector = carla.Vector2D(
            x=float(info[i][1:-1]),
            y=float(info[i+1][:-1]),
        )
        vector_list.append(vector)

    return vector_list

def parse_gears_control(info):
    """Parses a list into a GearPhysicsControl"""
    gears_control = carla.GearPhysicsControl(
        ratio=float(info[3]),
        down_ratio=float(info[5]),
        up_ratio=float(info[7]),
    )
    return gears_control

def parse_wheels_control(info):
    """Parses a list into a WheelsPhysicsControl"""
    wheels_control = carla.WheelPhysicsControl(
        tire_friction=float(info[3]),
        damping_rate=float(info[5]),
        max_steer_angle=float(info[7]),
        radius=float(info[9]),
        max_brake_torque=float(info[11]),
        max_handbrake_torque=float(info[13]),
        position=carla.Vector3D(
            x=float(info[17][1:-1]) / 100,
            y=float(info[17][:-1]) / 100,
            z=float(info[17][:-1]) / 100)
    )
    return wheels_control


class MetricsParser(object):
    """
    Class used to parse the CARLA recorder into readable information
    """

    def __init__(self, recorder_info):

        self.recorder_info = recorder_info
        self.frame_list = None
        self.frame_row = None
        self.i = 0

    def get_row_elements(self, indent_num, split_string):
        """
        returns a list with the elements of the row
        """
        return self.frame_row[indent_num:].split(split_string)

    def next_row(self):
        """
        Gets the next row of the recorder
        """
        self.i += 1
        self.frame_row = self.frame_list[self.i]

    def parse_recorder_info(self):
        """
        Parses the recorder into readable information.

        Args:
            recorder_info (str): string given by the recorder
        """

        # Divide it into frames
        recorder_list = self.recorder_info.split("Frame")

        # Get general information
        header = recorder_list[0].split("\n")
        sim_map = header[1][5:]
        sim_date = header[2][6:]

        annex = recorder_list[-1].split("\n")
        sim_frames = int(annex[0][3:])
        sim_duration = float(annex[1][10:-8])

        recorder_list = recorder_list[1:-1]

        simulation_info = {
            "map": sim_map,
            "date:": sim_date,
            "total_frames": sim_frames,
            "duration": sim_duration
        }

        actors_info = {}
        frames_info = []

        for frame in recorder_list:

            # Divide the frame in lines
            self.frame_list = frame.split("\n")

            # Get the general frame information
            frame_info = self.frame_list[0].split(" ")
            frame_number = int(frame_info[1])
            frame_time = float(frame_info[3])

            try:
                prev_frame = frames_info[frame_number - 2]
                prev_time = prev_frame["frame"]["elapsed_time"]
                delta_time = round(frame_time - prev_time, 6)
            except IndexError:
                delta_time = 0

            # Variable to store all the information about the frame
            frame_state = {
                "frame": {
                    "elapsed_time": frame_time,
                    "delta_time": delta_time,
                    "platform_time": None
                },
                "actors": {},
                "events":{
                    "scene_lights": {},
                    "physics_control": {},
                    "traffic_light_state_time": {},
                    "collisions": {}
                }
            }

            # Loop through all the other rows.
            self.i = 0
            self.next_row()

            while self.frame_row.startswith(' Create') or self.frame_row.startswith('  '):

                if self.frame_row.startswith(' Create'):
                    elements = self.get_row_elements(1, " ")
                    actor_id = int(elements[1][:-1])

                    actor = parse_actor(elements)
                    actors_info.update({actor_id: actor})
                    actors_info[actor_id].update({"created": frame_number})
                else:
                    elements = self.get_row_elements(2, " = ")
                    actors_info[actor_id].update({elements[0]: elements[1]})

                self.next_row()

            while self.frame_row.startswith(' Destroy'):

                elements = self.get_row_elements(1, " ")

                actor_id = int(elements[1])
                actors_info[actor_id].update({"destroyed": frame_number})

                self.next_row()

            while self.frame_row.startswith(' Collision'):

                elements = self.get_row_elements(1, " ")

                actor_id = int(elements[4])
                other_id = int(elements[-1])

                if actor_id not in frame_state["events"]["collisions"]:
                    frame_state["events"]["collisions"][actor_id] = [other_id]
                else:
                    collisions = frame_state["events"]["collisions"][actor_id]
                    collisions.append(other_id)
                    frame_state["events"]["collisions"].update({actor_id: collisions})

                self.next_row()

            while self.frame_row.startswith(' Parenting'):

                elements = self.get_row_elements(1, " ")

                actor_id = int(elements[1])
                parent_id = int(elements[3])
                actors_info[actor_id].update({"parent": parent_id})

                self.next_row()

            if self.frame_row.startswith(' Positions'):
                self.next_row()

                while self.frame_row.startswith('  '):

                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    transform = parse_transform(elements)
                    frame_state["actors"].update({actor_id: {"transform": transform}})

                    self.next_row()

            if self.frame_row.startswith(' State traffic lights'):
                self.next_row()

                while self.frame_row.startswith('  '):

                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    traffic_light = parse_traffic_light(elements)
                    frame_state["actors"].update({actor_id: traffic_light})
                    self.next_row()

            if self.frame_row.startswith(' Vehicle animations'):
                self.next_row()

                while self.frame_row.startswith('  '):

                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    control = parse_control(elements)
                    frame_state["actors"][actor_id].update({"control": control})
                    self.next_row()

            if self.frame_row.startswith(' Walker animations'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    frame_state["actors"][actor_id].update({"speed": elements[3]})
                    self.next_row()

            if self.frame_row.startswith(' Vehicle light animations'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    lights = parse_vehicle_lights(elements)
                    frame_state["actors"][actor_id].update({"lights": lights})
                    self.next_row()

            if self.frame_row.startswith(' Scene light changes'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    scene_light = parse_scene_lights(elements)
                    frame_state["events"]["scene_lights"].update({actor_id: scene_light})
                    self.next_row()

            if self.frame_row.startswith(' Dynamic actors'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    velocity = parse_velocity(elements)
                    frame_state["actors"][actor_id].update({"velocity": velocity})

                    angular_v = parse_angular_velocity(elements)
                    frame_state["actors"][actor_id].update({"angular_velocity": angular_v})

                    if delta_time == 0:
                        acceleration = carla.Vector3D(0, 0, 0)
                    else:
                        prev_velocity = frame_state["actors"][actor_id]["velocity"]
                        acceleration = (velocity - prev_velocity) / delta_time

                    frame_state["actors"][actor_id].update({"acceleration": acceleration})
                    self.next_row()

            if self.frame_row.startswith(' Actor bounding boxes'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    bbox = parse_bounding_box(elements)
                    actors_info[actor_id].update({"bounding_box": bbox})
                    self.next_row()

            if self.frame_row.startswith(' Actor trigger volumes'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    trigvol = parse_bounding_box(elements)
                    actors_info[actor_id].update({"trigger_volume": trigvol})
                    self.next_row()

            if self.frame_row.startswith(' Current platform time'):

                elements = self.get_row_elements(1, " ")

                platform_time = float(elements[-1])
                frame_state["frame"]["platform_time"] = platform_time
                self.next_row()

            if self.frame_row.startswith(' Physics Control'):
                self.next_row()

                actor_id = None
                while self.frame_row.startswith('  '):

                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])
                    physics_control = carla.VehiclePhysicsControl()
                    self.next_row()

                    forward_gears = []
                    wheels = []
                    while self.frame_row.startswith('   '):

                        if self.frame_row.startswith('    '):
                            elements = self.get_row_elements(4, " ")
                            if elements[0] == "gear":
                                forward_gears.append(parse_gears_control(elements))
                            elif elements[0] == "wheel":
                                wheels.append(parse_wheels_control(elements))

                        else:
                            elements = self.get_row_elements(3, " = ")
                            name = elements[0]

                            if name == "center_of_mass":
                                values = elements[1].split(" ")
                                value = carla.Vector3D(
                                    float(values[0][1:-1]),
                                    float(values[1][:-1]),
                                    float(values[2][:-1]),
                                )
                                setattr(physics_control, name, value)
                            elif name == "torque_curve" or name == "steering_curve":
                                values = elements[1].split(" ")
                                value = parse_vector_list(values)
                                setattr(physics_control, name, value)

                            elif name == "use_gear_auto_box":
                                name = "use_gear_autobox"
                                value = True if elements[1] == "true" else False
                                setattr(physics_control, name, value)

                            elif "forward_gears" in name or "wheels" in name:
                                pass

                            else:
                                name = name.lower()
                                value = float(elements[1])
                                setattr(physics_control, name, value)

                        self.next_row()

                    setattr(physics_control, "forward_gears", forward_gears)
                    setattr(physics_control, "wheels", wheels)
                    frame_state["events"]["physics_control"].update({actor_id: physics_control})

            if self.frame_row.startswith(' Traffic Light time events'):
                self.next_row()

                while self.frame_row.startswith('  '):
                    elements = self.get_row_elements(2, " ")
                    actor_id = int(elements[1])

                    state_times = parse_state_times(elements)
                    frame_state["events"]["traffic_light_state_time"].update({actor_id: state_times})
                    self.next_row()

            frames_info.append(frame_state)

        return simulation_info, actors_info, frames_info
