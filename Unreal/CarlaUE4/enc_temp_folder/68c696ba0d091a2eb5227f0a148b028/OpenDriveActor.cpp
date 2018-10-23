// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "OpenDriveActor.h"

#include "DrawDebugHelpers.h"
#include "Algo/Reverse.h"

void AOpenDriveActor::GenerateWaypoints(const carla::road::element::RoadSegment *road)
{
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);

    if (generalInfo->GetIsJunction())
    {
        return;
    }

    std::vector<std::pair<double, double>> lanesOffset = generalInfo->GetLanesOffset();
    size_t laneOffsetIndex = 0;

    std::vector<int> rightLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right);
    std::vector<int> leftLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left);

    // NOTE(Andrei): If we have left and right lanes
    // clear the offset as the offset of each each lane
    // is calculated.
    if (rightLanes.size() && leftLanes.size())
    {
        lanesOffset.clear();
        lanesOffset.emplace_back(std::pair<double, double>(0.0, 0.0));
    }

    ///////////////////////////////////////////////////////////////////////////
    // NOTE(Andrei): Calculate the offset of each driving driving lane.
    // Starting from the center of the road, search for the driving lane
    // and store the width of each lane and the offset starting from the
    // center of the road.

    double totalWidth = 0.0, laneWidth = 0.0;
    int drivingLane = 0;

    struct driving_info_t { int lane; double halfLaneWidth; double totalWidth; };
    TArray<driving_info_t> drivingLanes;

    for (size_t i = 0; i < rightLanes.size(); ++i)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(rightLanes[i]);
        totalWidth += laneInfo->_width;

        if (laneInfo->_type == "driving")
        {
            driving_info_t dl = { rightLanes[i], laneInfo->_width * 50.0, totalWidth * 100.0 };
            drivingLanes.Add(dl);
        }
    }

    totalWidth = 0.0;

    for (size_t i = 0; i < leftLanes.size(); ++i)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(leftLanes[leftLanes.size() - 1 - i]);
        totalWidth += laneInfo->_width;

        if (laneInfo->_type == "driving")
        {
            driving_info_t dl = { leftLanes[leftLanes.size() - 1 - i], laneInfo->_width  * 50.0, totalWidth * 100.0 };
            drivingLanes.Add(dl);
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    // NOTE(Andrei): For each one of the lanes, ones that we have the center
    // point of the road, place it on the center of the corresponding lane.
    //

    for (int i = 0; i < drivingLanes.Num(); ++i)
    {
        TArray<FVector> points;
        size_t lanesOffsetIndex = 0;

        for (float offset = 0.0f; offset < road->GetLength(); offset += 2.0)
        {
            carla::road::element::DirectedPoint p = road->GetDirectedPointIn(offset);
            double heading = p.tangent + carla::geom::Math::pi_half();

            if (laneOffsetIndex < lanesOffset.size() - 1  && offset >= lanesOffset[lanesOffsetIndex + 1].first)
            {
                ++lanesOffsetIndex;
            }

            FVector loc(p.location.x * 100, -p.location.y * 100, p.location.z * 100 + 100);
            FVector dir(0.0f, 0.0f, 0.0f);

            dir.X = std::cos(heading) * (drivingLanes[i].totalWidth - (lanesOffset[lanesOffsetIndex].second * 100.0) - drivingLanes[i].halfLaneWidth);
            dir.Y = std::sin(-heading) * (drivingLanes[i].totalWidth - (lanesOffset[lanesOffsetIndex].second * 100.0) - drivingLanes[i].halfLaneWidth);

            FVector waypoint = loc + (drivingLanes[i].lane < 0 ? dir : -dir);
            points.Add(waypoint);
        }

        if (drivingLanes[i].lane < 0)
        {
            Algo::Reverse(points);
        }

        if (points.Num() >= 2)
        {
            for (int i = 0, len = points.Num() - 1; i < len; ++i)
            {
                float f = (float) i / (float) len; // green to red
                //DrawDebugPoint(GetWorld(), points[i], 3, FColor(255 * f, 255 - 255 * f, 0), true);

                FColor c(255 * f, 255 - 255 * f, 0);
                DrawDebugLine(GetWorld(), points[i + 0], points[i + 1], c, true);
            }

            //DrawDebugString(GetWorld(), points[0], FString().Printf(TEXT("RoadID: %d\nLaneID: %d"), road->GetId(), drivingLanes[i].lane));
            //ARoutePlanner *routePlanner = GetWorld()->SpawnActor<ARoutePlanner>(ARoutePlanner::StaticClass(), FTransform{points[0]});

            ARoutePlanner *routePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
            routePlanner->SetActorLocation(points[0]);

            routePlanner->AddRoute(1.0f, points);
            routePlanner->Init();

            RoutePlanners.Add(routePlanner);
        }
    }
}

void AOpenDriveActor::BeginDestroy()
{
    for (int i = 0; i < RoutePlanners.Num(); ++i)
    {
        RoutePlanners[i]->Destroy();
    }

    RoutePlanners.Empty();
    Super::BeginDestroy();
}

AOpenDriveActor::AOpenDriveActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AOpenDriveActor::OnConstruction(const FTransform &transform)
{
    carla::road::Map map = carla::opendrive::OpenDrive::Load("C:\\Users\\ajianu\\Desktop\\xodr\\test_03.xodr");

    std::vector<carla::road::id_type> roadIDs = map.GetAllIds();
    std::sort(roadIDs.begin(), roadIDs.end());

    for (auto &&id : roadIDs)
    {
        GenerateWaypoints(map.GetRoad(id));
    }
}
