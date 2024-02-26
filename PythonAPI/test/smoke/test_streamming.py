# Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from . import SmokeTest

import time
import threading
import carla

class TestStreamming(SmokeTest):

    lat = 0.0
    lon = 0.0

    def on_gnss_set(self, event):
        self.lat = event.latitude
        self.lon = event.longitude

    def on_gnss_check(self, event):
        self.assertAlmostEqual(event.latitude, self.lat, places=4)
        self.assertAlmostEqual(event.longitude, self.lon, places=4)

    def create_client(self):
        client = carla.Client(*self.testing_address)
        client.set_timeout(60.0)
        world = client.get_world()
        actors = world.get_actors()
        for actor in actors:
            if (actor.type_id == "sensor.other.gnss"):
                actor.listen(self.on_gnss_check)
        time.sleep(5)
        # stop
        for actor in actors:
            if (actor.type_id == "sensor.other.gnss"):
                actor.stop()


    def test_multistream(self):
        print("TestStreamming.test_multistream")
        # create the sensor
        world = self.client.get_world()
        bp = world.get_blueprint_library().find('sensor.other.gnss')
        bp.set_attribute("sensor_tick", str(1.0))
        gnss_sensor = world.spawn_actor(bp, carla.Transform())
        gnss_sensor.listen(self.on_gnss_set)
        world.wait_for_tick()

        # create 5 clients
        t = [0] * 5
        for i in range(5):
            t[i] = threading.Thread(target=self.create_client)
            t[i].setDaemon(True)
            t[i].start()

        # wait for ending clients
        for i in range(5):
            t[i].join()

        gnss_sensor.destroy()
