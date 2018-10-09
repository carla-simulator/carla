#pragma once
/*
    Project includes
*/
#include "../types.h"

/*
    Lib includes
*/
#include "./pugixml/pugixml.hpp"

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

namespace carla
{
    namespace opendrive
    {
        namespace parser
        {
            class LaneParser
            {
                private:
                    void ParseLane(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::Lane> & out_lane);
                    void ParseLaneSpeed(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneSpeed> & out_lane_speed);

                    void ParseLaneLink(const pugi::xml_node & xmlNode, carla::opendrive::types::LaneLink ** out_lane_link);
                    void ParseLaneWidth(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneWidth> & out_lane_width);
                    void ParseLaneRoadMark(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneRoadMark> & out_lane_mark);

                public:
                    static void Parse(const pugi::xml_node & xmlNode, carla::opendrive::types::LaneSection & out_lane_section);
            };
        }
    }
}
