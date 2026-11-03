// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

// The Traffic Manager's LocalizationStage remembers, for each vehicle at the
// entrance of a junction, the end of the junction and a safe point after it.
// It must forget them when the vehicle is removed or the stage is reset:
// otherwise it keeps waypoints of a destroyed InMemoryMap alive, and a vehicle
// that reuses the actor id gets the old junction's points.

#include "test.h"

#include <carla/Memory.h>
#include <carla/client/Map.h>
#include <carla/trafficmanager/InMemoryMap.h>
#include <carla/trafficmanager/LocalizationStage.h>
#include <carla/trafficmanager/SimpleWaypoint.h>

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ctm = carla::traffic_manager;

namespace {

// A one-lane road (0 to 100 m) that enters a junction, in which it either
// goes straight on for 20 m into a second road or turns right into a third.
// The InMemoryMap counts a junction only if a road has two paths into or out
// of it, and the junction's id must not be a road id, or CARLA links the
// incoming road straight to that road.
std::string RoadIntoJunction() {
  const auto lanes = [](const std::string &lane_link) {
    return R"(
    <elevationProfile><elevation s="0" a="0" b="0" c="0" d="0"/></elevationProfile>
    <lateralProfile/>
    <lanes>
      <laneSection s="0">
        <center><lane id="0" type="none" level="false"/></center>
        <right>
          <lane id="-1" type="driving" level="false">)" + lane_link + R"(
            <width sOffset="0" a="3.5" b="0" c="0" d="0"/>
          </lane>
        </right>
      </laneSection>
    </lanes>
  </road>)";
  };
  // Only the roads in the junction link their lane to the roads around it.
  const std::string road_lanes = lanes("");
  const std::string path_lanes = lanes(R"(
            <link><predecessor id="-1"/><successor id="-1"/></link>)");
  const std::string type = R"(
    <type s="0" type="town"><speed max="50" unit="km/h"/></type>)";
  return R"(<?xml version="1.0" encoding="UTF-8"?>
<OpenDRIVE>
  <header revMajor="1" revMinor="4" name="" version="1"
          north="100" south="-200" east="300" west="-100"/>
  <road name="In" length="100" id="0" junction="-1">
    <link><successor elementType="junction" elementId="100"/></link>)" + type + R"(
    <planView>
      <geometry s="0" x="0" y="0" hdg="0" length="100"><line/></geometry>
    </planView>)" + road_lanes + R"(
  <road name="Ahead" length="100" id="1" junction="-1">
    <link><predecessor elementType="junction" elementId="100"/></link>)" + type + R"(
    <planView>
      <geometry s="0" x="120" y="0" hdg="0" length="100"><line/></geometry>
    </planView>)" + road_lanes + R"(
  <road name="Right" length="100" id="2" junction="-1">
    <link><predecessor elementType="junction" elementId="100"/></link>)" + type + R"(
    <planView>
      <geometry s="0" x="110" y="-10" hdg="-1.5707963267948966" length="100"><line/></geometry>
    </planView>)" + road_lanes + R"(
  <road name="Straight" length="20" id="3" junction="100">
    <link>
      <predecessor elementType="road" elementId="0" contactPoint="end"/>
      <successor elementType="road" elementId="1" contactPoint="start"/>
    </link>)" + type + R"(
    <planView>
      <geometry s="0" x="100" y="0" hdg="0" length="20"><line/></geometry>
    </planView>)" + path_lanes + R"(
  <road name="Turn" length="15.707963267948966" id="4" junction="100">
    <link>
      <predecessor elementType="road" elementId="0" contactPoint="end"/>
      <successor elementType="road" elementId="2" contactPoint="start"/>
    </link>)" + type + R"(
    <planView>
      <geometry s="0" x="100" y="0" hdg="0" length="15.707963267948966">
        <arc curvature="-0.1"/>
      </geometry>
    </planView>)" + path_lanes + R"(
  <junction id="100" name="J">
    <connection id="0" incomingRoad="0" connectingRoad="3" contactPoint="start">
      <laneLink from="-1" to="-1"/>
    </connection>
    <connection id="1" incomingRoad="0" connectingRoad="4" contactPoint="start">
      <laneLink from="-1" to="-1"/>
    </connection>
  </junction>
</OpenDRIVE>
)";
}

