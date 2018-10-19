// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Traffic/RoutePlanner.h"
#include "carla/geom/Math.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDrive.h>
#include <compiler/enable-ue4-macros.h>

#include "OpenDriveActor.generated.h"


UCLASS()
class CARLA_API AOpenDriveActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AOpenDriveActor();

    virtual void OnConstruction(const FTransform &transform) override;
    void GenerateWaypoints(const carla::road::element::RoadSegment *road);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

public:
    // Called every frame
    virtual void Tick(float DeltaTime) override;
};
