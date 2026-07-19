// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/SceneCaptureCamera_RayTracedLens.h"
#include "Carla.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Sensor/ImageUtil.h"
#include "Carla/Sensor/RTLensEngineAdapter.h"

#include "Actor/ActorBlueprintFunctionLibrary.h"

#include <type_traits>

// Path-tracer DOF piggybacks on the standard cinematic depth-of-field
// post-process fields (DepthOfFieldFstop / DepthOfFieldFocalDistance,
// inherited from ASceneCaptureSensor) -- the path tracer's circle-of-confusion
// model already reads those to derive its physical lens radius, no separate
// DOF mechanism needed. aperture_fstop=0 requests a pinhole (no depth of
// field); there is no literal "infinite f-stop" so approximate it with a very
// small aperture instead.
static constexpr float GPinholeApertureFstop = 32.0f;

// =============================================================================
// -- Local static helpers ------------------------------------------------------
// =============================================================================

namespace SceneCaptureCameraRayTracedLens_local_ns
{

  static ECameraModel ParseCameraModelName(const FString &Name)
  {
    static const FString Lookup[] =
    {
      TEXT("perspective"),
      TEXT("stereographic"),
      TEXT("equidistant"),
      TEXT("equisolid"),
      TEXT("orthographic"),
      TEXT("kannala_brandt"),
      TEXT("brown_conrady"),
      TEXT("lut"),
    };

    using I = std::underlying_type_t<ECameraModel>;

    static_assert(
        sizeof(Lookup) / sizeof(Lookup[0]) == (I)ECameraModel::MaxEnum,
        "camera_model string lookup size mismatch.");

    for (I i = 0; i != (I)ECameraModel::MaxEnum; ++i)
    {
      if (Name == Lookup[i])
        return (ECameraModel)i;
    }

    UE_LOG(LogCarla, Warning,
        TEXT("ASceneCaptureCamera_RayTracedLens: unknown camera_model '%s', defaulting to perspective."),
        *Name);
    return ECameraModel::Default;
  }

  static TArray<float> ParseFloatCSV(const FString &CSV)
  {
    TArray<float> Result;
    if (CSV.IsEmpty())
      return Result;
    TArray<FString> Parts;
    CSV.ParseIntoArray(Parts, TEXT(","), true);
    Result.Reserve(Parts.Num());
    for (const auto &Part : Parts)
      Result.Add(FCString::Atof(*Part));
    return Result;
  }

} // namespace SceneCaptureCameraRayTracedLens_local_ns

// =============================================================================
// -- ASceneCaptureCamera_RayTracedLens -----------------------------------------
// =============================================================================

FActorDefinition ASceneCaptureCamera_RayTracedLens::GetSensorDefinition()
{
  constexpr bool bEnableModifyingPostProcessEffects = true;
  return UActorBlueprintFunctionLibrary::MakeRayTracedLensCameraDefinition(
      TEXT("rt_lens"),
      bEnableModifyingPostProcessEffects);
}

ASceneCaptureCamera_RayTracedLens::ASceneCaptureCamera_RayTracedLens(const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  // The path tracer has no rasterized fallback the way ASceneCaptureCamera
  // does; without hardware ray tracing this sensor cannot render at all.
  SetUseRayTracing(true);

  check(CaptureComponent2D != nullptr);
  auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;
  PostProcessSettings.bOverride_PathTracingSamplesPerPixel = true;
  PostProcessSettings.bOverride_PathTracingEnableDenoiser = true;
  PostProcessSettings.PathTracingSamplesPerPixel = SamplesPerPixel;
  PostProcessSettings.PathTracingEnableDenoiser = bEnableDenoiser;
}

