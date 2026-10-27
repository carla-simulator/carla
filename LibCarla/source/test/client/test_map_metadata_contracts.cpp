// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/client/Junction.h>
#include <carla/client/Landmark.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <carla/geom/Math.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>

namespace {

  const std::string Header =
      "<OpenDRIVE><header revMajor='1' revMinor='4'><geoReference>"
      "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
      "</geoReference></header>";

  std::string LaneXml(int lane, const std::string &links = "") {
    const auto side = lane < 0 ? "right" : "left";
    return "<lanes><laneSection s='0'><center><lane id='0' type='none'/></center><" + std::string(side) +
        "><lane id='" + std::to_string(lane) + "' type='driving'><link>" + links +
        "</link><width sOffset='0' a='3.5' b='0' c='0' d='0'/></lane></" + side + "></laneSection></lanes>";
  }

  std::string SignalXml(const std::string &id, double station) {
    return "<signal s='" + std::to_string(station) + "' t='0' id='" + id +
        "' name='signal' dynamic='yes' orientation='none' zOffset='0' country='DE' type='1000001' "
        "subtype='0' value='0' height='1' width='1'><validity fromLane='-1' toLane='-1'/></signal>";
  }

  std::string ControllerXml(const std::string &id, const std::vector<std::string> &signals) {
    std::string xml = "<controller id='" + id + "' name='group' sequence='0'>";
    for (const auto &signal : signals) {
      xml += "<control signalId='" + signal + "' type='0'/>";
    }
    return xml + "</controller>";
  }

  std::string PlainRoad(unsigned id, const std::string &signals) {
    return "<road id='" + std::to_string(id) + "' length='100' junction='-1' rule='RHT'><link/>"
        "<planView><geometry s='0' x='0' y='" + std::to_string(id == 1u ? 0u : 20u) +
        "' hdg='0' length='100'><line/></geometry></planView>" +
        LaneXml(-1) + "<signals>" + signals + "</signals></road>";
  }

  carla::SharedPtr<carla::client::Map> PlainMap(
      const std::string &signals, const std::string &controllers, const std::string &extra_road = "") {
    return carla::MakeShared<carla::client::Map>(
        "signal-groups", Header + PlainRoad(1u, signals) + extra_road + controllers + "</OpenDRIVE>");
  }

  carla::SharedPtr<carla::client::Map> JunctionMap(
      bool rht, int lane, double curvature, bool grouped = false, bool global_extra = false) {
    const double length = 60.0;
    const double end_heading = curvature * length;
    const double end_x = curvature == 0.0 ? length : std::sin(end_heading) / curvature;
    const double end_y = curvature == 0.0 ? 0.0 : (1.0 - std::cos(end_heading)) / curvature;
    const bool increasing_s = rht == (lane < 0);
    const std::string lane_id = std::to_string(lane);
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << std::setprecision(17) << Header;
    for (unsigned id : {1u, 2u, 3u}) {
      const double road_length = id == 2u ? length : 10.0;
      const double x = id == 1u ? -10.0 : (id == 2u ? 0.0 : end_x);
      const double y = id == 3u ? end_y : 0.0;
      const double heading = id == 3u ? end_heading : 0.0;
      std::string road_links;
      std::string lane_links;
      if (id == 1u) {
        road_links = "<successor elementType='junction' elementId='10'/>";
        lane_links = "<successor id='" + lane_id + "'/>";
      } else if (id == 2u) {
        road_links = "<predecessor elementType='road' elementId='1' contactPoint='end'/>"
            "<successor elementType='road' elementId='3' contactPoint='start'/>";
        lane_links = "<predecessor id='" + lane_id + "'/><successor id='" + lane_id + "'/>";
      } else {
        road_links = "<predecessor elementType='junction' elementId='10'/>";
        lane_links = "<predecessor id='" + lane_id + "'/>";
      }
      xml << "<road id='" << id << "' length='" << road_length << "' junction='" << (id == 2u ? 10 : -1)
          << "' rule='" << (rht ? "RHT" : "LHT") << "'><link>" << road_links << "</link><planView>"
          << "<geometry s='0' x='" << x << "' y='" << y << "' hdg='" << heading << "' length='" << road_length
          << "'>";
      if (id == 2u && curvature != 0.0) {
        xml << "<arc curvature='" << curvature << "'/>";
      } else {
        xml << "<line/>";
      }
      xml << "</geometry></planView>" << LaneXml(lane, lane_links);
      if (grouped && id == 1u) {
        xml << "<signals>" << SignalXml("101", 2.0) << SignalXml("102", 5.0);
        if (global_extra) {
          xml << SignalXml("103", 7.0) << SignalXml("104", 8.0);
        }
        xml << "</signals>";
      }
      xml << "</road>";
    }
    xml << "<junction id='10'><connection id='0' incomingRoad='" << (increasing_s ? 1u : 3u)
        << "' connectingRoad='2' contactPoint='" << (increasing_s ? "start" : "end")
        << "'><laneLink from='" << lane << "' to='" << lane << "'/></connection>";
    if (grouped) {
      xml << "<controller id='201'/>";
    }
    xml << "</junction>";
    if (grouped) {
      xml << ControllerXml("201", {"101", "102"});
      if (global_extra) {
        xml << ControllerXml("302", {"103", "104"});
      }
    }
    xml << "</OpenDRIVE>";
    return carla::MakeShared<carla::client::Map>("junction-bounds", xml.str());
  }

  std::vector<std::string> GroupIds(const carla::SharedPtr<carla::client::Map> &map, const std::string &id) {
    const auto references = map->GetLandmarksFromId(id);
    EXPECT_FALSE(references.empty());
    if (references.empty()) {
      return {};
    }
    std::vector<std::string> result;
    for (const auto &landmark : map->GetLandmarkGroup(*references.front())) {
      result.push_back(landmark->GetId());
    }
    std::sort(result.begin(), result.end());
    return result;
  }

