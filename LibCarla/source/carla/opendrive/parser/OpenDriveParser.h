/*
    Project includes
*/
#include "TrafficSignalsParser.h"
#include "RoadLinkParser.h"
#include "JunctionParser.h"

#include "ProfilesParser.h"
#include "GeometryParser.h"
#include "LaneParser.h"

/*
    Lib includes
*/
#include "./pugixml/pugixml.hpp"

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

struct OpenDriveParser
{
    static bool Parse(const char *OpenDriveXMLFile, carla::opendrive::types::OpenDriveData &out_open_drive_data)
    {
        pugi::xml_document xmlDoc;
        pugi::xml_parse_result pugiParseResult = xmlDoc.load_file(OpenDriveXMLFile);

        if (pugiParseResult == false)
        {
            return false;
        }

        for (pugi::xml_node road = xmlDoc.child("OpenDRIVE").child("road"); road; road = road.next_sibling("road"))
        {
            carla::opendrive::types::RoadInformation openDriveRoadInformation;

            openDriveRoadInformation.attributes.name = road.attribute("name").value();
            openDriveRoadInformation.attributes.id = std::atoi(road.attribute("id").value());
            openDriveRoadInformation.attributes.length = std::stod(road.attribute("length").value());
            openDriveRoadInformation.attributes.junction = std::atoi(road.attribute("junction").value());

            ///////////////////////////////////////////////////////////////////////////////

            carla::opendrive::parser::ProfilesParser::Parse(road, openDriveRoadInformation.road_profiles);

            carla::opendrive::parser::RoadLinkParser::Parse(road.child("link"), openDriveRoadInformation.road_link);
            carla::opendrive::parser::TrafficSignalsParser::Parse(road.child("signals"), openDriveRoadInformation.trafic_signals);

            carla::opendrive::parser::LaneParser::Parse(road.child("lanes"), openDriveRoadInformation.lane_sections);
            carla::opendrive::parser::GeometryParser::Parse(road.child("planView"), openDriveRoadInformation.geometry_attributes);

            out_open_drive_data.roads.push_back(openDriveRoadInformation);
        }

        for (pugi::xml_node junction = xmlDoc.child("OpenDRIVE").child("junction"); junction; junction = junction.next_sibling("junction"))
        {
            carla::opendrive::parser::JunctionParser::Parse(junction, out_open_drive_data.junctions);
        }

        return true;
    }
};