#!/usr/bin/env python

# Copyright (c) 2019 Aptiv
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Module that mocks Carla PythonAPI.
It reads back data saved with `../record_data.py` script.
"""

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import json
import logging
from os import listdir
from os.path import isfile, join, isdir
import re
from threading import Timer
import time

# ==============================================================================
# -- setup ---------------------------------------------------------------------
# ==============================================================================


path_to_recorded_data = './output/'

start_time = time.time()
with open(path_to_recorded_data + 'start.dict', 'r') as f:
    recorded_start_time = json.loads(f.read().replace("'", '"'))['timestamp']


# ==============================================================================
# -- libcarla mocks ------------------------------------------------------------
# ==============================================================================


class carla:
    def __init__(self):
        pass

    class libcarla:
        def __init__(self):
            pass

        class LaneChange:
            def __init__(self):
                pass

            NONE = 'NONE'
            Right = 'Right'
            Left = 'Left'
            Both = 'Both'

        class TrafficLightState:
            def __init__(self):
                pass

            Green = 'Green'
            Red = 'Red'
            Yellow = 'Yellow'

        class LaneMarkingColor:
            def __init__(self):
                pass

            Standard = 'White'
            White = 'White'
            Blue = 'Blue'
            Green = 'Green'
            Red = 'Red'
            Yellow = 'Yellow'
            Other = 'Other'

        class LaneMarkingType:
            def __init__(self):
                pass

            NONE = 'NONE'
            Other = 'Other'
            Broken = 'Broken'
            Solid = 'Solid'
            SolidSolid = 'SolidSolid'
            SolidBroken = 'SolidBroken'
            BrokenSolid = 'BrokenSolid'
            BrokenBroken = 'BrokenBroken'
            BottsDots = 'BottsDots'
            Grass = 'Grass'
            Curb = 'Curb'


safe_expressions = {"carla": carla, "True": True, "False": False}


# ==============================================================================
# -- utils ---------------------------------------------------------------------
# ==============================================================================


def safe_eval(expression):
    # pylint: disable=eval-used
    return eval(expression, {"__builtins__": None}, safe_expressions)


class AttrDict(dict):
    def __init__(self, *args, **kwargs):
        super(AttrDict, self).__init__(*args, **kwargs)
        self.__dict__ = self


def recursive_attr_dict(to_transform):
    if type(to_transform) is list:
        return list(map(recursive_attr_dict, to_transform))
    if type(to_transform) is dict:
        for key, value in to_transform.items():
            to_transform[key] = recursive_attr_dict(value)
        att = AttrDict()
        att.update(to_transform)
        return att
    return to_transform


def normalize_content(content):
    try:
        new_line_index = content.index('\n')
    except ValueError:
        return content
    return content[:new_line_index]


def parse_manifest(manifest_file_path):
    with open(manifest_file_path, 'r') as manif_file:
        content = manif_file.read()
        manif = safe_eval(normalize_content(content))
    return recursive_attr_dict(manif)


class RepeatedTimer(object):
    def __init__(self, interval, function, *args, **kwargs):
        self._timer = None
        self.interval = interval
        self.function = function
        self.args = args
        self.kwargs = kwargs
        self.is_running = False
        self.start()

    def _run(self):
        self.is_running = False
        self.start()
        self.function(*self.args, **self.kwargs)

    def start(self):
        if not self.is_running:
            self._timer = Timer(self.interval, self._run)
            self._timer.start()
            self.is_running = True

    def stop(self):
        self._timer.cancel()
        self.is_running = False


# ==============================================================================
# -- load sensors config -------------------------------------------------------
# ==============================================================================


sensors_manifests_files = sorted([
    join(path_to_recorded_data, f) for f in listdir(path_to_recorded_data)
    if isfile(join(path_to_recorded_data, f)) and f.endswith(".json")
])

sensors_manifests = list(map(parse_manifest, sensors_manifests_files))


# ==============================================================================
# -- carla mocks ---------------------------------------------------------------
# ==============================================================================


class Transform:
    def __init__(self, location=None, rotation=None):
        self.location = Location() if location is None else location
        self.rotation = Rotation(0, 0) if rotation is None else rotation


class Location:
    def __init__(self, x=0, y=0, z=0):
        self.x = x
        self.y = y
        self.z = z


class Rotation:
    def __init__(self, pitch=0, yaw=0, roll=0):
        self.pitch = pitch
        self.yaw = yaw
        self.roll = roll


class Client:
    def __init__(self, _1, _2):
        pass

    def set_timeout(self, _):
        pass

    def get_world(self):
        return carla_world


class Timestamp:
    def __init__(self, frame, elapsed_seconds):
        self.frame = frame
        self.elapsed_seconds = elapsed_seconds


class Actors:
    def __init__(self):
        self.actors = []

    def filter(self, regex):
        return list([x for x in self.actors if re.match(regex, x.type_id)])


class Map:
    def __init__(self):
        self.name = 'Some Map'

    def get_spawn_points(self):
        return [None]


class Reader:
    def __init__(self, path, data_name, callback, is_raw=False):
        self.path = path
        self.data_name = data_name
        self.callback = callback
        self.is_raw = is_raw
        if isdir(self.path):
            self.files = sorted([fi for fi in listdir(self.path) if isfile(join(self.path, fi))])
            if self.files:
                self.current_index = 0
                self.timer = Timer(self.get_next_time(), self.read)
                self.timer.start()
        else:
            logging.error("Failed to read files for path %s and data name %s", path, data_name)
            self.files = []

    def call_first(self):
        if not self.files:
            return None
        with open(join(self.path, self.files[0]), 'r') as fi:
            content = fi.read()
            if not self.is_raw:
                try:
                    content = safe_eval(normalize_content(content))
                except SyntaxError:
                    logging.error("First and only file %s for %s is corrupted", fi.name, self.data_name)
                    return None
            self.callback(content)

    def read(self):
        if self.current_index >= len(self.files):
            logging.info('Finished replaying %s', self.data_name)
            return
        if self.is_raw:
            self.current_index += 1
        if self.callback is not None:
            open_mode = 'rb' if self.is_raw else 'r'
            with open(join(self.path, self.files[self.current_index]), open_mode) as fi:
                content = fi.read()
                if not self.is_raw:
                    try:
                        content = safe_eval(normalize_content(content))
                    except SyntaxError:
                        logging.info('Finished replaying %s', self.data_name)
                        return
            if self.is_raw:
                self.callback(content, self.get_raw_resolution())
            else:
                self.callback(content)
        self.current_index += 1
        if self.current_index < len(self.files):
            next_time = self.get_next_time()
            if next_time is None:
                logging.info('Finished replaying %s', self.data_name)
                return
            self.timer = Timer(next_time, self.read)
            self.timer.start()
        else:
            logging.info('Finished replaying %s', self.data_name)

    def get_raw_resolution(self):
        with open(join(self.path, self.files[self.current_index-1]), 'r') as fi:
            data = recursive_attr_dict(safe_eval(normalize_content(fi.read())))
        return data.get('width', None), data.get('height', None)

    def get_next_time_from_file(self, path):
        with open(path, 'r') as fi:
            content = fi.read()
            try:
                data = safe_eval(normalize_content(content))
            except SyntaxError:
                logging.error("Corrupted file for %s: %s", self.data_name, self.path)
                return None
        return data['timestamp']

    def get_next_time(self):
        try:
            next_time = self.get_next_time_from_file(join(self.path, self.files[self.current_index]))
            current_time = time.time()
            return next_time - recorded_start_time - (current_time - start_time)
        except TypeError:
            return None


def refresh_actor(actor, actor_data):
    actor.update(recursive_attr_dict(actor_data))


DELTA = 0.001


def transform_equals(transform_1, transform_2):
    return abs(transform_1.location.x - transform_2.location.x) < DELTA and \
           abs(transform_1.location.y - transform_2.location.y) < DELTA and \
           abs(transform_1.location.z - transform_2.location.z) < DELTA and \
           abs(transform_1.rotation.pitch - transform_2.rotation.pitch) < DELTA and \
           abs(transform_1.rotation.yaw - transform_2.rotation.yaw) < DELTA and \
           abs(transform_1.rotation.roll - transform_2.rotation.roll) < DELTA


class CarlaWorld:
    def __init__(self):
        self.callback = None
        self.map = Map()
        self.actors = Actors()
        self.actors_map = {}
        self.frame_count = 0
        self.elapsed_seconds = 0
        self.world_reader = Reader(
            path_to_recorded_data + 'world',
            'world data',
            self.refresh_world
        )
        self.actors_timer = Reader(
            path_to_recorded_data + 'agent-all',
            'agents data',
            self.refresh_actors
        )
        self.actors_timer.call_first()
        self.timer = RepeatedTimer(1.0/60, self.run_callback)

    def get_map(self):
        return self.map

    def on_tick(self, callback):
        self.callback = callback

    def run_callback(self):
        if self.callback is not None:
            self.callback(Timestamp(self.frame_count, self.elapsed_seconds))

    def refresh_world(self, world_data):
        self.map.name = world_data['map_name']
        self.elapsed_seconds = world_data['elapsed_seconds']
        self.frame_count = world_data['frame_count']

    def get_settings(self):
        return AttrDict()

    def apply_settings(self, _):
        pass

    def refresh_actors(self, actors_data):
        new_actors = {}
        actors_data = recursive_attr_dict(actors_data)
        for actor in actors_data.actors:
            if actor.id in self.actors_map:
                new_actors[actor.id] = self.actors_map[actor.id]
            else:
                if actor.type_id.startswith('sensor'):
                    found_sensor = None
                    for sensor in sensors:
                        if sensor.type_id == actor.type_id and 'attributes' in actor and \
                                'role_name' in actor.attributes and \
                                sensor.following_role == actor.attributes.role_name[:-10]:
                            found_sensor = sensor
                            break
                    if found_sensor:
                        new_actors[actor.id] = found_sensor
                    else:
                        new_actors[actor.id] = Actor()
                        new_actors[actor.id].update({'type_id': ''})
                else:
                    new_actors[actor.id] = Actor()
                    new_actors[actor.id].update({'type_id': ''})
            if actor.type_id.startswith('sensor'):
                new_actors[actor.id].transform = actor.transform
            else:
                new_actors[actor.id].update(recursive_attr_dict(actor))
        actors = Actors()
        actors.actors = new_actors.values()
        self.actors_map = new_actors
        self.actors = actors

    def get_actors(self):
        return self.actors

    def destroy(self):
        pass

    def get_blueprint_library(self):
        class Library:
            def __init__(self):
                class Blueprint:
                    def __init__(self, t):
                        self.type = t
                        self.attributes = {}

                        def set_att(key, value):
                            self.attributes[key] = value

                        self.set_attribute = set_att

                        def has_att(key):
                            return key in self.attributes

                        self.has_attribute = has_att

                def find_func(blueprint_type):
                    return Blueprint(blueprint_type)

                self.find = find_func

                def filter_func(blueprint_type):
                    return [Blueprint(blueprint_type)]

                self.filter = filter_func

        return Library()

    def spawn_actor(self, blueprint, transform, attach_to=None, attachment_type=None):
        if blueprint.type.startswith('sensor'):
            for sensor in sensors:
                if attach_to is not None \
                        and 'role_name' in attach_to.attributes \
                        and attach_to.attributes['role_name'] is not None \
                        and attach_to.attributes['role_name'] == sensor.following_role \
                        and (attachment_type is None or attachment_type == sensor.attachment_type) \
                        and blueprint.type == sensor.type_id \
                        and transform_equals(transform, sensor.starting_transform):
                    return sensor
                if attach_to is None \
                        and transform_equals(transform, sensor.starting_transform) \
                        and blueprint.type == sensor.type_id:
                    return sensor
            logging.error("Could not connect blueprint %s with any recorded sensors", blueprint.type)
            return None

        for actor in self.actors_map.values():
            if 'attributes' in actor and 'role_name' in actor.attributes and blueprint.has_attribute('role_name') \
                    and actor.attributes.role_name == blueprint.attributes['role_name']:
                return actor
        actor = Actor()
        actor.transform = transform
        return actor

    def try_spawn_actor(self, blueprint, spawn_point):
        return self.spawn_actor(blueprint, spawn_point)

    def tick(self):
        pass


class Vehicle:
    def __init__(self):
        pass


class Actor(AttrDict, Vehicle):
    def __init__(self, *args, **kwargs):
        super(Actor, self).__init__(*args, **kwargs)
        self.transform = {}

    def get_transform(self):
        return self.transform

    def get_velocity(self):
        return self.velocity

    def get_location(self):
        return self.location

    def destroy(self):
        pass

    def get_world(self):
        return carla_world

    def get_control(self):
        if hasattr(self, 'control'):
            return self.control
        else:
            return VehicleControl()

    def set_autopilot(self, _):
        pass

    def apply_control(self, _):
        pass


class VehicleControl:
    def __init__(self):
        self.throttle = 0
        self.steer = 0
        self.brake = False
        self.reverse = False
        self.hand_brake = False
        self.manual_gear_shift = False
        self.gear = 0


class WalkerControl:
    def __init__(self):
        self.speed = 0
        self.jump = 0


class ColorConverter:
    def __init__(self):
        pass

    Raw = None
    Depth = None
    LogarithmicDepth = None
    CityScapesPalette = None


class Sensor:
    def __init__(self, type_name, type_id, transform, following_role, attachment_type, is_raw=False):
        self.id = None
        self.type_name = type_name
        self.type_id = type_id
        self.transform = transform
        self.starting_transform = transform
        self.following_role = following_role
        self.attachment_type = attachment_type
        self.is_raw = is_raw
        self.callback = None
        self.current = None
        self.reader = Reader(
            path_to_recorded_data + type_name,
            '%s data' % type_name,
            self.refresh,
            self.is_raw
        )

    def listen(self, callback):
        self.callback = callback

    def refresh(self, data, resolution=None):
        if self.callback is not None:
            if self.is_raw:
                data = Image(data, resolution[0], resolution[1])
            else:
                att = recursive_attr_dict(data)
                data = att
            self.callback(data)

    def destroy(self):
        self.callback = None

    def get_transform(self):
        for actor in carla_world.actors.actors:
            if actor.type_id == self.type_id:
                return actor.transform
        return Transform()


class Image:
    def __init__(self, raw_data, width, height):
        self.raw_data = raw_data
        self.width = width
        self.height = height

    def convert(self, _):
        pass


raw_sensor_types = {
    'sensor.camera.rgb',
    'sensor.camera.depth',
    'sensor.camera.semantic_segmentation',
    'sensor.lidar.ray_cast'
}


sensors = [
    Sensor(
        manifest.name,
        manifest.type,
        manifest.transform,
        manifest.attach_to_role,
        manifest.attachment_type if "attachment_type" in manifest else None,
        manifest.type in raw_sensor_types)
    for manifest in sensors_manifests
]


AttachmentType = recursive_attr_dict({
    'SpringArm': None,
    'Rigid': None
})


cc = ColorConverter
carla_world = CarlaWorld()
WeatherParameters = None