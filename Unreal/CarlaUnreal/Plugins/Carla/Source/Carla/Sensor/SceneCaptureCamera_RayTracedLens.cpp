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

// Rollback valve for the synchronous-mode blocking delivery below. Off, the
// sensor keeps the asynchronous one-frame-late readback in every mode.
static TAutoConsoleVariable<int32> CVarRTLensSyncBlockingReadback(
    TEXT("carla.RTLens.SyncModeBlockingReadback"),
    1,
    TEXT("In synchronous mode, deliver the rt_lens image captured on the same tick.\n")
    TEXT("  0: always use the asynchronous, one-tick-late, droppable readback.\n")
    TEXT("  1: capture then read back through the per-tick batched GPU sync (default)."),
    ECVF_Default);

// Rollback valve for the synchronous-mode complete-pipeline policy (see
// RTLensEngineAdapter::SetBlockingRayTracingPipelineCreation). Off, the sensor
// leaves r.RayTracing.NonBlockingPipelineCreation alone and freshly spawned
// actors render black and untagged for the ticks their ray tracing pipeline
// takes to compile.
static TAutoConsoleVariable<int32> CVarRTLensSyncBlockingPipelineCreation(
    TEXT("carla.RTLens.SyncModeBlockingPipelineCreation"),
    1,
    TEXT("In synchronous mode, wait for the ray tracing material pipeline before rendering.\n")
    TEXT("  0: leave r.RayTracing.NonBlockingPipelineCreation as configured (new actors may render with the fallback hit shader for a few ticks).\n")
    TEXT("  1: set r.RayTracing.NonBlockingPipelineCreation 0 while synchronous, restore it when asynchronous (default)."),
    ECVF_Default);

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
      TEXT("ftheta"),
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

  // Image-plane radius (normalized units, half-width = 0.5 at fx = 1) that the
  // given model maps a ray at angle Theta from the optical axis to. Used to
  // derive fx from a requested horizontal field of view. LUT1D is excluded:
  // its scale is part of the calibration data itself.
  static float RadialAtTheta(
      ECameraModel Model,
      float Theta,
      TArrayView<const float> Coeffs)
  {
    switch (Model)
    {
    case ECameraModel::Perspective:
    case ECameraModel::BrownConrady: // distortion applies on top of a tan projection
      return FMath::Tan(FMath::Min(Theta, FMath::DegreesToRadians(89.5f)));
    case ECameraModel::Stereographic:
      return 2.0f * FMath::Tan(Theta * 0.5f);
    case ECameraModel::Equidistant:
      return Theta;
    case ECameraModel::Equisolid:
      return 2.0f * FMath::Sin(Theta * 0.5f);
    case ECameraModel::Orthographic:
      return FMath::Sin(FMath::Min(Theta, PI * 0.5f));
    case ECameraModel::KannalaBrandt:
      return CameraModelUtil::KannalaBrandt::ComputeCameraPolynomial(Theta, Coeffs);
    case ECameraModel::FTheta:
      return CameraModelUtil::FTheta::SolveRadius(Theta, Coeffs);
    default:
      return FMath::Tan(FMath::Min(Theta, FMath::DegreesToRadians(89.5f)));
    }
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
  SetRayTracedLensExposure();
}

