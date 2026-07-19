// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "Carla/Util/CameraModelUtil.h"

// Adapter seam between ASceneCaptureCamera_RayTracedLens and the engine's
// path-tracer lens-model mechanism.
//
// Today (rt-lens-engine, in progress) that mechanism is the r.PathTracing.Lens*
// CVars in Engine/Source/Runtime/Renderer/Private/PathTracing.cpp
// (LensModel / LensCoeffs / LensFocal / LensThetaMax / LensCAScale). Those are
// process-global, ECVF_RenderThreadSafe CVars with no per-view scoping, so
// running more than one rt_lens camera with different lens models in the same
// frame will race -- last CVar write before the render thread consumes it
// wins for every capture that frame. This is a known limitation, not an
// oversight.
//
// If/when a genuinely per-view mechanism lands (e.g. a FLensModelDescriptor
// surfaced through FPostProcessSettings, the same way
// PathTracingSamplesPerPixel / PathTracingEnableDenoiser already are -- see
// Engine/Classes/Engine/Scene.h), only ApplyLensModel's body needs to change.
// Call sites stay the same.
namespace RTLensEngineAdapter
{
  // Pushes Descriptor into the path tracer's ray-generation state for the
  // *next* CaptureScene() enqueued on any capture component. Call once per
  // tick, immediately before the owning sensor triggers its capture, so the
  // values are current by the time the render thread reads them.
  void ApplyLensModel(const FLensModelDescriptor &Descriptor);
}
