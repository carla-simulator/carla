// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Game/CarlaStatics.h"
#include "Runtime/Core/Public/HAL/LowLevelMemStats.h"
// #include "Runtime/Core/Public/HAL/LowLevelMemTracker.h"

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

void ASceneCaptureSensor::EnqueueRenderSceneImmediate() {
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureSensor::EnqueueRenderSceneImmediate);
  // Equivalent to "CaptureComponent2D->CaptureScene" + (optional) GBuffer extraction.
  CaptureSceneExtended();
}

constexpr const TCHAR* GBufferNames[] =
{
  TEXT("SceneColor"),
  TEXT("SceneDepth"),
  TEXT("SceneStencil"),
  TEXT("GBufferA"),
  TEXT("GBufferB"),
  TEXT("GBufferC"),
  TEXT("GBufferD"),
  TEXT("GBufferE"),
  TEXT("GBufferF"),
  TEXT("Velocity"),
  TEXT("SSAO"),
  TEXT("CustomDepth"),
  TEXT("CustomStencil"),
};

template <EGBufferTextureID ID, typename T>
static void CheckGBufferStream(uint64_t& Mask, T& GBufferStreams)
{
  auto& GBufferStream = std::get<(size_t)ID>(GBufferStreams);
  GBufferStream.bIsUsed = GBufferStream.Stream.AreClientsListening();
  if (GBufferStream.bIsUsed)
      FGBufferRequest::MarkAsRequested(Mask, ID);
}

static std::once_flag once_flag;

DECLARE_LLM_MEMORY_STAT(TEXT("CARLA_LLM_TAG_00"), STAT_CARLA, STATGROUP_LLMFULL);

