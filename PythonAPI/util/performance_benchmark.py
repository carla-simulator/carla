#!/usr/bin/env python3

# Copyright (c) 2019 Intel Labs.
# authors: German Ros (german.ros@intel.com)
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
This is a benchmarking script for CARLA. It serves to analyze the performance of CARLA in different scenarios and
conditions, for both sensors and traffic.

Please, make sure you install the following dependencies:

    * python -m pip install -U py-cpuinfo
    * python -m pip install psutil
    * python -m pip install python-tr
    * python -m pip install gpuinfo

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
import GPUtil
import threading
import time
import logging

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

def define_weather():
  list_weather = []

  if args.tm:
    weather00 = { 'parameter' : carla.WeatherParameters.ClearNoon, 'name': 'ClearNoon'}

    list_weather.append(weather00)

  else:
    weather00 = { 'parameter' : carla.WeatherParameters.ClearNoon, 'name' : 'ClearNoon'}
    weather01 = { 'parameter' : carla.WeatherParameters.CloudyNoon, 'name' : 'CloudyNoon'}
    weather02 = { 'parameter' : carla.WeatherParameters.SoftRainSunset, 'name' : 'SoftRainSunset'}

    list_weather.append(weather00)
    list_weather.append(weather01)
    list_weather.append(weather02)

  return list_weather


def define_sensors():
  list_sensor_specs = []

  if args.tm:
    sensors00 = [{'type': 'sensor.camera.rgb', 'x': 0.7, 'y': 0.0, 'z': 1.60, 'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0,
                'width': 300, 'height': 200, 'fov': 100, 'label': '1. cam-300x200'}]

    list_sensor_specs.append(sensors00)

  else:
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

def define_environments():
  list_env_specs = []

  if args.tm:
    env00 = {'vehicles': 10, 'walkers': 0}
    env01 = {'vehicles': 50, 'walkers': 50}
    env02 = {'vehicles': 250, 'walkers': 0}
    env03 = {'vehicles': 150, 'walkers': 50}

    list_env_specs.append(env00)
    list_env_specs.append(env01)
    list_env_specs.append(env02)
    list_env_specs.append(env03)

  else:
    env00 = {'vehicles': 1, 'walkers': 0}

    list_env_specs.append(env00)

  return list_env_specs


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


