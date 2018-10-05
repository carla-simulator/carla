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
        class ProfilesParser
        {
            private:
                void ParseElevation(const pugi::xml_node & xmlNode, std::vector<opendrive::types::ElevationProfile> & out_elevation_profile);
                void ParseLateral(const pugi::xml_node & xmlNode, std::vector<opendrive::types::LateralProfile> & out_lateral_profile);

            public:
                static void Parse(const pugi::xml_node & xmlNode, opendrive::types::RoadProfiles & out_road_profiles);
        };
    }
}
