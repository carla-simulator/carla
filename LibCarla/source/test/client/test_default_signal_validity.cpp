// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB). Licensed under the MIT license.

#include "test.h"
#include <carla/client/Map.h>
#include <carla/client/Waypoint.h>
#include <carla/client/Landmark.h>
#include <sstream>
#include <tuple>

namespace {
std::string Road(const std::string &rule, int id, const std::string &orientation,
                 bool explicit_validity=false, int only_sign=0, bool reference=false) {
  std::ostringstream xml;
  xml << "<road id=\"" << id << "\" name=\"signals\" length=\"20\" junction=\"-1\" rule=\"" << rule
      << R"("><planView><geometry s="0" x="0" y=")" << 30*(id-1)
      << R"(" hdg="0" length="20"><line/></geometry></planView><lanes><laneSection s="0">)";
  const auto lane=[&](int i) {
    xml << "<lane id=\"" << i << R"(" type="driving"><width sOffset="0" a="3.5" b="0" c="0" d="0"/></lane>)";
  };
  if (only_sign>=0) { xml << "<left>"; lane(2);lane(1);xml << "</left>"; }
  xml << R"(<center><lane id="0" type="none"/></center>)";
  if (only_sign<=0) { xml << "<right>";lane(-1);lane(-2);xml << "</right>"; }
  xml << "</laneSection></lanes><signals><" << (reference ? "signalReference" : "signal")
      << R"( id="sign" s="10" t="-3.5" orientation=")" << orientation << '"';
  if (!reference) xml << R"( name="test" dynamic="no" zOffset="0" country="DE" type="274" subtype="0" value="50" unit="km/h" height="1" width="1")";
  xml << '>';
  if (explicit_validity) {
    const int lane_id = rule=="RHT" ? -2 : 2;
    xml << "<validity fromLane=\"" << lane_id << "\" toLane=\"" << lane_id << "\"/>";
  }
  xml << "</" << (reference ? "signalReference" : "signal") << "></signals></road>";
  return xml.str();
}

std::string MapXml(const std::string &roads) {
  return "<OpenDRIVE><header revMajor=\"1\" revMinor=\"4\"/>"+roads+"</OpenDRIVE>";
}

bool Seen(const carla::SharedPtr<carla::client::Map> &map, unsigned road, int lane) {
  const auto waypoint=map->GetWaypointXODR(road,lane,5.0);
  const auto start = map->GetWaypointXODR(
      road, lane, static_cast<float>(waypoint->IsPositiveDirection() ? 1.0 : 19.0));
  return !start->GetAllLandmarksInDistance(15.0,false).empty();
}

class DefaultSignalValidity : public ::testing::TestWithParam<std::tuple<std::string,std::string,int>> {};

TEST_P(DefaultSignalValidity, applies_to_matching_lane_travel_direction) {
  const auto &[rule,orientation,lane]=GetParam();
  auto map=carla::MakeShared<carla::client::Map>("signals",MapXml(Road(rule,1,orientation)));
  const bool positive=(rule=="RHT")== (lane<0);
  const bool expected=orientation=="none" || ((orientation=="+")==positive);
  EXPECT_EQ(Seen(map,1u,lane),expected);
}

INSTANTIATE_TEST_SUITE_P(
    rules_orientations_and_lane_sides,DefaultSignalValidity,
    ::testing::Combine(::testing::Values(std::string("RHT"),std::string("LHT")),
                       ::testing::Values(std::string("+"),std::string("-"),std::string("none")),
                       ::testing::Values(-1,1)));

TEST(signal_validity_controls, explicit_validity_remains_restricted_to_its_lane) {
  for (const auto &rule:{std::string("RHT"),std::string("LHT")}) {
    auto map=carla::MakeShared<carla::client::Map>("explicit",MapXml(Road(rule,1,"+",true)));
    const int sign=rule=="RHT"?-1:1;
    EXPECT_FALSE(Seen(map,1u,sign));
    EXPECT_TRUE(Seen(map,1u,2*sign));
    EXPECT_FALSE(Seen(map,1u,-sign));
  }
}

TEST(signal_validity_controls, reference_uses_its_own_road_traffic_rule) {
  for (const auto &first_rule:{std::string("RHT"),std::string("LHT")}) {
    const std::string second_rule=first_rule=="RHT"?"LHT":"RHT";
    auto map=carla::MakeShared<carla::client::Map>("references",MapXml(
        Road(first_rule,1,"+")+Road(second_rule,2,"+",false,0,true)));
    for (int lane:{-1,1}) {
      EXPECT_EQ(Seen(map,1u,lane),(first_rule=="RHT")== (lane<0));
      EXPECT_EQ(Seen(map,2u,lane),(second_rule=="RHT")== (lane<0));
    }
  }
}

TEST(signal_validity_controls, one_way_road_matches_direction_and_rejects_the_other) {
  for (const auto &rule:{std::string("RHT"),std::string("LHT")}) {
    for (int sign:{-1,1}) {
      const bool positive=(rule=="RHT")== (sign<0);
      auto matching=carla::MakeShared<carla::client::Map>("one-way",MapXml(Road(rule,1,positive?"+":"-",false,sign)));
      auto opposing=carla::MakeShared<carla::client::Map>("one-way",MapXml(Road(rule,1,positive?"-":"+",false,sign)));
      EXPECT_TRUE(Seen(matching,1u,sign));
      EXPECT_FALSE(Seen(opposing,1u,sign));
    }
  }
}
}
