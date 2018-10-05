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

namespace opendrive
{
    namespace parser
    {
        class RoadLinkParser
        {
            private:
                void ParseLink(const pugi::xml_node & xmlNode, opendrive::types::RoadLinkInformation ** out_link_information);

            public:
                static void Parse(const pugi::xml_node & xmlNode, opendrive::types::RoadLink & out_road_link);
        };
    }
}
