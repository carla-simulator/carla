// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/OpticalFlowCamera.h"
#include "Carla.h"
#include "HAL/IConsoleManager.h"

#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition AOpticalFlowCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("optical_flow"));
}

AOpticalFlowCamera::AOpticalFlowCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  Enable16BitFormat(true);
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
          TEXT("Material'/Carla/PostProcessingMaterials/VelocityMaterial.VelocityMaterial'"));
}

void AOpticalFlowCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AOpticalFlowCamera::PostPhysTick);
  
  // auto CVarForceOutputsVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BasePassForceOutputsVelocity"));
  // int32 OldValue = CVarForceOutputsVelocity->GetInt();
  // CVarForceOutputsVelocity->Set(1);

  auto FrameIndex = FCarlaEngine::GetFrameCounter();
  ImageUtil::ReadImageDataAsync(
      *GetCaptureRenderTarget(),
      [this, FrameIndex](
          const void* MappedPtr,
          size_t RowPitch,
          size_t BufferHeight,
          EPixelFormat Format,
          FIntPoint Extent)
      {
          TArray<FVector2f> ImageData;
          ImageData.SetNum(Extent.X * Extent.Y);
          auto Ptr = reinterpret_cast<const FFloat16Color*>(MappedPtr);
          for (auto& Out : ImageData)
          {
              Out.X = Ptr->R.GetFloat();
              Out.Y = Ptr->G.GetFloat();
              Out -= FVector2f(0.5F);
              Out *= 4.0F;
          }
          SendImageDataToClient(
              *this,
              TArrayView<FVector2f>(ImageData),
              FrameIndex);
          return true;
      });

  // CVarForceOutputsVelocity->Set(OldValue);
}
