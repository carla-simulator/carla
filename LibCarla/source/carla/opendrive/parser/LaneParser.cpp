/*
    Project includes
*/
#include "LaneParser.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

void carla::opendrive::parser::LaneParser::ParseLane(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::Lane> & out_lane)
{
    for (pugi::xml_node lane = xmlNode.child("lane"); lane; lane = lane.next_sibling("lane"))
    {
        carla::opendrive::types::Lane currentLane;

        currentLane.attributes.type = lane.attribute("type").value();
        currentLane.attributes.level = lane.attribute("level").value();
        currentLane.attributes.id = std::atoi(lane.attribute("id").value());

        ParseLaneSpeed(lane, currentLane.lane_speed);
        ParseLaneWidth(lane, currentLane.lane_width);

        ParseLaneLink(lane.child("link"), &currentLane.link);
        ParseLaneRoadMark(lane.child("roadMark"), currentLane.road_marker);

        out_lane.push_back(currentLane);
    }
}

void carla::opendrive::parser::LaneParser::ParseLaneWidth(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneWidth> & out_lane_width)
{
    for (pugi::xml_node laneWidth = xmlNode.child("width"); laneWidth; laneWidth = laneWidth.next_sibling("width"))
    {
        carla::opendrive::types::LaneWidth laneWidthInfo;

        laneWidthInfo.soffset = std::stod(laneWidth.attribute("sOffset").value());

        laneWidthInfo.width = std::stod(laneWidth.attribute("a").value());
        laneWidthInfo.slope = std::stod(laneWidth.attribute("b").value());

        laneWidthInfo.vertical_curvature = std::stod(laneWidth.attribute("c").value());
        laneWidthInfo.curvature_change = std::stod(laneWidth.attribute("d").value());

        out_lane_width.push_back(laneWidthInfo);
    }
}

void carla::opendrive::parser::LaneParser::ParseLaneLink(const pugi::xml_node & xmlNode, carla::opendrive::types::LaneLink ** out_lane_link)
{
    const pugi::xml_node predecessorNode = xmlNode.child("predecessor");
    const pugi::xml_node successorNode = xmlNode.child("successor");

    (*out_lane_link) = (predecessorNode || successorNode) ? new opendrive::types::LaneLink : nullptr;
    if (*out_lane_link == nullptr) return;

    (*out_lane_link)->predecessor_id = predecessorNode ? std::atoi(predecessorNode.attribute("id").value()) : 0;
    (*out_lane_link)->successor_id = successorNode ? std::atoi(successorNode.attribute("id").value()) : 0;
}

void carla::opendrive::parser::LaneParser::ParseLaneRoadMark(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneRoadMark> & out_lane_mark)
{
    if (xmlNode == nullptr) return;
    carla::opendrive::types::LaneRoadMark roadMarker;

    roadMarker.soffset = std::stod(xmlNode.attribute("sOffset").value());
    roadMarker.width = std::stod(xmlNode.attribute("width").value());

    roadMarker.type = xmlNode.attribute("type").value();
    roadMarker.weigth = xmlNode.attribute("weight").value();

    roadMarker.color = xmlNode.attribute("color").value();
    roadMarker.lange_change = xmlNode.attribute("laneChange").value();

    out_lane_mark.push_back(roadMarker);
}

void carla::opendrive::parser::LaneParser::ParseLaneSpeed(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneSpeed>& out_lane_speed)
{
    for (pugi::xml_node laneSpeed = xmlNode.child("speed"); laneSpeed; laneSpeed = laneSpeed.next_sibling("speed"))
    {
        carla::opendrive::types::LaneSpeed lane_speed = { 0.0, 0.0 };

        lane_speed.soffset = std::stod(laneSpeed.attribute("sOffset").value());
        lane_speed.max_speed = std::stod(laneSpeed.attribute("max").value());

        out_lane_speed.push_back(lane_speed);
    }
}

void carla::opendrive::parser::LaneParser::Parse(const pugi::xml_node & xmlNode, carla::opendrive::types::LaneSection & out_lane_section)
{
    carla::opendrive::parser::LaneParser laneParser;

    for (pugi::xml_node laneSection = xmlNode.child("laneSection"); laneSection; laneSection = laneSection.next_sibling("laneSection"))
    {
        out_lane_section.start_position = std::stod(laneSection.attribute("s").value());

        pugi::xml_node lane = laneSection.child("left");
        laneParser.ParseLane(lane, out_lane_section.left);

        lane = laneSection.child("center");
        laneParser.ParseLane(lane, out_lane_section.center);

        lane = laneSection.child("right");
        laneParser.ParseLane(lane, out_lane_section.right);
    }
}
