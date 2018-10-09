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

namespace carla
{
    namespace opendrive
    {
        namespace logic
        {
            class ArcPointsGenerator
            {
                private:
                    const carla::opendrive::types::GeometryAttributesArc *geometry_attributes;
                    const carla::opendrive::types::RoadProfiles *road_profile;

                    double start_angle, arc_radius;
                    double start_X, start_Y;

                public:
                    ArcPointsGenerator() : geometry_attributes(nullptr), road_profile(nullptr)
                    {
                    }

                    inline void SetGeometry(const carla::opendrive::types::GeometryAttributesArc *geometryAttributes)
                    {
                        geometry_attributes = geometryAttributes;
                    }

                    inline void SetRoadProfile(const carla::opendrive::types::RoadProfiles *roadProfile)
                    {
                        road_profile = roadProfile;
                    }

                    void Init();
                    void GeneratePoint(double sOffset, carla::opendrive::types::Waypoint &outWaypoint);
            };
        }
    }
}
