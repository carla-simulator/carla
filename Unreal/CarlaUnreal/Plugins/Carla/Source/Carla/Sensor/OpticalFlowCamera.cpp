// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/OpticalFlowCamera.h"
#include "Carla.h"
#include "Carla/Actor/ActorBlueprintFunctionLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "HAL/IConsoleManager.h"
#include <util/ue-header-guard-end.h>

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
  Super::PostPhysTick(World, TickType, DeltaSeconds);

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
        check(sizeof(FVector2f) == sizeof(float) * 2);
        check(RowPitch >= Extent.X);
        check(BufferHeight >= Extent.Y);
        // UE_LOG(LogCarla, Log, TEXT("Format=%u"), (unsigned)Format);
        TArray<FVector2f> ImageData;
        ImageData.Reserve(Extent.X * Extent.Y);
        auto BasePtr = reinterpret_cast<const FFloat16Color*>(MappedPtr);
        for (uint32 i = 0; i != Extent.Y; ++i)
        {
          auto Ptr = BasePtr;
          for (uint32 j = 0; j != Extent.X; ++j)
          {
            FVector2f Out(
              Ptr->R.GetFloat(),
              Ptr->G.GetFloat());
            Out -= FVector2f(0.5F, 0.5F);
            Out *= 4.0F;
            ImageData.Add(Out);
            ++Ptr;
          }
          BasePtr += RowPitch;
        }
        SendDataToClient(
            *this,
            TArrayView<FVector2f>(ImageData),
            FrameIndex);
        return true;
      });
}
