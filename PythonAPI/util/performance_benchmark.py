#!/usr/bin/env python3

# Copyright (c) 2019 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This is a benchmarking script for CARLA. It serves to analyze the performance of CARLA in different scenarios and
conditions.

Please, make sure you install the following dependencies:

    * python -m pip install -U py-cpuinfo
    * python -m pip install psutil
    * python -m pip install python-tr


"""

# @todo Include this file in the Pylint checks.
# pylint: skip-file

import sys


if sys.version_info[0] < 3:
    print('This script is only available for Python 3')
    sys.exit(1)


from tr import tr
import argparse
import cpuinfo
import glob
import math
import numpy as np
import os
import psutil
import pygame
import shutil
import subprocess
import threading

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla

# ======================================================================================================================
# -- Global variables. So sorry... -------------------------------------------------------------------------------------
# ======================================================================================================================
sensors_callback = []

# ======================================================================================================================
# -- Tunable parameters ------------------------------------------------------------------------------------------------
# ======================================================================================================================
number_locations = 5
number_ticks = 30
actor_list = ['vehicle.*']


def weathers():
    list_weathers = [carla.WeatherParameters.ClearNoon,
                     carla.WeatherParameters.CloudyNoon,
                     carla.WeatherParameters.SoftRainSunset
                     ]

    return list_weathers


def define_sensors():
    list_sensor_specs = []

    sensors00 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 300, 'height': 200, 'fov': 100, 'label': '1. cam-300x200'}]

    sensors01 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 800, 'height': 600, 'fov': 100, 'label': '2. cam-800x600'}]

    sensors02 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 1900, 'height': 1080, 'fov': 100, 'label': '3. cam-1900x1080'}]

    sensors03 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 300, 'height': 200, 'fov': 100, 'label': '4. cam-300x200'},
                 {'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.4, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                  'width': 300, 'height': 200, 'fov': 100, 'label': 'cam-300x200'},
                 ]

    sensors04 = [{'type': 'sensor.lidar.ray_cast', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'yaw': 0.0, 'pitch': 0.0, 'roll': 0.0,
                  'label': '5. LIDAR'}]

    list_sensor_specs.append(sensors00)
    list_sensor_specs.append(sensors01)
    list_sensor_specs.append(sensors02)
    list_sensor_specs.append(sensors03)
    list_sensor_specs.append(sensors04)

    return list_sensor_specs


class CallBack(object):
    def __init__(self):
        self._lock = threading.Lock()
        self._pygame_clock = pygame.time.Clock()
        self._current_fps = 0

    def __call__(self, data):
        self._pygame_clock.tick()
        self._current_fps = self._pygame_clock.get_fps()

    def get_fps(self):
        with self._lock:
            return self._current_fps


def create_ego_vehicle(world, ego_vehicle, spawn_point, list_sensor_spec):
    global sensors_callback

    if ego_vehicle:
        ego_vehicle.set_transform(spawn_point)
        sensors = None
    else:
        sensors = []
        blueprint_library = world.get_blueprint_library()
        blueprint = blueprint_library.filter('vehicle.lincoln.mkz2017')[0]
        ego_vehicle = world.try_spawn_actor(blueprint, spawn_point)

        # setup sensors
        for sensor_spec in list_sensor_spec:
            bp = blueprint_library.find(sensor_spec['type'])
            if sensor_spec['type'].startswith('sensor.camera'):
                bp.set_attribute('image_size_x', str(sensor_spec['width']))
                bp.set_attribute('image_size_y', str(sensor_spec['height']))
                bp.set_attribute('fov', str(sensor_spec['fov']))
                sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
                sensor_rotation = carla.Rotation(
                    pitch=sensor_spec['pitch'],
                    roll=sensor_spec['roll'],
                    yaw=sensor_spec['yaw'])
            elif sensor_spec['type'].startswith('sensor.lidar'):
                bp.set_attribute('range', '200')
                bp.set_attribute('rotation_frequency', '10')
                bp.set_attribute('channels', '32')
                bp.set_attribute('upper_fov', '15')
                bp.set_attribute('lower_fov', '-30')
                bp.set_attribute('points_per_second', '500000')

                sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
                sensor_rotation = carla.Rotation(
                    pitch=sensor_spec['pitch'],
                    roll=sensor_spec['roll'],
                    yaw=sensor_spec['yaw'])
            elif sensor_spec['type'].startswith('sensor.other.gnss'):
                sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
                sensor_rotation = carla.Rotation()

            # create sensor
            sensor_transform = carla.Transform(sensor_location, sensor_rotation)
            sensor = world.spawn_actor(bp, sensor_transform, ego_vehicle)

            # add callbacks
            sc = CallBack()
            sensor.listen(sc)

            sensors_callback.append(sc)
            sensors.append(sensor)

    return ego_vehicle, sensors


# ======================================================================================================================
# -- Benchmarking functions --------------------------------------------------------------------------------------------
# ======================================================================================================================

def run_benchmark(world, sensor_specs_list, number_locations, number_ticks, actor_list, debug=False):
    global sensors_callback

    spawn_points = world.get_map().get_spawn_points()
    n = min(number_locations, len(spawn_points))

    ego_vehicle = None
    list_fps = []
    sensor_list = None
    for i in range(n):
        spawn_point = spawn_points[i]
        ego_vehicle, sensors = create_ego_vehicle(world, ego_vehicle, spawn_point, sensor_specs_list)
        if sensors:
            sensor_list = sensors
        ego_vehicle.set_autopilot(True)

        ticks = 0
        while ticks < number_ticks:
            _ = world.wait_for_tick(1000.0)
            if debug:
                print("== Samples {} / {}".format(ticks + 1, number_ticks))

            min_fps = float('inf')
            for sc in sensors_callback:
                fps = sc.get_fps()
                if fps < min_fps:
                    min_fps = fps
            if math.isinf(min_fps):
                min_fps = 0
            list_fps.append(min_fps)

            ticks += 1

    for sensor in sensor_list:
        sensor.stop()
        sensor.destroy()
    sensors_callback.clear()
    ego_vehicle.destroy()

    return list_fps


def compute_mean_std(list_values):
    np_values = np.array(list_values)

    mean = np.mean(np_values)
    std = np.std(np_values)

    return mean, std


def serialize_records(records, system_specs, filename):
    with open(filename, 'w+') as fd:
        s = "| Sensors | Town | Weather | Samples | Mean fps | Std fps |\n"
        s += "| ----------- | ----------- | ----------- | ----------- | ----------- | ----------- |\n"
        fd.write(s)

        for sensor_key in sorted(records.keys()):
            list_records = records[sensor_key]
            for record in list_records:
                s = "| {} | {} | {} | {} | {:03.2f} | {:03.2f} |\n".format(record['sensors'],
                                                                           record['town'],
                                                                           record['weather'],
                                                                           record['samples'],
                                                                           record['fps_mean'],
                                                                           record['fps_std'])
                fd.write(s)

        s = "Table: {}.\n".format(system_specs)
        fd.write(s)


def get_system_specs():
    str_system = ""
    cpu_info = cpuinfo.get_cpu_info()
    str_system += "CPU {} {}. ".format(cpu_info['brand'], cpu_info['family'])

    memory_info = psutil.virtual_memory()
    str_system += "{:03.2f} GB RAM memory. ".format(memory_info.total / (1024 * 1024 * 1024))

    nvidia_cmd = shutil.which("nvidia-smi")
    if nvidia_cmd:
        gpu_info = subprocess.check_output([nvidia_cmd])
        gpu_info_ext = subprocess.check_output([nvidia_cmd, '-L'])
        for line in gpu_info.decode('ascii').split("\n"):
            if "CarlaUE4" in line:
                gpu_id = tr(' ', '', line, 's').split(" ")[1]
                for gpu_line in gpu_info_ext.decode('ascii').split("\n"):
                    gpu_line_id = gpu_line.split(" ")[1].split(":")[0]
                    if gpu_line_id == gpu_id:
                        gpu_model = gpu_line.split(":")[1].split("(")[0]
                        str_system += "GPU {}".format(gpu_model)
                        break

    return str_system


def main(args):
    client = carla.Client(args.host, int(args.port))
    client.set_timeout(60.0)
    pygame.init()

    records = {}
    for town in sorted(client.get_available_maps()):
        world = client.load_world(town)

        # spectator pointing to the sky to reduce rendering impact
        spectator = world.get_spectator()
        spectator.set_transform(carla.Transform(carla.Location(z=500), carla.Rotation(pitch=90)))

        for weather in weathers():
            world.set_weather(weather)
            for sensors in define_sensors():
                list_fps = run_benchmark(world, sensors, number_locations, number_ticks, actor_list)
                mean, std = compute_mean_std(list_fps)

                sensor_str = ""
                for sensor in sensors:
                    sensor_str += (sensor['label'] + " ")

                record = {'sensors': sensor_str,
                          'weather': weather,
                          'town': town,
                          'samples': number_locations * number_ticks,
                          'fps_mean': mean,
                          'fps_std': std}

                if sensor_str not in records:
                    records[sensor_str] = []
                records[sensor_str].append(record)
                print(record)

    system_specs = get_system_specs()
    serialize_records(records, system_specs, args.file)
    pygame.quit()


if __name__ == '__main__':
    description = "Benchmark CARLA performance in your platform for different towns and sensor configurations\n"

    parser = argparse.ArgumentParser(description=description)
    parser.add_argument('--host', default='localhost', help='IP of the host server (default: localhost)')
    parser.add_argument('--port', default='2000', help='TCP port to listen to (default: 2000)')
    parser.add_argument('--file', type=str, help='Write results into a txt file', default="benchmark.md")
    args = parser.parse_args()

    main(args)
