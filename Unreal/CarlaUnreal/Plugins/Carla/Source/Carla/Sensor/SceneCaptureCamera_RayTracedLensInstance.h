// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/SceneCaptureCamera_RayTracedLens.h"

#include "SceneCaptureCamera_RayTracedLensInstance.generated.h"

/// A path-traced instance/semantic sensor ("sensor.camera.rt_lens_instance"):
/// the labelling twin of sensor.camera.rt_lens.
///
/// It renders through exactly the same lens as ASceneCaptureCamera_RayTracedLens
/// -- same attributes, same Set(), same show_only_* filtering, evaluated per ray
/// inside the path tracer -- but instead of radiance it outputs, for every
/// pixel, the CARLA tag of the primary hit, in the byte layout
/// sensor.camera.instance_segmentation already uses:
///
///     R = semantic label (ATagger's crp::CityObjectLabel)
///     G = actor id, low byte
///     B = actor id, high byte
///
/// so the same client-side decode works for both, and the semantic image is
/// just the R channel of this one.
///
/// The tag comes from the same place as the raster path's: ATagger writes it
/// into custom primitive data slot 4 (Carla/Game/Tagger.cpp), which the raster
/// segmentation pass reads as CustomPrimitiveData[1].xy
/// (Engine/Shaders/Private/SegmentationSensor.usf). The path tracer's
/// closest-hit shader reads the very same GPUScene field and carries it to the
/// ray generation shader in the material payload, which records it into a
/// dedicated R32_UINT render-graph texture next to the albedo/normal/depth AOVs
/// (FPostProcessSettings::PathTracingLensAOV == 2, PathTracer.LensTag). The
/// capture resolves that texture through
/// ESceneCaptureSource::SCS_PathTracingLensTag.
///
/// Why the raster pass cannot simply be reused: ShowFlags.PathTracing makes
/// FDeferredShadingSceneRenderer treat the frame as a ray-traced overlay and
/// skip the depth prepass, base pass and Nanite raster entirely, so
/// AddSegmentationSensorPass would silently produce an empty or stale buffer
/// rather than fail.
///
/// Nothing on this route applies exposure, tone mapping, denoising or
/// filtering, and the primary ray is fired through the exact pixel centre, so
/// the ids are exact 8-bit values, never blended between two actors.
UCLASS()
class CARLA_API ASceneCaptureCamera_RayTracedLensInstance : public ASceneCaptureCamera_RayTracedLens
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ASceneCaptureCamera_RayTracedLensInstance(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

protected:

  void BeginPlay() override;

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

private:

  /// Re-asserts everything that makes this capture a tag AOV rather than a
  /// colour render. Called from Set() and from every tick, because several code
  /// paths replace the capture component's PostProcessSettings wholesale (see
  /// the note in ASceneCaptureCamera_RayTracedLens::PostPhysTick).
  void ApplyInstanceAOVConfig();
};