void ASceneCaptureCamera_RayTracedLens::SetRayTracedLensExposure()
{
  // Pin the capture to the same daylight histogram exposure the viewport
  // uses (see AWeather::PushWeatherToSky). Left to the world's volumes, the
  // path-traced view's eye adaptation can meter the raw sun disk radiance
  // and adapt to ~EV18 (measured on Town12), crushing the whole frame to
  // black while the identical view on another map sits at EV10.
  auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;
  PostProcessSettings.bOverride_AutoExposureMethod = true;
  PostProcessSettings.AutoExposureMethod = AEM_Histogram;
  PostProcessSettings.bOverride_AutoExposureBias = true;
  PostProcessSettings.AutoExposureBias = 0.0f;
  PostProcessSettings.bOverride_AutoExposureMinBrightness = true;
  PostProcessSettings.AutoExposureMinBrightness = 10.0f;
  PostProcessSettings.bOverride_AutoExposureMaxBrightness = true;
  PostProcessSettings.AutoExposureMaxBrightness = 12.0f;
  PostProcessSettings.bOverride_AutoExposureSpeedUp = true;
  PostProcessSettings.AutoExposureSpeedUp = 3.0f;
  PostProcessSettings.bOverride_AutoExposureSpeedDown = true;
  PostProcessSettings.AutoExposureSpeedDown = 1.0f;
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
      "fx", Variations, 0.0f);
  LensModel.FocalY = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "fy", Variations, 0.0f);
  LensModel.CenterX = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "cx", Variations, 0.5f);
  LensModel.CenterY = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "cy", Variations, 0.5f);

  // fx/fy <= 0 requests automatic focal length: place the requested horizontal
  // fov (through the selected model's projection) exactly across the image
  // width. An explicit calibration (fx > 0) always wins.
  // Cap at 170 degrees for now: half-angles at/above 90 degrees (rays entering
  // the backward hemisphere) currently spin the lens ray-generation shader and
  // hang the GPU. Lift the cap once the shader handles theta >= pi/2 robustly.
  const float FOVDegRequested = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "fov", Variations, 90.0f);
  const float FOVDeg = FMath::Clamp(FOVDegRequested, 1.0f, 170.0f);
  if (FOVDegRequested > 170.0f)
  {
    UE_LOG(LogCarla, Warning,
        TEXT("ASceneCaptureCamera_RayTracedLens: fov=%.1f clamped to 170 (known shader limitation beyond 170 degrees)."),
        FOVDegRequested);
  }
  const float ThetaHalf = FMath::DegreesToRadians(FOVDeg) * 0.5f;
  if (LensModel.FocalX <= 0.0f)
  {
    if (LensModel.Model == ECameraModel::LUT1D || LensModel.Model == ECameraModel::FTheta)
    {
      UE_LOG(LogCarla, Warning,
          TEXT("ASceneCaptureCamera_RayTracedLens: camera_model=lut/ftheta needs an explicit fx (1/width for pixel-unit coefficients); using fx=1."));
      LensModel.FocalX = 1.0f;
    }
    else
    {
      const float Radius = RadialAtTheta(LensModel.Model, ThetaHalf, LensModel.Coeffs);
      LensModel.FocalX = 0.5f / FMath::Max(Radius, 1.0e-6f);
    }
  }
  if (LensModel.FocalY <= 0.0f)
  {
    LensModel.FocalY = LensModel.FocalX; // square pixels
  }

  // theta_max_deg <= 0 requests automatic coverage: extend the acceptance
  // angle to the frame corners so no pixel is left without a ray.
  const float ThetaMaxDeg = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
      "theta_max_deg", Variations, 0.0f);
  if (ThetaMaxDeg > 0.0f)
  {
    LensModel.ThetaMax = FMath::DegreesToRadians(ThetaMaxDeg);
  }
  else
  {
    const float Width = FMath::Max((float)GetImageWidth(), 1.0f);
    const float Height = FMath::Max((float)GetImageHeight(), 1.0f);
    const float CornerScale = FMath::Sqrt(1.0f + FMath::Square(Height / Width));
    LensModel.ThetaMax = FMath::Min(ThetaHalf * CornerScale, FMath::DegreesToRadians(89.0f));
  }
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

  // The path tracer runs at most r.PathTracing.MaxFramePassCount sample passes per
  // frame. A sensor asking for more can never finish (and denoise) within a tick:
  // every frame it would deliver a partially accumulated image, which is far noisier
  // than a smaller count that converges. Clamp to the cap and say so.
  if (const IConsoleVariable* MaxPassCountVar =
          IConsoleManager::Get().FindConsoleVariable(TEXT("r.PathTracing.MaxFramePassCount")))
  {
    const int32 MaxPassCount = FMath::Max(MaxPassCountVar->GetInt(), 1);
    if (SamplesPerPixel > MaxPassCount)
    {
      UE_LOG(LogCarla, Warning,
          TEXT("rt_lens: samples_per_pixel %d exceeds r.PathTracing.MaxFramePassCount %d; "
               "clamping so every frame converges and denoises within the tick"),
          SamplesPerPixel, MaxPassCount);
      SamplesPerPixel = MaxPassCount;
    }
  }

  CaptureComponent2D->PostProcessSettings.bOverride_PathTracingSamplesPerPixel = true;
  CaptureComponent2D->PostProcessSettings.bOverride_PathTracingEnableDenoiser = true;
  CaptureComponent2D->PostProcessSettings.PathTracingSamplesPerPixel = SamplesPerPixel;
  CaptureComponent2D->PostProcessSettings.PathTracingEnableDenoiser = bEnableDenoiser;

  // SetCamera above may have rewritten the exposure block from generic
  // attributes; re-pin it (see SetRayTracedLensExposure).
  SetRayTracedLensExposure();

  // exposure_mode=manual: deterministic exposure from the profile's physical
  // camera plus exposure_compensation (EV); needed when the layer is composited
  // against an external render and the frame content (show-only actors over a
  // black background) would drive the histogram.
  {
    const FString ExposureMode = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToString(
        "exposure_mode", Variations, TEXT("auto"));
    auto &PPS = CaptureComponent2D->PostProcessSettings;
    if (ExposureMode == TEXT("manual"))
    {
      PPS.bOverride_AutoExposureMethod = true;
      PPS.AutoExposureMethod = AEM_Manual;
    }
    PPS.bOverride_AutoExposureBias = true;
    PPS.AutoExposureBias = UActorBlueprintFunctionLibrary::RetrieveActorAttributeToFloat(
        "exposure_compensation", Variations, 0.0f);
  }
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

