// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Actor/ActorDefinition.h"
#include "Carla/Sensor/SceneCaptureSensor.h"
#include "Carla/Util/CameraModelUtil.h"

#include <util/ue-header-guard-begin.h>
#include "Actor/ActorBlueprintFunctionLibrary.h"
#include <util/ue-header-guard-end.h>

#include "SceneCaptureCamera_RayTracedLens.generated.h"

/// A path-traced camera sensor ("sensor.camera.rt_lens") that renders through
/// a configurable lens model (see Util/CameraModelUtil.h for the model
/// contract) instead of the stock linear pinhole projection.
///
/// Unlike ASceneCaptureCamera_WideAngleLens, the lens distortion here is not
/// a 2D post-process warp applied to a rasterized cubemap: the lens model is
/// evaluated per-ray inside the path tracer itself, via genuine per-view
/// FPostProcessSettings fields (see RTLensEngineAdapter.h), so multiple
/// simultaneous rt_lens cameras with different lens models do not interfere
/// with each other. Consequently this sensor captures through a single
/// USceneCaptureComponent2D, exactly like ASceneCaptureCamera, and reuses the
/// same standard RGB readback path instead of the WideAngleLens family's
/// 6-face cubemap + compute-shader resample pipeline.
UCLASS()
class CARLA_API ASceneCaptureCamera_RayTracedLens : public ASceneCaptureSensor
{
  GENERATED_BODY()

public:

  static FActorDefinition GetSensorDefinition();

  ASceneCaptureCamera_RayTracedLens(const FObjectInitializer &ObjectInitializer);

  void Set(const FActorDescription &ActorDescription) override;

protected:

  void PostPhysTick(UWorld *World, ELevelTick TickType, float DeltaSeconds) override;

  /// Forces path-tracing view mode for this sensor's capture. Hooked through
  /// the same extension point ASceneCaptureSensor::BeginPlay already calls
  /// for every subclass, so no BeginPlay override is needed here.
  void UpdatePostProcessConfig(FPostProcessConfig &InOutPostProcessConfig) override;

  virtual void OnFirstClientConnected() override;
  virtual void OnLastClientDisconnected() override;

private:

  /// Resolved lens-model parameters, rebuilt in Set() from the
  /// camera_model / distortion_coeffs / lut / fx / fy / cx / cy /
  /// theta_max_deg / ca_shift_r / ca_shift_b Blueprint attributes and pushed
  /// into CaptureComponent2D->PostProcessSettings there via
  /// RTLensEngineAdapter::ApplyLensModel. Kept as a member (rather than a
  /// Set()-local) purely for inspection/debugging.
  FLensModelDescriptor LensModel;

  /// Path-tracer quality knobs. These map directly onto genuine per-view
  /// engine fields (FPostProcessSettings::PathTracingSamplesPerPixel /
  /// PathTracingEnableDenoiser, see Engine/Classes/Engine/Scene.h), so unlike
  /// LensModel they do not need the RTLensEngineAdapter seam.
  int32 SamplesPerPixel = 16;
  bool bEnableDenoiser = true;
};
