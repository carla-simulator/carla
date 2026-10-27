// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "test.h"

#include "carla/client/Map.h"
#include "carla/client/Waypoint.h"

#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <tuple>

namespace {

  constexpr double pi = 3.14159265358979323846;
  constexpr double road_length = 100.0 * pi;
  using Parameters = std::tuple<bool, int, bool>;

  carla::SharedPtr<carla::client::Map> MakeMap(
      bool left_hand, int lane, bool closed = true, bool driving = true) {
    std::ostringstream xml;
    xml.imbue(std::locale::classic());
    xml << std::setprecision(17)
        << "<OpenDRIVE><header revMajor='1' revMinor='4'><geoReference>"
           "+proj=tmerc +lat_0=0 +lon_0=0 +datum=WGS84</geoReference></header>";
    for (int identity : {1, 2}) {
      const int other = 3 - identity;
      const auto side = lane < 0 ? "right" : "left";
      xml << "<road id='" << identity << "' length='" << road_length
          << "' junction='-1' rule='" << (left_hand ? "LHT" : "RHT") << "'><link>";
      if (closed || identity == 2) {
        xml << "<predecessor elementType='road' elementId='" << other << "' contactPoint='end'/>";
      }
      if (closed || identity == 1) {
        xml << "<successor elementType='road' elementId='" << other << "' contactPoint='start'/>";
      }
      xml << "</link><planView><geometry s='0' x='0' y='" << (identity == 1 ? 0 : 200)
          << "' hdg='" << (identity == 1 ? 0.0 : pi) << "' length='" << road_length
          << "'><arc curvature='.01'/></geometry></planView><lanes><laneSection s='0'>"
             "<center><lane id='0' type='none'/></center><" << side << "><lane id='" << lane
          << "' type='" << (driving ? "driving" : "shoulder") << "'><link><predecessor id='" << lane
          << "'/><successor id='" << lane << "'/></link><width sOffset='0' a='3.5' b='0' c='0' d='0'/>"
             "</lane></" << side << "></laneSection></lanes></road>";
    }
    xml << "</OpenDRIVE>";
    return carla::MakeShared<carla::client::Map>("two-road-ring", xml.str());
  }

  class DrivingLaneLoop : public testing::TestWithParam<Parameters> {
  protected:
    void Check(double distance, bool closed = true) {
      const auto [left_hand, lane, previous] = GetParam();
      const bool increasing = ((lane < 0) != left_hand) != previous;
      const auto map = MakeMap(left_hand, lane, closed);
      const auto start_road = closed || increasing ? 1u : 2u;
      const double station = increasing ? 300.0 : 5.0;
      const auto start = map->GetWaypointXODR(start_road, lane, static_cast<float>(station));
      ASSERT_NE(start, nullptr);
      const auto result = previous ? start->GetPrevious(distance) : start->GetNext(distance);
      ASSERT_EQ(result.size(), 1u);
      double expected = (start_road - 1u) * road_length + station + (increasing ? distance : -distance);
      if (closed) {
        expected = std::fmod(expected, 2.0 * road_length);
        if (expected < 0.0) {
          expected += 2.0 * road_length;
        }
      }
      const auto expected_road = expected < road_length ? 1u : 2u;
      const double expected_station = expected - (expected_road - 1u) * road_length;
      EXPECT_EQ(result.front()->GetRoadId(), expected_road);
      EXPECT_EQ(result.front()->GetLaneId(), lane);
      EXPECT_NEAR(result.front()->GetDistance(), expected_station, 1e-7);
      EXPECT_TRUE(std::isfinite(result.front()->GetTransform().location.x));
    }
  };

  TEST_P(DrivingLaneLoop, crossing_a_road_boundary_keeps_the_driving_path) {
    Check(20.0);
  }

  TEST_P(DrivingLaneLoop, multiple_laps_consume_the_requested_distance) {
    Check(1000.0);
  }

  TEST_P(DrivingLaneLoop, one_complete_lap_returns_to_the_same_station) {
    Check(2.0 * road_length);
  }

  TEST_P(DrivingLaneLoop, within_lane_control) {
    Check(1.0);
  }

  TEST_P(DrivingLaneLoop, open_chain_control) {
    Check(20.0, false);
  }

  TEST_P(DrivingLaneLoop, legacy_shoulder_reciprocal_filter_control) {
    const auto [left_hand, lane, previous] = GetParam();
    const bool increasing = ((lane < 0) != left_hand) != previous;
    const auto map = MakeMap(left_hand, lane, true, false);
    const auto start = map->GetWaypointXODR(1u, lane, increasing ? 300.0f : 5.0f);
    ASSERT_NE(start, nullptr);
    const auto result = previous ? start->GetPrevious(20.0) : start->GetNext(20.0);
    EXPECT_TRUE(result.empty());
  }

  INSTANTIATE_TEST_SUITE_P(
      traffic_rules_lane_signs_and_query_direction, DrivingLaneLoop,
      testing::Combine(testing::Bool(), testing::Values(-1, 1), testing::Bool()));

} // namespace
