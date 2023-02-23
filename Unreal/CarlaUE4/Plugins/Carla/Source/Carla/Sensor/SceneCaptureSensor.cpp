// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Game/CarlaStatics.h"
#include "HighResScreenshot.h"
#include "Runtime/ImageWriteQueue/Public/ImageWriteQueue.h"

#include <mutex>
#include <atomic>
#include <thread>

static auto SCENE_CAPTURE_COUNTER = 0u;

// =============================================================================
// -- Local static methods -----------------------------------------------------
// =============================================================================

// Local namespace to avoid name collisions on unit builds.
namespace SceneCaptureSensor_local_ns {

  static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D);

  static void ConfigureShowFlags(FEngineShowFlags &ShowFlags, bool bPostProcessing = true);

  static auto GetQualitySettings(UWorld *World)
  {
    auto Settings = UCarlaStatics::GetCarlaSettings(World);
    check(Settings != nullptr);
    return Settings->GetQualityLevel();
  }

} // namespace SceneCaptureSensor_local_ns

// =============================================================================
// -- ASceneCaptureSensor ------------------------------------------------------
// =============================================================================

ASceneCaptureSensor::ASceneCaptureSensor(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  PrimaryActorTick.bCanEverTick = true;
  PrimaryActorTick.TickGroup = TG_PrePhysics;

  CaptureRenderTarget = CreateDefaultSubobject<UTextureRenderTarget2D>(
      FName(*FString::Printf(TEXT("CaptureRenderTarget_d%d"), SCENE_CAPTURE_COUNTER)));
  CaptureRenderTarget->CompressionSettings = TextureCompressionSettings::TC_Default;
  CaptureRenderTarget->SRGB = false;
  CaptureRenderTarget->bAutoGenerateMips = false;
  CaptureRenderTarget->bGPUSharedFlag = true;
  CaptureRenderTarget->AddressX = TextureAddress::TA_Clamp;
  CaptureRenderTarget->AddressY = TextureAddress::TA_Clamp;

  CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D_CARLA>(
      FName(*FString::Printf(TEXT("USceneCaptureComponent2D_CARLA_%d"), SCENE_CAPTURE_COUNTER)));
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->ViewActor = this;
  CaptureComponent2D->SetupAttachment(RootComponent);
  CaptureComponent2D->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;
  CaptureComponent2D->bCaptureOnMovement = false;
  CaptureComponent2D->bCaptureEveryFrame = false;
  CaptureComponent2D->bAlwaysPersistRenderingState = true;

  SceneCaptureSensor_local_ns::SetCameraDefaultOverrides(*CaptureComponent2D);

  ++SCENE_CAPTURE_COUNTER;
}

void ASceneCaptureSensor::Set(const FActorDescription &Description)
{
  Super::Set(Description);
  UActorBlueprintFunctionLibrary::SetCamera(Description, this);
}

void ASceneCaptureSensor::SetImageSize(uint32 InWidth, uint32 InHeight)
{
  ImageWidth = InWidth;
  ImageHeight = InHeight;
}

void ASceneCaptureSensor::SetFOVAngle(const float FOVAngle)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->FOVAngle = FOVAngle;
}

float ASceneCaptureSensor::GetFOVAngle() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->FOVAngle;
}

void ASceneCaptureSensor::SetExposureMethod(EAutoExposureMethod Method)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.AutoExposureMethod = Method;
}

EAutoExposureMethod ASceneCaptureSensor::GetExposureMethod() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureMethod;
}

void ASceneCaptureSensor::SetExposureCompensation(float Compensation)
{
  check(CaptureComponent2D != nullptr);
#if PLATFORM_LINUX
  // Looks like Windows and Linux have different outputs with the
  // same exposure compensation, this fixes it.
  CaptureComponent2D->PostProcessSettings.AutoExposureBias = Compensation + 0.75f;
#else
  CaptureComponent2D->PostProcessSettings.AutoExposureBias = Compensation;
#endif
}

float ASceneCaptureSensor::GetExposureCompensation() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureBias;
}

void ASceneCaptureSensor::SetShutterSpeed(float Speed)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.CameraShutterSpeed = Speed;
}

float ASceneCaptureSensor::GetShutterSpeed() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.CameraShutterSpeed;
}

void ASceneCaptureSensor::SetISO(float ISO)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.CameraISO = ISO;
}

float ASceneCaptureSensor::GetISO() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.CameraISO;
}

void ASceneCaptureSensor::SetAperture(float Aperture)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.DepthOfFieldFstop = Aperture;
}

float ASceneCaptureSensor::GetAperture() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.DepthOfFieldFstop;
}

void ASceneCaptureSensor::SetFocalDistance(float Distance)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.DepthOfFieldFocalDistance = Distance;
}

float ASceneCaptureSensor::GetFocalDistance() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.DepthOfFieldFocalDistance;
}

