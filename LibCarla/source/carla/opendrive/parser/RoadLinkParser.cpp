/*
    Project includes
*/
#include "RoadLinkParser.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

void opendrive::parser::RoadLinkParser::ParseLink(const pugi::xml_node & xmlNode, opendrive::types::RoadLinkInformation ** out_link_information)
{
    (*out_link_information) = new opendrive::types::RoadLinkInformation;

    (*out_link_information)->id = std::atoi(xmlNode.attribute("elementId").value());
    (*out_link_information)->contact_point = xmlNode.attribute("contactPoint").value();
    (*out_link_information)->element_type = xmlNode.attribute("elementType").value();
}

void opendrive::parser::RoadLinkParser::Parse(const pugi::xml_node & xmlNode, opendrive::types::RoadLink & out_road_link)
{
    out_road_link.predecessor = nullptr;
    out_road_link.successor = nullptr;
    RoadLinkParser parser;

    const pugi::xml_node predecessorNode = xmlNode.child("predecessor");
    const pugi::xml_node successorNode = xmlNode.child("successor");

    if (predecessorNode)
    {
        parser.ParseLink(predecessorNode, &out_road_link.predecessor);
    }

    if (successorNode)
    {
        parser.ParseLink(successorNode, &out_road_link.successor);
    }
}
