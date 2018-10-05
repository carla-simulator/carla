/*
    Project includes
*/
#include "LineGenerator.h"

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

///////////////////////////////////////////////////////////////////////////////

void opendrive::logic::LinePointsGenerator::Init()
{
    orientation_vector.x = std::cos(geometry_attributes->heading);
    orientation_vector.y = std::sin(geometry_attributes->heading);

    start_point = { geometry_attributes->start_position_x, geometry_attributes->start_position_y, 0.0 };
    end_point = start_point + (orientation_vector * geometry_attributes->length);

    distance_vector = end_point - start_point;
}


void opendrive::logic::LinePointsGenerator::GeneratePoint(double sOffset, opendrive::types::Waypoint &outWaypoint)
{
    int elevationIndex = road_profile->elevation_profile.size() ? 0 : -1;
    math::vector::Vector3<double> p = start_point + (sOffset * distance_vector);

    while (elevationIndex >= 0 && geometry_attributes->start_position < road_profile->elevation_profile[elevationIndex].start_position)
    {
        ++elevationIndex;
    }

    if (elevationIndex >= 0 && (elevationIndex + 1) < (int)road_profile->elevation_profile.size() && (sOffset + geometry_attributes->start_position) >= road_profile->elevation_profile[elevationIndex + 1].start_position)
    {
        ++elevationIndex;
    }

    if (elevationIndex >= 0)
    {
        p.z = road_profile->elevation_profile[elevationIndex].elevation
            + sOffset * road_profile->elevation_profile[elevationIndex].slope
            + sOffset * sOffset * road_profile->elevation_profile[elevationIndex].vertical_curvature
            + sOffset * sOffset * sOffset * road_profile->elevation_profile[elevationIndex].curvature_change;
    }

    outWaypoint.distance_in_section = geometry_attributes->start_position + sOffset;
    outWaypoint.heading = geometry_attributes->heading;

    outWaypoint.x = p.x;
    outWaypoint.y = p.y;
    outWaypoint.z = p.z;
}