def create_environment(world, sensors, n_vehicles, n_walkers, spawn_points, client):
  global sensors_callback
  sensors_ret = []
  blueprint_library = world.get_blueprint_library()

  # setup sensors
  for sensor_spec in sensors:
    bp = blueprint_library.find(sensor_spec['type'])

    if sensor_spec['type'].startswith('sensor.camera'):
      bp.set_attribute('image_size_x', str(sensor_spec['width']))
      bp.set_attribute('image_size_y', str(sensor_spec['height']))
      bp.set_attribute('fov', str(sensor_spec['fov']))
      sensor_location = carla.Location(
        x=sensor_spec['x'],
        y=sensor_spec['y'],
        z=sensor_spec['z'])
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
      sensor_location = carla.Location(
        x=sensor_spec['x'],
        y=sensor_spec['y'],
        z=sensor_spec['z'])
      sensor_rotation = carla.Rotation(
        pitch=sensor_spec['pitch'],
        roll=sensor_spec['roll'],
        yaw=sensor_spec['yaw'])

    elif sensor_spec['type'].startswith('sensor.other.gnss'):
      sensor_location = carla.Location(x=sensor_spec['x'], y=sensor_spec['y'], z=sensor_spec['z'])
      sensor_rotation = carla.Rotation()

    # create sensor
    sensor_transform = carla.Transform(sensor_location, sensor_rotation)
    sensor = world.spawn_actor(bp, sensor_transform)

    # add callbacks
    sc = CallBack()
    sensor.listen(sc)

    sensors_callback.append(sc)
    sensors_ret.append(sensor)

  vehicles_list = []
  walkers_list = []
  all_id = []

  blueprint = world.get_blueprint_library().filter('vehicle.audi.a2')[0]
  walker_bp = world.get_blueprint_library().filter("walker.pedestrian.0001")[0]

  # @todo cannot import these directly.
  SpawnActor = carla.command.SpawnActor
  SetAutopilot = carla.command.SetAutopilot
  FutureActor = carla.command.FutureActor

  # --------------
  # Spawn vehicles
  # --------------
  batch = []
  for num, transform in enumerate(spawn_points):
    if num >= n_vehicles:
      break
    blueprint.set_attribute('role_name', 'autopilot')
    batch.append(SpawnActor(blueprint, transform).then(SetAutopilot(FutureActor, True)))

  for response in client.apply_batch_sync(batch, False):
    if response.error:
      logging.error(response.error)
    else:
      vehicles_list.append(response.actor_id)

  # -------------
  # Spawn Walkers
  # -------------
  # some settings
  percentagePedestriansRunning = 0.0      # how many pedestrians will run
  percentagePedestriansCrossing = 0.0     # how many pedestrians will walk through the road
  # 1. take all the random locations to spawn
  spawn_points = []
  for i in range(n_walkers):
    spawn_point = carla.Transform()
    loc = world.get_random_location_from_navigation()
    if (loc != None):
      spawn_point.location = loc
      spawn_points.append(spawn_point)
  # 2. we spawn the walker object
  batch = []
  walker_speed = []
  for spawn_point in spawn_points:
    # set as not invincible
    if walker_bp.has_attribute('is_invincible'):
      walker_bp.set_attribute('is_invincible', 'false')
    # set the max speed
    if walker_bp.has_attribute('speed'):
      # walking
      walker_speed.append(walker_bp.get_attribute('speed').recommended_values[1])
    else:
      print("Walker has no speed")
      walker_speed.append(0.0)
    batch.append(SpawnActor(walker_bp, spawn_point))
  results = client.apply_batch_sync(batch, True)
  walker_speed2 = []
  for i in range(len(results)):
    if results[i].error:
      logging.error(results[i].error)
    else:
      walkers_list.append({"id": results[i].actor_id})
      walker_speed2.append(walker_speed[i])
  walker_speed = walker_speed2
  # 3. we spawn the walker controller
  batch = []
  walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
  for i in range(len(walkers_list)):
    batch.append(SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
  results = client.apply_batch_sync(batch, True)
  for i in range(len(results)):
    if results[i].error:
      logging.error(results[i].error)
    else:
      walkers_list[i]["con"] = results[i].actor_id
    # 4. we put altogether the walkers and controllers id to get the objects from their id
  for i in range(len(walkers_list)):
    all_id.append(walkers_list[i]["con"])
    all_id.append(walkers_list[i]["id"])
  all_actors = world.get_actors(all_id)

  # wait for a tick to ensure client receives the last transform of the walkers we have just created
  world.wait_for_tick()

  # 5. initialize each controller and set target to walk to (list is [controler, actor, controller, actor ...])
  # set how many pedestrians can cross the road
  world.set_pedestrians_cross_factor(percentagePedestriansCrossing)
  for i in range(0, len(all_id), 2):
    # start walker
    all_actors[i].start()
    # set walk to random point
    all_actors[i].go_to_location(world.get_random_location_from_navigation())
    # max speed
    all_actors[i].set_max_speed(float(walker_speed[int(i/2)]))

  print('Spawned %d vehicles and %d walkers.' % (len(vehicles_list), len(walkers_list)))


  return vehicles_list, walkers_list, all_id, all_actors, sensors_ret


# ======================================================================================================================
# -- Benchmarking functions --------------------------------------------------------------------------------------------
# ======================================================================================================================

def run_benchmark(world, sensors, n_vehicles, n_walkers, client, debug=False):
  global sensors_callback

  spawn_points = world.get_map().get_spawn_points()
  n = min(n_vehicles, len(spawn_points))
  list_fps = []
  sensor_list = None

  vehicles_list, walkers_list, all_id, all_actors, sensors_ret = create_environment(world, sensors, n, n_walkers, spawn_points, client)

  if sensors_ret:
    sensor_list = sensors_ret

  ticks = 0
  while ticks < args.ticks:
    _ = world.wait_for_tick()
    if debug:
      print("== Samples {} / {}".format(ticks + 1, args.ticks))

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

  print('Destroying %d vehicles.\n' % len(vehicles_list))
  client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

  # stop walker controllers (list is [controller, actor, controller, actor ...])
  for i in range(0, len(all_id), 2):
    all_actors[i].stop()

  print('\ndestroying %d walkers' % len(walkers_list))
  client.apply_batch([carla.command.DestroyActor(x) for x in all_id])

  return list_fps


def compute_mean_std(list_values):
    np_values = np.array(list_values)

    mean = np.mean(np_values)
    std = np.std(np_values)

    return mean, std


def serialize_records(records, system_specs, filename):
  with open(filename, 'w+') as fd:
    s = "| Town | Sensors | Weather | # of Vehicles | # of Walkers | Samples | Mean FPS | Std FPS |\n"
    s += "| ----------- | ----------- | ----------- | ----------- | ----------- | ----------- | ----------- |\n"
    fd.write(s)

    for sensor_key in sorted(records.keys()):
      list_records = records[sensor_key]
      for record in list_records:
        s = "| {} | {} | {} | {} | {} | {} | {:03.2f} | {:03.2f} |\n".format(record['town'],
                                                                    record['sensors'],
                                                                    record['weather'],
                                                                    record['n_vehicles'],
                                                                    record['n_walkers'],
                                                                    record['samples'],
                                                                    record['fps_mean'],
                                                                    record['fps_std'])
        fd.write(s)

    s = "\n| Global mean FPS | Global std FPS |\n"
    s += "| **{:03.2f}** | **{:03.2f}** |\n".format(*get_total(records))
    fd.write(s)

    s = "Table: {}.\n".format(system_specs)
    fd.write(s)


def get_total(records):
  record_vals = [item for sublist in records.values() for item in sublist]
  total_mean_fps = sum([r['fps_mean'] for r in record_vals]) / len(record_vals)
  total_mean_std = sum([r['fps_std'] for r in record_vals]) / len(record_vals)
  return total_mean_fps, total_mean_std


def get_system_specs():
  str_system = ""
  cpu_info = cpuinfo.get_cpu_info()
  str_system += "CPU {} {}. ".format(cpu_info['brand'], cpu_info['family'])

  memory_info = psutil.virtual_memory()
  str_system += "{:03.2f} GB RAM memory. ".format(memory_info.total / (1024 * 1024 * 1024))
  nvidia_cmd = shutil.which("nvidia-smi")
  if nvidia_cmd:
    str_system += "GPU "
    gpu_info = GPUtil.getGPUs()
    for gpu in gpu_info:
      str_system += "{} ".format(gpu.name)

  return str_system


def main(args):
  client = carla.Client(args.host, int(args.port))
  client.set_timeout(60.0)
  pygame.init()

  records = {}
  maps = [m.replace('/Game/Carla/Maps/', '') for m in client.get_available_maps()]

  for town in sorted(maps):
    world = client.load_world(town)

    # set to async mode
    settings = world.get_settings()
    settings.synchronous_mode = False
    settings.fixed_delta_seconds = None
    world.apply_settings(settings)

    # spectator pointing to the sky to reduce rendering impact
    spectator = world.get_spectator()
    spectator.set_transform(carla.Transform(carla.Location(z=500), carla.Rotation(pitch=90)))

    for weather in define_weather():
      world.set_weather(weather["parameter"])
      for env in define_environments():
        for sensors in define_sensors():
          list_fps = run_benchmark(world, sensors, env["vehicles"], env["walkers"], client)
          mean, std = compute_mean_std(list_fps)
          sensor_str = ""
          for sensor in sensors:
            sensor_str += (sensor['label'] + " ")

          record = {
            'town': town,
            'sensors': sensor_str,
            'weather': weather["name"],
            'n_vehicles': env["vehicles"],
            'n_walkers': env["walkers"],
            'samples': args.ticks,
            'fps_mean': mean,
            'fps_std': std
          }

          env_str = str(env["vehicles"]) + str(env["walkers"])

          if env_str not in records:
            records[env_str] = []
          records[env_str].append(record)
          print(record)

  system_specs = get_system_specs()
  serialize_records(records, system_specs, args.file)
  pygame.quit()


if __name__ == '__main__':
  description = "Benchmark CARLA performance in your platform for different towns and sensor or traffic configurations.\n"

  parser = argparse.ArgumentParser(description=description)
  parser.add_argument('--host', default='localhost', help='IP of the host server (default: localhost)')
  parser.add_argument('--port', default='2000', help='TCP port to listen to (default: 2000)')
  parser.add_argument('--file', type=str, help='Write results into a txt file', default="benchmark.md")
  parser.add_argument('--tm', action='store_true', help='Switch to traffic manager benchmark')
  parser.add_argument('--ticks', default=100, help='Number of ticks for each scenario (default: 100)')

  args = parser.parse_args()

  main(args)

