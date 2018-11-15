// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "ProfilesParser.h"

void carla::opendrive::parser::ProfilesParser::ParseElevation(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::ElevationProfile> &out_elevation_profile) {
  for (pugi::xml_node laneSection = xmlNode.child("elevation");
      laneSection;
      laneSection = laneSection.next_sibling("elevation")) {
    carla::opendrive::types::ElevationProfile elevationProfile;

    elevationProfile.start_position = std::stod(laneSection.attribute("s").value());
    elevationProfile.elevation = std::stod(laneSection.attribute("a").value());
    elevationProfile.slope = std::stod(laneSection.attribute("b").value());
    elevationProfile.vertical_curvature = std::stod(laneSection.attribute("c").value());
    elevationProfile.curvature_change = std::stod(laneSection.attribute("d").value());

    out_elevation_profile.emplace_back(elevationProfile);
  }
}

void carla::opendrive::parser::ProfilesParser::ParseLateral(
    const pugi::xml_node &xmlNode,
    std::vector<carla::opendrive::types::LateralProfile> &out_lateral_profile) {
  for (pugi::xml_node laneSection = xmlNode.child("superelevation");
      laneSection;
      laneSection = laneSection.next_sibling("superelevation")) {
    carla::opendrive::types::LateralProfile lateralProfile;

    lateralProfile.start_position = std::stod(laneSection.attribute("s").value());
    lateralProfile.elevation = std::stod(laneSection.attribute("a").value());
    lateralProfile.slope = std::stod(laneSection.attribute("b").value());
    lateralProfile.vertical_curvature = std::stod(laneSection.attribute("c").value());
    lateralProfile.curvature_change = std::stod(laneSection.attribute("d").value());

    out_lateral_profile.emplace_back(lateralProfile);
  }
}

void carla::opendrive::parser::ProfilesParser::Parse(
    const pugi::xml_node &xmlNode,
    carla::opendrive::types::RoadProfiles &out_road_profiles) {
  carla::opendrive::parser::ProfilesParser profilesParser;

  profilesParser.ParseElevation(xmlNode.child("elevationProfile"), out_road_profiles.elevation_profile);
  profilesParser.ParseLateral(xmlNode.child("lateralProfile"), out_road_profiles.lateral_profile);
}
