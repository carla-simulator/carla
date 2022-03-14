# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla
import time
import math

class Sensor():
  def __init__(self, world, bp_sensor, sensor_tick):
    self.bp_sensor = bp_sensor
    bp_sensor.set_attribute("sensor_tick", str(sensor_tick))
    self.sensor = world.spawn_actor(bp_sensor, carla.Transform())
    self.sensor.listen(lambda sensor_data: self.listen(sensor_data))
    self.num_ticks = 0

  def destroy(self):
    self.sensor.destroy()

  def listen(self, sensor_data):
    self.num_ticks += 1

class TestSensorTickTime(SyncSmokeTest):
  def test_sensor_tick_time(self):
    print("TestSensorTickTime.test_sensor_tick_time")

    bp_lib = self.world.get_blueprint_library()

    sensor_exception = {
      "sensor.camera.depth",
      "sensor.camera.normals",
      "sensor.camera.optical_flow",
      "sensor.camera.rgb",
      "sensor.camera.semantic_segmentation",
      "sensor.camera.dvs",
      "sensor.other.obstacle",
      "sensor.camera.instance_segmentation"
    }
    spawned_sensors = []
    sensor_tick = 1.0

    for bp_sensor in bp_lib.filter("sensor.*"):

      if bp_sensor.id in sensor_exception:
         continue

      if bp_sensor.has_attribute("sensor_tick"):
        spawned_sensors.append(Sensor(self.world, bp_sensor, sensor_tick))

    num_ticks = 50
    for _ in range(0, num_ticks):
      self.world.tick()
    time.sleep(1.0)

    dt = self.world.get_settings().fixed_delta_seconds
    total_time = num_ticks * dt
    num_sensor_ticks = int(math.ceil(total_time/sensor_tick))

    for sensor in spawned_sensors:
      self.assertEqual(sensor.num_ticks, num_sensor_ticks,
        "\n\n {} does not match tick count".format(sensor.bp_sensor.id))
      sensor.destroy()


