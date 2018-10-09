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
            class GeometryParser
            {
                private:
                    void ParseArc(const pugi::xml_node &xmlNode, carla::opendrive::types::GeometryAttributesArc *out_geometry_arc);
                    void ParseLine(const pugi::xml_node &xmlNode, carla::opendrive::types::GeometryAttributesLine *out_geometry_line);
                    void ParseSpiral(const pugi::xml_node &xmlNode, carla::opendrive::types::GeometryAttributesSpiral *out_geometry_spiral);

                public:
                    static void Parse(const pugi::xml_node &xmlNode, std::vector<carla::opendrive::types::GeometryAttributes *> &out_geometry_attributes);
            };
        }
    }
}
