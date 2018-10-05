#pragma once
/*
    Project includes
*/
#include "../types.h"
#include "../math/Vector3.h"

/*
    Lib includes
*/
#include <cmath>

/*
    STD/C++ includes
*/

///////////////////////////////////////////////////////////////////////////////

namespace opendrive
{
    namespace logic
    {
        enum class which_lane_e : int
        {
            Left,
            Right
        };

        struct road_generator_input
        {
            const opendrive::types::GeometryAttributes * geometry_attributes = nullptr;
            const opendrive::types::RoadProfiles * road_profile  = nullptr;
            const opendrive::types::LaneSection * lane_section  = nullptr;

            which_lane_e which_lane = which_lane_e::Right;
            bool is_junction = false;

            int from_lane = 0;
            int to_lane = 0;
        };

        class RoadPointsGenerator
        {
            private:
                int last_lane_width_index;
                int last_lane_speed_index;

            private:
                void GenerateArcPoints(const road_generator_input *input, std::vector<opendrive::types::Waypoint>& out_points, int numPoints = 20);
                void GenerateLinePoints(const road_generator_input *input, std::vector<opendrive::types::Waypoint>& out_points, int numPoints = 5);
                void GenerateSpiralPoints(const road_generator_input *input, std::vector<opendrive::types::Waypoint>& out_points, int numPoints = 10);

            private:
                void GenerateLaneInformation(const opendrive::logic::road_generator_input *input, double sOffset, opendrive::types::Waypoint &outWaypoint);

            public:
                RoadPointsGenerator() : last_lane_width_index(0), last_lane_speed_index(0) {}

            public:
                void Generate(const opendrive::types::OpenDriveData & openDriveRoad, std::vector<opendrive::types::Waypoint> &out_road_roints, double scale = 1.0, int fromLane = 1, int toLane = 1);
                void Generate(const opendrive::types::RoadInformation & roadInformation, std::vector<opendrive::types::Waypoint> &out_road_roints, double scale = 1.0, int fromLane = 1, int toLane = 1, bool isJunction = false);
        };
    }
}
