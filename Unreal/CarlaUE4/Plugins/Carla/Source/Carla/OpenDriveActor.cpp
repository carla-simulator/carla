// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"

#include "OpenDriveActor.h"

#include "DrawDebugHelpers.h"
#include "Algo/Reverse.h"

///////////////////////////////////////////////////////////
/*
void AOpenDriveActor::fnc_generate_points(const carla::road::element::RoadSegment *road, ARoutePlanner *outRoutePlaner, bool isStart)
{
    TArray<FVector> points;
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);

    std::vector<int> rightLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right);
    std::vector<int> leftLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left);

    double totalWidth = 0.0, laneWidth = 0.0;
    int drivingLane = 0;

    struct driving_info_t { int lane; double halfLaneWidth; double totalWidth; };
    TArray<driving_info_t> drivingLanes;

    for (size_t i = 0; i < rightLanes.size(); ++i)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(rightLanes[i]);
        totalWidth += laneInfo->_width * 100.0;

        if (laneInfo->_type == "driving")
        {
            driving_info_t dl = { rightLanes[i], laneInfo->_width * 50.0 , totalWidth};
            drivingLanes.Add(dl);
        }
    }

    totalWidth = 0.0;

    for (size_t i = 0; i < leftLanes.size(); ++i)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(leftLanes[leftLanes.size() - 1 - i]);
        totalWidth += laneInfo->_width * 100.0;

        if (laneInfo->_type == "driving")
        {
            driving_info_t dl = { leftLanes[leftLanes.size() - 1 - i], laneInfo->_width  * 50.0, totalWidth };
            drivingLanes.Add(dl);
        }
    }

    for (float offset = 0.0f; offset < road->GetLength(); offset += .5)
    {
        carla::road::element::DirectedPoint p = road->GetDirectedPointIn(offset);
        double heading = p.tangent + carla::geom::Math::pi_half();

        FVector loc(p.location.x * 100, -p.location.y * 100, p.location.z * 100 + 100);
        DrawDebugPoint(GetWorld(), loc, 3, FColor(255 * (offset / road->GetLength()), 255 - 255 * (offset / road->GetLength()), 0), true);

        for (int i = 0; i < drivingLanes.Num(); ++i)
        {
            FVector dir(0.0f, 0.0f, 0.0f);
            FVector waypoint = loc;

            dir.X = std::cos(heading) * (drivingLanes[i].totalWidth - drivingLanes[i].halfLaneWidth);
            dir.Y = std::sin(-heading) * (drivingLanes[i].totalWidth - drivingLanes[i].halfLaneWidth);

            if (generalInfo->GetIsJunction() && !isStart)
            {
                waypoint += -dir;
            }
            else
            {
                waypoint += (drivingLanes[i].lane < 0 ? dir : -dir);
            }
            
            points.Add(waypoint);

            if (generalInfo->GetIsJunction())
            {
                DrawDebugPoint(GetWorld(), waypoint, 3, FColor::Yellow, true);
            }
            else if (drivingLanes[i].lane < 0)
            {
                DrawDebugPoint(GetWorld(), waypoint, 3, FColor::Magenta, true);
            }
            else
            {
                DrawDebugPoint(GetWorld(), waypoint, 3, FColor::Cyan, true);
            }

            if (offset - 3.0f < 0.0000001) {
                DrawDebugString(GetWorld(), waypoint, FString().Printf(TEXT("RoadID: %d\nLaneID: %d"), road->GetId(), drivingLanes[i].lane));
            }
        }
    }

    outRoutePlaner->AddRoute(1.0f, points);
}
*/
void AOpenDriveActor::GenerateWaypoints(const carla::road::element::RoadSegment *road)
{
    const carla::road::element::RoadInfoLane *lanesInfo = road->GetInfo<carla::road::element::RoadInfoLane>(0.0);
    const carla::road::element::RoadGeneralInfo *generalInfo = road->GetInfo<carla::road::element::RoadGeneralInfo>(0.0);

    if (generalInfo->GetIsJunction())
    {
        return;
    }

    std::vector<int> rightLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Right);
    std::vector<int> leftLanes = lanesInfo->getLanesIDs(carla::road::element::RoadInfoLane::which_lane_e::Left);

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
            driving_info_t dl = { rightLanes[i], laneInfo->_width * 50.0 , totalWidth * 100.0 };
            drivingLanes.Add(dl);
        }
    }

    totalWidth = 0.0;

    for (size_t i = 0; i < leftLanes.size(); ++i)
    {
        const carla::road::element::LaneInfo *laneInfo = lanesInfo->getLane(leftLanes[leftLanes.size() - 1 - i]);
        totalWidth += laneInfo->_width * 100.0;

        if (laneInfo->_type == "driving")
        {
            driving_info_t dl = { leftLanes[leftLanes.size() - 1 - i], laneInfo->_width  * 50.0, totalWidth };
            drivingLanes.Add(dl);
        }
    }

    for (int i = 0; i < drivingLanes.Num(); ++i)
    {
        ARoutePlanner *routePlanner = GetWorld()->SpawnActor<ARoutePlanner>();
        TArray<FVector> points;

        for (float offset = 0.0f; offset < road->GetLength(); offset += .5)
        {
            carla::road::element::DirectedPoint p = road->GetDirectedPointIn(offset);
            double heading = p.tangent + carla::geom::Math::pi_half();

            FVector loc(p.location.x * 100, -p.location.y * 100, p.location.z * 100 + 100);
            FVector dir(0.0f, 0.0f, 0.0f);

            dir.X = std::cos(heading) * (drivingLanes[i].totalWidth - drivingLanes[i].halfLaneWidth);
            dir.Y = std::sin(-heading) * (drivingLanes[i].totalWidth - drivingLanes[i].halfLaneWidth);

            FVector waypoint = loc + (drivingLanes[i].lane < 0 ? dir : -dir);
            points.Add(waypoint);

            float f = offset / road->GetLength(); // green to red
            DrawDebugPoint(GetWorld(), waypoint, 3, FColor(255 * f, 255 - 255 * f, 0), true);
        }

        if (drivingLanes[i].lane < 0)
        {
            Algo::Reverse(points);
        }

        DrawDebugString(GetWorld(), points[0], FString().Printf(TEXT("RoadID: %d\nLaneID: %d"), road->GetId(), drivingLanes[i].lane));

        routePlanner->SetActorLocation(points[0]);
        routePlanner->AddRoute(1.0f, points);
        routePlanner->Init();

    }

    /*for (float offset = 0.0f; offset < road->GetLength(); offset += .5)
    {
        carla::road::element::DirectedPoint p = road->GetDirectedPointIn(offset);
        double heading = p.tangent + carla::geom::Math::pi_half();

        FVector loc(p.location.x * 100, -p.location.y * 100, p.location.z * 100 + 100);
        DrawDebugPoint(GetWorld(), loc, 3, FColor(255 * (offset / road->GetLength()), 255 - 255 * (offset / road->GetLength()), 0), true);

        for (int i = 0; i < drivingLanes.Num(); ++i)
        {
            FVector dir(0.0f, 0.0f, 0.0f);
            FVector waypoint = loc;

            dir.X = std::cos(heading) * (drivingLanes[i].totalWidth - drivingLanes[i].halfLaneWidth);
            dir.Y = std::sin(-heading) * (drivingLanes[i].totalWidth - drivingLanes[i].halfLaneWidth);
            
            waypoint += (drivingLanes[i].lane < 0 ? dir : -dir);
            points.Add(waypoint);

            if (drivingLanes[i].lane < 0)
            {
                DrawDebugPoint(GetWorld(), waypoint, 3, FColor::Magenta, true);
            }
            else
            {
                DrawDebugPoint(GetWorld(), waypoint, 3, FColor::Cyan, true);
            }

            if (std::abs(offset - 3.0f) < 0.0000001) {
                DrawDebugString(GetWorld(), waypoint, FString().Printf(TEXT("RoadID: %d\nLaneID: %d"), road->GetId(), drivingLanes[i].lane));
            }
        }
    }*/
}
///////////////////////////////////////////////////////////

