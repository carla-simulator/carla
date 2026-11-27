# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import time

import carla

from . import SyncSmokeTest


OUTSIDE_JUNCTION_TRAFFIC_LIGHT_XODR = """<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE>
  <header revMajor="1" revMinor="4" name="outside_junction_traffic_light"
      version="1.0" date="2026-08-04" north="10.0" south="-10.0"
      east="80.0" west="0.0" vendor="CARLA regression test"/>
  <road name="straight_road" length="80.0" id="1" junction="-1">
    <link/>
    <type s="0.0" type="town">
      <speed max="30.0" unit="km/h"/>
    </type>
    <planView>
      <geometry s="0.0" x="0.0" y="0.0" hdg="0.0" length="80.0">
        <line/>
      </geometry>
    </planView>
    <elevationProfile>
      <elevation s="0.0" a="0.0" b="0.0" c="0.0" d="0.0"/>
    </elevationProfile>
    <lateralProfile>
      <superelevation s="0.0" a="0.0" b="0.0" c="0.0" d="0.0"/>
    </lateralProfile>
    <lanes>
      <laneOffset s="0.0" a="0.0" b="0.0" c="0.0" d="0.0"/>
      <laneSection s="0.0">
        <center>
          <lane id="0" type="none" level="false">
            <roadMark sOffset="0.0" type="solid" material="standard"
                color="white" width="0.12" laneChange="none"/>
          </lane>
        </center>
        <right>
          <lane id="-1" type="driving" level="false">
            <link/>
            <width sOffset="0.0" a="3.5" b="0.0" c="0.0" d="0.0"/>
            <roadMark sOffset="0.0" type="solid" material="standard"
                color="white" width="0.12" laneChange="none"/>
            <speed sOffset="0.0" max="30.0" unit="km/h"/>
          </lane>
        </right>
      </laneSection>
    </lanes>
    <objects/>
    <signals>
      <signal id="10" name="outside_junction_traffic_light" s="40.0"
          t="-5.0" zOffset="0.0" hOffset="0.0" roll="0.0" pitch="0.0"
          orientation="+" dynamic="yes" country="OpenDRIVE" type="1000001"
          subtype="-1" value="-1.0" text="" height="1.2" width="0.6">
        <validity fromLane="-1" toLane="-1"/>
      </signal>
    </signals>
  </road>
  <controller name="outside_junction_controller" id="100" sequence="0">
    <control signalId="10" type="0"/>
  </controller>
</OpenDRIVE>
"""