void ASceneCaptureCamera_RayTracedLens::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
  // Process-wide override made in PostPhysTick for synchronous mode; a sensor
  // destroyed while synchronous must not leave it behind (any other live
  // rt_lens sensor sets it again on its next tick).
  RTLensEngineAdapter::SetBlockingRayTracingPipelineCreation(false);
  Super::EndPlay(EndPlayReason);
}

void ASceneCaptureCamera_RayTracedLens::PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera_RayTracedLens::PostPhysTick);

  // Reassert the lens configuration every tick. Several code paths replace the
  // capture component's PostProcessSettings wholesale after Set() ran -- most
  // notably ApplyPostProcessVolumeToSensor, which copies the map's
  // PostProcessVolume settings and restores only the standard camera fields
  // from its cache. Losing the PathTracingLens* fields silently reverts the
  // camera to an undistorted pinhole and drops the sample/denoiser targets.
  //
  // The scene is captured EXACTLY ONCE per tick, inside Super::PostPhysTick.
  // The path tracer accumulates one sample per rendered frame and, with
  // bAlwaysPersistRenderingState (set on the base capture component), keeps its
  // accumulation and spatiotemporal-denoiser history across ticks: a static
  // camera converges over successive ticks, a moving camera relies on the
  // spatial denoiser. Never issue extra CaptureScene() calls here -- each one
  // is a full blocking path-trace on the game thread, and looping them starves
  // the tick and the RPC server (the sensor and the whole server freeze).
  if (AreClientsListening() && CaptureComponent2D != nullptr)
  {
    RTLensEngineAdapter::ApplyLensModel(CaptureComponent2D->PostProcessSettings, LensModel);
    // The bOverride_ flags must be reasserted together with the values: the
    // wholesale PostProcessSettings replacement clears them, and without the
    // flags the FinalPostProcessSettings blend ignores these fields entirely
    // (the map's PostProcessVolume then wins -- e.g. 2048 spp -- and the
    // denoiser gate Iteration+1 == MaxSPP can never fire on a moving camera).
    CaptureComponent2D->PostProcessSettings.bOverride_PathTracingSamplesPerPixel = true;
    CaptureComponent2D->PostProcessSettings.bOverride_PathTracingEnableDenoiser = true;
    CaptureComponent2D->PostProcessSettings.PathTracingSamplesPerPixel = SamplesPerPixel;
    CaptureComponent2D->PostProcessSettings.PathTracingEnableDenoiser = bEnableDenoiser;
  }

  TickCaptureAndReadback(World, TickType, DeltaSeconds,
    [this](bool bNonBlocking)
    {
      auto *RenderTarget = GetCaptureRenderTarget();
      if (RenderTarget == nullptr)
        return;
      const auto FrameIndex = FCarlaEngine::GetFrameCounter();
      // Use the per-sensor recycling readback pool (not a fresh per-frame
      // FRHIGPUTextureReadback): the async path holds each readback until its
      // off-thread delivery, so per-call staging buffers would accumulate and
      // exhaust GPU/host memory within seconds under load.
      ImageUtil::ReadImageDataAsyncFColor(*RenderTarget, [this, FrameIndex](
        TArrayView<const FColor> Pixels,
        FIntPoint Size) -> bool
      {
        SendDataToClient(*this, Pixels, FrameIndex);
        return true;
      }, bNonBlocking, GetReadbackPool());
    });
}