  class CurvedJunctionBounds : public testing::TestWithParam<std::tuple<bool, int, double>> {};

  TEST_P(CurvedJunctionBounds, contains_the_native_lane_midpoint_in_either_travel_direction) {
    const auto [rht, lane, curvature] = GetParam();
    const auto map = JunctionMap(rht, lane, curvature);
    const auto waypoint = map->GetWaypointXODR(2u, lane, 30.0f);
    ASSERT_NE(waypoint, nullptr);
    const auto junction = waypoint->GetJunction();
    ASSERT_NE(junction, nullptr);
    const auto box = junction->GetBoundingBox();
    EXPECT_TRUE(box.Contains(waypoint->GetTransform().location, carla::geom::Transform{}));
    EXPECT_GT(box.extent.x, 5.0f);
  }

  constexpr double Curvature = carla::geom::Math::Pi<double>() / 60.0;
  INSTANTIATE_TEST_SUITE_P(BothTrafficRulesAndLaneSigns, CurvedJunctionBounds,
      testing::Combine(testing::Bool(), testing::Values(-1, 1), testing::Values(-Curvature, Curvature)));

  class StraightJunctionBounds : public testing::TestWithParam<std::tuple<bool, int>> {};

  TEST_P(StraightJunctionBounds, straight_endpoint_bounds_control) {
    const auto [rht, lane] = GetParam();
    const auto map = JunctionMap(rht, lane, 0.0);
    const auto waypoint = map->GetWaypointXODR(2u, lane, 30.0f);
    ASSERT_NE(waypoint, nullptr);
    const auto junction = waypoint->GetJunction();
    ASSERT_NE(junction, nullptr);
    EXPECT_TRUE(junction->GetBoundingBox().Contains(waypoint->GetTransform().location, carla::geom::Transform{}));
  }

  INSTANTIATE_TEST_SUITE_P(BothTrafficRulesAndLaneSigns, StraightJunctionBounds,
      testing::Combine(testing::Bool(), testing::Values(-1, 1)));

  class ControllerSignalGroup : public testing::TestWithParam<std::string> {};

  TEST_P(ControllerSignalGroup, global_controller_does_not_require_a_junction_sync_reference) {
    const auto map = PlainMap(SignalXml("101", 25.0) + SignalXml("102", 50.0), ControllerXml("201", {"101", "102"}));
    EXPECT_EQ(GroupIds(map, GetParam()), (std::vector<std::string>{"101", "102"}));
  }

  INSTANTIATE_TEST_SUITE_P(EachControlledSignal, ControllerSignalGroup, testing::Values("101", "102"));

  TEST(controller_signal_group, uncontrolled_signal_has_no_group_control) {
    const auto map = PlainMap(SignalXml("103", 75.0), "");
    EXPECT_TRUE(GroupIds(map, "103").empty());
  }

  TEST(controller_signal_group, existing_junction_controller_reference_control) {
    const auto map = JunctionMap(true, -1, 0.0, true);
    EXPECT_EQ(GroupIds(map, "101"), (std::vector<std::string>{"101", "102"}));
    EXPECT_EQ(GroupIds(map, "102"), (std::vector<std::string>{"101", "102"}));
  }

  TEST(controller_signal_group, unattached_controller_keeps_the_engine_cache_compatibility_control) {
    const auto map = PlainMap(SignalXml("101", 25.0) + SignalXml("102", 50.0),
        ControllerXml("201", {"101", "102"}));
    EXPECT_TRUE(map->GetMap().GetSignals().at("101")->GetControllers().empty());
    EXPECT_TRUE(map->GetMap().GetSignals().at("102")->GetControllers().empty());
  }

  TEST(controller_signal_group, junction_backed_and_global_only_groups_coexist) {
    const auto map = JunctionMap(true, -1, 0.0, true, true);
    EXPECT_EQ(map->GetMap().GetSignals().at("101")->GetControllers().count("201"), 1u);
    EXPECT_TRUE(map->GetMap().GetSignals().at("103")->GetControllers().empty());
    EXPECT_EQ(GroupIds(map, "101"), (std::vector<std::string>{"101", "102"}));
    EXPECT_EQ(GroupIds(map, "103"), (std::vector<std::string>{"103", "104"}));
  }

  TEST(controller_signal_group, separate_controllers_do_not_mix_signal_groups) {
    const auto map = PlainMap(
        SignalXml("101", 10.0) + SignalXml("102", 20.0) + SignalXml("201", 30.0) + SignalXml("202", 40.0),
        ControllerXml("301", {"101", "102"}) + ControllerXml("302", {"201", "202"}));
    EXPECT_EQ(GroupIds(map, "101"), (std::vector<std::string>{"101", "102"}));
    EXPECT_EQ(GroupIds(map, "201"), (std::vector<std::string>{"201", "202"}));
  }

  TEST(controller_signal_group, all_road_references_of_a_controlled_signal_are_preserved) {
    const std::string reference =
        "<signalReference id='101' s='50' t='0' orientation='none'>"
        "<validity fromLane='-1' toLane='-1'/></signalReference>";
    const auto map = PlainMap(SignalXml("101", 25.0) + SignalXml("102", 50.0),
        ControllerXml("201", {"101", "102"}), PlainRoad(2u, reference));
    ASSERT_EQ(map->GetLandmarksFromId("101").size(), 2u);
    EXPECT_EQ(GroupIds(map, "101"), (std::vector<std::string>{"101", "101", "102"}));
  }

} // namespace
