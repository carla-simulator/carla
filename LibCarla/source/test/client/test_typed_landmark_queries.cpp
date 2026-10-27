// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include <carla/client/Landmark.h>
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>

#include <cmath>
#include <sstream>
#include <string>
#include <tuple>

namespace {

  std::string LaneSection(double s, int lane, bool first, bool linked) {
    const auto side = lane < 0 ? "right" : "left";
    std::ostringstream xml;
    xml << "<laneSection s='" << s << "'><center><lane id='0' type='none' level='false'/></center><"
        << side << "><lane id='" << lane << "' type='driving' level='false'><link>";
    if (linked) {
      if (first) {
        xml << "<successor id='" << lane << "'/>";
      } else {
        xml << "<predecessor id='" << lane << "'/>";
      }
    }
    xml << "</link><width sOffset='0' a='3.5' b='0' c='0' d='0'/></lane></"
        << side << "></laneSection>";
    return xml.str();
  }

  std::string Signal(const std::string &id, const std::string &type, double s, int lane) {
    std::ostringstream xml;
    xml << "<signal s='" << s << "' t='0' id='" << id << "' name='sign' dynamic='no' orientation='none' "
           "zOffset='0' country='DE' type='" << type << "' subtype='0' value='30' unit='km/h' height='1' width='1'>"
        << "<validity fromLane='" << lane << "' toLane='" << lane << "'/></signal>";
    return xml.str();
  }

  carla::SharedPtr<carla::client::Map> MakeMap(bool rht, int lane, bool split, const std::string &signals) {
    std::ostringstream xml;
    xml << "<OpenDRIVE><header revMajor='1' revMinor='4' name='landmarks' version='1' date='' "
           "north='0' south='0' east='0' west='0'><geoReference>+proj=tmerc +lat_0=0 +lon_0=0 "
           "+k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs</geoReference></header>"
        << "<road name='road' length='100' id='1' junction='-1' rule='" << (rht ? "RHT" : "LHT") << "'>"
           "<link/><planView><geometry s='0' x='0' y='0' hdg='0' length='100'><line/></geometry></planView>"
           "<elevationProfile><elevation s='0' a='0' b='0' c='0' d='0'/></elevationProfile><lanes>"
        << LaneSection(0.0, lane, true, split);
    if (split) {
      xml << LaneSection(50.0, lane, false, true);
    }
    xml << "</lanes><signals>" << signals << "</signals></road></OpenDRIVE>";
    return carla::MakeShared<carla::client::Map>("landmarks", xml.str());
  }

  using QueryCase = std::tuple<bool, int, bool, double>;

  class TypedLandmarkQuery : public testing::TestWithParam<QueryCase> {};

  TEST_P(TypedLandmarkQuery, each_reference_is_returned_once) {
    const auto [rht, lane, split, signal_s] = GetParam();
    const bool increasing_s = rht == (lane < 0);
    const auto map = MakeMap(rht, lane, split, Signal("sign", "274", signal_s, lane));
    const auto start = map->GetWaypointXODR(1u, lane, increasing_s ? 10.0f : 90.0f);
    ASSERT_NE(start, nullptr);
    const auto all = start->GetAllLandmarksInDistance(80.0);
    const auto typed = start->GetLandmarksOfTypeInDistance(80.0, "274");
    ASSERT_EQ(all.size(), 1u);
    ASSERT_EQ(typed.size(), 1u);
    EXPECT_EQ(typed.front()->GetId(), all.front()->GetId());
    EXPECT_EQ(typed.front()->GetType(), "274");
    EXPECT_DOUBLE_EQ(typed.front()->GetDistance(), std::abs(signal_s - start->GetDistance()));
    EXPECT_DOUBLE_EQ(typed.front()->GetDistance(), all.front()->GetDistance());
  }

  INSTANTIATE_TEST_SUITE_P(
      RoadRulesAndSections, TypedLandmarkQuery,
      testing::Combine(testing::Bool(), testing::Values(-1, 1), testing::Bool(), testing::Values(25.0, 50.0, 75.0)));

  TEST(typed_landmarks, distinct_signals_of_the_same_type_are_preserved) {
    const auto map = MakeMap(true, -1, true, Signal("first", "274", 25.0, -1) + Signal("second", "274", 75.0, -1));
    const auto start = map->GetWaypointXODR(1u, -1, 10.0f);
    ASSERT_NE(start, nullptr);
    const auto result = start->GetLandmarksOfTypeInDistance(80.0, "274");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0]->GetId(), "first");
    EXPECT_EQ(result[1]->GetId(), "second");
    EXPECT_DOUBLE_EQ(result[0]->GetDistance(), 15.0);
    EXPECT_DOUBLE_EQ(result[1]->GetDistance(), 65.0);
  }

  TEST(typed_landmarks, other_types_and_out_of_range_signals_are_excluded) {
    const auto map = MakeMap(true, -1, true, Signal("other", "206", 50.0, -1) + Signal("far", "274", 75.0, -1));
    const auto start = map->GetWaypointXODR(1u, -1, 10.0f);
    ASSERT_NE(start, nullptr);
    EXPECT_TRUE(start->GetLandmarksOfTypeInDistance(50.0, "274").empty());
    EXPECT_TRUE(start->GetLandmarksOfTypeInDistance(80.0, "missing").empty());
    const auto result = start->GetLandmarksOfTypeInDistance(80.0, "274");
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result.front()->GetId(), "far");
  }

} // namespace
