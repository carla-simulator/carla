// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"
#include "carla/agents/navigation/GlobalRoutePlanner.h"
#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"
#include <algorithm>
#include <array>
#include <locale>
#include <sstream>
#include <tuple>

namespace {

  using RoadIds = std::array<int, 3>;

  std::shared_ptr<carla::client::Map> IntersectionMap(
      const RoadIds &ids, const RoadIds &order, bool junction) {
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
           "</geoReference></header>";
    for (const int role : order) {
      const auto length = role == 0 ? "100" : role == 1 ? "200" : "100.1";
      const int x = role == 0 ? 0 : role == 1 ? 100 : 300;
      xml << "<road id=\"" << ids[role] << "\" length=\"" << length << "\" junction=\""
          << (junction && role == 1 ? 10 : -1) << "\" rule=\"RHT\"><link>";
      if (role == 0 && junction) {
        xml << "<successor elementType=\"junction\" elementId=\"10\"/>";
      } else if (role < 2) {
        xml << "<successor elementType=\"road\" elementId=\"" << ids[role + 1]
            << "\" contactPoint=\"start\"/>";
      }
      if (role == 2 && junction) {
        xml << "<predecessor elementType=\"junction\" elementId=\"10\"/>";
      } else if (role > 0) {
        xml << "<predecessor elementType=\"road\" elementId=\"" << ids[role - 1]
            << "\" contactPoint=\"end\"/>";
      }
      xml << "</link><planView><geometry s=\"0\" x=\"" << x
          << "\" y=\"0\" hdg=\"0\" length=\"" << length << "\"><line/></geometry></planView><lanes>";
      const auto section = [&](int station) {
        xml << "<laneSection s=\"" << station << "\"><center><lane id=\"0\" type=\"none\"/></center>"
               "<right><lane id=\"-1\" type=\"driving\"><link>";
        if (role > 0) {
          xml << "<predecessor id=\"-1\"/>";
        }
        if (role < 2) {
          xml << "<successor id=\"-1\"/>";
        }
        xml << "</link><width sOffset=\"0\" a=\"3.5\" b=\"0\" c=\"0\" d=\"0\"/>"
               "</lane></right></laneSection>";
      };
      section(0);
      if (role == 1) {
        section(100);
      }
      xml << "</lanes></road>";
    }
    if (junction) {
      xml << "<junction id=\"10\"><connection id=\"0\" incomingRoad=\"" << ids[0]
          << "\" connectingRoad=\"" << ids[1]
          << "\" contactPoint=\"start\"><laneLink from=\"-1\" to=\"-1\"/></connection></junction>";
    }
    xml << "</OpenDRIVE>";
    return std::make_shared<carla::client::Map>("junction-node-order", xml.str());
  }

  std::pair<std::shared_ptr<carla::client::Map>, RoadIds> NativeOrdering(bool junction, bool zero_end) {
    RoadIds ids{1, 2, 3};
    do {
      RoadIds order{0, 1, 2};
      do {
        const auto map = IntersectionMap(ids, order, junction);
        const auto topology = map->GetTopology();
        if (topology.size() == 4u && topology.front().first->GetRoadId() ==
            static_cast<unsigned>(ids[zero_end ? 2 : 0])) {
          return {map, ids};
        }
      } while (std::next_permutation(order.begin(), order.end()));
    } while (std::next_permutation(ids.begin(), ids.end()));
    return {nullptr, ids};
  }

  class IntersectionExitNode : public testing::TestWithParam<std::tuple<bool, bool>> {};

  TEST_P(IntersectionExitNode, public_turn_labels_do_not_depend_on_native_topology_order) {
    const auto [junction, zero_end] = GetParam();
    const auto [map, ids] = NativeOrdering(junction, zero_end);
    ASSERT_NE(map, nullptr);
    const auto origin = map->GetWaypointXODR(ids[0], -1, 90.0f);
    const auto destination = map->GetWaypointXODR(ids[1], -1, 105.0f);
    ASSERT_NE(origin, nullptr);
    ASSERT_NE(destination, nullptr);
    carla::agents::navigation::GlobalRoutePlanner planner(map, 2.0);
    const auto route = planner.TraceRoute(origin->GetTransform().location, destination->GetTransform().location);
    const auto expected = junction ? carla::agents::navigation::RoadOption::Straight
                                   : carla::agents::navigation::RoadOption::LaneFollow;
    bool found = false;
    for (const auto &item : route) {
      if (item.first->GetRoadId() == static_cast<unsigned>(ids[1]) && item.first->GetDistance() > 100.000001) {
        found = true;
        EXPECT_EQ(item.second, expected);
      }
    }
    EXPECT_TRUE(found);
  }

  INSTANTIATE_TEST_SUITE_P(junction_and_order_controls, IntersectionExitNode,
                          testing::Combine(testing::Bool(), testing::Bool()));

} // namespace