class TestTrafficLight(SyncSmokeTest):
    def test_outside_junction_snapshot(self):
        print("TestTrafficLight.test_outside_junction_snapshot")
        parameters = carla.OpendriveGenerationParameters(
            vertex_distance=2.0,
            max_road_length=50.0,
            wall_height=0.0,
            additional_width=0.6,
            smooth_junctions=True,
            enable_mesh_visibility=True)
        # A synchronous tick returns once the client holds its frame, so it reflects the set_state() before it.
        world = self.client.generate_opendrive_world(
            OUTSIDE_JUNCTION_TRAFFIC_LIGHT_XODR, parameters, reset_settings=False)
        self.world = world

        traffic_lights = []
        for _ in range(100):
            world.tick()
            traffic_lights = list(
                world.get_actors().filter("traffic.traffic_light*"))
            if traffic_lights:
                break

        self.assertEqual(1, len(traffic_lights))
        traffic_light = traffic_lights[0]
        self.assertEqual("10", traffic_light.get_opendrive_id())
        self.assertGreater(traffic_light.get_green_time(), 0.0)
        self.assertGreater(traffic_light.get_yellow_time(), 0.0)
        self.assertGreater(traffic_light.get_red_time(), 0.0)

        # Only set_state() changes the light, not its controller's own cycle.
        traffic_light.freeze(True)
        for expected_state in (
                carla.TrafficLightState.Green,
                carla.TrafficLightState.Yellow,
                carla.TrafficLightState.Red):
            traffic_light.set_state(expected_state)
            world.tick()
            self.assertEqual(expected_state, traffic_light.get_state())

    def _tick_until(self, world, condition, ticks=5):
        for _ in range(ticks):
            world.tick()
            if condition():
                return True
        return False

    def test_static_light_keeps_its_data(self):
        """A traffic light is a static actor: only its state, elapsed time and
        frozen flag reach the client every tick. Everything else it had in the
        snapshot (transform, OpenDRIVE id, phase times, pole index) must read
        the same, also after it changes."""
        print("TestTrafficLight.test_static_light_keeps_its_data")
        parameters = carla.OpendriveGenerationParameters(
            vertex_distance=2.0,
            max_road_length=50.0,
            wall_height=0.0,
            additional_width=0.6,
            smooth_junctions=True,
            enable_mesh_visibility=True)
        world = self.client.generate_opendrive_world(
            OUTSIDE_JUNCTION_TRAFFIC_LIGHT_XODR, parameters, reset_settings=False)
        self.world = world
        self.assertTrue(self._tick_until(
            world, lambda: len(world.get_actors().filter("traffic.traffic_light*")) == 1, ticks=100))
        light = world.get_actors().filter("traffic.traffic_light*")[0]

        snapshot = world.get_snapshot()
        self.assertIsNone(snapshot.find(light.id))
        self.assertFalse(snapshot.has_actor(light.id))
        self.assertIn(light.id, {actor.id for actor in world.get_static_actors()})
        self.assertTrue(light.is_alive)
        self.assertEqual("10", light.get_opendrive_id())
        self.assertEqual(light.id, world.get_actor(light.id).id)

        # The signal is 40 m along a road that runs on the x axis, 5 m aside.
        transform = light.get_transform()
        self.assertTrue(20.0 < transform.location.x < 60.0, transform)
        self.assertTrue(abs(transform.location.y) < 15.0, transform)
        self.assertEqual(light.get_velocity().length(), 0.0)
        pole_index = light.get_pole_index()

        # Running: the elapsed time advances one step per tick while the
        # state holds, and starts again when the state changes.
        light.freeze(False)
        self.assertTrue(self._tick_until(world, lambda: not light.is_frozen()))
        steps = 0
        for _ in range(60):
            state, elapsed = light.get_state(), light.get_elapsed_time()
            world.tick()
            if light.get_state() == state:
                self.assertAlmostEqual(light.get_elapsed_time() - elapsed, 0.05, places=3)
                steps += 1
            else:
                self.assertLess(light.get_elapsed_time(), elapsed)
        self.assertGreater(steps, 30)

        # Frozen: the flag arrives and the elapsed time stands still.
        light.freeze(True)
        self.assertTrue(self._tick_until(world, light.is_frozen))
        elapsed = light.get_elapsed_time()
        for _ in range(5):
            world.tick()
            self.assertTrue(light.is_frozen())
            self.assertEqual(elapsed, light.get_elapsed_time())

        # The phase times are not sent every tick: a change still arrives.
        for setter, getter, seconds in (
                (light.set_green_time, light.get_green_time, 7.5),
                (light.set_yellow_time, light.get_yellow_time, 2.5),
                (light.set_red_time, light.get_red_time, 4.5)):
            self.assertNotAlmostEqual(getter(), seconds, places=3)
            setter(seconds)
            self.assertTrue(
                self._tick_until(world, lambda: abs(getter() - seconds) < 1e-4), (getter(), seconds))
        self.assertAlmostEqual(light.get_green_time(), 7.5, places=4)
        self.assertAlmostEqual(light.get_yellow_time(), 2.5, places=4)
        self.assertAlmostEqual(light.get_red_time(), 4.5, places=4)

        # A client that connects now reads the same, and nothing else changed.
        late = carla.Client(*self.testing_address)
        late.set_timeout(120.0)
        late_world = late.get_world()
        # Its stream may connect after a tick was sent: tick until one reaches it.
        for _ in range(20):
            frame = world.tick()
            deadline = time.time() + 1.0
            while late_world.get_snapshot().frame < frame and time.time() < deadline:
                time.sleep(0.001)
            if late_world.get_snapshot().frame == frame:
                break
        self.assertEqual(late_world.get_snapshot().frame, frame)
        for seen in (light, late_world.get_actor(light.id)):
            self.assertEqual("10", seen.get_opendrive_id())
            self.assertEqual(pole_index, seen.get_pole_index())
            self.assertEqual(transform.location.distance(seen.get_transform().location), 0.0)
            self.assertEqual(transform.rotation.yaw, seen.get_transform().rotation.yaw)
            self.assertAlmostEqual(seen.get_green_time(), 7.5, places=4)
            self.assertAlmostEqual(seen.get_yellow_time(), 2.5, places=4)
            self.assertAlmostEqual(seen.get_red_time(), 4.5, places=4)
            self.assertTrue(seen.is_frozen())
            self.assertEqual(light.get_state(), seen.get_state())
            self.assertTrue(seen.is_alive)