void ASceneCaptureSensor::SetDepthBlurAmount(float Amount)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.DepthOfFieldDepthBlurAmount = Amount;
}

float ASceneCaptureSensor::GetDepthBlurAmount() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.DepthOfFieldDepthBlurAmount;
}

void ASceneCaptureSensor::SetDepthBlurRadius(float Radius)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.DepthOfFieldDepthBlurRadius = Radius;
}

float ASceneCaptureSensor::GetDepthBlurRadius() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.DepthOfFieldDepthBlurRadius;
}

void ASceneCaptureSensor::SetDepthOfFieldMinFstop(float MinFstop)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.DepthOfFieldMinFstop = MinFstop;
}

float ASceneCaptureSensor::GetDepthOfFieldMinFstop() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.DepthOfFieldMinFstop;
}

void ASceneCaptureSensor::SetBladeCount(int Count)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.DepthOfFieldBladeCount = Count;
}

int ASceneCaptureSensor::GetBladeCount() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.DepthOfFieldBladeCount;
}

void ASceneCaptureSensor::SetFilmSlope(float Slope)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.FilmSlope = Slope;
}

float ASceneCaptureSensor::GetFilmSlope() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.FilmSlope;
}

void ASceneCaptureSensor::SetFilmToe(float Toe)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.FilmToe = Toe; // FilmToeAmount?
}

float ASceneCaptureSensor::GetFilmToe() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.FilmToe;
}

void ASceneCaptureSensor::SetFilmShoulder(float Shoulder)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.FilmShoulder = Shoulder;
}

float ASceneCaptureSensor::GetFilmShoulder() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.FilmShoulder;
}

void ASceneCaptureSensor::SetFilmBlackClip(float BlackClip)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.FilmBlackClip = BlackClip;
}

float ASceneCaptureSensor::GetFilmBlackClip() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.FilmBlackClip;
}

void ASceneCaptureSensor::SetFilmWhiteClip(float WhiteClip)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.FilmWhiteClip = WhiteClip;
}

float ASceneCaptureSensor::GetFilmWhiteClip() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.FilmWhiteClip;
}

void ASceneCaptureSensor::SetExposureMinBrightness(float Brightness)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.AutoExposureMinBrightness = Brightness;
}

float ASceneCaptureSensor::GetExposureMinBrightness() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureMinBrightness;
}

void ASceneCaptureSensor::SetExposureMaxBrightness(float Brightness)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.AutoExposureMaxBrightness = Brightness;
}

float ASceneCaptureSensor::GetExposureMaxBrightness() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureMaxBrightness;
}

void ASceneCaptureSensor::SetExposureSpeedDown(float Speed)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.AutoExposureSpeedDown = Speed;
}

float ASceneCaptureSensor::GetExposureSpeedDown() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureSpeedDown;
}

void ASceneCaptureSensor::SetExposureSpeedUp(float Speed)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.AutoExposureSpeedUp = Speed;
}

float ASceneCaptureSensor::GetExposureSpeedUp() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureSpeedUp;
}

void ASceneCaptureSensor::SetExposureCalibrationConstant(float Constant)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.AutoExposureCalibrationConstant_DEPRECATED = Constant;
}

float ASceneCaptureSensor::GetExposureCalibrationConstant() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.AutoExposureCalibrationConstant_DEPRECATED;
}

void ASceneCaptureSensor::SetMotionBlurIntensity(float Intensity)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.MotionBlurAmount = Intensity;
}

float ASceneCaptureSensor::GetMotionBlurIntensity() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.MotionBlurAmount;
}

void ASceneCaptureSensor::SetMotionBlurMaxDistortion(float MaxDistortion)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.MotionBlurMax = MaxDistortion;
}

float ASceneCaptureSensor::GetMotionBlurMaxDistortion() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.MotionBlurMax;
}

void ASceneCaptureSensor::SetMotionBlurMinObjectScreenSize(float ScreenSize)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.MotionBlurPerObjectSize = ScreenSize;
}

float ASceneCaptureSensor::GetMotionBlurMinObjectScreenSize() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.MotionBlurPerObjectSize;
}

void ASceneCaptureSensor::SetLensFlareIntensity(float Intensity)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.LensFlareIntensity = Intensity;
}

float ASceneCaptureSensor::GetLensFlareIntensity() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.LensFlareIntensity;
}

void ASceneCaptureSensor::SetBloomIntensity(float Intensity)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.BloomIntensity = Intensity;
}

float ASceneCaptureSensor::GetBloomIntensity() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.BloomIntensity;
}

void ASceneCaptureSensor::SetWhiteTemp(float Temp)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.WhiteTemp = Temp;
}

float ASceneCaptureSensor::GetWhiteTemp() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.WhiteTemp;
}

void ASceneCaptureSensor::SetWhiteTint(float Tint)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.WhiteTint = Tint;
}

float ASceneCaptureSensor::GetWhiteTint() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.WhiteTint;
}

