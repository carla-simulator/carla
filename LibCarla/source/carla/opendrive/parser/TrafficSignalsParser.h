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
            class TrafficSignalsParser
            {
                private:

                public:
                    static void Parse(const pugi::xml_node & xmlNode, std::vector<carla::opendrive::types::TrafficSignalInformation> & out_traffic_signals);
            };
        }
    }
}
