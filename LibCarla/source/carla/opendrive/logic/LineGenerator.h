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
            class LinePointsGenerator
            {
                private:
                    const carla::opendrive::types::GeometryAttributesLine *geometry_attributes;
                    const carla::opendrive::types::RoadProfiles *road_profile;

                    math::vector::Vector3<double> orientation_vector;
                    math::vector::Vector3<double> distance_vector;

                    math::vector::Vector3<double> start_point;
                    math::vector::Vector3<double> end_point;

                public:
                    LinePointsGenerator() : geometry_attributes(nullptr), road_profile(nullptr)
                    {
                    }

                    inline void SetGeometry(const carla::opendrive::types::GeometryAttributesLine *geometryAttributes)
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
