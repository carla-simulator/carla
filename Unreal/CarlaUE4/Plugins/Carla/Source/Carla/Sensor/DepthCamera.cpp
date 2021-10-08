// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/DepthCamera.h"

#include "Carla/Sensor/PixelReader.h"

FActorDefinition ADepthCamera::GetSensorDefinition()
{
  return UActorBlueprintFunctionLibrary::MakeCameraDefinition(TEXT("depth"));
}

ADepthCamera::ADepthCamera(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  AddPostProcessingMaterial(
      TEXT("Material'/Carla/PostProcessingMaterials/PhysicLensDistortion.PhysicLensDistortion'"));
  AddPostProcessingMaterial(
#if PLATFORM_LINUX
      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial_GLSL.DepthEffectMaterial_GLSL'")
#else
      TEXT("Material'/Carla/PostProcessingMaterials/DepthEffectMaterial.DepthEffectMaterial'")
#endif
  );
  BufferHeaderOffset = carla::sensor::SensorRegistry::get<ADepthCamera *>::type::header_offset;
}

void ADepthCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ADepthCamera::PostPhysTick);
  EnqueueRenderSceneImmediate();
}

void ADepthCamera::SendPixelsInRenderThread(const TArray<FColor>& AtlasPixels, uint32 AtlasTextureWidth)
{
  FString ScopeStr = FString::Printf(
    TEXT("ADepthCamera::SendPixelsInRenderThread %d %s"),
    FCarlaEngine::GetFrameCounter(),
    *(FDateTime::Now().ToString(TEXT("%H:%M:%S.%s")))
  );
  TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*ScopeStr);

  auto Stream = GetDataStream(*this);
  carla::Buffer Buffer = Stream.PopBufferFromPool();

  CopyTextureFromAtlas(Buffer, AtlasPixels, AtlasTextureWidth);

  if(Buffer.data())
  {
    SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
    Stream.Send(*this, std::move(Buffer));
  }
}

void ADepthCamera::BeginPlay()
{
  Super::BeginPlay();
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
  if(GameMode)
  {
    ASensorManager* SensorManager = GameMode->GetSensorManager();
    SensorManager->Register8BitCameraSensor(this);
  }
}

void ADepthCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
  if(GameMode)
  {
    ASensorManager* SensorManager = GameMode->GetSensorManager();
    SensorManager->Unregister8BitCameraSensor(this);
  }
}