void ASceneCaptureSensor::SetChromAberrIntensity(float Intensity)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.SceneFringeIntensity = Intensity;
}

float ASceneCaptureSensor::GetChromAberrIntensity() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.SceneFringeIntensity;
}

void ASceneCaptureSensor::SetChromAberrOffset(float ChromAberrOffset)
{
  check(CaptureComponent2D != nullptr);
  CaptureComponent2D->PostProcessSettings.ChromaticAberrationStartOffset = ChromAberrOffset;
}

float ASceneCaptureSensor::GetChromAberrOffset() const
{
  check(CaptureComponent2D != nullptr);
  return CaptureComponent2D->PostProcessSettings.ChromaticAberrationStartOffset;
}

using GBufferID = EGBufferTextureID;

template <GBufferID ID, typename T>
static void CheckGBufferStream(uint64_t& Mask, T& GBufferStreams)
{
    auto& GBufferStream = std::get<(size_t)ID>(GBufferStreams);
    GBufferStream.bIsUsed = GBufferStream.Stream.AreClientsListening();
    if (GBufferStream.bIsUsed)
        FGBufferRequest::MarkAsRequested(Mask, ID);
}

void ASceneCaptureSensor::EnqueueRenderSceneImmediate()
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor::EnqueueRenderSceneImmediate);

  uint64 Mask = 0;
  CheckGBufferStream<GBufferID::SceneColor>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::SceneDepth>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::SceneStencil>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::GBufferA>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::GBufferB>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::GBufferC>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::GBufferD>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::GBufferE>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::GBufferF>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::Velocity>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::SSAO>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::CustomDepth>(Mask, GBufferStreams);
  CheckGBufferStream<GBufferID::CustomStencil>(Mask, GBufferStreams);

  if (Mask == 0)
  {
      // Creates an snapshot of the scene, requieres bCaptureEveryFrame = false.
      CaptureComponent2D->CaptureScene();
  }
  else
  {
      EnqueueRenderSceneImmediateWithGBuffers(Mask);
  }
}

template <GBufferID TextureID, typename SensorT, typename StreamT>
static void SendGBufferTexture(
    SensorT& Sensor,
    StreamT& GBufferStream,
    TArrayView<uint8> TextureData,
    FIntPoint TextureSize)
{
  static_assert ((size_t)TextureID < (size_t)FGBufferRequest::TextureCount, "Invalid GBuffer texture index.");

  using GBufferStreamType = std::remove_reference_t<decltype(GBufferStream)>;

  constexpr auto Index = (size_t)TextureID;

  if (TextureData.Num() == 0)
      return;

#if 0
  if (TextureID == GBufferID::SceneDepth || TextureID == GBufferID::CustomDepth)
  {
      auto path = FString::Printf(TEXT("Out/%s/Frame%u.exr"), GBufferTextureNames[(uint8)TextureID], FCarlaEngine::GetFrameCounter());

      auto PixelData = MakeUnique<TImagePixelData<FLinearColor>>(TextureSize);
      PixelData->Pixels.SetNum(TextureSize.X * TextureSize.Y);
      (void)memcpy(PixelData->Pixels.GetData(), TextureData.GetData(), TextureData.Num());
      TUniquePtr<FImageWriteTask> ImageTask = MakeUnique<FImageWriteTask>();
      ImageTask->PixelData = MoveTemp(PixelData);
      ImageTask->Filename = path;
      ImageTask->Format = EImageFormat::EXR;
      ImageTask->CompressionQuality = (int32)EImageCompressionQuality::Default;
      ImageTask->bOverwriteFile = true;

      FHighResScreenshotConfig& HighResScreenshotConfig = GetHighResScreenshotConfig();
      HighResScreenshotConfig.ImageWriteQueue->Enqueue(MoveTemp(ImageTask));
  }
  else
#endif
  {

#ifdef CARLA_ENABLE_LLM
#define LLM_SCOPE_WRAPPER(Tag) LLM_SCOPE(Tag)
#else
#define LLM_SCOPE_WRAPPER(Tag)
#endif

      LLM_SCOPE_WRAPPER(ELLMTag::CARLA_LLM_TAG_01);

      auto Stream = GBufferStream.GetDataStream(Sensor);

      {
          LLM_SCOPE_WRAPPER(ELLMTag::CARLA_LLM_TAG_02);

          auto Buffer = Stream.PopBufferFromPool();

          {
              LLM_SCOPE_WRAPPER(ELLMTag::CARLA_LLM_TAG_03);

              Buffer.copy_from(
                  carla::sensor::SensorRegistry::get<GBufferStreamType*>::type::header_offset,
                  boost::asio::buffer(&TextureData[0], TextureData.Num()));

              {
                  LLM_SCOPE_WRAPPER(ELLMTag::CARLA_LLM_TAG_04);


                  if (!Buffer.empty())
                  {
                      Stream.Send(
                          GBufferStream,
                          std::move(Buffer),
                          TextureSize.X,
                          TextureSize.Y,
                          Sensor.GetFOVAngle());
                  }
              }
          }
      }
  }
}

