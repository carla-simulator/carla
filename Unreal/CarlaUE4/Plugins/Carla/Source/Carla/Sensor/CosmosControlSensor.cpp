// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/CosmosControlSensor.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

#include "Components/SceneCaptureComponent2D.h"


FActorDefinition ACosmosControlSensor::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("cosmos_visualization"));
}

ACosmosControlSensor::ACosmosControlSensor(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  //AddPostProcessingMaterial(TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  // TODO: Setup OnActorSpawnHandler so we can refresh components
  // World->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateRaw(this, &ACosmosControlSensor::OnActorSpawned));
}

void ACosmosControlSensor::SetUpSceneCaptureComponent(USceneCaptureComponent2D &SceneCapture)
{
 Super::SetUpSceneCaptureComponent(SceneCapture);

 ApplyViewMode(VMI_Unlit, true, SceneCapture.ShowFlags);

 SceneCapture.ShowFlags.SetAtmosphere(false);

 SceneCapture.PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

//   TArray<UObject *> StaticMeshComps;
//   GetObjectsOfClass(UStaticMeshComponent::StaticClass(), StaticMeshComps, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

//   TArray<UPrimitiveComponent *> ShowOnlyComponents;
//   for (UObject *Object : StaticMeshComps) {
//     UPrimitiveComponent *Component = Cast<UStaticMeshComponent>(Object);
//     SceneCapture.ShowOnlyComponents.Emplace(Component);
//   }
}

void ACosmosControlSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ACosmosControlSensor::PostPhysTick);

  //USceneCaptureComponent2D* SceneCapture = GetCaptureComponent2D();
//   TArray<UObject *> StaticMeshComps;
//   GetObjectsOfClass(UStaticMeshComponent::StaticClass(), StaticMeshComps, false, EObjectFlags::RF_ClassDefaultObject, EInternalObjectFlags::AllFlags);

//   TArray<UPrimitiveComponent *> ShowOnlyComponents;
//   for (UObject *Object : StaticMeshComps) {
//     UPrimitiveComponent *Component = Cast<UStaticMeshComponent>(Object);
//     SceneCapture.ShowOnlyComponents.Emplace(Component);
//   }

  //FPixelReader::SendPixelsInRenderThread<ACosmosControlSensor, FColor>(*this);

}
