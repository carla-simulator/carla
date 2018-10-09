/*
    Project includes
*/
#include "JunctionParser.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

void carla::opendrive::parser::JunctionParser::Parse(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::Junction> & out_junction)
{
    carla::opendrive::parser::JunctionParser parser;
    carla::opendrive::types::Junction junction;

    junction.attributes.id = std::atoi(xmlNode.attribute("id").value());
    junction.attributes.name = xmlNode.attribute("name").value();

    parser.ParseConnection(xmlNode, junction.connections);
    out_junction.push_back(junction);
}

void carla::opendrive::parser::JunctionParser::ParseConnection(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::JunctionConnection> & out_connections)
{
    for (pugi::xml_node junctionConnection = xmlNode.child("connection"); junctionConnection; junctionConnection = junctionConnection.next_sibling("connection"))
    {
        carla::opendrive::types::JunctionConnection jConnection;

        jConnection.attributes.id = std::atoi(junctionConnection.attribute("id").value());
        jConnection.attributes.contact_point = junctionConnection.attribute("contactPoint").value();

        jConnection.attributes.incoming_road = std::atoi(junctionConnection.attribute("incomingRoad").value());
        jConnection.attributes.connecting_road = std::atoi(junctionConnection.attribute("connectingRoad").value());

        ParseLaneLink(junctionConnection, jConnection.links);
        out_connections.push_back(jConnection);
    }
}

void carla::opendrive::parser::JunctionParser::ParseLaneLink(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::JunctionLaneLink> & out_lane_link)
{
    for (pugi::xml_node junctionLaneLink = xmlNode.child("laneLink"); junctionLaneLink; junctionLaneLink = junctionLaneLink.next_sibling("laneLink"))
    {
        carla::opendrive::types::JunctionLaneLink jLaneLink;

        jLaneLink.from = std::atoi(junctionLaneLink.attribute("from").value());
        jLaneLink.to = std::atoi(junctionLaneLink.attribute("to").value());

        out_lane_link.push_back(jLaneLink);
    }
}
