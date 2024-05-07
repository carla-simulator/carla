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
import time
from pathlib import Path


DEFAULT_HOST = '127.0.0.1'
DEFAULT_PORT = 2000
OUTPUT_PATH = Path(__file__).parent / '_out' / 'semantic_segmentation'
for e in OUTPUT_PATH.parents:
  e.mkdir(exist_ok=True)
TEST_TRANSFORMS = [
  carla.Transform(
    carla.Location(x=-9.872277, y=9.048162, z=8.958106),
    carla.Rotation(pitch=-31.830936, yaw=-46.900402, roll=0.000000)),
  carla.Transform(
    carla.Location(x=-25.324059, y=25.735699, z=23.037737),
    carla.Rotation(pitch=-31.760944, yaw=-47.201935, roll=-0.000000)),
  carla.Transform(
    carla.Location(x=-40.903774, y=42.561401, z=37.233944),
    carla.Rotation(pitch=-31.760944, yaw=-47.201935, roll=-0.000000)),
  carla.Transform(
    carla.Location(x=-72.925125, y=77.143654, z=66.411736),
    carla.Rotation(pitch=-31.760944, yaw=-47.201935, roll=-0.000000))
]



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



class TestSemanticSegmentation(unittest.TestCase):

  def __init__(self, methodName: str = "runTest"):
    super().__init__(methodName)
    self.camera = None

  @staticmethod
  def save_image(image):
    image.convert(carla.ColorConverter.CityScapesPalette)
    image.save_to_disk(str(OUTPUT_PATH / f'frame-{image.frame}'))

  def test_semantic_segmentation(self, host = DEFAULT_HOST, port = DEFAULT_PORT):
    try:
      client = carla.Client(host, port)
      self.assertIsNotNone(client)
      client.set_timeout(20.0)
      world = client.load_world('Town10HD_Opt')
      self.assertIsNotNone(world)
      bpl = world.get_blueprint_library()
      camera = world.spawn_actor(
        bpl.find('sensor.camera.semantic_segmentation'),
        TEST_TRANSFORMS[0])
      self.camera = camera
      camera.listen(lambda image: TestSemanticSegmentation.save_image(image))
      self.assertIsNotNone(bpl)
      spectator = world.get_spectator()
      self.assertIsNotNone(spectator)
      spectator.set_transform(TEST_TRANSFORMS[0])
      for e in TEST_TRANSFORMS:
        camera.set_transform(e)
        spectator.set_transform(e)
        world.tick()
        time.sleep(0.5)
      time.sleep(1)
    finally:
      if self.camera != None:
        self.camera.destroy()
        self.camera = None
