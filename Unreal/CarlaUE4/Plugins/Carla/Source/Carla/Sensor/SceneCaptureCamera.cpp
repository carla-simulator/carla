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
  MediaOutput->BaseFileName = "Captured";
  MediaOutput->FilePath.Path = "c:/captured/";
  MediaOutput->WriteOptions.bAsync = true;
  MediaOutput->WriteOptions.bOverwriteFile = true;
  MediaOutput->WriteOptions.CompressionQuality = 0;
  MediaOutput->WriteOptions.Format = EDesiredImageFormat::JPG;
  MediaOutput->DesiredPixelFormat = ENetMediaOutputPixelFormat::B8G8R8A8;
  MediaOutput->DesiredSize = FIntPoint(GetCaptureRenderTarget()->SizeX, GetCaptureRenderTarget()->SizeY);
  MediaOutput->CreateMediaCapture();
  UNetMediaCapture *MediaCapture = MediaOutput->GetMediaCapture();
  UE_LOG(LogCarla, Log, TEXT("NetMediaCapture: RenderTarget size %d x %d"), MediaOutput->DesiredSize.X, MediaOutput->DesiredSize.Y);
  if (MediaCapture)
  {
    FIntPoint Size1 = MediaOutput->GetRequestedSize();
    UE_LOG(LogCarla, Log, TEXT("NetMediaCapture size %d x %d"), Size1.X, Size1.Y);
    if (MediaCapture->CaptureTextureRenderTarget2D(GetCaptureRenderTarget(), FMediaCaptureOptions()))
    {
      UE_LOG(LogCarla, Log, TEXT("NetMediaCapture created and started"));
    }
    else
    {
      UE_LOG(LogCarla, Log, TEXT("NetMediaCapture created but failing to start"));
    }
    Size1 = MediaOutput->GetRequestedSize();
    UE_LOG(LogCarla, Log, TEXT("NetMediaCapture size %d x %d"), Size1.X, Size1.Y);
    
    // callback
    MediaCapture->SetCallback([Sensor=this](void *InBuffer, int32 Width, int32 Height, EPixelFormat PixelFormat)
	  {
      TRACE_CPUPROFILER_EVENT_SCOPE_STR("OnCapturedUserCallback");

      /// @todo Can we make sure the sensor is not going to be destroyed?
      if (!Sensor->IsPendingKill())
      {
        auto Stream = Sensor->GetDataStream(*Sensor);
        auto Buffer = Stream.PopBufferFromPool();

        if (PixelFormat == PF_B8G8R8A8)
        {
          TArray<FColor> Pixels = TArray<FColor>(reinterpret_cast<FColor*>(InBuffer), Width * Height);
          // Buffer.copy_from(carla::sensor::SensorRegistry::get<ASceneCaptureCamera *>.second::type::header_offset, Pixels);
          Buffer.copy_from(carla::sensor::s11n::ImageSerializer::header_offset, Pixels);
        }
        else if (PixelFormat == PF_FloatRGBA)
        {
          TArray<FFloat16Color> Pixels = TArray<FFloat16Color>(reinterpret_cast<FFloat16Color*>(InBuffer), Width * Height);
          Buffer.copy_from(carla::sensor::s11n::ImageSerializer::header_offset, Pixels);
        }

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

void ASceneCaptureCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // stop capturing
  UNetMediaCapture *MediaCapture = MediaOutput->GetMediaCapture();
  if (MediaCapture)
  {
    MediaCapture->StopCapture(true);
  }

  Super::EndPlay(EndPlayReason);
}

void ASceneCaptureCamera::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera::PostPhysTick);
  // FPixelReader::SendPixelsInRenderThread(*this);
  CaptureComponent2D->CaptureScene();
}
