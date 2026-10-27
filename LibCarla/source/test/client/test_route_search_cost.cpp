// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/agents/navigation/GlobalRoutePlanner.h"
#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"

#include <cmath>
#include <iomanip>
#include <locale>
#include <set>
#include <sstream>

namespace {

  std::shared_ptr<carla::client::Map> RouteCostMap(bool left_hand) {
    const int lane = left_hand ? 1 : -1;
    const double offset = lane * 1.75;
    const double pi = std::acos(-1.0);
    const double radius = 90.0 / std::sqrt(2.0) + offset;
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << std::setprecision(17);
    xml << "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m"
           "</geoReference></header>";
    const auto road = [&](int id, double length, int junction, double x, double y,
                          double heading, const std::string &geometry, const std::string &links) {
      const auto side = left_hand ? "left" : "right";
      xml << "<road id=\"" << id << "\" length=\"" << length << "\" junction=\"" << junction
          << "\" rule=\"" << (left_hand ? "LHT" : "RHT") << "\"><link>" << links
          << "</link><planView><geometry s=\"0\" x=\"" << x << "\" y=\"" << y
          << "\" hdg=\"" << heading << "\" length=\"" << length << "\">" << geometry
          << "</geometry></planView><lanes><laneSection s=\"0\"><center>"
             "<lane id=\"0\" type=\"none\"/></center><" << side << "><lane id=\"" << lane
          << "\" type=\"driving\"><link><predecessor id=\"" << lane << "\"/><successor id=\""
          << lane << "\"/></link><width sOffset=\"0\" a=\"3.5\" b=\"0\" c=\"0\" d=\"0\"/>"
          << "</lane></" << side << "></laneSection></lanes></road>";
    };
    road(1, 10, -1, 0, 0, 0, "<line/>",
         "<successor elementType=\"junction\" elementId=\"9\"/>");
    std::ostringstream arc;
    arc.imbue(std::locale::classic());
    arc << std::setprecision(17) << "<arc curvature=\"" << 1.0 / radius << "\"/>";
    road(2, radius * pi / 2, 9, 10 + std::sin(-pi / 4) * offset,
         offset * (1 - std::cos(pi / 4)), -pi / 4, arc.str(),
         "<predecessor elementType=\"road\" elementId=\"1\" contactPoint=\"end\"/>"
         "<successor elementType=\"road\" elementId=\"5\" contactPoint=\"start\"/>");
    road(3, 45, 9, 10, 0, 0, "<line/>",
         "<predecessor elementType=\"road\" elementId=\"1\" contactPoint=\"end\"/>"
         "<successor elementType=\"road\" elementId=\"4\" contactPoint=\"start\"/>");
    road(4, 45, 9, 55, 0, 0, "<line/>",
         "<predecessor elementType=\"road\" elementId=\"3\" contactPoint=\"end\"/>"
         "<successor elementType=\"road\" elementId=\"5\" contactPoint=\"start\"/>");
    road(5, 10, -1, 100, 0, 0, "<line/>",
         "<predecessor elementType=\"junction\" elementId=\"9\"/>");
    xml << "<junction id=\"9\">";
    for (const int connector : {2, 3}) {
      xml << "<connection id=\"" << connector << "\" incomingRoad=\"1\" connectingRoad=\""
          << connector << "\" contactPoint=\"start\"><laneLink from=\"" << lane
          << "\" to=\"" << lane << "\"/></connection>";
    }
    xml << "</junction></OpenDRIVE>";
    return std::make_shared<carla::client::Map>("route-cost-fork", xml.str());
  }

  class RouteSearchCost : public testing::TestWithParam<std::pair<bool, double>> {};

  TEST_P(RouteSearchCost, public_trace_selects_the_shorter_sampled_route) {
    const bool left_hand = GetParam().first;
    const double resolution = GetParam().second;
    const auto map = RouteCostMap(left_hand);
    const int lane = left_hand ? 1 : -1;
    const auto origin = map->GetWaypointXODR(1, lane, 5);
    const auto destination = map->GetWaypointXODR(5, lane, 5);
    ASSERT_NE(origin, nullptr);
    ASSERT_NE(destination, nullptr);
    ASSERT_EQ(map->GetWaypointXODR(1, lane, 9)->GetNext(2).size(), 2u);
    carla::agents::navigation::GlobalRoutePlanner planner(map, resolution);
    const auto route = planner.TraceRoute(origin->GetTransform().location, destination->GetTransform().location);
    ASSERT_FALSE(route.empty());
    std::set<carla::road::RoadId> roads;
    for (const auto &item : route) {
      ASSERT_NE(item.first, nullptr);
      // A shared graph node can use another branch's co-located s=0 endpoint.
      // Check actual traversed road interiors rather than that boundary label.
      if (item.first->GetRoadId() != 2 || item.first->GetDistance() > 1e-6) {
        roads.insert(item.first->GetRoadId());
      }
    }
    EXPECT_EQ(roads.count(2), 0u);
    EXPECT_EQ(roads.count(3), 1u);
    EXPECT_EQ(roads.count(4), 1u);
  }

  INSTANTIATE_TEST_SUITE_P(
      native_lane_networks, RouteSearchCost,
      testing::Values(std::make_pair(false, 0.5), std::make_pair(true, 0.5),
                      std::make_pair(false, 2.0), std::make_pair(true, 2.0),
                      std::make_pair(false, 4.0), std::make_pair(true, 4.0)));

} // namespace
