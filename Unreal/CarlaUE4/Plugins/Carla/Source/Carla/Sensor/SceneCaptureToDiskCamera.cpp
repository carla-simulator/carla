// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "SceneCaptureToDiskCamera.h"

#include "HighResScreenshot.h"
#include "Paths.h"

ASceneCaptureToDiskCamera::ASceneCaptureToDiskCamera(const FObjectInitializer& ObjectInitializer) :
  Super(ObjectInitializer),
  SaveToFolder(*FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("SceneCaptures"))),
  FileName(TEXT("%06d.png")) {}

void ASceneCaptureToDiskCamera::BeginPlay()
{
  Super::BeginPlay();

  PrimaryActorTick.bCanEverTick = bCaptureScene;
  PrimaryActorTick.TickInterval = 1.0f / CapturesPerSecond;

  CaptureFileNameCount = 0u;
}

void ASceneCaptureToDiskCamera::Tick(const float DeltaTime)
{
  Super::Tick(DeltaTime);

  if (bCaptureScene) {
    const FString FilePath = FPaths::Combine(SaveToFolder, FString::Printf(TEXT("%06d.png"), CaptureFileNameCount));
    UE_LOG(LogCarla, Log, TEXT("DeltaTime %fs: Capture %s"), DeltaTime, *FilePath);
    SaveCaptureToDisk(FilePath);
    ++CaptureFileNameCount;
  }
}

bool ASceneCaptureToDiskCamera::SaveCaptureToDisk(const FString &FilePath) const
{
  TArray<FColor> OutBMP;
  if (!ReadPixels(OutBMP)) {
    return false;
  }
  for (FColor &color : OutBMP) {
    color.A = 255;
  }
  const FIntPoint DestSize(GetImageSizeX(), GetImageSizeY());
  FString ResultPath;
  FHighResScreenshotConfig &HighResScreenshotConfig = GetHighResScreenshotConfig();
  return HighResScreenshotConfig.SaveImage(FilePath, OutBMP, DestSize, &ResultPath);
}
