#pragma once
/*
    Project includes
*/
#include "../types.h"
#include "../math/Vector3.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

namespace opendrive
{
    namespace logic
    {
        class ArcPointsGenerator
        {
            private:
                const opendrive::types::GeometryAttributesArc *geometry_attributes;
                const opendrive::types::RoadProfiles *road_profile;

                double start_angle, arc_radius;
                double start_X, start_Y;

            public:
                ArcPointsGenerator() : geometry_attributes(nullptr), road_profile(nullptr)
                {
                }

                inline void SetGeometry(const opendrive::types::GeometryAttributesArc *geometryAttributes)
                {
                    geometry_attributes = geometryAttributes;
                }

                inline void SetRoadProfile(const opendrive::types::RoadProfiles *roadProfile)
                {
                    road_profile = roadProfile;
                }

                void Init();
                void GeneratePoint(double sOffset, opendrive::types::Waypoint &outWaypoint);
        };
    }
}

