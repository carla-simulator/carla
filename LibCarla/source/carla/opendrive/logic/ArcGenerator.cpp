/*
    Project includes
*/
#include "ArcGenerator.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/
#include <cmath>

/*
    Defines and constants
*/
static const double FULL_PI_LOCAL = 3.14159265358979323846;
static const double HALF_PI_LOCAL = 1.57079632679489661923;

///////////////////////////////////////////////////////////////////////////////

void carla::opendrive::logic::ArcPointsGenerator::Init()
{
    start_angle = 0.0;
    arc_radius = std::abs(1.0 / geometry_attributes->curvature);

    if (geometry_attributes->curvature > 0.0)
    {
        start_angle = geometry_attributes->heading - HALF_PI_LOCAL;
    }
    else
    {
        start_angle = geometry_attributes->heading + HALF_PI_LOCAL;
    }

    start_X = geometry_attributes->start_position_x + std::cos(start_angle - FULL_PI_LOCAL) * arc_radius;
    start_Y = geometry_attributes->start_position_y + std::sin(start_angle - FULL_PI_LOCAL) * arc_radius;
}

void carla::opendrive::logic::ArcPointsGenerator::GeneratePoint(double sOffset, carla::opendrive::types::Waypoint & outWaypoint)
{
    int elevationIndex = road_profile->elevation_profile.size() ? 0 : -1;

    while (elevationIndex >= 0 && geometry_attributes->start_position < road_profile->elevation_profile[elevationIndex].start_position)
    {
        ++elevationIndex;
    }

    double stepAngle = sOffset / (1.0 / geometry_attributes->curvature);
    math::vector::Vector3<double> currentPos = { 0.0, 0.0, 0.0 };

    currentPos.x = start_X + std::cos(start_angle + stepAngle) * arc_radius;
    currentPos.y = start_Y + std::sin(start_angle + stepAngle) * arc_radius;

    if (elevationIndex >= 0 && (elevationIndex + 1) < (int)road_profile->elevation_profile.size() && (sOffset + geometry_attributes->start_position) >= road_profile->elevation_profile[elevationIndex + 1].start_position)
    {
        ++elevationIndex;
    }

    if (elevationIndex >= 0)
    {
        currentPos.z = road_profile->elevation_profile[elevationIndex].elevation
            + sOffset * road_profile->elevation_profile[elevationIndex].slope
            + sOffset * sOffset * road_profile->elevation_profile[elevationIndex].vertical_curvature
            + sOffset * sOffset * sOffset * road_profile->elevation_profile[elevationIndex].curvature_change;
    }

    outWaypoint.distance_in_section = geometry_attributes->start_position + sOffset;
    outWaypoint.heading = (start_angle + stepAngle) + (geometry_attributes->curvature <= 0.0 ? -HALF_PI_LOCAL : HALF_PI_LOCAL);

    outWaypoint.x = currentPos.x;
    outWaypoint.y = currentPos.y;
    outWaypoint.z = currentPos.z;
}
