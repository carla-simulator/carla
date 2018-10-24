// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Runtime/Engine/Classes/Engine/World.h"


#include "OpenDriveActor.h"
#include "Traffic/RoutePlanner.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDrive.h>
#include <compiler/enable-ue4-macros.h>



///////////////////////////////////////////////////////////

void fnc_generate_points(const carla::road::element::RoadSegment *road, ARoutePlanner *outRoutePlaner)
{
    TArray<FVector> points;

    for (float offset = 0.0f; offset < road->GetLength(); offset += (road->GetLength() / 30.0f))
    {
        carla::road::element::DirectedPoint p = road->GetDirectedPointIn(offset);
        points.Add(FVector(p.location.x * 100, -p.location.y * 100, p.location.z * 100 + 100));

        outRoutePlaner->AddRoute(1.0f, points);
    }
}

///////////////////////////////////////////////////////////

// Sets default values
AOpenDriveActor::AOpenDriveActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AOpenDriveActor::BeginPlay()
{
    Super::BeginPlay();
    auto MapPtr = carla::opendrive::OpenDrive::Load("C:\\Users\\ajianu\\Desktop\\xodr\\test_03.xodr");
    auto &map = MapPtr->GetData();

    std::vector<carla::road::element::id_type> roadIDs = map.GetAllIds();
    TArray<carla::road::element::id_type> processed;

    for (auto &&id : roadIDs)
    {
        if (!processed.Contains(id))
        {
            ARoutePlanner *routePlaner = GetWorld()->SpawnActor<ARoutePlanner>();
            fnc_generate_points(map.GetRoad(id), routePlaner);

            routePlaner->Init();
            processed.Add(id);
        }

        /*std::vector<carla::road::id_type> successorIds = map.GetRoad(id)->GetSuccessorsIds();
        if (successorIds.size())
        {
            ARoutePlanner *routePlaner = GetWorld()->SpawnActor<ARoutePlanner>();

            for (auto &&successorID : successorIds)
            {
                fnc_generate_points(map.GetRoad(successorID), routePlaner);
                processed.Add(successorID);
            }
        }

        std::vector<carla::road::id_type> predecessorIds = map.GetRoad(id)->GetPredecessorsIds();
        if (predecessorIds.size())
        {
            ARoutePlanner *routePlaner = GetWorld()->SpawnActor<ARoutePlanner>();

            for (auto &&predeccesorID : predecessorIds)
            {
                fnc_generate_points(map.GetRoad(predeccesorID), routePlaner);
                processed.Add(predeccesorID);
            }
        }*/
    }
}

// Called every frame
void AOpenDriveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
