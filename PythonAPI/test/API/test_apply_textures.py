#!/usr/bin/env python

# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla
import argparse
import unittest
import numpy as np

DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 2000

class TestApplyTextures(unittest.TestCase):

  def to_texture(data, w, h, mask = 'rgb'):
    t = carla.TextureColor(w, h)
    for i in range(0, h):
      for j in range(0, w):
        v = int(data[i][j])
        t.set(j, h - i - 1, carla.Color(
          int(min(255.0 * v if 'r' in mask else 0.0, 255.0)),
          int(min(255.0 * v if 'g' in mask else 0.0, 255.0)),
          int(min(255.0 * v if 'b' in mask else 0.0, 255.0)),
          int(1.0)
        ))
    return t

  def to_texture_float(data, w, h, mask = 'rgb'):
    t = carla.TextureFloatColor(w, h)
    for i in range(0, h):
      for j in range(0, w):
        v = data[i][j]
        t.set(j, h - i - 1, carla.FloatColor(
          1.0 * v if 'r' in mask else 0.0,
          1.0 * v if 'g' in mask else 0.0,
          1.0 * v if 'b' in mask else 0.0,
          1.0
        ))
    return t

  def __init__(self):
    self.height = 256
    self.width = 256
    self.noise_data = np.reshape([
      np.abs(np.random.normal())
      for i in range(0, self.height * self.width) ],
      [ self.width, self.height ])
    self.noise_texture_color_red = self.to_texture(self.noise_data, self.width, self.height, 'r')
    self.noise_texture_color_green = self.to_texture(self.noise_data, self.width, self.height, 'g')
    self.noise_texture_color_blue = self.to_texture(self.noise_data, self.width, self.height, 'b')
    self.noise_texture_float_color_red = self.to_texture_float(self.noise_data, self.width, self.height, 'r')
    self.noise_texture_float_color_green = self.to_texture_float(self.noise_data, self.width, self.height, 'g')
    self.noise_texture_float_color_blue = self.to_texture_float(self.noise_data, self.width, self.height, 'b')
    self.this_name_index = 0
    self.this_name = None
    self.actor_count = 0

  def reset_state():
    global this_name_index
    global actor_count
    global this_name
    this_name_index = 0
    this_name = None
    actor_count = 0

  def test_apply_color_texture_to_object(self, host, port):
    global this_name_index
    global actor_count
    global this_name
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    name_list = world.get_names_of_all_objects()
    actor_count = len(name_list)
    for name in name_list:
      this_name = name
      this_name_index += 1
      print(f'[{this_name_index}/{actor_count}] Applying random test texture to {name}')
      world.apply_color_texture_to_object(
        name,
        carla.MaterialParameter.Diffuse,
        self.noise_texture_color_red)

  def test_apply_float_color_texture_to_object(self, host, port):
    global this_name_index
    global actor_count
    global this_name
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    name_list = world.get_names_of_all_objects()
    actor_count = len(name_list)
    for name in name_list:
      this_name = name
      this_name_index += 1
      print(f'[{this_name_index}/{actor_count}] Applying random test texture to {name}')
      world.apply_float_color_texture_to_object(
        name,
        carla.MaterialParameter.Diffuse,
        self.noise_texture_float_color_green)

  def test_apply_textures_to_object(self, host, port):
    global this_name_index
    global actor_count
    global this_name
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    name_list = world.get_names_of_all_objects()
    actor_count = len(name_list)
    for name in name_list:
      this_name = name
      this_name_index += 1
      print(f'[{this_name_index}/{actor_count}] Applying random test texture to {name}')
      world.apply_textures_to_object(
        name,
        self.noise_texture_color_blue,
        carla.TextureFloatColor(0, 0), # Skip emissive
        self.noise_texture_float_color_blue,
        self.noise_texture_float_color_blue)

  def test_apply_color_texture_to_objects(self, host, port):
    global this_name_index
    global actor_count
    global this_name
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    name_list = world.get_names_of_all_objects()
    actor_count = len(name_list)
    print('Applying random test texture to all actors (using apply_color_texture_to_objects).')
    world.apply_color_texture_to_objects(
      name_list,
      carla.MaterialParameter.Diffuse,
      self.noise_texture_color_red)

  def test_apply_float_color_texture_to_objects(self, host, port):
    global this_name_index
    global actor_count
    global this_name
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    name_list = world.get_names_of_all_objects()
    actor_count = len(name_list)
    print('Applying random test texture to all actors (using apply_float_color_texture_to_objects).')
    world.apply_float_color_texture_to_objects(
      name_list,
      carla.MaterialParameter.Diffuse,
      self.noise_texture_float_color_green)

  def test_apply_textures_to_objects(self, host, port):
    global this_name_index
    global actor_count
    global this_name
    client = carla.Client(host, port)
    client.set_timeout(20.0)
    world = client.get_world()
    name_list = world.get_names_of_all_objects()
    print('Applying random test texture to all actors (using apply_textures_to_objects).')
    world.apply_textures_to_objects(
      name_list,
      self.noise_texture_color_blue,
      carla.TextureFloatColor(0, 0), # Skip emissive
      self.noise_texture_float_color_blue,
      self.noise_texture_float_color_blue)

  def test_apply_textures(self):
      ec = 0
      try:
        argparser = argparse.ArgumentParser()
        argparser.add_argument(
            '--host',
            metavar='H',
            default=DEFAULT_HOST,
            help=f'IP of the host server (default: {DEFAULT_HOST})')
        argparser.add_argument(
            '-p', '--port',
            metavar='P',
            default=DEFAULT_PORT,
            type=int,
            help=f'TCP port to listen to (default: {DEFAULT_PORT})')
        args = argparser.parse_args()
        self.reset_state()
        self.test_apply_color_texture_to_objects(args.host, args.port)
        self.reset_state()
        self.test_apply_float_color_texture_to_objects(args.host, args.port)
        self.reset_state()
        self.test_apply_textures_to_objects(args.host, args.port)
        self.reset_state()
        self.test_apply_color_texture_to_object(args.host, args.port)
        self.reset_state()
        self.test_apply_float_color_texture_to_object(args.host, args.port)
        self.reset_state()
        self.test_apply_textures_to_object(args.host, args.port)
      except Exception as e:
        ec = -1
        print('Exception caught while running texture change test.')
        print(f' Actor Name: "{this_name}"')
        print(f' Actor Index: {this_name_index}')
        raise e
      finally:
        if this_name_index != actor_count:
          print()
      exit(ec)
