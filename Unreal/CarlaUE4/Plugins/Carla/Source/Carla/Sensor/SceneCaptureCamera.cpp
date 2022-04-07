// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureCamera.h"
#include "Carla/Sensor/NetMediaCapture.h"

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

void ASceneCaptureCamera::BeginPlay()
{
  Super::BeginPlay();

  MediaOutput = NewObject<UNetMediaOutput>();
  UE_LOG(LogCarla, Log, TEXT("NetMediaOutput created"));
  MediaOutput->CreateMediaCapture();
  UNetMediaCapture *MediaCapture = MediaOutput->GetMediaCapture();
  if (MediaCapture)
  {
    // callback
    MediaCapture->SetCallback([Sensor=this](std::vector<uint8_t> InBuffer, int32 Width, int32 Height, EPixelFormat PixelFormat)
	  {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("OnCapturedUserCallback");

      /// @todo Can we make sure the sensor is not going to be destroyed?
      if (!Sensor->IsPendingKill())
      {
        auto Stream = Sensor->GetDataStream(*Sensor);
        auto Buffer = Stream.PopBufferFromPool();
        Buffer.copy_from(carla::sensor::s11n::ImageSerializer::header_offset, boost::asio::buffer(InBuffer.data(), InBuffer.size()));
        if(Buffer.data())
        {
          SCOPE_CYCLE_COUNTER(STAT_CarlaSensorStreamSend);
          TRACE_CPUPROFILER_EVENT_SCOPE_STR("Stream Send");
          Stream.Send(*Sensor, std::move(Buffer));
        }
      }
    });
  }
}

void ASceneCaptureCamera::OnFirstClientConnected()
{
  if (MediaOutput)
  {
    UNetMediaCapture *MediaCapture = MediaOutput->GetMediaCapture();
    FMediaCaptureOptions CaptureOptions;
    CaptureOptions.bSkipFrameWhenRunningExpensiveTasks = false;
    MediaCapture->CaptureTextureRenderTarget2D(GetCaptureRenderTarget(), CaptureOptions);
  }
}

void ASceneCaptureCamera::OnLastClientDisconnected()
{
  // stop capturing
  if(MediaOutput)
  {
    UNetMediaCapture *MediaCapture = MediaOutput->GetMediaCapture();
    if (MediaCapture)
    {
      MediaCapture->StopCapture(false);
    }
  }
}

void ASceneCaptureCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // stop capturing
  UNetMediaCapture *MediaCapture = MediaOutput->GetMediaCapture();
  if (MediaCapture)
  {
    MediaCapture->StopCapture(false);
  }
  Super::EndPlay(EndPlayReason);
}

void ASceneCaptureCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::PostPhysTick);
  CaptureComponent2D->CaptureScene();
}
