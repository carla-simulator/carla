/*
    Project includes
*/
#include "RoadPointsGenerator.h"

#include "../math/Vector3.h"
#include "../math/Vector2.h"

#include "SpiralGenerator.h"
#include "LineGenerator.h"
#include "ArcGenerator.h"

/*
    Lib includes
*/

/*
    STD/C++ includes
*/
#include <algorithm>
#include <cassert>


///////////////////////////////////////////////////////////////////////////////

// Some useful information/source code for xodr file parsing and road points
// generation.
// https://gupea.ub.gu.se/handle/2077/23047 (pdf explaining the theory)
// https://gitlab.com/OpenRoadEd            (source code)

///////////////////////////////////////////////////////////////////////////////

void carla::opendrive::logic::RoadPointsGenerator::GenerateLaneInformation(const carla::opendrive::logic::road_generator_input *input, double sOffset, carla::opendrive::types::Waypoint &outWaypoint)
{
    const std::vector<carla::opendrive::types::Lane> *laneInformation = nullptr;
    int /*numLaneInVector = -1,*/ laneNumber = std::abs(input->to_lane);

    switch (input->which_lane)
    {
        default:
        {

        } break;

        case carla::opendrive::logic::which_lane_e::Left:
        {
            laneInformation = &input->lane_section->left;
        } break;

        case carla::opendrive::logic::which_lane_e::Right:
        {
            laneInformation = &input->lane_section->right;
        } break;
    }

    if (laneInformation->empty())
    {
        return;
    }

    assert(laneNumber >= (int)laneInformation->size());
    const carla::opendrive::types::Lane *lane = &laneInformation->at(laneNumber - 1);

    const std::vector<carla::opendrive::types::LaneWidth> &laneWidth = lane->lane_width;
    const std::vector<carla::opendrive::types::LaneSpeed> &laneSpeed = lane->lane_speed;

    if (last_lane_width_index + 1 < (int)laneWidth.size())
    {
        if (std::abs(laneWidth[last_lane_width_index + 1].soffset - sOffset) < 0.01)
        {
            ++last_lane_width_index;
        }
    }

    if (last_lane_speed_index + 1 < (int)laneSpeed.size())
    {
        if (std::abs(laneSpeed[last_lane_speed_index + 1].soffset - sOffset) < 0.01)
        {
            ++last_lane_speed_index;
        }
    }

    if (laneWidth.size())
    {
        outWaypoint.width = laneWidth[last_lane_width_index].width;             // in meters
    }

    if (laneSpeed.size())
    {
        outWaypoint.speed = laneSpeed[last_lane_speed_index].max_speed * 3.6;   // m/s to km/h
    }

    // NOTE(Andrei): set the waypoint into the correct lane
    math::vector::Vector2<double> dir = { 0.0, 0.0 };
    double angle = outWaypoint.heading + (input->is_junction ? +1.5708 : -1.5708);

    dir.x = std::cos(angle) - std::sin(angle);
    dir.y = std::sin(angle) + std::cos(angle);

    dir = dir * (input->to_lane * (outWaypoint.width * 0.5));
    outWaypoint.x += dir.x, outWaypoint.y += dir.y;
}

///////////////////////////////////////////////////////////////////////////////

void carla::opendrive::logic::RoadPointsGenerator::GenerateArcPoints(const road_generator_input *input, std::vector<carla::opendrive::types::Waypoint>& out_points, int numPoints)
{
    carla::opendrive::logic::ArcPointsGenerator arcGenerator;

    arcGenerator.SetGeometry((carla::opendrive::types::GeometryAttributesArc *)input->geometry_attributes);
    arcGenerator.SetRoadProfile(input->road_profile);

    arcGenerator.Init();

    for (double sOffset = 0, step = input->geometry_attributes->length / numPoints; sOffset < input->geometry_attributes->length; sOffset += step)
    {
        carla::opendrive::types::Waypoint waypoint;

        arcGenerator.GeneratePoint(sOffset, waypoint);
        GenerateLaneInformation(input, sOffset, waypoint);

        out_points.push_back(waypoint);
    }
}

