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
            class SpiralPointsGenerator
            {
                private:
                    const carla::opendrive::types::GeometryAttributesSpiral *geometry_attributes;
                    const carla::opendrive::types::RoadProfiles *road_profile;

                    double end_X, end_Y;
                    double start_X, start_Y;

                    bool inverse;
                    double curvature, a, m_a;
                    double rot_angle_cos, rot_angle_sin;

                public:
                    SpiralPointsGenerator() : geometry_attributes(nullptr), road_profile(nullptr)
                    {
                    }

                    inline void SetGeometry(const carla::opendrive::types::GeometryAttributesSpiral *geometryAttributes)
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
