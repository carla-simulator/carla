// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "Carla/Sensor/SceneCaptureCamera_RayTracedLensDistance.h"
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
static constexpr int32 GPathTracingLensAOVDistance = 1;

// Unreal world units are centimetres; the CARLA sensor API reports metres.
static constexpr float GUnrealUnitsToMeters = 0.01f;

FActorDefinition ASceneCaptureCamera_RayTracedLensDistance::GetSensorDefinition()
{
  // Same attribute surface as sensor.camera.rt_lens (camera_model,
  // distortion_coeffs, fx/fy/cx/cy, theta_max_deg, fov, show_only_*, ...) so a
  // colour/distance pair can be configured from one dictionary. Post-process
  // attributes are deliberately absent: this capture never goes through the
  // post-process chain at all.
  constexpr bool bEnableModifyingPostProcessEffects = false;
  FActorDefinition Definition =
      UActorBlueprintFunctionLibrary::MakeRayTracedLensCameraDefinition(
          TEXT("rt_lens_distance"),
          bEnableModifyingPostProcessEffects);

  // The AOV is deterministic and noise-free by construction (one pixel-centre
  // primary ray, no reconstruction filter, no denoiser), so the colour camera's
  // sample budget would only cost time. Retune the inherited defaults rather
  // than duplicating the whole definition.
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

ASceneCaptureCamera_RayTracedLensDistance::ASceneCaptureCamera_RayTracedLensDistance(
    const FObjectInitializer &ObjectInitializer)
  : Super(ObjectInitializer)
{
  // Nothing on this sensor's route touches the post-process chain: the capture
  // source resolves the path tracer's AOV texture straight into the render
  // target. Disabling post-processing keeps the show flags (and therefore the
  // work the renderer does per capture) down to what a geometry pass needs.
  EnablePostProcessingEffects(false);
  ApplyDistanceAOVConfig();
}

void ASceneCaptureCamera_RayTracedLensDistance::ApplyDistanceAOVConfig()
{
  if (CaptureComponent2D == nullptr)
  {
    return;
  }
  auto &PostProcessSettings = CaptureComponent2D->PostProcessSettings;

  // Per-view AOV selection (Engine/Classes/Engine/Scene.h). Per-view rather
  // than a cvar so a rig can run colour and distance cameras simultaneously.
  PostProcessSettings.bOverride_PathTracingLensAOV = true;
  PostProcessSettings.PathTracingLensAOV = GPathTracingLensAOVDistance;

  // One sample is all the AOV needs, and the denoiser must never touch it.
  // These are the inherited knobs (reasserted by the base's PostPhysTick), so
  // they have to be written through the base's members, not just the settings.
  SamplesPerPixel = 1;
  bEnableDenoiser = false;
  PostProcessSettings.bOverride_PathTracingSamplesPerPixel = true;
  PostProcessSettings.bOverride_PathTracingEnableDenoiser = true;
  PostProcessSettings.PathTracingSamplesPerPixel = SamplesPerPixel;
  PostProcessSettings.PathTracingEnableDenoiser = bEnableDenoiser;

  // Depth of field would scatter the primary ray origin across the aperture and
  // turn a distance measurement into an average over the circle of confusion.
  // DepthOfFieldFocalDistance == 0 is what the path tracer tests for
  // (PathTracing.cpp, PathTracingData.CameraLensRadius).
  SetFocalDistance(0.0f);
}

void ASceneCaptureCamera_RayTracedLensDistance::Set(const FActorDescription &Description)
{
  // Lens model, resolution, fov, show_only_* -- everything is parsed by the
  // colour sensor's Set(); this class only changes what the capture outputs.
  Super::Set(Description);
  ApplyDistanceAOVConfig();
}

void ASceneCaptureCamera_RayTracedLensDistance::BeginPlay()
{
  Super::BeginPlay();

  // The AOV is metric data: an 8-bit target would quantize 1000 m into 4 m
  // steps and even FP16 loses 6 cm at 100 m. ASceneCaptureSensor::BeginPlay
  // hard-codes its choice between PF_B8G8R8A8 and PF_FloatRGBA, so re-init the
  // target here, after it ran, with a single float32 channel.
  if (UTextureRenderTarget2D *RenderTarget = GetCaptureRenderTarget())
  {
    RenderTarget->InitCustomFormat(
        GetImageWidth(),
        GetImageHeight(),
        PF_R32_FLOAT,
        /*bInForceLinearGamma=*/true);
  }

  ApplyDistanceAOVConfig();
}

void ASceneCaptureCamera_RayTracedLensDistance::PostPhysTick(
    UWorld *World,
    ELevelTick TickType,
    float DeltaSeconds)
{
  TRACE_CPUPROFILER_EVENT_SCOPE(ASceneCaptureCamera_RayTracedLensDistance::PostPhysTick);

  if (AreClientsListening() && CaptureComponent2D != nullptr)
  {
    // Same reason as ASceneCaptureCamera_RayTracedLens::PostPhysTick: code
    // paths such as ApplyPostProcessVolumeToSensor replace PostProcessSettings
    // wholesale and would silently turn this back into an undistorted pinhole
    // colour camera.
    RTLensEngineAdapter::ApplyLensModel(CaptureComponent2D->PostProcessSettings, LensModel);
    ApplyDistanceAOVConfig();
    CaptureComponent2D->CaptureSource = ESceneCaptureSource::SCS_PathTracingLensDistance;
  }

  // Deliberately skips ASceneCaptureCamera_RayTracedLens::PostPhysTick: its body
  // is the colour readback (an FColor decode of what is here a float32 distance
  // buffer, serialized as the colour sensor's type). The lens reassertion above
  // reproduces what else it does, and the capture/readback ORDER -- which is
  // what keeps a colour and a distance camera on the same frame -- comes from
  // the base class's TickCaptureAndReadback so both sensors share one policy.
  TickCaptureAndReadback(World, TickType, DeltaSeconds,
    [this](bool bNonBlocking)
    {
      UTextureRenderTarget2D *RenderTarget = GetCaptureRenderTarget();
      if (RenderTarget == nullptr)
      {
        return;
      }
      const auto FrameIndex = FCarlaEngine::GetFrameCounter();
      ImageUtil::ReadImageDataAsync(
          *RenderTarget,
          GetReadbackPool(),
          [this, FrameIndex](
              const void *MappedPtr,
              size_t RowPitch,       // in pixels, not bytes
              size_t BufferHeight,
              EPixelFormat Format,
              FIntPoint Extent) -> bool
          {
            if (MappedPtr == nullptr || Format != PF_R32_FLOAT)
            {
              return false;
            }
            check(RowPitch >= (size_t)Extent.X);
            check(BufferHeight >= (size_t)Extent.Y);

            TArray<float> Distances;
            Distances.Reserve(Extent.X * Extent.Y);
            auto RowPtr = reinterpret_cast<const float *>(MappedPtr);
            for (int32 Y = 0; Y != Extent.Y; ++Y)
            {
              for (int32 X = 0; X != Extent.X; ++X)
              {
                Distances.Add(RowPtr[X] * GUnrealUnitsToMeters);
              }
              RowPtr += RowPitch;
            }
            SendDataToClient(*this, TArrayView<float>(Distances), FrameIndex);
            return true;
          },
          bNonBlocking);
    });
}
