#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Allows controlling a vehicle with a keyboard. For a simpler and more
# documented example, please take a look at tutorial.py.


import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla
import queue
import random
import time
import weakref


# ==============================================================================
# -- BenchmarkSensor ---------------------------------------------------------------
# ==============================================================================


class BenchmarkSensor(object):
    def __init__(self, world):
        self.sensor = None
        self.world = world

        bp = world.get_blueprint_library().find('sensor.other.benchmark')

        # queries = "{\"STATGROUP_SceneRendering\": [ \"STAT_MeshDrawCalls\", \"STAT_DecalsDrawTime\" ]}"
        queries = """
          {\"stat scenerendering\": [ \"STAT_MeshDrawCalls\",
                                      \"STAT_DecalsDrawTime\",
                                      \"STAT_SceneDecals\",
                                      \"STAT_Decals\",
                                      \"STAT_DecalsDrawTime\",
                                      \"STAT_SceneLights\",
                                      \"STAT_InitViewsTime\",
                                      \"STAT_RenderQueryResultTime\"
                                    ]
          }"""
        bp.set_attribute('queries', queries)

        self.sensor = world.spawn_actor(bp, carla.Transform())

        # We need a weak reference to self to avoid circular reference.
        weak_self = weakref.ref(self)
        self.sensor.listen(
            lambda benchmark_data: BenchmarkSensor._Benchmark_callback(weak_self, benchmark_data))

    def destroy(self):
        self.sensor.destroy()

    @staticmethod
    def _Benchmark_callback(weak_self, benchmark_data):
        self = weak_self()
        if not self:
            return
        print(benchmark_data.result)

# ==============================================================================
# -- game_loop() ---------------------------------------------------------------
# ==============================================================================

def main():
    run_sync = False

    try:
        client = carla.Client("localhost", 2000)
        client.set_timeout(3)
        # world = client.load_world("Town03")
        world = client.get_world()

        benchmark_sensor = BenchmarkSensor(world)

        world.send_console_command("show decals")

        if run_sync:
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=True,
                fixed_delta_seconds=1.0/30.0)
            world.world.apply_settings(settings)

        while True:
            if run_sync:
                world.tick()
            else:
                world.wait_for_tick()

    finally:

        if run_sync:
            settings = carla.WorldSettings(
                no_rendering_mode=False,
                synchronous_mode=False,
                fixed_delta_seconds=0.0)
            world.apply_settings(settings)

        benchmark_sensor.destroy()


if __name__ == '__main__':
    main()