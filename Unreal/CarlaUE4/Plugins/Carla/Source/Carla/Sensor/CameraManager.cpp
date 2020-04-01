// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "CameraManager.h"

ACameraManager::ACameraManager(const FObjectInitializer &ObjectInitializer)
    : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics; // Before SceneCaptureSensor's TG_PostPhysics.
}

void ACameraManager::AddCamera(ASceneCaptureCamera *camera)
{
  Cameras.Add(camera);
}

void ACameraManager::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  // Remove dead sensors.
  int i;
  for (i = 0; i < Cameras.Num(); i++)
  {
    if (!IsValid(Cameras[i]))
      Cameras.RemoveAt(i);
  }

  if (Cameras.Num() < 1)
    return;

  // Config change.
  if (CameraManagerMode != CameraManagerMode_old)
  {
    switch (CameraManagerMode)
    {
    case ECameraManagerMode::TimeSharing:
      Cameras[0]->bToken = true;
      for (i = 1; i < Cameras.Num(); i++)
        Cameras[i]->bToken = false;
      break;
    case ECameraManagerMode::Normal:
      for (i = 0; i < Cameras.Num(); i++)
        Cameras[i]->bToken = true;
      break;
    }
    CameraManagerMode_old = CameraManagerMode;
  }

  // Management.
  bool ok = false;
  switch (CameraManagerMode)
  {
  case ECameraManagerMode::TimeSharing:
    for (i = 0; i < Cameras.Num(); i++)
    {
      if (Cameras[i]->bToken)
      {
        Cameras[i]->bToken = false;
        if (i == Cameras.Num() - 1)
          Cameras[0]->bToken = true;
        else
          Cameras[i + 1]->bToken = true;
        ok = true;
        break;
      }
    }
    if (!ok)
      Cameras[0]->bToken = true;
    break;
  case ECameraManagerMode::Normal:
  default:
    break;
  }
}
