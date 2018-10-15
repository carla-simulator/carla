// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "OpenDriveActor.h"

#include "Traffic/RoutePlanner.h"

#include <compiler/disable-ue4-macros.h>
#include <carla/opendrive/OpenDrive.h>
#include <compiler/enable-ue4-macros.h>

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

      carla::road::Map map = carla::opendrive::OpenDrive::Load("");
}

// Called every frame
void AOpenDriveActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
