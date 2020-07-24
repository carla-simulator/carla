// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"

FActorDefinition ASceneCaptureCamera::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(
      TEXT("rgb"),
      bEnableModifyingPostProcessEffects);
}

ASceneCaptureCamera::ASceneCaptureCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));

  Offset = carla::sensor::SensorRegistry::get<ASceneCaptureCamera*>::type::header_offset;
}

void ASceneCaptureCamera::Tick(float DeltaTime)
{
  Super::Tick(DeltaTime);

  ACarlaGameModeBase* GameMode = Cast<ACarlaGameModeBase>(GetWorld()->GetAuthGameMode());
  SendPixelsInStream(*this, GameMode->GetCurrentAtlasPixels(), GameMode->GetAtlasTextureWidth());
}