template <GBufferID TextureID, typename StreamT>
static void ParseGBufferTexture(
    TArray<uint8>& OutData,
    FIntPoint& OutSize,
    StreamT& GBufferStream,
    FGBufferRequest& GBufferRequest)
{
    static_assert ((size_t)TextureID < (size_t)FGBufferRequest::TextureCount, "Invalid GBuffer texture index.");

    using GBufferStreamType = std::remove_reference_t<decltype(GBufferStream)>;
    using PixelType = typename GBufferStreamType::PixelType;

    if (!GBufferRequest.IsRequested(TextureID))
      return;

    bool Success = GBufferRequest.WaitForTextureTransfer(TextureID);

    if (Success)
    {
        FIntPoint SourceExtent;
        void* PixelData;
        int32 SourcePitch;

        Success = GBufferRequest.MapTextureData(TextureID, PixelData, SourcePitch, SourceExtent);

        if (Success)
        {
            OutSize = GBufferRequest.ViewRect.Size();
            auto PixelCount = OutSize.X * OutSize.Y;
            OutData.AddUninitialized(PixelCount * sizeof(PixelType));

            FReadSurfaceDataFlags Flags = {};
            Flags.SetLinearToGamma(true);

            ImageUtil::DecodePixelsByFormat(
                TArrayView<PixelType>(reinterpret_cast<PixelType*>(&OutData[0]), PixelCount),
                TArrayView<uint8>((uint8*)PixelData, SourcePitch * SourceExtent.Y),
                SourcePitch,
                SourceExtent,
                OutSize,
                GBufferRequest.Readbacks[(size_t)TextureID]->GetFormat(),
                Flags);

            GBufferRequest.UnmapTextureData(TextureID);
            GBufferRequest.ReleasePooledReadback(TextureID);
        }
    }

    if (!Success)
    {
        OutSize = GBufferRequest.ViewRect.Size();
        OutData.Add(OutSize.X * OutSize.Y * sizeof(PixelType));
    }
}

constexpr GBufferID
    SceneColor = GBufferID::SceneColor,
    SceneDepth = GBufferID::SceneDepth,
    SceneStencil = GBufferID::SceneStencil,
    GBufferA = GBufferID::GBufferA,
    GBufferB = GBufferID::GBufferB,
    GBufferC = GBufferID::GBufferC,
    GBufferD = GBufferID::GBufferD,
    GBufferE = GBufferID::GBufferE,
    GBufferF = GBufferID::GBufferF,
    Velocity = GBufferID::Velocity,
    SSAO = GBufferID::SSAO,
    CustomDepth = GBufferID::CustomDepth,
    CustomStencil = GBufferID::CustomStencil;

