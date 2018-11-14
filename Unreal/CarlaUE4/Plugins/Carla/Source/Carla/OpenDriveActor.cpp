// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "OpenDriveActor.h"
#include "Algo/Reverse.h"

#include "Util/OpenDrive.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/rpc/String.h>
#include <compiler/enable-ue4-macros.h>

AOpenDriveActor::AOpenDriveActor()
{
    PrimaryActorTick.bCanEverTick = true;
}

void AOpenDriveActor::BeginPlay()
{
    Super::BeginPlay();
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

void AOpenDriveActor::OnConstruction(const FTransform &transform)
{
    Super::OnConstruction(transform);
    BuildRoutes();
}

void AOpenDriveActor::BuildRoutes()
{
    std::string parseError;

    // NOTE(Andrei): As the OpenDrive file has the same name as level,
    // build the path to the xodr file using the lavel name and the
    // game content directory.
    FString mapName = GetWorld()->GetMapName();
    FString xodrContent = FOpenDrive::Load(mapName);

    auto map_ptr = carla::opendrive::OpenDrive::Load(TCHAR_TO_UTF8(*xodrContent), XmlInputType::CONTENT, &parseError);

    if (parseError.size())
    {
        UE_LOG(LogCarla, Error, TEXT("OpenDrive parsing error: '%s'."), *carla::rpc::ToFString(parseError));
        return;
    }

    const auto &map = map_ptr->GetData();
    std::vector<carla::road::lane_junction_t> junctionInfo = map.GetJunctionInformation();

    ///////////////////////////////////////////////////////////////////////////
    // NOTE(Andrei): Build the roads that are not junctions

    auto RoadIDsView = map.GetAllIds();
    std::vector<carla::road::element::id_type> roadIDs(RoadIDsView.begin(), RoadIDsView.end());
    std::sort(roadIDs.begin(), roadIDs.end());

    for (auto &&id : roadIDs)
    {
        GenerateWaypointsRoad(map.GetRoad(id));
    }

    ///////////////////////////////////////////////////////////////////////////
    // NOTE(Andrei): Build the roads that are junctions as one RoutePlanner
    // can have more than one path that can be taken

    //       junctionId    roadID        laneID
    std::map<int, std::map<int, std::map<int, ARoutePlanner *>>>junctions;

    for (size_t i = 0; i < junctionInfo.size(); ++i)
    {
        TArray<TArray<FVector>> waypoints;

        int fromRoadID = junctionInfo[i].incomming_road;
        int toRoadID = junctionInfo[i].connection_road;
        int junctonID = junctionInfo[i].junction_id;

        GenerateWaypointsJunction(map.GetRoad(toRoadID), waypoints);
        ARoutePlanner *routePlanner = nullptr;

        std::sort(junctionInfo[i].from_lane.begin(), junctionInfo[i].from_lane.end());
        if (junctionInfo[i].from_lane[0] < 0) std::reverse(junctionInfo[i].from_lane.begin(), junctionInfo[i].from_lane.end());

        for (size_t n = 0; n < junctionInfo[i].from_lane.size(); ++n)
        {
            int fromLaneID = junctionInfo[i].from_lane[n];
            routePlanner = junctions[junctonID][fromRoadID][fromLaneID];

            if (routePlanner == nullptr)
            {
                routePlanner = GenerateRoutePlanner(waypoints[n]);
                routePlanner->SetSplineColor(FColor::MakeRandomColor());
                junctions[junctonID][fromRoadID][fromLaneID] = routePlanner;
            }
            else
            {
                routePlanner->AddRoute(1.0, waypoints[n]);
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < RoutePlanners.Num(); ++i)
    {
        RoutePlanners[i]->DrawRoutes();
    }
}

ARoutePlanner *AOpenDriveActor::GenerateRoutePlanner(const TArray<FVector> &waypoints)
{
    ARoutePlanner *junctionRoutePlanner = nullptr;

    ARoutePlanner *routePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
    routePlanner->SetActorLocation(waypoints[0]);

    routePlanner->SetBoxExtent(FVector(70.0f, 70.0f, 50.0f));
    routePlanner->AddRoute(1.0f, waypoints);
    routePlanner->Init();

    RoutePlanners.Add(routePlanner);
    return routePlanner;
}

TArray<carla::road::element::DirectedPoint> AOpenDriveActor::GenerateLaneZeroPoints(const carla::road::element::RoadSegment * road)
{
    size_t lanesOffsetIndex = 0;
    TArray<carla::road::element::DirectedPoint> laneZeroPoints;

    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);
    std::vector<std::pair<double, double>> lanesOffset = generalInfo->GetLanesOffset();

    for (float waypointsOffset = 0.0f; waypointsOffset < road->GetLength() + 2.0; waypointsOffset += 2.0)
    {
        //NOTE(Andrei): Calculate the which laneOffset has to be used
        if (lanesOffsetIndex < lanesOffset.size() - 1 && waypointsOffset >= lanesOffset[lanesOffsetIndex + 1].first)
        {
            ++lanesOffsetIndex;
        }

        // NOTE(Andrei): Get waypoin at the offset, and invert the y axis
        carla::road::element::DirectedPoint waypoint = road->GetDirectedPointIn(waypointsOffset);
        waypoint.location.z = 1;

        // NOTE(Andrei): Applyed the laneOffset of the lane section
        waypoint.ApplyLateralOffset(lanesOffset[lanesOffsetIndex].second);

        laneZeroPoints.Add(waypoint);
    }

    return laneZeroPoints;
}

TArray<TArray<FVector>> AOpenDriveActor::GenerateRightLaneWaypoints(const carla::road::element::RoadSegment * road, const TArray<carla::road::element::DirectedPoint> &laneZeroPoints)
{
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    std::vector<int> rightLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right);

    TArray<TArray<FVector>> retWaypoints;
    double currentOffset = 0.0;

    for (size_t j = 0; j < rightLanes.size(); ++j)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(rightLanes[j]);
        currentOffset += laneInfo->_width * 0.5;
        TArray<FVector> roadWaypoints;

        if (laneInfo->_type == "driving")
        {
            for (int i = 0; i < laneZeroPoints.Num(); ++i)
            {
                carla::road::element::DirectedPoint currentPoint = laneZeroPoints[i];
                currentPoint.ApplyLateralOffset(-currentOffset);
                roadWaypoints.Add(currentPoint.location);
            }

            if (roadWaypoints.Num() >= 2)
            {
                retWaypoints.Add(roadWaypoints);
            }
        }

        currentOffset += laneInfo->_width * 0.5;
    }

    return retWaypoints;
}

