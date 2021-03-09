// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla.h"
#include "Carla/Sensor/SceneCaptureSensor.h"

#include "Carla/Game/CarlaStatics.h"

#include "Async/Async.h"
#include "Components/DrawFrustumComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/StaticMeshComponent.h"
#include "ContentStreaming.h"
#include "Engine/Classes/Engine/Scene.h"
#include "Engine/TextureRenderTarget2D.h"
#include "HAL/UnrealMemory.h"
#include "HighResScreenshot.h"
#include "Misc/CoreDelegates.h"
#include "RHICommandList.h"

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

  CaptureComponent2D = CreateDefaultSubobject<USceneCaptureComponent2D>(
      FName(*FString::Printf(TEXT("SceneCaptureComponent2D_%d"), SCENE_CAPTURE_COUNTER)));
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
  // Creates an snapshot of the scene, requieres bCaptureEveryFrame = false.
  CaptureComponent2D->CaptureScene();
}

void ASceneCaptureSensor::BeginPlay()
{
  using namespace SceneCaptureSensor_local_ns;

  // Determine the gamma of the player.
  const bool bInForceLinearGamma = !bEnablePostProcessingEffects;

  CaptureRenderTarget->InitCustomFormat(ImageWidth, ImageHeight, PF_B8G8R8A8, bInForceLinearGamma);

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
  GetEpisode().GetWeather()->NotifyWeather();

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
