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
import numpy as np

DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 2000

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

TEXTURE_HEIGHT = 256
TEXTURE_WIDTH = 256

NOISE_DATA = np.reshape([
  np.abs(np.random.normal())
  for i in range(0, TEXTURE_HEIGHT * TEXTURE_WIDTH)
], [ TEXTURE_WIDTH, TEXTURE_HEIGHT ])

NOISE_TEXTURE_COLOR_RED = to_texture(NOISE_DATA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 'r')
NOISE_TEXTURE_COLOR_GREEN = to_texture(NOISE_DATA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 'g')
NOISE_TEXTURE_COLOR_BLUE = to_texture(NOISE_DATA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 'b')

NOISE_TEXTURE_FLOAT_COLOR_RED = to_texture_float(NOISE_DATA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 'r')
NOISE_TEXTURE_FLOAT_COLOR_GREEN = to_texture_float(NOISE_DATA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 'g')
NOISE_TEXTURE_FLOAT_COLOR_BLUE = to_texture_float(NOISE_DATA, TEXTURE_WIDTH, TEXTURE_HEIGHT, 'b')

this_name_index = 0
this_name = None
actor_count = 0

def reset_state():
  global this_name_index
  global actor_count
  global this_name
  this_name_index = 0
  this_name = None
  actor_count = 0

def test_apply_color_texture_to_object(host, port):
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
      NOISE_TEXTURE_COLOR_RED)

def test_apply_float_color_texture_to_object(host, port):
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
      NOISE_TEXTURE_FLOAT_COLOR_GREEN)

def test_apply_textures_to_object(host, port):
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
      NOISE_TEXTURE_COLOR_BLUE,
      carla.TextureFloatColor(0, 0), # Skip emissive
      NOISE_TEXTURE_COLOR_BLUE,
      NOISE_TEXTURE_COLOR_BLUE)

def test_apply_color_texture_to_objects(host, port):
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
    NOISE_TEXTURE_COLOR_RED)

def test_apply_float_color_texture_to_objects(host, port):
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
    NOISE_TEXTURE_FLOAT_COLOR_GREEN)

def test_apply_textures_to_objects(host, port):
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
    NOISE_TEXTURE_COLOR_BLUE,
    carla.TextureFloatColor(0, 0), # Skip emissive
    NOISE_TEXTURE_FLOAT_COLOR_BLUE,
    NOISE_TEXTURE_FLOAT_COLOR_BLUE)

def main():
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
      reset_state()
      test_apply_color_texture_to_objects(args.host, args.port)
      reset_state()
      test_apply_float_color_texture_to_objects(args.host, args.port)
      reset_state()
      test_apply_textures_to_objects(args.host, args.port)
      reset_state()
      test_apply_color_texture_to_object(args.host, args.port)
      reset_state()
      test_apply_float_color_texture_to_object(args.host, args.port)
      reset_state()
      test_apply_textures_to_object(args.host, args.port)
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

if __name__ == '__main__':
    main()
