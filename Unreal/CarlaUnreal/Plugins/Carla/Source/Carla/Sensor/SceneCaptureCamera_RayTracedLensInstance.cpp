// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/SceneCaptureCamera_RayTracedLensInstance.h"
#include "Carla.h"
#include "Carla/Game/CarlaEngine.h"
#include "Carla/Sensor/ImageUtil.h"
#include "Carla/Sensor/RTLensEngineAdapter.h"

#include "Actor/ActorBlueprintFunctionLibrary.h"

#include <util/ue-header-guard-begin.h>
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include <util/ue-header-guard-end.h>

// FPostProcessSettings::PathTracingLensAOV values, mirrored from
// Engine/Shaders/Private/PathTracing/PathTracingCore.ush.
static constexpr int32 GPathTracingLensAOVTag = 2;

FActorDefinition ASceneCaptureCamera_RayTracedLensInstance::GetSensorDefinition()
{
  // Same attribute surface as sensor.camera.rt_lens (camera_model,
  // distortion_coeffs, fx/fy/cx/cy, theta_max_deg, fov, show_only_*, ...) so a
  // colour/distance/instance rig can be configured from one dictionary.
  // Post-process attributes are deliberately absent: this capture never goes
  // through the post-process chain at all.
  constexpr bool bEnableModifyingPostProcessEffects = false;
  FActorDefinition Definition =
      UActorBlueprintFunctionLibrary::MakeRayTracedLensCameraDefinition(
          TEXT("rt_lens_instance"),
          bEnableModifyingPostProcessEffects);

  // A tag is an exact integer read off the primary hit; extra samples would
  // only cost time, and there is nothing for a denoiser to do to it.
  for (auto &Variation : Definition.Variations)
  {
    if (Variation.Id == TEXT("samples_per_pixel"))
    {
      Variation.RecommendedValues = {TEXT("1")};
    }
    else if (Variation.Id == TEXT("enable_denoiser"))
    {
      Variation.RecommendedValues = {TEXT("false")};
    }
  }
  return Definition;
}

ASceneCaptureCamera_RayTracedLensInstance::ASceneCaptureCamera_RayTracedLensInstance(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  // Nothing on this sensor's route touches the post-process chain: the capture
  // source resolves the path tracer's tag AOV straight into the render target.
  // Disabling post-processing also forces the render target to linear gamma,
  // which is what keeps the label/id bytes exact.
  EnablePostProcessingEffects(false);
  ApplyInstanceAOVConfig();
}

void ASceneCaptureCamera_RayTracedLensInstance::ApplyInstanceAOVConfig()
{
  if (CaptureComponent2D == nullptr)
  {
    return;
  }
  auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;

  // Per-view AOV selection (Engine/Classes/Engine/Scene.h). Per-view rather
  // than a cvar so a rig can run colour, distance and instance cameras at once.
  PostProcessSettings.bOverride_PathTracingLensAOV = true;
  PostProcessSettings.PathTracingLensAOV = GPathTracingLensAOVTag;

  // One sample is all the AOV needs, and the denoiser must never touch it.
  // These are the inherited knobs (reasserted by the base's PostPhysTick), so
  // they have to be written through the base's members, not just the settings.
  SamplesPerPixel = 1;
  bEnableDenoiser = false;
  PostProcessSettings.bOverride_PathTracingSamplesPerPixel = true;
  PostProcessSettings.bOverride_PathTracingEnableDenoiser = true;
  PostProcessSettings.PathTracingSamplesPerPixel = SamplesPerPixel;
  PostProcessSettings.PathTracingEnableDenoiser = bEnableDenoiser;

  // Depth of field would scatter the primary ray across the aperture and blend
  // two actors' ids into one pixel. DepthOfFieldFocalDistance == 0 is what the
  // path tracer tests for (PathTracing.cpp, PathTracingData.CameraLensRadius).
  SetFocalDistance(0.0f);
}

void ASceneCaptureCamera_RayTracedLensInstance::Set(const FActorDescription &Description)
{
  // Lens model, resolution, fov, show_only_* -- everything is parsed by the
  // colour sensor's Set(); this class only changes what the capture outputs.
  Super::Set(Description);
  ApplyInstanceAOVConfig();
}

void ASceneCaptureCamera_RayTracedLensInstance::BeginPlay()
{
  Super::BeginPlay();

  // ASceneCaptureSensor::BeginPlay ends by forcing SCS_FinalToneCurveHDR; take
  // the capture source back afterwards. The render target it created is already
  // what this sensor wants (PF_B8G8R8A8 with linear gamma, because
  // post-processing is disabled), so unlike the distance sensor there is
  // nothing to re-initialise.
  if (CaptureComponent2D != nullptr)
  {
    CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_PathTracingLensTag;
  }
  ApplyInstanceAOVConfig();
}

void ASceneCaptureCamera_RayTracedLensInstance::PostPhysTick(
    UWorld *World,
    ELevelTick TickType,
    float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera_RayTracedLensInstance::PostPhysTick);

  if (AreClientsListening() && CaptureComponent2D != nullptr)
  {
    // Same reason as ASceneCaptureCamera_RayTracedLens::PostPhysTick: code
    // paths such as ApplyPostProcessVolumeToSensor replace PostProcessSettings
    // wholesale and would silently turn this back into an undistorted pinhole
    // colour camera.
    RTLensEngineAdapter::ApplyLensModel(CaptureComponent2D->PostProcessSettings, LensModel);
    ApplyInstanceAOVConfig();
    CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_PathTracingLensTag;
  }

  // Deliberately skips ASceneCaptureCamera_RayTracedLens::PostPhysTick: its body
  // is the colour readback, which would serialize this frame a second time as
  // the colour sensor's type. The lens reassertion above reproduces what else it
  // does, and the capture/readback ORDER -- which is what keeps a colour, a
  // distance and an instance camera on the same frame -- comes from the base
  // class's TickCaptureAndReadback so every path-traced lens sensor shares one
  // policy: blocking readback after the capture in synchronous mode, and
  // readback-then-capture without ever waiting on the render thread otherwise.
  TickCaptureAndReadback(World, TickType, DeltaSeconds,
    [this](bool bNonBlocking)
    {
      UTextureRenderTarget2D *RenderTarget = GetCaptureRenderTarget();
      if (RenderTarget == nullptr)
      {
        return;
      }
      const auto FrameIndex = FCarlaEngine::GetFrameCounter();
      ImageUtil::ReadImageDataAsyncFColor(*RenderTarget, [this, FrameIndex](
        TArrayView<const FColor> Pixels,
        FIntPoint Size) -> bool
      {
        SendDataToClient(*this, Pixels, FrameIndex);
        return true;
      }, bNonBlocking, GetReadbackPool());
    });
}
