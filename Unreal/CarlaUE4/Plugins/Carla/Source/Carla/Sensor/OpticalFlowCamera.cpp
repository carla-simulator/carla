#include "Carla.h"
#include "Carla/Sensor/OpticalFlowCamera.h"
#include "HAL/IConsoleManager.h"

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
  BufferHeaderOffset = carla::sensor::SensorRegistry::get<AOpticalFlowCamera *>::type::header_offset;
}

void AOpticalFlowCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  auto CVarForceOutputsVelocity = IConsoleManager::Get().FindConsoleVariable(TEXT("r.BasePassForceOutputsVelocity"));
  int32 OldValue = CVarForceOutputsVelocity->GetInt();
  CVarForceOutputsVelocity->Set(1);
  EnqueueRenderSceneImmediate();
  CVarForceOutputsVelocity->Set(OldValue);
}

void AOpticalFlowCamera::SendPixelsInRenderThread(const TArray<FFloat16Color>& AtlasPixels, uint32 AtlasTextureWidth)
{
  FString ScopeStr = FString::Printf(
    TEXT("AOpticalFlowCamera::SendPixelsInRenderThread %d %s"),
    FCarlaEngine::GetFrameCounter(),
    *(FDateTime::Now().ToString(TEXT("%H:%M:%S.%s")))
  );
  TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*ScopeStr);

  auto Stream = GetDataStream(*this);
  carla::Buffer Buffer = Stream.PopBufferFromPool();

  CopyOpticalFlowFromAtlas(Buffer, AtlasPixels, AtlasTextureWidth);

  if(Buffer.data())
  {
    SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
    Stream.Send(*this, std::move(Buffer));
  }
}

void AOpticalFlowCamera::CopyOpticalFlowFromAtlas(carla::Buffer &Buffer,
    const TArray<FFloat16Color>& AtlasPixels,
    uint32 AtlasTextureWidth)
{
  TRACE_CPUPROFILER_EVENT_SCOPE_STR("CopyOpticalFlowFromAtlas");

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer.reset");
    Buffer.reset(BufferHeaderOffset + ImageWidth * ImageHeight * sizeof(FColor));
  }

  // Check that the atlas alreay contains our texture
  // and our image has been initialized
  uint32 ExpectedSize = (uint32)(PositionInAtlas.Y * AtlasTextureWidth + ImageWidth * ImageHeight);
  uint32 TotalSize = (uint32)AtlasPixels.Num();
  if(AtlasPixels.GetData() && TotalSize < ExpectedSize)
  {
    return;
  }

  TArray<float> IntermediateBuffer;
  IntermediateBuffer.Reserve(ImageWidth*ImageHeight*2);
  for (uint32 j = PositionInAtlas.Y; j < PositionInAtlas.Y + ImageHeight; ++j)
  {
    for (uint32 i = PositionInAtlas.X; i < PositionInAtlas.X + ImageWidth; ++i)
    {
      const FFloat16Color& Color = AtlasPixels[i + j*AtlasTextureWidth];
      float x = (Color.R.GetFloat() - 0.5f)*4.f;
      float y = (Color.G.GetFloat() - 0.5f)*4.f;
      IntermediateBuffer.Add(x);
      IntermediateBuffer.Add(y);
    }
  }
  Buffer.copy_from(BufferHeaderOffset, IntermediateBuffer);
}

void AOpticalFlowCamera::BeginPlay()
{
  Super::BeginPlay();
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
  if(GameMode)
  {
    ASensorManager* SensorManager = GameMode->GetSensorManager();
    SensorManager->Register16BitCameraSensor(this);
  }
}

void AOpticalFlowCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  ACarlaGameModeBase* GameMode = UCarlaStatics::GetGameMode(GetWorld());
  if(GameMode)
  {
    ASensorManager* SensorManager = GameMode->GetSensorManager();
    SensorManager->Unregister16BitCameraSensor(this);
  }
}