void carla::opendrive::logic::RoadPointsGenerator::GenerateLinePoints(const road_generator_input *input, std::vector<carla::opendrive::types::Waypoint>& out_points, int numPoints)
{
    carla::opendrive::logic::LinePointsGenerator lineGenerator;

    lineGenerator.SetGeometry((carla::opendrive::types::GeometryAttributesLine *)input->geometry_attributes);
    lineGenerator.SetRoadProfile(input->road_profile);

    lineGenerator.Init();

    for (int i = 0; i < numPoints; ++i)
    {
        double sOffset = (double)(i + 1) / (double)numPoints;
        carla::opendrive::types::Waypoint waypoint;

        lineGenerator.GeneratePoint(sOffset, waypoint);
        GenerateLaneInformation(input, sOffset, waypoint);

        out_points.push_back(waypoint);
    }
}

void carla::opendrive::logic::RoadPointsGenerator::GenerateSpiralPoints(const road_generator_input *input, std::vector<carla::opendrive::types::Waypoint>& out_points, int numPoints)
{
    carla::opendrive::logic::SpiralPointsGenerator spiralGenerator;

    spiralGenerator.SetGeometry((opendrive::types::GeometryAttributesSpiral *)input->geometry_attributes);
    spiralGenerator.SetRoadProfile(input->road_profile);

    spiralGenerator.Init();

    for (double sOffset = 0, step = input->geometry_attributes->length / numPoints; sOffset < input->geometry_attributes->length; sOffset += step)
    {
        carla::opendrive::types::Waypoint waypoint;

        spiralGenerator.GeneratePoint(sOffset, waypoint);
        GenerateLaneInformation(input, sOffset, waypoint);

        out_points.push_back(waypoint);
    }

}

void carla::opendrive::logic::RoadPointsGenerator::Generate(const carla::opendrive::types::OpenDriveData & openDriveRoad, std::vector<carla::opendrive::types::Waypoint> &out_road_roints, double scale, int fromLane, int toLane)
{
    for (size_t i = 0; i < openDriveRoad.roads.size(); ++i)
    {
        for (size_t j = 0; j < openDriveRoad.roads[i].geometry_attributes.size(); ++j)
        {
            std::vector<carla::opendrive::types::Waypoint> tmpWaypoints;

            Generate(openDriveRoad.roads[i], tmpWaypoints, scale, fromLane, toLane);
            out_road_roints.insert(out_road_roints.end(), tmpWaypoints.begin(), tmpWaypoints.end());
        }
    }
}

void carla::opendrive::logic::RoadPointsGenerator::Generate(const carla::opendrive::types::RoadInformation & roadInformation, std::vector<carla::opendrive::types::Waypoint> &out_road_roints, double scale, int fromLane, int toLane, bool isJunction)
{
    for (size_t i = 0; i < roadInformation.geometry_attributes.size(); ++i)
    {
        carla::opendrive::logic::road_generator_input input;

        input.geometry_attributes = roadInformation.geometry_attributes[i];
        input.is_junction = isJunction;

        input.road_profile = &roadInformation.road_profiles;
        input.lane_section = &roadInformation.lane_sections;

        input.which_lane = (toLane > 0 || isJunction) ? which_lane_e::Right : which_lane_e::Left;
        input.from_lane = fromLane;
        input.to_lane = toLane;

        switch (roadInformation.geometry_attributes[i]->type)
        {
            case opendrive::types::GeometryType::ARC:
            {
                GenerateArcPoints(&input, out_road_roints);
            } break;

            case opendrive::types::GeometryType::LINE:
            {
                GenerateLinePoints(&input, out_road_roints);
            } break;

            case opendrive::types::GeometryType::SPIRAL:
            {
                GenerateSpiralPoints(&input, out_road_roints);
            } break;
        }
    }

    std::sort
    (
        out_road_roints.begin(),
        out_road_roints.end(),

        [](const opendrive::types::Waypoint &w1, const opendrive::types::Waypoint &w2) -> bool
        {
            return w1.distance_in_section < w2.distance_in_section;
        }
    );

    for (size_t i = 0; i < out_road_roints.size(); ++i)
    {
        out_road_roints[i].x *= scale;
        out_road_roints[i].y *= scale;
        out_road_roints[i].z *= scale;
    }
}
