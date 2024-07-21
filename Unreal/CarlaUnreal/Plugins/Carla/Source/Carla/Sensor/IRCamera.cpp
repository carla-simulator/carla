// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/IRCamera.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Carla.h"
#include "Carla/Game/CarlaGameInstance.h"
#include "Carla/Game/CarlaEngine.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Materials/MaterialParameterCollection.h"

#include <chrono>

#include "Actor/ActorBlueprintFunctionLibrary.h"

#include "Runtime/RenderCore/Public/RenderingThread.h"

FActorDefinition AIRCamera::GetSensorDefinition()
{
    constexpr bool bEnableModifyingPostProcessEffects = true;
    return UActorBlueprintFunctionLibrary::MakeCameraDefinition(
        TEXT("ir"),
        bEnableModifyingPostProcessEffects);
}

AIRCamera::AIRCamera(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AddPostProcessingMaterial(
        TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));

	AddPostProcessingMaterial(TEXT("Material'/Game/DTC/IRCamera/PP_IR_Thermal.PP_IR_Thermal'"));
	AddPostProcessingMaterial(TEXT("Material'/Game/DTC/IRCamera/PP_CameraBlur.PP_CameraBlur'"));
}

void AIRCamera::BeginPlay()
{
  Super::BeginPlay();

}

void AIRCamera::OnFirstClientConnected()
{
}

void AIRCamera::OnLastClientDisconnected()
{
}

void AIRCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
}

void AIRCamera::PrePhysTick(float DeltaSeconds)
{
    Super::PrePhysTick(DeltaSeconds);
    UCarlaGameInstance* gi = UCarlaGameInstance::Get(this);
    if (gi != nullptr)
    {
        gi->HandlePreIRCapture();
    }
}

void AIRCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(AIRCamera::PostPhysTick);
  Super::PostPhysTick(World, TickType, DeltaSeconds);
  
  ENQUEUE_RENDER_COMMAND(MeasureTime)
  (
    [](auto &InRHICmdList)
    {
      std::chrono::time_point<std::chrono::high_resolution_clock> Time = 
          std::chrono::high_resolution_clock::now();
      auto Duration = std::chrono::duration_cast< std::chrono::milliseconds >(Time.time_since_epoch());
      uint64_t Milliseconds = Duration.count();
      FString ProfilerText = FString("(Render)Frame: ") + FString::FromInt(FCarlaEngine::GetFrameCounter()) + 
          FString(" Time: ") + FString::FromInt(Milliseconds);
      TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*ProfilerText);
    }
  );

  UCarlaGameInstance* gi = UCarlaGameInstance::Get(this);
  if (gi != nullptr)
  {
      gi->HandlePostIRCapture();
  }

  auto FrameIndex = FCarlaEngine::GetFrameCounter();
  ImageUtil::ReadSensorImageDataAsyncFColor(*this, [this, FrameIndex](
    TArrayView<const FColor> Pixels,
    FIntPoint Size) -> bool
  {
    SendImageDataToClient(*this, Pixels, FrameIndex);
    return true;
  });
}

#ifdef CARLA_HAS_GBUFFER_API
void AIRCamera::SendGBufferTextures(FGBufferRequest& GBuffer)
{
    SendGBufferTexturesInternal(*this, GBuffer);
}
#endif
