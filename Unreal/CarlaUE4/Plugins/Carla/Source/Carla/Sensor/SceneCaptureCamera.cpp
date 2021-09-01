// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Components/SceneCaptureComponent2D.h"

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
  BufferHeaderOffset = carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>::type::header_offset;
}

void ASceneCaptureCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::PostPhysTick);
  UE_LOG(LogCarla, Error, TEXT("ASceneCaptureCamera::PostPhysTick"));
  EnqueueRenderSceneImmediate();
  ENQUEUE_RENDER_COMMAND(ASceneCaptureCamera_PostPhysTick)
  (
    [](FRHICommandListImmediate& RHICmdList)
    {
      RHICmdList.ImmediateFlush(EImmediateFlushType::Type::DispatchToRHIThread);
    }
  );
  //SendPixelsInRenderThread();
  /*
  if(CaptureComponent2D->PixelsBuffer.Num() > 0)
  {
    auto Stream = GetDataStream(*this);
    auto Buffer = Stream.PopBufferFromPool();
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer Copy");
      Buffer.copy_from(
        carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>::type::header_offset,
        CaptureComponent2D->PixelsBuffer);
    }

    if(Buffer.data())
    {
      SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
      Stream.Send(*this, std::move(Buffer));
    }
  }
  */
}

void ASceneCaptureCamera::SendPixelsInRenderThread(const TArray<FColor>& AtlasPixels, uint32 AtlasTextureWidth)
{
  //TRACE_CPUPROFILER_EVENT_SCOPE("ASceneCaptureCamera::SendPixelsInRenderThread");
  FString ScopeStr = FString::Printf(
    TEXT("ASceneCaptureCamera::SendPixelsInRenderThread %d %s"),
    FCarlaEngine::GetFrameCounter(),
    *(FDateTime::Now().ToString(TEXT("%H:%M:%S.%s")))
  );
  TRACE_CPUPROFILER_EVENT_SCOPE_TEXT(*ScopeStr);
  UE_LOG(LogCarla, Error, TEXT("SendPxRT %d %s"), FCarlaEngine::GetFrameCounter(), *(FDateTime::Now().ToString(TEXT("%H:%M:%S.%s"))));

  auto Stream = GetDataStream(*this);
  carla::Buffer Buffer = Stream.PopBufferFromPool();

  CopyTextureFromAtlas(Buffer, AtlasPixels, AtlasTextureWidth);

  if(Buffer.data())
  {
    SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
    Stream.Send(*this, std::move(Buffer));
  }
  UE_LOG(LogCarla, Error, TEXT("SendPxRT end %d %s"), FCarlaEngine::GetFrameCounter(), *(FDateTime::Now().ToString(TEXT("%H:%M:%S.%s"))));
}