void ASceneCaptureCamera_RayTracedLens::Set(const FActorDescription &Description)
{
  // Handles image_size_x/y, fov, use_ray_tracing, enable_postprocess_effects,
  // post_process_profile, sensor_tick -- via SetCamera(Description,
  // ASceneCaptureSensor*), the same as every other scene-capture camera.
  Super::Set(Description);

  using namespace SceneCaptureCameraRayTracedLens_local_ns;

  const auto &Variations = Description.Variations;

  check(CaptureComponent2D != nullptr);

  // -- Lens model -------------------------------------------------------------
  LensModel.Model = ParseCameraModelName(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString(
          "camera_model", Variations, TEXT("perspective")));
  LensModel.Coeffs = ParseFloatCSV(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString(
          "distortion_coeffs", Variations, TEXT("")));
  LensModel.LUT = ParseFloatCSV(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString(
          "lut", Variations, TEXT("")));
  LensModel.FocalX = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "fx", Variations, 1.0f);
  LensModel.FocalY = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "fy", Variations, 1.0f);
  LensModel.CenterX = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "cx", Variations, 0.5f);
  LensModel.CenterY = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "cy", Variations, 0.5f);
  LensModel.ThetaMax = FMath::DegreesToRadians(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
          "theta_max_deg", Variations, 90.0f));
  LensModel.CAScaleR = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "ca_shift_r", Variations, 1.0f);
  LensModel.CAScaleB = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "ca_shift_b", Variations, 1.0f);

  RTLensEngineAdapter::ApplyLensModel(CaptureComponent2D->PostProcessSettings, LensModel);

  // -- Depth of field, through the standard scene-capture post-process ------
  // fields inherited from ASceneCaptureSensor (see GPinholeApertureFstop).
  const float ApertureFstop = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "aperture_fstop", Variations, 0.0f);
  SetAperture(ApertureFstop > 0.0f ? ApertureFstop : GPinholeApertureFstop);
  SetFocalDistance(
      UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
          "focus_distance_m", Variations, 1000.0f)
      * 100.0f); // meters -> UE centimeters.

  // -- Path-tracer quality -----------------------------------------------------
  SamplesPerPixel = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToInt(
      "samples_per_pixel", Variations, 16);
  bEnableDenoiser = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToBool(
      "enable_denoiser", Variations, true);

  CaptureComponent2D->PostProcessSettings.PathTracingSamplesPerPixel = SamplesPerPixel;
  CaptureComponent2D->PostProcessSettings.PathTracingEnableDenoiser = bEnableDenoiser;
}

void ASceneCaptureCamera_RayTracedLens::UpdatePostProcessConfig(FPostProcessConfig &InOutPostProcessConfig)
{
  Super::UpdatePostProcessConfig(InOutPostProcessConfig);
  InOutPostProcessConfig.EngineShowFlags.SetPathTracing(true);
}

void ASceneCaptureCamera_RayTracedLens::OnFirstClientConnected()
{
}

void ASceneCaptureCamera_RayTracedLens::OnLastClientDisconnected()
{
}

void ASceneCaptureCamera_RayTracedLens::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera_RayTracedLens::PostPhysTick);

  // Captures the scene. The lens model is already resident in
  // CaptureComponent2D->PostProcessSettings (applied once in Set(), see
  // RTLensEngineAdapter::ApplyLensModel) -- these are per-view fields, not a
  // per-tick push.
  Super::PostPhysTick(World, TickType, DeltaSeconds);

  if (!AreClientsListening())
    return;

  // Standard RGB readback path: read the capture render target and hand the
  // decoded pixels to the normal CARLA image stream, exactly like
  // ASceneCaptureCamera. Called directly against ImageUtil rather than via
  // ImageUtil::ReadSensorImageDataAsyncFColor (which takes an
  // AShaderBasedSensor&) since this sensor derives from ASceneCaptureSensor
  // directly -- it applies its lens distortion inside the path tracer's ray
  // generation, not through a 2D post-process material.
  auto *RenderTarget = GetCaptureRenderTarget();
  if (RenderTarget == nullptr)
    return;

  auto FrameIndex = FCarlaEngine::GetFrameCounter();
  ImageUtil::ReadImageDataAsyncFColor(*RenderTarget, [this, FrameIndex](
    TArrayView<const FColor> Pixels,
    FIntPoint Size) -> bool
  {
    SendDataToClient(*this, Pixels, FrameIndex);
    return true;
  });
}