TArray<TArray<FVector>> AOpenDriveActor::GenerateLeftLaneWaypoints(const carla::road::element::RoadSegment * road, const TArray<carla::road::element::DirectedPoint> &laneZeroPoints)
{
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    std::vector<int> leftLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left);

    TArray<TArray<FVector>> retWaypoints;
    double currentOffset = 0.0;

    for (size_t j = 0; j < leftLanes.size(); ++j)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(leftLanes[j]);
        currentOffset += laneInfo->_width * 0.5;
        TArray<FVector> roadWaypoints;

        if (laneInfo->_type == "driving")
        {
            for (int i = 0; i < laneZeroPoints.Num(); ++i)
            {
                carla::road::element::DirectedPoint currentPoint = laneZeroPoints[i];
                currentPoint.ApplyLateralOffset(currentOffset);
                roadWaypoints.Add(currentPoint.location);
            }

            if (roadWaypoints.Num() >= 2)
            {
                Algo::Reverse(roadWaypoints);
                retWaypoints.Add(roadWaypoints);
            }
        }

        currentOffset += laneInfo->_width * 0.5;
    }
    return retWaypoints;
}

void AOpenDriveActor::GenerateWaypointsRoad(const carla::road::element::RoadSegment *road)
{
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);
    if (generalInfo->GetJunctionId() > -1) return;

    TArray<carla::road::element::DirectedPoint> laneZeroPoints = GenerateLaneZeroPoints(road);

    TArray<TArray<FVector>> rightLaneWaypoints = GenerateRightLaneWaypoints(road, laneZeroPoints);
    TArray<TArray<FVector>> leftLaneWaypoints = GenerateLeftLaneWaypoints(road, laneZeroPoints);

    for (int i = 0; i < rightLaneWaypoints.Num(); ++i)
    {
        GenerateRoutePlanner(rightLaneWaypoints[i]);
    }

    for (int i = 0; i < leftLaneWaypoints.Num(); ++i)
    {
        GenerateRoutePlanner(leftLaneWaypoints[i]);
    }
}

void AOpenDriveActor::GenerateWaypointsJunction(const carla::road::element::RoadSegment *road, TArray<TArray<FVector>> &out_waypoints)
{
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);
    if (generalInfo->GetJunctionId() == -1) return;

    TArray<carla::road::element::DirectedPoint> laneZeroPoints = GenerateLaneZeroPoints(road);
    out_waypoints = GenerateRightLaneWaypoints(road, laneZeroPoints);

    if (out_waypoints.Num() == 0)
    {
        out_waypoints = GenerateLeftLaneWaypoints(road, laneZeroPoints);
    }
}