/*
void ASceneCaptureCamera::SendPixelsInRenderThread()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::SendPixelsInRenderThread);
  check(CaptureRenderTarget != nullptr);

  if (!HasActorBegunPlay() || IsPendingKill())
  {
    return;
  }

  ACarlaGameModeBase* GM = UCarlaStatics::GetGameMode(GWorld);
  bool CaptureEveryFrame = GM->IsCaptureEveryFrameEnabled();
  bool DownloadTexture = GM->IsDownloadTextureEnabled();
  bool RHIGPUReadBack = GM->IsRHIGPUReadBackEnabled();
  bool ReadSurfaceWaitUntilIdle = GM->IsReadSurfaceWaitUntilIdleEnabled();

  if(!CaptureEveryFrame)
  {
    /// Blocks until the render thread has finished all it's tasks.
    EnqueueRenderSceneImmediate();
  }

  // Enqueue a command in the render-thread that will write the image buffer to
  // the data stream. The stream is created in the capture thus executed in the
  // game-thread.
  ENQUEUE_RENDER_COMMAND(SendPixelsInRenderThread)
  (
    [&, DownloadTexture, RHIGPUReadBack, Stream=GetDataStream(*this)](auto &InRHICmdList) mutable
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("SendPixelsInRenderThread");

      /// @todo Can we make sure the sensor is not going to be destroyed?
      if (!IsPendingKill())
      {
        auto Buffer = Stream.PopBufferFromPool();

        WritePixelsToBuffer(
            *CaptureRenderTarget,
            Buffer,
            carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>::type::header_offset,
            InRHICmdList,
            DownloadTexture,
            RHIGPUReadBack);

        if(Buffer.data())
        {
          SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
          TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
          Stream.Send(*this, std::move(Buffer));
        }
      }
    }
  );
  // Blocks until the render thread has finished all it's tasks
  //Sensor.WaitForRenderThreadToFinsih();
}

void ASceneCaptureCamera::WritePixelsToBuffer(
    UTextureRenderTarget2D &RenderTarget,
    carla::Buffer &Buffer,
    uint32 Offset,
    FRHICommandListImmediate &InRHICmdList,
    bool DownloadTexture,
    bool RHIGPUReadBack
    )
{

  TRACE_CPUPROFILER_EVENT_SCOPE_STR("ASceneCaptureCamera::WritePixelsToBuffer");
  check(IsInRenderingThread());

  const FTextureRenderTarget2DResource* RenderResource = static_cast<const FTextureRenderTarget2DResource*>(RenderTarget.Resource);
  FTexture2DRHIRef CameraTexture = RenderResource->GetRenderTargetTexture();
  if (!CameraTexture)
  {
    return;
  }

  FIntPoint Rect = RenderResource->GetSizeXY();

  // NS: Extra copy here, don't know how to avoid it.
  if(DownloadTexture)
  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("DownloadTexture");

    if (RHIGPUReadBack)
    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("RHIGPUReadBack");

      if (!Fence)
      {
        Fence = RHICreateGPUFence("CameraSensorFence");
      }

      Fence->Clear();
      FIntVector TextureSize = CameraTexture->GetSizeXYZ();

      if (!DestinationStagingTexture)
      {
        FString FenceName = Fence->GetFName().ToString();
        FRHIResourceCreateInfo CreateInfo(*FenceName);
        DestinationStagingTexture = RHICreateTexture2D(TextureSize.X, TextureSize.Y, CameraTexture->GetFormat(), 1, 1, TexCreate_CPUReadback | TexCreate_HideInVisualizeTexture, CreateInfo);
      }

      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("FRHITransition_CopyDest");
        // We need the destination texture to be writable from a copy operation
        InRHICmdList.Transition(FRHITransitionInfo(DestinationStagingTexture, ERHIAccess::Unknown, ERHIAccess::CopyDest));
      }

      FResolveParams ResolveParams;
      ResolveParams.Rect = FResolveRect(0, 0, TextureSize.X, TextureSize.Y);
      // Ensure this copy call does not perform any transitions. We're handling them manually.
      ResolveParams.SourceAccessFinal = ERHIAccess::Unknown;
      ResolveParams.DestAccessFinal = ERHIAccess::Unknown;

      // Transfer memory GPU -> CPU
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("FRHITransition_CopyToResolveTarget");
        InRHICmdList.CopyToResolveTarget(CameraTexture, DestinationStagingTexture, ResolveParams);
      }

      // Transition the dest to CPURead *before* signaling the fence, otherwise ordering is not guaranteed.
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("FRHITransition_CPURead");
        InRHICmdList.Transition(FRHITransitionInfo(DestinationStagingTexture, ERHIAccess::CopyDest, ERHIAccess::CPURead));
      }
      {
        TRACE_CPUPROFILER_EVENT_SCOPE_STR("WriteGPUFence");
        InRHICmdList.WriteGPUFence(Fence);
      }

      FRHITexture* RHITexture = DestinationStagingTexture.GetReference();
      FRHITexture2D* TempTexture2D = static_cast<FRHITexture2D*>(RHITexture);
      CameraTexture = TempTexture2D;
    }

    {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("Read Surface");
      InRHICmdList.ReadSurfaceData(
          CameraTexture,
          FIntRect(0, 0, Rect.X, Rect.Y),
          Pixels,
          FReadSurfaceDataFlags(RCM_UNorm, CubeFace_MAX));
    }
  }

  {
    TRACE_CPUPROFILER_EVENT_SCOPE_STR("Buffer Copy");
    Buffer.copy_from(Offset, Pixels);
  }
}
*/
