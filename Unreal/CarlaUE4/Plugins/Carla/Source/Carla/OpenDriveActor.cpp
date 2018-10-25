// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "OpenDriveActor.h"

#include "DrawDebugHelpers.h"
#include "Algo/Reverse.h"

void AOpenDriveActor::DrawWaypoints(const TArray<FVector>& points)
{
    for (int i = 0, len = points.Num() - 1; i < len; ++i)
    {
        float f = (float)i / (float)len;
        FColor c(255 * f, 255 - 255 * f, 0);
        DrawDebugLine(GetWorld(), points[i + 0], points[i + 1], c, true);
    }
}

ARoutePlanner *AOpenDriveActor::GenerateRoutePlanner(const TArray<FVector> &waypoints)
{
    ARoutePlanner *junctionRoutePlanner = nullptr;

    ARoutePlanner *routePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
    routePlanner->SetActorLocation(waypoints[0]);

    routePlanner->AddRoute(1.0f, waypoints);
    routePlanner->Init();

    RoutePlanners.Add(routePlanner);
    return routePlanner;
}

void AOpenDriveActor::GenerateWaypoints(const carla::road::element::RoadSegment *road)
{
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);

    if (generalInfo->GetJunctionId() > -1)
    {
        return;
    }

    std::vector<std::pair<double, double>> lanesOffset = generalInfo->GetLanesOffset();

    std::vector<int> rightLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right);
    std::vector<int> leftLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left);

    size_t lanesOffsetIndex = 0;
    TArray<carla::road::element::DirectedPoint> laneZeroPoints;

    for (float waypointsOffset = 0.0f; waypointsOffset < road->GetLength() + 2.0; waypointsOffset += 2.0)
    {
        if (lanesOffsetIndex < lanesOffset.size() - 1 && waypointsOffset >= lanesOffset[lanesOffsetIndex + 1].first)
        {
            ++lanesOffsetIndex;
        }

        // NOTE(Andrei): Get waypoin at the offset, and invert the y axis
        carla::road::element::DirectedPoint waypoint = road->GetDirectedPointIn(waypointsOffset);
        waypoint.location.y *= (-1); waypoint.location.z = 1;

        // NOTE(Andrei): Applyed the offset of the lane section
        waypoint.ApplyLateralOffset(lanesOffset[lanesOffsetIndex].second);

        laneZeroPoints.Add(waypoint);
    }

    double currentOffset = 0.0;
    for (size_t j = 0; j < rightLanes.size(); ++j)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(rightLanes[j]);
        TArray<FVector> unrealRoadWaypoints;
        currentOffset += laneInfo->_width * 0.5;

        if (laneInfo->_type == "driving")
        {
            for (int i = 0; i < laneZeroPoints.Num(); ++i)
            {
                carla::road::element::DirectedPoint currentPoint = laneZeroPoints[i];
                currentPoint.ApplyLateralOffset(-currentOffset);
                unrealRoadWaypoints.Add(currentPoint.location);
            }

            if (unrealRoadWaypoints.Num() >= 2)
            {
                DrawWaypoints(unrealRoadWaypoints);
                GenerateRoutePlanner(unrealRoadWaypoints);
            }
        }

        currentOffset += laneInfo->_width * 0.5;
    }

    currentOffset = 0.0;
    for (size_t j = 0; j < leftLanes.size(); ++j)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(leftLanes[leftLanes.size() - 1 -j]);
        TArray<FVector> unrealRoadWaypoints;
        currentOffset += laneInfo->_width * 0.5;

        if (laneInfo->_type == "driving")
        {
            for (int i = 0; i < laneZeroPoints.Num(); ++i)
            {
                carla::road::element::DirectedPoint currentPoint = laneZeroPoints[i];
                currentPoint.ApplyLateralOffset(currentOffset);
                unrealRoadWaypoints.Add(currentPoint.location);
            }

            if (unrealRoadWaypoints.Num() >= 2)
            {
                Algo::Reverse(unrealRoadWaypoints);
                DrawWaypoints(unrealRoadWaypoints);
                GenerateRoutePlanner(unrealRoadWaypoints);
            }
        }

        currentOffset += laneInfo->_width * 0.5;
    }
}

