// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/CameraModelUtil.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/Scene.h"
#include <util/ue-header-guard-end.h>

// Adapter seam between ASceneCaptureCamera_RayTracedLens and the engine's
// path-tracer lens-model mechanism: FPostProcessSettings::PathTracingLens*
// (PathTracingLensModel / PathTracingLensFocal / PathTracingLensCoeffs01 /
// PathTracingLensCoeffs23 / PathTracingLensThetaMax / PathTracingLensCAScale,
// see Engine/Classes/Engine/Scene.h) landed by rt-lens-engine. These fields
// are genuine per-view, non-interpolated state, so multiple simultaneous
// rt_lens cameras with different lens configurations do not interfere with
// each other (this replaces an earlier CVar-based mechanism, which was
// process-global and did not support that).
//
// Kept as a seam (rather than writing PostProcessSettings fields directly
// from the sensor) so a further engine-side change only needs to update this
// file's body; call sites do not change.
namespace RTLensEngineAdapter
{
  // Writes Descriptor into PostProcessSettings' PathTracingLens* fields and
  // sets their bOverride_ flags. Call whenever the descriptor changes (e.g.
  // from ASceneCaptureCamera_RayTracedLens::Set()); these are per-view
  // settings, not a per-tick push -- the renderer only invalidates
  // path-tracer accumulation when a value actually changes.
  void ApplyLensModel(FPostProcessSettings &PostProcessSettings, const FLensModelDescriptor &Descriptor);
}