// Localizes a vehicle standing just before the junction, facing it, then lets
// `forget` drop the vehicle from the stage and destroys the map. Returns the
// waypoints of the map that are still alive while the stage lives on.
size_t WaypointsAliveAfter(
    const std::function<void(ctm::LocalizationStage &, ctm::ActorId)> &forget) {
  const ctm::ActorId vehicle = 1u;
  const std::vector<ctm::ActorId> vehicle_id_list{vehicle};
  ctm::BufferMap buffer_map;
  ctm::SimulationState simulation_state;
  ctm::TrackTraffic track_traffic;
  std::unordered_map<ctm::ActorId, std::pair<float, bool>> large_vehicles;
  ctm::Parameters parameters;
  std::vector<ctm::ActorId> marked_for_removal;
  ctm::LocalizationFrame output_array(1u);
  ctm::RandomGenerator random_device(0u);

  auto world_map = carla::MakeShared<carla::client::Map>("junction", RoadIntoJunction());
  ctm::LocalMapPtr local_map = std::make_shared<ctm::InMemoryMap>(world_map);
  local_map->SetUp();
  std::vector<std::weak_ptr<ctm::SimpleWaypoint>> waypoints;
  for (const auto &waypoint : local_map->GetDenseTopology()) {
    waypoints.emplace_back(waypoint);
  }

  ctm::KinematicState state;
  state.location = carla::geom::Location(97.0f, 1.75f, 0.0f);
  state.rotation = carla::geom::Rotation(0.0f, 0.0f, 0.0f);
  state.velocity = carla::geom::Vector3D(0.0f, 0.0f, 0.0f);
  state.speed_limit = 50.0f;
  state.physics_enabled = true;
  state.is_dormant = false;
  ctm::StaticAttributes attributes;
  attributes.actor_type = ctm::ActorType::Vehicle;
  attributes.half_length = 2.0f;
  attributes.half_width = 1.0f;
  attributes.half_height = 0.75f;
  simulation_state.AddActor(vehicle, state, attributes,
                            ctm::TrafficLightState{ctm::TLS::Green, false});

  ctm::LocalizationStage stage(vehicle_id_list, buffer_map, simulation_state,
                              track_traffic, local_map, large_vehicles,
                              parameters, marked_for_removal, output_array,
                              random_device);
  // The first update fills the vehicle's waypoint buffer, the second finds the
  // junction ahead in it.
  stage.Update(0u);
  stage.Update(0u);
  EXPECT_TRUE(output_array.front().is_at_junction_entrance);
  EXPECT_NE(output_array.front().junction_end_point, nullptr);
  EXPECT_NE(output_array.front().safe_point, nullptr);

  forget(stage, vehicle);
  output_array.clear();
  buffer_map.clear();
  local_map.reset();

  EXPECT_FALSE(waypoints.empty());
  size_t alive = 0u;
  for (const auto &waypoint : waypoints) {
    alive += waypoint.expired() ? 0u : 1u;
  }
  return alive;
}

} // namespace

TEST(LocalizationStage, remove_actor_forgets_its_junction_entrance) {
  EXPECT_EQ(WaypointsAliveAfter([](ctm::LocalizationStage &stage, ctm::ActorId vehicle) {
    stage.RemoveActor(vehicle);
  }), 0u);
}

TEST(LocalizationStage, reset_forgets_every_junction_entrance) {
  EXPECT_EQ(WaypointsAliveAfter([](ctm::LocalizationStage &stage, ctm::ActorId) {
    stage.Reset();
  }), 0u);
}
