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
                    void ParseLane(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneInfo> & out_lane);
                    void ParseLaneSpeed(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneSpeed> & out_lane_speed);

                    void ParseLaneLink(const pugi::xml_node & xmlNode, std::unique_ptr<carla::opendrive::types::LaneLink> & out_lane_link);
                    void ParseLaneOffset(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneOffset> & out_lane_offset);

                    void ParseLaneWidth(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneWidth> & out_lane_width);
                    void ParseLaneRoadMark(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::LaneRoadMark> & out_lane_mark);

                public:
                    static void Parse(const pugi::xml_node & xmlNode, carla::opendrive::types::Lanes & out_lanes);
            };
        }
    }
}
