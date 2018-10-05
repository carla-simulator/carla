/*
    Project includes
*/
#include "ProfilesParser.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

void opendrive::parser::ProfilesParser::ParseElevation(const pugi::xml_node & xmlNode, std::vector<opendrive::types::ElevationProfile>& out_elevation_profile)
{
    for (pugi::xml_node laneSection = xmlNode.child("elevation"); laneSection; laneSection = laneSection.next_sibling("elevation"))
    {
        opendrive::types::ElevationProfile elevationProfile;

        elevationProfile.start_position = std::stod(laneSection.attribute("s").value());
        elevationProfile.elevation = std::stod(laneSection.attribute("a").value());
        elevationProfile.slope = std::stod(laneSection.attribute("b").value());
        elevationProfile.vertical_curvature = std::stod(laneSection.attribute("c").value());
        elevationProfile.curvature_change = std::stod(laneSection.attribute("d").value());

        out_elevation_profile.push_back(elevationProfile);
    }
}

void opendrive::parser::ProfilesParser::ParseLateral(const pugi::xml_node & xmlNode, std::vector<opendrive::types::LateralProfile> & out_lateral_profile)
{
    for (pugi::xml_node laneSection = xmlNode.child("superelevation"); laneSection; laneSection = laneSection.next_sibling("superelevation"))
    {
        opendrive::types::LateralProfile lateralProfile;

        lateralProfile.start_position = std::stod(laneSection.attribute("s").value());
        lateralProfile.elevation = std::stod(laneSection.attribute("a").value());
        lateralProfile.slope = std::stod(laneSection.attribute("b").value());
        lateralProfile.vertical_curvature = std::stod(laneSection.attribute("c").value());
        lateralProfile.curvature_change = std::stod(laneSection.attribute("d").value());

        out_lateral_profile.push_back(lateralProfile);
    }
}

void opendrive::parser::ProfilesParser::Parse(const pugi::xml_node & xmlNode, opendrive::types::RoadProfiles & out_road_profiles)
{
    opendrive::parser::ProfilesParser profilesParser;

    profilesParser.ParseElevation(xmlNode.child("elevationProfile"), out_road_profiles.elevation_profile);
    profilesParser.ParseLateral(xmlNode.child("lateralProfile"), out_road_profiles.lateral_profile);
}

