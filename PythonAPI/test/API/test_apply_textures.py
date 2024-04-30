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
OUTPUT_PATH = '_out/apply_textures'



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



class TestApplyTextures(unittest.TestCase):

  def to_texture(self, data, w, h, mask = 'rgb'):
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

  def to_texture_float(self, data, w, h, mask = 'rgb'):
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

  def test_apply_textures(self, host = DEFAULT_HOST, port = DEFAULT_PORT):
    client = carla.Client(host, port)
    self.assertIsNotNone(client)
    client.set_timeout(20.0)
    world = client.get_world()
    self.assertIsNotNone(world)
    height = 256
    width = 256
    print('Creating noise data.')
    noise_data = np.reshape([
      np.abs(np.random.normal())
      for i in range(0, height * width) ],
      [ width, height ])
    print('Creating noise texture.')
    noise_texture_color_red = self.to_texture(noise_data, width, height, 'r')
    names = set(world.get_names_of_all_objects())
    print('Spawning test actor.')
    bpl = world.get_blueprint_library()
    spectator = world.get_spectator()
    spectator.set_transform(
      carla.Transform(
        carla.Location(-27.8, -63.28, 10.6),
        carla.Rotation(pitch=-29, yaw=135)))
    bp = np.random.choice(bpl.filter('static.prop.*'))
    self.assertIsNotNone(bp)
    target = world.try_spawn_actor(
      bp,
      carla.Transform(
        carla.Location(-4270.000000, -5550.000000, 0.000000)))
    world.tick()
    self.assertIsNotNone(target)
    actors = set.difference(names, set(world.get_names_of_all_objects()))
    self.assertNotEqual(len(actors), 0)
    for e in actors:
      print(f'Applying random test texture to {e}')
      world.apply_color_texture_to_object(
        e,
        carla.MaterialParameter.Diffuse,
        noise_texture_color_red)

if __name__ == '__main__':
  unittest.main()
