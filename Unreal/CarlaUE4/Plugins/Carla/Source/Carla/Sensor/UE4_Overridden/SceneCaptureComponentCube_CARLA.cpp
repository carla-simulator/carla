// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SceneCaptureComponentCube_CARLA.h"



USceneCaptureComponentCube_CARLA::USceneCaptureComponentCube_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponentCube_CARLA::GetViewOwner() const
{
  return ViewActor;
}
