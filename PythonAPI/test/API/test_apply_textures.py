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
import weakref
import time
from pathlib import Path


DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 2000
OUTPUT_PATH = Path(__file__).parent / '_out' / 'apply_textures'
for e in OUTPUT_PATH.parents:
  e.mkdir(exist_ok=True)
HEIGHT = 256
WIDTH = 256



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

  def __init__(self, methodName: str = "runTest"):
    super().__init__(methodName)
    self.camera = None

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

  @staticmethod
  def save_image(weak_self, image):
    self = weak_self()
    image.convert(carla.ColorConverter.Raw)
    image.save_to_disk(str(OUTPUT_PATH / f'frame-{image.frame}'))

  def test_apply_textures(self, host = DEFAULT_HOST, port = DEFAULT_PORT):
    try:
      client = carla.Client(host, port)
      self.assertIsNotNone(client)
      client.set_timeout(20.0)
      world = client.load_world('Town10HD_Opt')
      self.assertIsNotNone(world)
      noise_data = np.reshape([
        np.abs(np.random.normal())
        for i in range(0, HEIGHT * WIDTH) ],
        [ WIDTH, HEIGHT ])
      noise_texture_color_red = self.to_texture(noise_data, WIDTH, HEIGHT, 'r')
      bpl = world.get_blueprint_library()
      transform = carla.Transform(
          carla.Location(x=-9.872277, y=9.048162, z=8.958106),
          carla.Rotation(pitch=-31.830936, yaw=-46.900402, roll=0.000000))
      camera = world.spawn_actor(
        bpl.find('sensor.camera.rgb'),
        transform)
      weak_self = weakref.ref(self)
      self.camera = camera
      camera.listen(lambda image: TestApplyTextures.save_image(weak_self, image))
      self.assertIsNotNone(bpl)
      world.tick()
      spectator = world.get_spectator()
      self.assertIsNotNone(spectator)
      # spectator.set_transform(transform)
      world.tick()
      bp = np.random.choice(bpl.filter('static.prop.*'))
      self.assertIsNotNone(bp)
      world.tick()
      for e in world.get_actors():
        print(f'Applying texture to {e}.')
        e.apply_texture(
          carla.MaterialParameter.Diffuse,
          noise_texture_color_red)
      time.sleep(2)
    finally:
      if self.camera != None:
        self.camera.destroy()
        self.camera = None
