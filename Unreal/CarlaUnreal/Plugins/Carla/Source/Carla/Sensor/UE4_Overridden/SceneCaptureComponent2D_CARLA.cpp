// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "SceneCaptureComponent2D_CARLA.h"



USceneCaptureComponent2D_CARLA::USceneCaptureComponent2D_CARLA(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer)
{
}

const AActor* USceneCaptureComponent2D_CARLA::GetViewOwner() const
{
  return ViewActor;
}
