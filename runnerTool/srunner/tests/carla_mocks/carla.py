#!/usr/bin/env python

# Copyright (c) 2021 Intel Corporation
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This module provides mocked CARLA classes for unittesting
"""

import copy


class command:
    blueprint = None

    def SpawnActor(blueprint, point):
        new_command = command()
        new_command.blueprint = copy.deepcopy(blueprint)
        return new_command

    def SetSimulatePhysics(blueprint, physics):
        return None

    def FutureActor():
        return None

    def ApplyTransform():
        return None

    def SetAutopilot(actor, autopilot, port):
        return None

    def SetVehicleLightState():
        return None

    def DestroyActor(actor):
        return None

    def then(self, other_command):
        return self


class CarlaBluePrint(object):

    def __init__(self):
        self.id = 0
        self.attributes = {'role_name': ''}

    def has_attribute(self, attribute_string=''):
        return attribute_string in self.attributes

    def set_attribute(self, key, value):
        self.attributes[key] = value

    def has_tag(self, tag_string=''):
        return False


class CarlaBluePrintLibrary:
    def filter(self, filterstring):
        return [CarlaBluePrint()]

    def __len__(self):
        return 1

    def find(self, filterstring):
        return CarlaBluePrint()


class GeoLocation:
    longitude = 0
    latitude = 0


class Vector3D:
    x = 0
    y = 0
    z = 0

    def __init__(self, x=0, y=0, z=0):
        self.x = x
        self.y = y
        self.z = z


class Location():
    x = 0
    y = 0
    z = 0

    def __init__(self, x=0, y=0, z=0):
        self.x = x
        self.y = y
        self.z = z

    def distance(self, other):
        return 0


class Rotation():
    pitch = 0
    roll = 0
    yaw = 0

    def __init__(self, pitch=0, roll=0, yaw=0):
        self.pitch = pitch
        self.roll = roll
        self.yaw = yaw

    def get_forward_vector(self):
        return Vector3D()


class Transform:
    location = Location(0, 0, 0)
    rotation = Rotation(0, 0, 0)

    def __init__(self, location=Location(0, 0, 0), rotation=Rotation(0, 0, 0)):
        self.location = location
        self.rotation = rotation


class Waypoint():
    transform = Transform(Location(), Rotation())
    road_id = 0
    lane_id = 0
    s = 0
    lane_width = 0


class Map:
    name = ""

    def get_spawn_points(self):
        return []

    def transform_to_geolocation(self, transform):
        return GeoLocation()

    def get_waypoint(self, transform):
        return Waypoint()

    def get_waypoint_xodr(self, a, b, c):
        return Waypoint()

    def get_topology(self):
        return []


class TrafficLightState:
    Red = 0
    Green = 1
    Yellow = 2
    Off = 3


class WeatherParameters:
    cloudiness = 0.000000
    cloudiness = 0.000000
    precipitation = 0.000000
    precipitation_deposits = 0.000000
    wind_intensity = 0.000000
    sun_azimuth_angle = 0.000000
    sun_altitude_angle = 0.000000
    fog_density = 0.000000
    fog_distance = 0.000000
    fog_falloff = 0.000000
    wetness = 0.000000
    scattering_intensity = 0.000000
    mie_scattering_scale = 0.000000
    rayleigh_scattering_scale = 0.033100


class WorldSettings:
    synchronous_mode = False
    no_rendering_mode = False
    fixed_delta_seconds = 0
    substepping = True
    max_substep_delta_time = 0.01
    max_substeps = 10
    max_culling_distance = 0
    deterministic_ragdolls = False


class ActorList:

    def __init__(self, actor_list):
        self.actor_list = actor_list

    def filter(self, filterstring):
        return []

    def __len__(self):
        return len(self.actor_list)

    def __getitem__(self, i):
        return self.actor_list[i]


class Control:
    steer = 0
    throttle = 0
    brake = 0


class Actor:

    def __init__(self):
        self.attributes = {'role_name': ''}
        self.id = 0
        self.type_id = None
        self.location = Location()
        self.rotation = Rotation()
        self.transform = Transform(self.location, self.rotation)
        self.is_alive = True

    def get_transform(self):
        return self.transform

    def get_location(self):
        return self.location

    def get_world(self):
        return World()

    def get_control(self):
        return Control()

    def destroy(self):
        del self

    def listen(self, callback):
        pass


class Walker(Actor):
    is_walker = True


class Vehicle(Actor):
    is_vehicle = True


class World:
    actors = []

    def get_settings(self):
        return WorldSettings()

    def get_map(self):
        return Map()

    def get_blueprint_library(self):
        return CarlaBluePrintLibrary()

    def wait_for_tick(self):
        pass

    def get_actors(self, ids=[]):
        actor_list = []
        for actor in self.actors:
            if actor.id in ids:
                actor_list.append(actor)

        return ActorList(actor_list)

    def try_spawn_actor(self, blueprint, spawn_point):
        new_actor = Vehicle()
        new_actor.attributes['role_name'] = blueprint.attributes['role_name']
        new_actor.id = len(self.actors)
        self.actors.append(new_actor)
        return new_actor

    def spawn_actor(self, blueprint, spawn_point, attach_to):
        new_actor = self.try_spawn_actor(blueprint, spawn_point)
        return new_actor


class Client:
    world = World()

    def load_world(self, name):
        return None

    def get_world(self):
        return self.world

    def get_trafficmanager(self, port):
        return None

    def apply_batch_sync(self, batch, sync_mode=False):
        class Response:
            def __init__(self, id):
                self.actor_id = id
                self.error = None

        reponse_list = []
        for batch_cmd in batch:
            if batch_cmd is not None:
                new_actor = Vehicle()
                new_actor.attributes['role_name'] = batch_cmd.blueprint.attributes['role_name']
                new_actor.id = len(self.world.actors)
                self.world.actors.append(new_actor)
                reponse_list.append(Response(new_actor.id))

        return reponse_list
