// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/SceneCaptureCamera_RayTracedLens.h"

#include "SceneCaptureCamera_RayTracedLensDistance.generated.h"

/// A path-traced distance sensor ("sensor.camera.rt_lens_distance"): the
/// geometric twin of sensor.camera.rt_lens.
///
/// It renders through exactly the same lens as ASceneCaptureCamera_RayTracedLens
/// -- same attributes, same Set(), same show_only_* filtering, evaluated per ray
/// inside the path tracer -- but instead of radiance it outputs, for every
/// pixel, the EUCLIDEAN distance from the camera origin to the primary hit,
/// measured along that pixel's (possibly heavily distorted) lens ray. That is
/// the only depth quantity that is well defined for a fisheye: the view-space Z
/// a pinhole depth sensor reports collapses to zero at 90 degrees off axis, and
/// warping a pinhole depth image into the lens can only cover the part of the
/// frame the pinhole frustum reached (~95% for the NuRec f-theta rigs).
///
/// The value comes from a real path-tracer AOV, not from post-processing the
/// colour image: the ray generation shader records the primary hit distance
/// into a dedicated R32F render-graph texture next to the existing
/// albedo/normal/depth AOVs (FPostProcessSettings::PathTracingLensAOV ->
/// FPathTracingData::LensAOV, see Engine/Shaders/Private/PathTracing/
/// PathTracingCore.ush), and the capture resolves that texture directly through
/// the ESceneCaptureSource::SCS_PathTracingLensDistance capture source. Nothing
/// on that route applies exposure, tone mapping, denoising or quantization, and
/// the primary ray is fired through the exact pixel centre, so the output is
/// deterministic metric data rather than an image.
///
/// Wire format: one float32 per pixel, in METRES, row major (see
/// carla::sensor::data::DistanceImage on the client). Pixels whose ray hit
/// nothing report a very large distance (see PATH_TRACER_LENS_AOV_SKY_DISTANCE).
UCLASS()
class CARLA_API ASceneCaptureCamera_RayTracedLensDistance : public ASceneCaptureCamera_RayTracedLens
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ASceneCaptureCamera_RayTracedLensDistance(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

protected:

  void BeginPlay() override;

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

private:

  /// Re-asserts everything that makes this capture a geometry AOV rather than a
  /// colour render. Called from Set() and from every tick, because several code
  /// paths replace the capture component's PostProcessSettings wholesale (see
  /// the note in ASceneCaptureCamera_RayTracedLens::PostPhysTick).
  void ApplyDistanceAOVConfig();
};
