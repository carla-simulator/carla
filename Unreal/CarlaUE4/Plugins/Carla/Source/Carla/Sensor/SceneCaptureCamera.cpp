// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Runtime/RenderCore/Public/RenderingThread.h"

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
}

template <typename T>
static void SendGBuffer(ASceneCaptureSensor& self, T& CameraGBuffer, FGBufferData& GBuffer, size_t index)
{
  static constexpr const TCHAR* TextureNames[] =
  {
    TEXT("SceneColor"),
    TEXT("SceneDepth"),
    TEXT("GBufferA"),
    TEXT("GBufferB"),
    TEXT("GBufferC"),
    TEXT("GBufferD"),
    TEXT("GBufferE"),
    TEXT("GBufferF"),
    TEXT("Velocity"),
    TEXT("SSAO")
  };
  auto Name = TextureNames[index];
  auto GBufferStream = CameraGBuffer.GetDataStream(self);
  auto Buffer = GBufferStream.PopBufferFromPool();
  auto& Payload = GBuffer.Payloads[index];
  auto Readback = Payload.Readback.Get();
  while (!Readback->IsReady())
  {
    if (Payload.is_dummy_texture.load(std::memory_order_acquire))
      return;
    std::this_thread::yield();
  }
  auto DestinationExtent = Payload.Extent;
  auto SourceExtent = Payload.InternalExtent;
  auto Format = Payload.Format;
  check(Format != PF_Unknown);
  auto Pixels = ImageUtil::ExtractTexturePixelsFromReadback(
    Readback,
    SourceExtent,
    DestinationExtent,
    Format);
  check(Pixels.Num() != 0);
  check(Pixels.Num() == DestinationExtent.X * DestinationExtent.Y);
#if 0
  static int Counter;
  ++Counter;
  auto ImageTask = MakeUnique<FImageWriteTask>();
  ImageTask->PixelData = MakeUnique<TImagePixelData<FColor>>(DestinationExtent, TArray64<FColor>(MoveTemp(Pixels)));
  ImageTask->Filename = FString::Printf(TEXT("M:\\carla-screenshots\\%u-%s.png"), Counter, Name);
  ImageTask->Format = EImageFormat::PNG;
  ImageTask->CompressionQuality = (int32)EImageCompressionQuality::Default;
  ImageTask->bOverwriteFile = true;
  ImageTask->PixelPreProcessors.Add(TAsyncAlphaWrite<FColor>(255));
  auto& HighResScreenshotConfig = GetHighResScreenshotConfig();
  check(HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask)).Get());
#else
  Buffer.copy_from(carla::sensor::SensorRegistry::get<T*>::type::header_offset, Pixels);
  UE_LOG(LogCarla, Log, TEXT("Sending GBuffer (%u, %u)"), DestinationExtent.X, DestinationExtent.Y);
  if (!Buffer.empty())
  {
    SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
    GBufferStream.Send(CameraGBuffer, std::move(Buffer), DestinationExtent.X, DestinationExtent.Y, self.GetFOVAngle());
  }
  else
  {
    UE_LOG(LogCarla, Log, TEXT("Failed to send GBuffer due to empty temporary buffer"));
  }
#endif
}

void ASceneCaptureCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::PostPhysTick);
  FPixelReader::SendPixelsInRenderThread(*this);
#if 0
  {
    auto GBufferStream = CameraGBuffers.SceneColor.GetDataStream(*this);
    auto Buffer = GBufferStream.PopBufferFromPool();
    TArray<FColor> SceneColorPixels;
    SceneColorPixels.SetNum(GetImageWidth()*GetImageHeight());
    for (FColor& color : SceneColorPixels)
      color = FColor(255, 128, 64, 200);
    auto Offset = carla::sensor::SensorRegistry::get<FCameraGBufferUint8*>::type::header_offset;
    Buffer.copy_from(Offset, SceneColorPixels);
    if(Buffer.data())
    {
      SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
      GBufferStream.Send(CameraGBuffers.SceneColor, std::move(Buffer), GetImageWidth(), GetImageHeight(), GetFOVAngle());
    }
  }
#else
  UE_LOG(LogCarla, Log, TEXT("FRenderCommandFence wait finished."));
  //AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this] {
  //ENQUEUE_RENDER_COMMAND(SendGBufferTask)([this](auto& RHICmdList) {
    for (size_t i = 0; i != FGBufferData::TextureCount; ++i)
    {
      if ((GBuffer.DesiredTexturesMask & (UINT64_C(1) << i)) == 0)
        continue;
      switch (i)
      {
        case 0:
          SendGBuffer(*this, CameraGBuffers.SceneColor, GBuffer, i);
          break;
        case 1:
          SendGBuffer(*this, CameraGBuffers.SceneDepth, GBuffer, i);
          break;
        case 2:
          SendGBuffer(*this, CameraGBuffers.GBufferA, GBuffer, i);
          break;
        case 3:
          SendGBuffer(*this, CameraGBuffers.GBufferB, GBuffer, i);
          break;
        case 4:
          SendGBuffer(*this, CameraGBuffers.GBufferC, GBuffer, i);
          break;
        case 5:
          SendGBuffer(*this, CameraGBuffers.GBufferD, GBuffer, i);
          break;
        case 6:
          SendGBuffer(*this, CameraGBuffers.GBufferE, GBuffer, i);
          break;
        case 7:
          SendGBuffer(*this, CameraGBuffers.GBufferF, GBuffer, i);
          break;
        case 8:
          SendGBuffer(*this, CameraGBuffers.Velocity, GBuffer, i);
          break;
        case 9:
          SendGBuffer(*this, CameraGBuffers.SSAO, GBuffer, i);
          break;
        default:
          check(false);
      }
    }
  //});
#endif
}