static void InitTags()
{
	LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_00, TEXT("CARLA_LLM_TAG_00"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag00"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_01, TEXT("CARLA_LLM_TAG_01"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag01"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_02, TEXT("CARLA_LLM_TAG_02"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag02"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_03, TEXT("CARLA_LLM_TAG_03"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag03"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_04, TEXT("CARLA_LLM_TAG_04"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag04"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_05, TEXT("CARLA_LLM_TAG_05"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag05"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_06, TEXT("CARLA_LLM_TAG_06"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag06"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_07, TEXT("CARLA_LLM_TAG_07"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag07"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_08, TEXT("CARLA_LLM_TAG_08"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag08"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_09, TEXT("CARLA_LLM_TAG_09"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag09"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_10, TEXT("CARLA_LLM_TAG_10"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag10"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_11, TEXT("CARLA_LLM_TAG_11"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag11"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_12, TEXT("CARLA_LLM_TAG_12"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag12"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_13, TEXT("CARLA_LLM_TAG_13"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag13"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_14, TEXT("CARLA_LLM_TAG_14"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag14"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_15, TEXT("CARLA_LLM_TAG_15"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag15"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_16, TEXT("CARLA_LLM_TAG_16"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag16"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_17, TEXT("CARLA_LLM_TAG_17"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag17"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_18, TEXT("CARLA_LLM_TAG_18"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag18"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_19, TEXT("CARLA_LLM_TAG_19"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag19"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_20, TEXT("CARLA_LLM_TAG_20"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag20"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_21, TEXT("CARLA_LLM_TAG_21"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag21"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_22, TEXT("CARLA_LLM_TAG_22"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag22"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_23, TEXT("CARLA_LLM_TAG_23"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag23"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_24, TEXT("CARLA_LLM_TAG_24"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag24"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_25, TEXT("CARLA_LLM_TAG_25"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag25"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_26, TEXT("CARLA_LLM_TAG_26"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag26"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_27, TEXT("CARLA_LLM_TAG_27"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag27"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_28, TEXT("CARLA_LLM_TAG_28"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag28"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_29, TEXT("CARLA_LLM_TAG_29"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag29"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_30, TEXT("CARLA_LLM_TAG_30"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag30"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_31, TEXT("CARLA_LLM_TAG_31"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag31"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_32, TEXT("CARLA_LLM_TAG_32"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag32"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_33, TEXT("CARLA_LLM_TAG_33"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag33"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_34, TEXT("CARLA_LLM_TAG_34"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag34"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_35, TEXT("CARLA_LLM_TAG_35"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag35"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_36, TEXT("CARLA_LLM_TAG_36"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag36"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_37, TEXT("CARLA_LLM_TAG_37"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag37"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_38, TEXT("CARLA_LLM_TAG_38"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag38"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_39, TEXT("CARLA_LLM_TAG_39"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag39"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_40, TEXT("CARLA_LLM_TAG_40"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag40"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_41, TEXT("CARLA_LLM_TAG_41"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag41"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_42, TEXT("CARLA_LLM_TAG_42"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag42"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_43, TEXT("CARLA_LLM_TAG_43"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag43"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_44, TEXT("CARLA_LLM_TAG_44"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag44"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_45, TEXT("CARLA_LLM_TAG_45"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag45"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_46, TEXT("CARLA_LLM_TAG_46"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag46"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_47, TEXT("CARLA_LLM_TAG_47"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag47"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_48, TEXT("CARLA_LLM_TAG_48"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag48"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_49, TEXT("CARLA_LLM_TAG_49"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag49"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_50, TEXT("CARLA_LLM_TAG_50"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag50"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_51, TEXT("CARLA_LLM_TAG_51"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag51"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_52, TEXT("CARLA_LLM_TAG_52"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag52"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_53, TEXT("CARLA_LLM_TAG_53"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag53"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_54, TEXT("CARLA_LLM_TAG_54"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag54"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_55, TEXT("CARLA_LLM_TAG_55"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag55"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_56, TEXT("CARLA_LLM_TAG_56"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag56"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_57, TEXT("CARLA_LLM_TAG_57"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag57"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_58, TEXT("CARLA_LLM_TAG_58"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag58"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_59, TEXT("CARLA_LLM_TAG_59"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag59"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_60, TEXT("CARLA_LLM_TAG_60"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag60"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_61, TEXT("CARLA_LLM_TAG_61"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag61"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_62, TEXT("CARLA_LLM_TAG_62"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag62"), NAME_None));
    LLM(FLowLevelMemTracker::Get().RegisterProjectTag((int32)ELLMTag::CARLA_LLM_TAG_63, TEXT("CARLA_LLM_TAG_63"), GET_STATFNAME(STAT_CARLA), TEXT("CARLATag63"), NAME_None));
}

void ASceneCaptureSensor::CaptureSceneExtended()
{
    std::call_once(once_flag, InitTags);

  decltype (FGBufferRequest::DesiredTexturesMask) Mask = 0;
  CheckGBufferStream<EGBufferTextureID::SceneColor>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::SceneDepth>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::SceneStencil>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::GBufferA>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::GBufferB>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::GBufferC>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::GBufferD>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::GBufferE>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::GBufferF>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::Velocity>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::SSAO>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::CustomDepth>(Mask, GBufferStreams);
  CheckGBufferStream<EGBufferTextureID::CustomStencil>(Mask, GBufferStreams);

  if (Mask == 0)
  {
    // Creates an snapshot of the scene, requieres bCaptureEveryFrame = false.
    CaptureComponent2D->CaptureScene();
    return;
  }

  TUniquePtr<FGBufferRequest> GBufferPtr;

  {
      LLM_SCOPE(ELLMTag::CARLA_LLM_TAG_00);
      GBufferPtr = MakeUnique<FGBufferRequest>(Mask);
  }

  GBufferPtr->OwningActor = CaptureComponent2D->GetViewOwner();

  bool EnableTAA = false;
  bool Prior = CaptureComponent2D->ShowFlags.TemporalAA;

  CaptureComponent2D->ShowFlags.TemporalAA = EnableTAA;
  CaptureComponent2D->CaptureSceneWithGBuffer(*GBufferPtr);
  CaptureComponent2D->ShowFlags.TemporalAA = Prior;

  FlushRenderingCommands();
  AsyncTask(ENamedThreads::ActualRenderingThread, [this, GBuffer = MoveTemp(GBufferPtr)]() mutable
  {
    this->SendGBufferTextures(*GBuffer);
  });
}

void ASceneCaptureSensor::SendGBufferTextures(FGBufferRequest& GBufferData)
{
    SendGBufferTexturesInternal(*this, GBufferData);
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
