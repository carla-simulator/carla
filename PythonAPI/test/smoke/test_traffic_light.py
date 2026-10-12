# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import time

import carla

from . import SmokeTest


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


class TestTrafficLight(SmokeTest):
    def test_outside_junction_snapshot(self):
        print("TestTrafficLight.test_outside_junction_snapshot")
        parameters = carla.OpendriveGenerationParameters(
            vertex_distance=2.0,
            max_road_length=50.0,
            wall_height=0.0,
            additional_width=0.6,
            smooth_junctions=True,
            enable_mesh_visibility=True)
        world = self.client.generate_opendrive_world(
            OUTSIDE_JUNCTION_TRAFFIC_LIGHT_XODR, parameters)

        deadline = time.time() + 30.0
        traffic_lights = []
        while time.time() < deadline:
            traffic_lights = list(
                world.get_actors().filter("traffic.traffic_light*"))
            if traffic_lights:
                break
            time.sleep(0.25)

        self.assertEqual(1, len(traffic_lights))
        traffic_light = traffic_lights[0]
        self.assertEqual("10", traffic_light.get_opendrive_id())
        self.assertGreater(traffic_light.get_green_time(), 0.0)
        self.assertGreater(traffic_light.get_yellow_time(), 0.0)
        self.assertGreater(traffic_light.get_red_time(), 0.0)

        for expected_state in (
                carla.TrafficLightState.Green,
                carla.TrafficLightState.Yellow,
                carla.TrafficLightState.Red):
            traffic_light.set_state(expected_state)
            self.assertIsNotNone(world.wait_for_tick(seconds=30.0))
            self.assertEqual(expected_state, traffic_light.get_state())
