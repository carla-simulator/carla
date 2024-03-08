// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SceneCaptureComponent_CARLA.h"



USceneCaptureComponent_CARLA::USceneCaptureComponent_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent_CARLA::GetViewOwner() const
{
  return ViewActor;
}
