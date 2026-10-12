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

  // Ray tracing material pipeline completeness.
  //
  // Every path-traced capture binds one ray tracing pipeline state (RTPSO)
  // holding the closest-hit shader of every loaded ray tracing material. When
  // that set changes -- a vehicle or walker blueprint whose materials the
  // scene has not rendered before finishes compiling its shaders -- the
  // renderer needs a new RTPSO. With the engine default
  // r.RayTracing.NonBlockingPipelineCreation=1 that RTPSO compiles in the
  // background and, until it is ready, the frame renders with the previous
  // pipeline: every mesh segment whose material is not in it is bound to
  // PathTracingDefaultOpaqueCHS instead (Renderer/Private/RayTracing/
  // RayTracingMaterialHitShaders.cpp, CreateMaterialRayTracingMaterialPipeline
  // and SetupMaterialRayTracingHitGroupBindings; the shader is
  // Shaders/Private/PathTracing/PathTracingDefaultHitShader.usf). That shader
  // shades opaque black and never calls SetCarlaTag, so the new actor is black
  // in rt_lens and tagged 0 in rt_lens_instance -- absent from both -- for as
  // long as the compile takes (measured: 5-6 ticks after the material shaders
  // land, with the actor's already-known materials, e.g. glass or tyres,
  // showing normally in between). The distance AOV still sees it, because the
  // fallback shader does report its hit distance.
  //
  // A synchronous-mode frame has to be the complete render of its tick, so the
  // sensor asks the renderer to wait for the pipeline instead (the engine's
  // documented meaning of r.RayTracing.NonBlockingPipelineCreation=0:
  // "rendering will always use correct requested material"; offline renders
  // get the same treatment through FSceneView::bIsOfflineRender). The wait is
  // one deferred RHI dispatch per NEW material set -- measured 90-200 ms once,
  // then never again for those materials -- and zero on every other frame. In
  // asynchronous mode the game thread must never stall behind a sensor, so the
  // previous value is restored and the engine's fallback is accepted.
  //
  // The cvar is ECVF_RenderThreadSafe: FConsoleVariableRef::Set copies the new
  // value to the render thread through a render command, so a value written
  // before CaptureScene() is what that capture's render sees. A value forced
  // from the console (-ExecCmds / console_command, SetByConsole) outranks
  // SetByCode and is left alone, with one warning.
  void SetBlockingRayTracingPipelineCreation(bool bBlocking);
}