void ASceneCaptureCamera_RayTracedLens::TickCaptureAndReadback(
    UWorld *World,
    ELevelTick TickType,
    float DeltaSeconds,
    TFunctionRef<void(bool bNonBlocking)> EnqueueReadback)
{
  // SYNCHRONOUS MODE -- capture first, then a BLOCKING readback of what we just
  // rendered. The client's world.tick() is the frame boundary, so the image
  // stamped with frame k has to BE tick k's render, and no frame may be
  // dropped. Recording the copy after CaptureScene() puts it behind this tick's
  // render commands; ImageUtil batches it and FSensorManager's single per-tick
  // ImageUtil::FlushBatchedReadbacks() waits for the GPU once for the whole
  // camera batch and delivers it before the tick ends. That is exactly the path
  // every raster scene-capture camera takes (see
  // ASceneCaptureCamera::PostPhysTick -> ImageUtil::ReadSensorImageDataAsyncFColor),
  // and because the batch sync waits for the copy, a frame cannot be dropped at
  // any samples_per_pixel. The cost is one pipeline drain per tick shared by
  // every camera -- not one per camera, and not the per-camera flush +
  // query-wait that used to deadlock (see ImageUtil::ReadImageDataBegin).
  //
  // Completeness is the other half of the same contract: the render of tick k
  // must contain every actor tick k has, with its real materials. The engine
  // otherwise renders an actor whose materials are new to the ray tracing
  // pipeline with a black, untagged fallback shader while that pipeline
  // compiles in the background (see the adapter for the mechanism and the
  // measured cost), so synchronous mode asks for blocking pipeline creation
  // before this tick's capture is enqueued; asynchronous mode, which must never
  // stall the game thread, restores the engine's non-blocking default.
  const bool bSynchronousMode = GetEpisode().GetSettings().bSynchronousMode;
  if (CVarRTLensSyncBlockingPipelineCreation.GetValueOnGameThread() != 0)
  {
    RTLensEngineAdapter::SetBlockingRayTracingPipelineCreation(bSynchronousMode);
  }

  if (bSynchronousMode &&
      CVarRTLensSyncBlockingReadback.GetValueOnGameThread() != 0)
  {
    ASceneCaptureSensor::PostPhysTick(World, TickType, DeltaSeconds);
    if (AreClientsListening())
    {
      EnqueueReadback(/*bNonBlocking=*/false);
    }
    return;
  }

  // ASYNCHRONOUS MODE -- NEVER-FREEZE READBACK + CAPTURE ORDER. Nothing paces
  // the client here, so the game thread (which also drives the RPC server) must
  // never block on this sensor's heavy path-traced render.
  //
  // 1) Enqueue a NON-BLOCKING readback of the render target produced by the
  //    PREVIOUS tick's capture. bNonBlocking=true records the GPU copy and
  //    returns; it does NOT wait for GPU completion on the render thread (the
  //    default path's RHIGetRenderQueryResult(bWait=true) wait DEADLOCKS the
  //    whole server under load -- render thread blocked mid-command, GPU
  //    submission can't advance, game thread freezes behind it at FFrameEndSync;
  //    proven via gdb). Completion is polled off-thread; frames just drop under
  //    load instead of freezing.
  // 2) Then CaptureScene() (inside ASceneCaptureSensor::PostPhysTick). Because
  //    the readback was enqueued FIRST, the recorded copy rides this capture's
  //    GPU submission -- which is what makes its readback fence actually signal
  //    -- and captures the previous frame's pixels (they precede this tick's
  //    render). Result: one frame of latency, never a stall.
  if (AreClientsListening())
  {
    EnqueueReadback(/*bNonBlocking=*/true);
  }

  ASceneCaptureSensor::PostPhysTick(World, TickType, DeltaSeconds);
}