void ASceneCaptureSensor::EnqueueRenderSceneImmediateWithGBuffers(uint64 RequestedMask)
{
    // Equivalent to "CaptureComponent2D->CaptureScene" + (optional) GBuffer extraction.

    auto GBuffer = MakeUnique<FGBufferRequest>(RequestedMask);

    GBuffer->OwningActor = CaptureComponent2D->GetViewOwner();

    bool EnableTAA = false;
    bool Prior = CaptureComponent2D->ShowFlags.TemporalAA;

    CaptureComponent2D->ShowFlags.TemporalAA = EnableTAA;
    
    CaptureComponent2D->CaptureSceneWithGBuffer(*GBuffer);

    CaptureComponent2D->ShowFlags.TemporalAA = Prior;

    std::array<TArray<uint8>, FGBufferRequest::TextureCount> TextureData = {};
    std::array<FIntPoint, FGBufferRequest::TextureCount> TextureSizes = {};

    FlushRenderingCommands();

    bool Completed = false;

    ENQUEUE_RENDER_COMMAND(Command)([this, &Completed, &TextureData, &TextureSizes, GBuffer = MoveTemp(GBuffer)](FRHICommandList& RHICmdList)
    {
      ParseGBufferTexture<SceneColor>(
        TextureData[(uint8)SceneColor],
        TextureSizes[(uint8)SceneColor],
        std::get<(uint8)SceneColor>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<SceneDepth>(
        TextureData[(uint8)SceneDepth],
        TextureSizes[(uint8)SceneDepth],
        std::get<(uint8)SceneDepth>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<SceneStencil>(
        TextureData[(uint8)SceneStencil],
        TextureSizes[(uint8)SceneStencil],
        std::get<(uint8)SceneStencil>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<GBufferA>(
        TextureData[(uint8)GBufferA],
        TextureSizes[(uint8)GBufferA],
        std::get<(uint8)GBufferA>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<GBufferB>(
        TextureData[(uint8)GBufferB],
        TextureSizes[(uint8)GBufferB],
        std::get<(uint8)GBufferB>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<GBufferC>(
        TextureData[(uint8)GBufferC],
        TextureSizes[(uint8)GBufferC],
        std::get<(uint8)GBufferC>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<GBufferD>(
        TextureData[(uint8)GBufferD],
        TextureSizes[(uint8)GBufferD],
        std::get<(uint8)GBufferD>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<GBufferE>(
        TextureData[(uint8)GBufferE],
        TextureSizes[(uint8)GBufferE],
        std::get<(uint8)GBufferE>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<GBufferF>(
        TextureData[(uint8)GBufferF],
        TextureSizes[(uint8)GBufferF],
        std::get<(uint8)GBufferF>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<Velocity>(
        TextureData[(uint8)Velocity],
        TextureSizes[(uint8)Velocity],
        std::get<(uint8)Velocity>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<SSAO>(
        TextureData[(uint8)SSAO],
        TextureSizes[(uint8)SSAO],
        std::get<(uint8)SSAO>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<CustomDepth>(
        TextureData[(uint8)CustomDepth],
        TextureSizes[(uint8)CustomDepth],
        std::get<(uint8)CustomDepth>(GBufferStreams),
        *GBuffer);

      ParseGBufferTexture<CustomStencil>(
        TextureData[(uint8)CustomStencil],
        TextureSizes[(uint8)CustomStencil],
        std::get<(uint8)CustomStencil>(GBufferStreams),
        *GBuffer);
    });

    FlushRenderingCommands();

    SendGBufferTexture<SceneColor>(
      *this,
      std::get<(uint8)SceneColor>(GBufferStreams),
      TextureData[(uint8)SceneColor],
      TextureSizes[(uint8)SceneColor]);

    SendGBufferTexture<SceneDepth>(
      *this,
      std::get<(uint8)SceneDepth>(GBufferStreams),
      TextureData[(uint8)SceneDepth],
      TextureSizes[(uint8)SceneDepth]);

    SendGBufferTexture<SceneStencil>(
      *this,
      std::get<(uint8)SceneStencil>(GBufferStreams),
      TextureData[(uint8)SceneStencil],
      TextureSizes[(uint8)SceneStencil]);

    SendGBufferTexture<GBufferA>(
      *this,
      std::get<(uint8)GBufferA>(GBufferStreams),
      TextureData[(uint8)GBufferA],
      TextureSizes[(uint8)GBufferA]);

    SendGBufferTexture<GBufferB>(
      *this,
      std::get<(uint8)GBufferB>(GBufferStreams),
      TextureData[(uint8)GBufferB],
      TextureSizes[(uint8)GBufferB]);

    SendGBufferTexture<GBufferC>(
      *this,
      std::get<(uint8)GBufferC>(GBufferStreams),
      TextureData[(uint8)GBufferC],
      TextureSizes[(uint8)GBufferC]);

    SendGBufferTexture<GBufferD>(
      *this,
      std::get<(uint8)GBufferD>(GBufferStreams),
      TextureData[(uint8)GBufferD],
      TextureSizes[(uint8)GBufferD]);

    SendGBufferTexture<GBufferE>(
      *this,
      std::get<(uint8)GBufferE>(GBufferStreams),
      TextureData[(uint8)GBufferE],
      TextureSizes[(uint8)GBufferE]);

    SendGBufferTexture<GBufferF>(
      *this,
      std::get<(uint8)GBufferF>(GBufferStreams),
      TextureData[(uint8)GBufferF],
      TextureSizes[(uint8)GBufferF]);

    SendGBufferTexture<Velocity>(
      *this,
      std::get<(uint8)Velocity>(GBufferStreams),
      TextureData[(uint8)Velocity],
      TextureSizes[(uint8)Velocity]);

    SendGBufferTexture<SSAO>(
      *this,
      std::get<(uint8)SSAO>(GBufferStreams),
      TextureData[(uint8)SSAO],
      TextureSizes[(uint8)SSAO]);

    SendGBufferTexture<CustomDepth>(
      *this,
      std::get<(uint8)CustomDepth>(GBufferStreams),
      TextureData[(uint8)CustomDepth],
      TextureSizes[(uint8)CustomDepth]);

    SendGBufferTexture<CustomStencil>(
      *this,
      std::get<(uint8)CustomStencil>(GBufferStreams),
      TextureData[(uint8)CustomStencil],
      TextureSizes[(uint8)CustomStencil]);
}

void ASceneCaptureSensor::BeginPlay()
{
  using namespace SceneCaptureSensor_local_ns;

  // Determine the gamma of the player.
  const bool bInForceLinearGamma = !bEnablePostProcessingEffects;

  CaptureRenderTarget->InitCustomFormat(ImageWidth, ImageHeight, bEnable16BitFormat ? PF_FloatRGBA : PF_B8G8R8A8,
                                        bInForceLinearGamma);

  if (bEnablePostProcessingEffects)
  {
    CaptureRenderTarget->TargetGamma = TargetGamma;
  }

  check(IsValid(CaptureComponent2D) && !CaptureComponent2D->IsPendingKill());

  CaptureComponent2D->Deactivate();
  CaptureComponent2D->TextureTarget = CaptureRenderTarget;

  // Call derived classes to set up their things.
  SetUpSceneCaptureComponent(*CaptureComponent2D);

  CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

  CaptureComponent2D->UpdateContent();
  CaptureComponent2D->Activate();

  // Make sure that there is enough time in the render queue.
  UKismetSystemLibrary::ExecuteConsoleCommand(
      GetWorld(),
      FString("g.TimeoutForBlockOnRenderFence 300000"));

  SceneCaptureSensor_local_ns::ConfigureShowFlags(CaptureComponent2D->ShowFlags,
      bEnablePostProcessingEffects);

  // This ensures the camera is always spawning the raindrops in case the
  // weather was previously set to have rain.
  GetEpisode().GetWeather()->NotifyWeather(this);

  Super::BeginPlay();
}

void ASceneCaptureSensor::PrePhysTick(float DeltaSeconds)
{
  Super::PrePhysTick(DeltaSeconds);

  // Add the view information every tick. It's only used for one tick and then
  // removed by the streamer.
  IStreamingManager::Get().AddViewInformation(
      CaptureComponent2D->GetComponentLocation(),
      ImageWidth,
      ImageWidth / FMath::Tan(CaptureComponent2D->FOVAngle));
}

void ASceneCaptureSensor::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaTime)
{
  Super::PostPhysTick(World, TickType, DeltaTime);
}

void ASceneCaptureSensor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  Super::EndPlay(EndPlayReason);
  SCENE_CAPTURE_COUNTER = 0u;
}

// =============================================================================
// -- Local static functions implementations -----------------------------------
// =============================================================================

namespace SceneCaptureSensor_local_ns {

  static void SetCameraDefaultOverrides(USceneCaptureComponent2D &CaptureComponent2D)
  {
    auto &PostProcessSettings = CaptureComponent2D.PostProcessSettings;

    // Exposure
    PostProcessSettings.bOverride_AutoExposureMethod = true;
    PostProcessSettings.AutoExposureMethod = EAutoExposureMethod::AEM_Histogram;
    PostProcessSettings.bOverride_AutoExposureBias = true;
    PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
    PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
    PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
    PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
    PostProcessSettings.bOverride_AutoExposureCalibrationConstant_DEPRECATED = true;
    PostProcessSettings.bOverride_HistogramLogMin = true;
    PostProcessSettings.HistogramLogMin = 1.0f;
    PostProcessSettings.bOverride_HistogramLogMax = true;
    PostProcessSettings.HistogramLogMax = 12.0f;

    // Camera
    PostProcessSettings.bOverride_CameraShutterSpeed = true;
    PostProcessSettings.bOverride_CameraISO = true;
    PostProcessSettings.bOverride_DepthOfFieldFstop = true;
    PostProcessSettings.bOverride_DepthOfFieldMinFstop = true;
    PostProcessSettings.bOverride_DepthOfFieldBladeCount = true;

    // Film (Tonemapper)
    PostProcessSettings.bOverride_FilmSlope = true;
    PostProcessSettings.bOverride_FilmToe = true;
    PostProcessSettings.bOverride_FilmShoulder = true;
    PostProcessSettings.bOverride_FilmWhiteClip = true;
    PostProcessSettings.bOverride_FilmBlackClip = true;

    // Motion blur
    PostProcessSettings.bOverride_MotionBlurAmount = true;
    PostProcessSettings.MotionBlurAmount = 0.45f;
    PostProcessSettings.bOverride_MotionBlurMax = true;
    PostProcessSettings.MotionBlurMax = 0.35f;
    PostProcessSettings.bOverride_MotionBlurPerObjectSize = true;
    PostProcessSettings.MotionBlurPerObjectSize = 0.1f;

    // Color Grading
    PostProcessSettings.bOverride_WhiteTemp = true;
    PostProcessSettings.bOverride_WhiteTint = true;
    PostProcessSettings.bOverride_ColorContrast = true;
#if PLATFORM_LINUX
  // Looks like Windows and Linux have different outputs with the
  // same exposure compensation, this fixes it.
  PostProcessSettings.ColorContrast = FVector4(1.2f, 1.2f, 1.2f, 1.0f);
#endif

    // Chromatic Aberration
    PostProcessSettings.bOverride_SceneFringeIntensity = true;
    PostProcessSettings.bOverride_ChromaticAberrationStartOffset = true;

    // Ambient Occlusion
    PostProcessSettings.bOverride_AmbientOcclusionIntensity = true;
    PostProcessSettings.AmbientOcclusionIntensity = 0.5f;
    PostProcessSettings.bOverride_AmbientOcclusionRadius = true;
    PostProcessSettings.AmbientOcclusionRadius = 100.0f;
    PostProcessSettings.bOverride_AmbientOcclusionStaticFraction = true;
    PostProcessSettings.AmbientOcclusionStaticFraction = 1.0f;
    PostProcessSettings.bOverride_AmbientOcclusionFadeDistance = true;
    PostProcessSettings.AmbientOcclusionFadeDistance = 50000.0f;
    PostProcessSettings.bOverride_AmbientOcclusionPower = true;
    PostProcessSettings.AmbientOcclusionPower = 2.0f;
    PostProcessSettings.bOverride_AmbientOcclusionBias = true;
    PostProcessSettings.AmbientOcclusionBias = 3.0f;
    PostProcessSettings.bOverride_AmbientOcclusionQuality = true;
    PostProcessSettings.AmbientOcclusionQuality = 100.0f;

    // Bloom
    PostProcessSettings.bOverride_BloomMethod = true;
    PostProcessSettings.BloomMethod = EBloomMethod::BM_SOG;
    PostProcessSettings.bOverride_BloomIntensity = true;
    PostProcessSettings.BloomIntensity = 0.675f;
    PostProcessSettings.bOverride_BloomThreshold = true;
    PostProcessSettings.BloomThreshold = -1.0f;

    // Lens
    PostProcessSettings.bOverride_LensFlareIntensity = true;
    PostProcessSettings.LensFlareIntensity = 0.1;
  }

  // Remove the show flags that might interfere with post-processing effects
  // like depth and semantic segmentation.
  static void ConfigureShowFlags(FEngineShowFlags &ShowFlags, bool bPostProcessing)
  {
    if (bPostProcessing)
    {
      ShowFlags.EnableAdvancedFeatures();
      ShowFlags.SetMotionBlur(true);
      return;
    }

    ShowFlags.SetAmbientOcclusion(false);
    ShowFlags.SetAntiAliasing(false);
    ShowFlags.SetVolumetricFog(false);
    // ShowFlags.SetAtmosphericFog(false);
    // ShowFlags.SetAudioRadius(false);
    // ShowFlags.SetBillboardSprites(false);
    ShowFlags.SetBloom(false);
    // ShowFlags.SetBounds(false);
    // ShowFlags.SetBrushes(false);
    // ShowFlags.SetBSP(false);
    // ShowFlags.SetBSPSplit(false);
    // ShowFlags.SetBSPTriangles(false);
    // ShowFlags.SetBuilderBrush(false);
    // ShowFlags.SetCameraAspectRatioBars(false);
    // ShowFlags.SetCameraFrustums(false);
    ShowFlags.SetCameraImperfections(false);
    ShowFlags.SetCameraInterpolation(false);
    // ShowFlags.SetCameraSafeFrames(false);
    // ShowFlags.SetCollision(false);
    // ShowFlags.SetCollisionPawn(false);
    // ShowFlags.SetCollisionVisibility(false);
    ShowFlags.SetColorGrading(false);
    // ShowFlags.SetCompositeEditorPrimitives(false);
    // ShowFlags.SetConstraints(false);
    // ShowFlags.SetCover(false);
    // ShowFlags.SetDebugAI(false);
    // ShowFlags.SetDecals(false);
    // ShowFlags.SetDeferredLighting(false);
    ShowFlags.SetDepthOfField(false);
    ShowFlags.SetDiffuse(false);
    ShowFlags.SetDirectionalLights(false);
    ShowFlags.SetDirectLighting(false);
    // ShowFlags.SetDistanceCulledPrimitives(false);
    // ShowFlags.SetDistanceFieldAO(false);
    // ShowFlags.SetDistanceFieldGI(false);
    ShowFlags.SetDynamicShadows(false);
    // ShowFlags.SetEditor(false);
    ShowFlags.SetEyeAdaptation(false);
    ShowFlags.SetFog(false);
    // ShowFlags.SetGame(false);
    // ShowFlags.SetGameplayDebug(false);
    // ShowFlags.SetGBufferHints(false);
    ShowFlags.SetGlobalIllumination(false);
    ShowFlags.SetGrain(false);
    // ShowFlags.SetGrid(false);
    // ShowFlags.SetHighResScreenshotMask(false);
    // ShowFlags.SetHitProxies(false);
    ShowFlags.SetHLODColoration(false);
    ShowFlags.SetHMDDistortion(false);
    // ShowFlags.SetIndirectLightingCache(false);
    // ShowFlags.SetInstancedFoliage(false);
    // ShowFlags.SetInstancedGrass(false);
    // ShowFlags.SetInstancedStaticMeshes(false);
    // ShowFlags.SetLandscape(false);
    // ShowFlags.SetLargeVertices(false);
    ShowFlags.SetLensFlares(false);
    ShowFlags.SetLevelColoration(false);
    ShowFlags.SetLightComplexity(false);
    ShowFlags.SetLightFunctions(false);
    ShowFlags.SetLightInfluences(false);
    ShowFlags.SetLighting(false);
    ShowFlags.SetLightMapDensity(false);
    ShowFlags.SetLightRadius(false);
    ShowFlags.SetLightShafts(false);
    // ShowFlags.SetLOD(false);
    ShowFlags.SetLODColoration(false);
    // ShowFlags.SetMaterials(false);
    // ShowFlags.SetMaterialTextureScaleAccuracy(false);
    // ShowFlags.SetMeshEdges(false);
    // ShowFlags.SetMeshUVDensityAccuracy(false);
    // ShowFlags.SetModeWidgets(false);
    ShowFlags.SetMotionBlur(false);
    // ShowFlags.SetNavigation(false);
    ShowFlags.SetOnScreenDebug(false);
    // ShowFlags.SetOutputMaterialTextureScales(false);
    // ShowFlags.SetOverrideDiffuseAndSpecular(false);
    // ShowFlags.SetPaper2DSprites(false);
    ShowFlags.SetParticles(false);
    // ShowFlags.SetPivot(false);
    ShowFlags.SetPointLights(false);
    // ShowFlags.SetPostProcessing(false);
    // ShowFlags.SetPostProcessMaterial(false);
    // ShowFlags.SetPrecomputedVisibility(false);
    // ShowFlags.SetPrecomputedVisibilityCells(false);
    // ShowFlags.SetPreviewShadowsIndicator(false);
    // ShowFlags.SetPrimitiveDistanceAccuracy(false);
    ShowFlags.SetPropertyColoration(false);
    // ShowFlags.SetQuadOverdraw(false);
    // ShowFlags.SetReflectionEnvironment(false);
    // ShowFlags.SetReflectionOverride(false);
    ShowFlags.SetRefraction(false);
    // ShowFlags.SetRendering(false);
    ShowFlags.SetSceneColorFringe(false);
    // ShowFlags.SetScreenPercentage(false);
    ShowFlags.SetScreenSpaceAO(false);
    ShowFlags.SetScreenSpaceReflections(false);
    // ShowFlags.SetSelection(false);
    // ShowFlags.SetSelectionOutline(false);
    // ShowFlags.SetSeparateTranslucency(false);
    // ShowFlags.SetShaderComplexity(false);
    // ShowFlags.SetShaderComplexityWithQuadOverdraw(false);
    // ShowFlags.SetShadowFrustums(false);
    // ShowFlags.SetSkeletalMeshes(false);
    // ShowFlags.SetSkinCache(false);
    ShowFlags.SetSkyLighting(false);
    // ShowFlags.SetSnap(false);
    // ShowFlags.SetSpecular(false);
    // ShowFlags.SetSplines(false);
    ShowFlags.SetSpotLights(false);
    // ShowFlags.SetStaticMeshes(false);
    ShowFlags.SetStationaryLightOverlap(false);
    // ShowFlags.SetStereoRendering(false);
    // ShowFlags.SetStreamingBounds(false);
    ShowFlags.SetSubsurfaceScattering(false);
    // ShowFlags.SetTemporalAA(false);
    // ShowFlags.SetTessellation(false);
    // ShowFlags.SetTestImage(false);
    // ShowFlags.SetTextRender(false);
    // ShowFlags.SetTexturedLightProfiles(false);
    ShowFlags.SetTonemapper(false);
    // ShowFlags.SetTranslucency(false);
    // ShowFlags.SetVectorFields(false);
    // ShowFlags.SetVertexColors(false);
    // ShowFlags.SetVignette(false);
    // ShowFlags.SetVisLog(false);
    // ShowFlags.SetVisualizeAdaptiveDOF(false);
    // ShowFlags.SetVisualizeBloom(false);
    ShowFlags.SetVisualizeBuffer(false);
    ShowFlags.SetVisualizeDistanceFieldAO(false);
    ShowFlags.SetVisualizeDOF(false);
    ShowFlags.SetVisualizeHDR(false);
    ShowFlags.SetVisualizeLightCulling(false);
    ShowFlags.SetVisualizeLPV(false);
    ShowFlags.SetVisualizeMeshDistanceFields(false);
    ShowFlags.SetVisualizeMotionBlur(false);
    ShowFlags.SetVisualizeOutOfBoundsPixels(false);
    ShowFlags.SetVisualizeSenses(false);
    ShowFlags.SetVisualizeShadingModels(false);
    ShowFlags.SetVisualizeSSR(false);
    ShowFlags.SetVisualizeSSS(false);
    // ShowFlags.SetVolumeLightingSamples(false);
    // ShowFlags.SetVolumes(false);
    // ShowFlags.SetWidgetComponents(false);
    // ShowFlags.SetWireframe(false);
  }

} // namespace SceneCaptureSensor_local_ns
