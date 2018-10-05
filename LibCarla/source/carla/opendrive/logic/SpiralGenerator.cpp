/*
    Project includes
*/
#include "SpiralGenerator.h"
#include "cephes/fresnel.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/

/*
    Defines and constants
*/
static const double FULL_PI_LOCAL = 3.14159265358979323846;
static const double HALF_PI_LOCAL = 1.57079632679489661923;
static const double sqrtPiO2 = std::sqrt(HALF_PI_LOCAL);

///////////////////////////////////////////////////////////////////////////////

void opendrive::logic::SpiralPointsGenerator::Init()
{
    double curvStart = geometry_attributes->curve_start;
    double curvEnd = geometry_attributes->curve_end;

    double length = geometry_attributes->length;

    start_X = geometry_attributes->start_position_x;
    start_Y = geometry_attributes->start_position_y;

    if ((std::abs(curvEnd) > 1.00e-15) && (std::abs(curvStart) <= 1.00e-15))
    {
        inverse = false;
        curvature = curvEnd;

        a = 1.0 / std::sqrt(2.0 * (1.0 / std::fabs(curvature)) * length);
        m_a = (1.0 / a);

        rot_angle_cos = std::cos(geometry_attributes->heading);
        rot_angle_sin = std::sin(geometry_attributes->heading);
    }
    else
    {
        inverse = true;
        curvature = curvStart;

        a = 1.0 / std::sqrt(2.0 * (1.0 / std::fabs(curvature)) * length);
        m_a = (1.0 / a);

        double l = geometry_attributes->length * a / sqrtPiO2;

        fresnl(l, &end_Y, &end_X);
        if (curvature < 0.0) end_Y = -end_Y;

        end_X = end_X * m_a * sqrtPiO2;
        end_Y = end_Y * m_a * sqrtPiO2;

        double differenceAngle = l * l * sqrtPiO2 * sqrtPiO2;
        double diffAngle = 0.0;

        if (curvature < 0)
        {
            diffAngle = geometry_attributes->heading - differenceAngle - FULL_PI_LOCAL;
        }
        else
        {
            diffAngle = geometry_attributes->heading + differenceAngle - FULL_PI_LOCAL;
        }

        rot_angle_cos = std::cos(diffAngle);
        rot_angle_sin = std::sin(diffAngle);
    }
}

void opendrive::logic::SpiralPointsGenerator::GeneratePoint(double sOffset, opendrive::types::Waypoint & outWaypoint)
{
    int elevationIndex = road_profile->elevation_profile.size() ? 0 : -1;

    while (elevationIndex >= 0 && geometry_attributes->start_position < road_profile->elevation_profile[elevationIndex].start_position)
    {
        ++elevationIndex;
    }

    double l = inverse ? ((geometry_attributes->length - sOffset) * a) / sqrtPiO2 : (sOffset * a) / sqrtPiO2;
    math::vector::Vector3<double> curvaturePoint = { 0.0, 0.0, 0.0 }, p = { 0.0, 0.0, 0.0 };

    fresnl(l, &curvaturePoint.y, &curvaturePoint.x);
    if (curvature < 0) curvaturePoint.y = -curvaturePoint.y;

    curvaturePoint.x = curvaturePoint.x * m_a * sqrtPiO2;
    curvaturePoint.y = curvaturePoint.y * m_a * sqrtPiO2;

    if (inverse)
    {
        curvaturePoint.x -= end_X;
        curvaturePoint.y -= end_Y;
        curvaturePoint.y *= (-1);
    }

    p.x = start_X + (curvaturePoint.x * rot_angle_cos - curvaturePoint.y * rot_angle_sin);
    p.y = start_Y + (curvaturePoint.x * rot_angle_sin + curvaturePoint.y * rot_angle_cos);

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

    l = sOffset * m_a;
    double tangentAngle = l * l;

    if (curvature < 0)
    {
        tangentAngle *= (-1);
    }

    outWaypoint.distance_in_section = geometry_attributes-> start_position + (inverse ? geometry_attributes->length - sOffset : sOffset);
    outWaypoint.heading = geometry_attributes->heading + tangentAngle;

    outWaypoint.x = p.x;
    outWaypoint.y = p.y;
    outWaypoint.z = p.z;
}