// Sets default values
AOpenDriveActor::AOpenDriveActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

// Called when the game starts or when spawned
void AOpenDriveActor::BeginPlay()
{
    Super::BeginPlay();

    /*
    for (auto &&id : roadIDs)
    {
        const carla::road::element::RoadSegment *roadSegment = map.GetRoad(id);

        if (!processed.Contains(id))
        {
            ARoutePlanner *routePlaner = GetWorld()->SpawnActor<ARoutePlanner>();
            fnc_generate_points(map.GetRoad(id), routePlaner);

            routePlaner->Init();
            processed.Add(id);
        }

        std::vector<carla::road::id_type> successorIds = map.GetRoad(id)->GetSuccessorsIds();
        std::vector<bool> isSuccessorStart = map.GetRoad(id)->GetSuccessorsIsSTart();

        if (successorIds.size())
        {
            ARoutePlanner *routePlaner = GetWorld()->SpawnActor<ARoutePlanner>();
            size_t i = 0;

            for (auto &&successorID : successorIds)
            {
                fnc_generate_points(map.GetRoad(successorID), routePlaner);
                processed.Add(successorID);
            }
        }

        std::vector<carla::road::id_type> predecessorIds = map.GetRoad(id)->GetPredecessorsIds();
        std::vector<bool> isPredecesorStart = map.GetRoad(id)->GetPredecessorsIsStart();

        if (predecessorIds.size())
        {
            ARoutePlanner *routePlaner = GetWorld()->SpawnActor<ARoutePlanner>();

            for (auto &&predeccesorID : predecessorIds)
            {
                fnc_generate_points(map.GetRoad(predeccesorID), routePlaner);
                processed.Add(predeccesorID);
            }
        }
    }*/
}

// Called every frame
void AOpenDriveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
