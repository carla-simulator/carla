#include "test.h"
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <sstream>
#include <tuple>

namespace {
using Change = carla::road::element::LaneMarking::LaneChange;
using Parameters = std::tuple<std::string, int, bool, std::string>;

std::string PermissionMap(const std::string &rule, int current, bool outer, const std::string &permission) {
  const int target = std::abs(current) - (outer ? 0 : 1);
  std::ostringstream xml;
  xml << R"(<OpenDRIVE><header revMajor="1" revMinor="4"/><road id="1" name="permissions" length="10" junction="-1" rule=")"
      << rule << R"("><planView><geometry s="0" x="0" y="0" hdg="0" length="10"><line/></geometry></planView><lanes><laneSection s="0">)";
  const auto lane = [&](int id) {
    xml << "<lane id=\"" << id << "\" type=\"" << (id == 0 ? "none" : "driving") << "\">";
    if (id != 0) xml << R"(<width sOffset="0" a="3.5" b="0" c="0" d="0"/>)";
    xml << R"(<roadMark sOffset="0" type="broken" color="white" laneChange=")"
        << (std::abs(id) == target ? permission : "none") << R"("/></lane>)";
  };
  xml << "<left>"; for (int id : {3, 2, 1}) lane(id); xml << "</left><center>";
  lane(0);
  xml << "</center><right>"; for (int id : {-1,-2,-3}) lane(id); xml << "</right>";
  xml << "</laneSection></lanes></road></OpenDRIVE>";
  return xml.str();
}

class LaneChangePermissions : public ::testing::TestWithParam<Parameters> {};

TEST_P(LaneChangePermissions, maps_opendrive_id_direction_to_driver_side) {
  const auto &[rule, lane_id, outer, permission] = GetParam();
  auto map = carla::MakeShared<carla::client::Map>("permissions", PermissionMap(rule, lane_id, outer, permission));
  const auto waypoint = map->GetWaypointXODR(1u, lane_id, 5.0);
  ASSERT_NE(waypoint, nullptr);
  // ASAM OpenDRIVE: laneChange increase/decrease refers to signed lane IDs,
  // numbered from right to left, independently of the traffic rule.
  const int signed_delta = outer ? (lane_id < 0 ? -1 : 1) : (lane_id < 0 ? 1 : -1);
  const bool permitted = permission == "both" ||
      permission == (signed_delta > 0 ? "increase" : "decrease");
  const bool to_right = outer == (rule == "RHT");
  const auto expected = permitted ? (to_right ? Change::Right : Change::Left) : Change::None;
  EXPECT_EQ(waypoint->GetLaneChange(), expected);
}

INSTANTIATE_TEST_SUITE_P(
    traffic_rules_lane_signs_borders_and_permissions,
    LaneChangePermissions,
    ::testing::Combine(::testing::Values(std::string("RHT"),std::string("LHT")),
                       ::testing::Values(-2,-1,1,2),::testing::Bool(),
                       ::testing::Values(std::string("increase"),std::string("decrease"),
                                         std::string("both"),std::string("none"))));
}