void AOpenDriveActor::GenerateWaypointsJunctions(const carla::road::element::RoadSegment *road, TArray<int> &out_lanesId, TArray<TArray<FVector>> &out_waypoints)
{
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);

    if (generalInfo->GetJunctionId() == -1)
    {
        return;
    }

    std::vector<std::pair<double, double>> lanesOffset = generalInfo->GetLanesOffset();

    std::vector<int> rightLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right);
    std::vector<int> leftLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left);

    size_t lanesOffsetIndex = 0;
    TArray<carla::road::element::DirectedPoint> laneZeroPoints;

    for (float waypointsOffset = 0.0f; waypointsOffset < road->GetLength() + 2.0; waypointsOffset += 2.0)
    {
        if (lanesOffsetIndex < lanesOffset.size() - 1 && waypointsOffset >= lanesOffset[lanesOffsetIndex + 1].first)
        {
            ++lanesOffsetIndex;
        }

        // NOTE(Andrei): Get waypoin at the offset, and invert the y axis
        carla::road::element::DirectedPoint waypoint = road->GetDirectedPointIn(waypointsOffset);
        waypoint.location.y *= (-1); waypoint.location.z = 1;

        // NOTE(Andrei): Applyed the offset of the lane section
        waypoint.ApplyLateralOffset(lanesOffset[lanesOffsetIndex].second);

        laneZeroPoints.Add(waypoint);
    }

    double currentOffset = 0.0;
    for (size_t j = 0; j < rightLanes.size(); ++j)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(rightLanes[j]);
        TArray<FVector> unrealRoadWaypoints;
        currentOffset += laneInfo->_width * 0.5;

        if (laneInfo->_type == "driving")
        {
            for (int i = 0; i < laneZeroPoints.Num(); ++i)
            {
                carla::road::element::DirectedPoint currentPoint = laneZeroPoints[i];
                currentPoint.ApplyLateralOffset(-currentOffset);
                unrealRoadWaypoints.Add(currentPoint.location);
            }

            if (unrealRoadWaypoints.Num() >= 2)
            {
                out_lanesId.Add(laneInfo->_id);
                out_waypoints.Add(unrealRoadWaypoints);
            }
        }

        currentOffset += laneInfo->_width * 0.5;
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

void AOpenDriveActor::BeginPlay()
{
    Super::BeginPlay();
}

AOpenDriveActor::AOpenDriveActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AOpenDriveActor::OnConstruction(const FTransform &transform)
{
    Super::OnConstruction(transform);

    carla::road::Map map = carla::opendrive::OpenDrive::Load("C:\\Users\\ajianu\\Desktop\\xodr\\test_03_new.xodr");
    std::vector<carla::road::lane_junction_t> junctionInfo = map.GetJunctionInformation();

    std::vector<carla::road::id_type> roadIDs = map.GetAllIds();
    std::sort(roadIDs.begin(), roadIDs.end());

    for (auto &&id : roadIDs)
    {
        GenerateWaypoints(map.GetRoad(id));
    }


    //       junctionId    roadID        laneID
    std::map<int, std::map<int, std::map<int, ARoutePlanner *>>>junctions;

    for (size_t i = 0; i < junctionInfo.size(); ++i)
    {
        TArray<int> lanesId;
        TArray<TArray<FVector>> waypoints;

        int fromRoadID = junctionInfo[i].incomming_road;
        int toRoadID = junctionInfo[i].connection_road;
        int junctonID = junctionInfo[i].junction_id;

        GenerateWaypointsJunctions(map.GetRoad(toRoadID), lanesId, waypoints);
        ARoutePlanner *routePlanner = nullptr;

        for (size_t n = 0; n < junctionInfo[i].from_lane.size(); ++n)
        {
            int fromLaneID = junctionInfo[i].from_lane[n];
            routePlanner = junctions[junctonID][fromRoadID][fromLaneID];

            if (routePlanner == nullptr)
            {
                char r = rand() % (255 - 100 + 1) + 100;
                char g = rand() % (255 - 100 + 1) + 100;
                char b = rand() % (255 - 100 + 1) + 100;

                routePlanner = GenerateRoutePlanner(waypoints[n]);
                routePlanner->SetSplineColor(FColor(r, g, b));

                junctions[junctonID][fromRoadID][fromLaneID] = routePlanner;
                routePlanner->DrawRoutes();
            }
            else
            {
                routePlanner->AddRoute(1.0, waypoints[n]);
                routePlanner->DrawRoutes();
            }
        }
    }
}
