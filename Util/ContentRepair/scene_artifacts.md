# Town10 skyline and temporal artifacts (2026-09-10)

## Confirmed skyline defect

The main viewport showed blue blocks around building silhouettes against clouds.
This reproduced with rain, wetness and wind at zero. Removing depth of field or
sharpening did not remove the blocks. Switching `r.VolumetricRenderTarget.Mode`
from 0 to 3 removed them, including with the original sharpening value of 3.

Mode 0 traces volumetric clouds at quarter resolution, reconstructs at half
resolution and upsamples. Mode 3 traces at full resolution. See the CVar
implementation in Engine/Source/Runtime/Renderer/Private/VolumetricRenderTarget.cpp.
The project now selects mode 3 at Epic effects quality. This has a GPU cost;
other quality tiers retain their existing cloud settings. RGB scene captures
use a different cloud rendering path: validate this defect in the main viewport.

## Sharpening mismatch

Default.json was previously changed from 3 to 0.3 for RGB cameras. Town10's
placed BP_Carla_Sky still serialized 3 in its PostProcessComponent, so the main
viewport did not receive that adjustment. The placed component now uses 0.3;
other authored post-process settings are preserved. The repair is reproducible
with fix_town10_sky_sharpen.py using an Unreal Python commandlet.

This reduces amplification of noise; it is not a demonstrated root-cause fix for
the remaining flicker.

## Remaining investigation

Persistent temporal variation exists in dry weather. Main-viewport unlit frames
were substantially more stable than lit frames. Disabling skylight illumination
reduced much of the variation, but also changed the lighting drastically; that
is diagnostic evidence, not an acceptable fix or proof of one faulty pass.
Hardware Lumen did not resolve the issue. Increased Lumen sampling did not remove
it. Neither change is retained. A six-texture mipmap pilot on the construction
building did not improve temporal stability and was reverted.

Do not report the remaining flicker as fixed. Verification must include a video
of the affected region in the main viewport, with matched weather and camera;
a global frame-difference score alone is insufficient.

## Road-level follow-up

The reproducible view is Town10 spawn point 0, raised 1.5 m (camera world Z
approximately 2.1 m), facing the construction site. Rain, deposits, wetness and
wind are zero; cloudiness is 95, sun altitude 35, fog density 7.

Base-color and world-normal buffer views were stable with temporal jitter off.
With otherwise simplified lighting, changing the temporal sample position
reintroduced variation on fine building detail. Freezing that position removed
the extra variation. This supports temporal aliasing as a contributor; freezing
samples is only a diagnostic and is not retained. A debugger check of the TAA
path found valid history, no camera cut, no previous-transform reset, and a
writable history in an unpaused world. It did not establish a history-reset bug.

The retained TSR settings are in DefaultEngine.ini [SystemSettings]:

- r.TSR.ThinGeometryDetection=1: detects subpixel geometry and relaxes history
  rejection around those edges.
- r.TSR.History.SampleCount=32: keeps more samples in the existing history
  (engine default is 16).

These are runtime TSR settings, not ECVF_Scalability variables, so they belong in
SystemSettings. Existing AA method selection is preserved. Other methods do not
use these settings. Increased history can cause trails on animated effects;
a short forward/backward camera move showed no obvious new trails in inspected
building edges, but moving traffic and night effects need broader validation.

At the same road viewpoint, 90 lossless desktop frames at 30 Hz (zero duplicate
frames in either recording) gave these mean absolute consecutive-frame RGB
changes on a 0–255 scale:

| Region | Before | After | Reduction |
| --- | ---: | ---: | ---: |
| Construction facade | 2.666 | 1.541 | 42% |
| Right-side trees | 1.870 | 1.082 | 42% |
| Right tower | 1.207 | 0.945 | 22% |
| Road | 0.772 | 0.682 | 12% |

This is a partial improvement, not elimination of all flicker. Weather motion and
rendering noise remain possible contributors. Do not equate these regional
metrics with a user-verified resolution of the original report.

Higher Lumen reference sampling, hardware Lumen, stronger TSR rejection settings,
positive texture mip bias, supersampling, and higher-precision history were not
retained. Diagnostic show flags, frozen jitter, and lighting overrides were
cleared by restarting the server.

## Lighting isolation follow-up

A controlled unlit cube with nonclipped output separated display dithering from
lighting variation. Its interior mean RGB value stayed approximately 199.24;
mean consecutive-frame difference fell from 0.495 to 0.031 when diagnostic
r.BackbufferQuantizationDitheringOverride=16 suppressed the normal 8-bit dither.
The override is NOT retained: suppressing dither can introduce banding.
PostProcessTonemap.cpp and PostProcessTonemap.usf implement this output noise.

With AA disabled (removing camera sample jitter) and display dither suppressed,
the construction ROI measured 2.650. Disabling only Lumen diffuse indirect
lighting reduced it to 0.399; additionally disabling Lumen reflections and SSR
reduced it to 0.266. This is strong evidence that Lumen diffuse lighting is a
major noise contributor in the isolated configuration. These altered-lighting
images are diagnostics, not equivalent-quality alternatives or final fixes.
Base-color and normal controls previously remained stable with AA disabled;
no evidence from these tests establishes z-fighting as the dominant cause.

With the same isolation, increasing Lumen screen-probe temporal history from
10 to 64 reduced facade variation to 0.721. Higher spatial sampling, fixed
full-resolution probe jitter, reference mode and disabling fast-update history
handling did not produce comparable improvements. The engine source explicitly
states the history tradeoff: lower values propagate lighting changes faster but
increase noise flicker (LumenScreenProbeGather.cpp, MaxFramesAccumulated).

With normal TSR and display dithering restored, history 10 measured 1.516,
history 32 measured 1.278, and history 64 measured 1.233 on the same facade.
These are approximately 16% and 19% additional reductions over the existing
TSR changes, with diminishing returns and a lighting-response tradeoff.
Increasing MaxRayDirections from 8 to 32 with history 32 measured 1.271;
that did not materially change the result. No new Lumen settings are retained.
A short camera-motion recording is evidence for further inspection, not a
validation of moving-object trails or dynamic lighting response.

Replacing the construction facade with a flat lit material changed brightness
and gave only modest improvement; removing its scaffolding material alone
measured 1.527 versus 1.520 with original materials. Scaffolding is therefore
not supported as the main cause in this ROI. All temporary material overrides
were restored, the control cube destroyed, and temporary FlickerDiagnostics
assets removed. The server was restarted to clear diagnostic console settings.

Evidence: workspace .omc/scene-artifacts-2026-09-10/lighting-isolation, including
regional measurements, screenshots, a cropped normal-rendering history
comparison video and the history-32 camera-motion recording. Measurements use
three-second desktop captures and do not constitute a perceptual quality score.

## DLSS viewport comparison

The engine DLSSRRDenoiser module now exposes an opt-in single-game-viewport
extension using the existing DLSSSR backend. It skips scene captures, reflection
captures, path tracing, non-temporal AA, views without history, multiple-view
families and families that already have a temporal upscaler. Default is off;
no project rendering defaults were changed for this experiment. This does NOT
integrate Ray Reconstruction with Lumen.

Console commands, with temporal AA enabled:

- Native DLAA: r.ScreenPercentage 100, then r.DLSSSR.Viewport 1.
- Return to TSR: r.DLSSSR.Viewport 0 (with current AA method 4).
- DLSS Quality experiment: r.ScreenPercentage 66.6667 with viewport switch 1.

Build and launch with DLSS_SDK pointing at the installed NVIDIA SDK. The engine
module compiled successfully. Runtime logs confirmed native DLSS-SR feature
creation at 1280x720 -> 1280x720 (the backend selects DLAA for this ratio), and
Quality at 854x481 -> 1280x720. No NGX evaluation failures were logged. Initial
feature creation stalled rendering; that first frozen recording was discarded.
The warmed native TSR and DLAA recordings each contained 90 frames and zero
consecutive duplicate frames.

Same road/weather, existing TSR settings retained for baseline:

| Region | TSR native | DLAA native | DLSS Quality |
| --- | ---: | ---: | ---: |
| Construction | 1.533 | 0.880 | 1.626 |
| Tower | 0.946 | 0.712 | 0.976 |
| Road | 0.706 | 0.668 | 0.739 |
| Trees | 1.045 | 1.760 | 2.702 |

These are consecutive-frame RGB differences, not perceptual scores. The native
result is mixed, and lower-resolution Quality increased variation in these
regions. Do not describe either as an established visible fix. Static and
forward/backward camera comparisons are in the workspace evidence directory
.omc/scene-artifacts-2026-09-10/dlss. Motion recordings follow the same scripted
path but are independent asynchronous runs, not precisely frame-synchronized.
Moving traffic, camera cuts, resizing, other GPUs and sensor regression have
not been validated. The underlying backend uses bilinear fallback without NGX;
this opt-in experiment therefore requires confirming successful DLSS feature
creation rather than relying on the switch value alone.

## Confirmed DLSS motion-vector decoding bug

The previous DLAA-versus-TSR vegetation comparison used an incorrect motion
vector decoder in the custom DLSS backend. It must not be treated as evidence
of an inherent DLAA limitation on foliage.

DLSSSRMotionVector.usf copied only the linear part of Unreal's velocity decode.
In this UE5.8 source, Common.ush enables VELOCITY_ENCODE_GAMMA for SM5 and above:
encoding applies sign(v)*sqrt(abs(v))*sqrt(2), and decoding must apply
(v*abs(v))*0.5 after undoing the packing. Omitting that last step greatly
exaggerates small velocities. For example, a 0.1-pixel horizontal displacement
at width 1280 becomes approximately 11.31 pixels before quantization. This
invalidates DLSS history reprojection for pixels using the velocity buffer;
the separate camera-only depth-reprojection fallback did not have this error.

Fix: include Common.ush and call DecodeVelocityFromTexture directly rather
than maintaining a copied decoder. This affects the existing DLSS-SR capture
backend and its new opt-in viewport extension. The path-tracer RR shader does
not contain the faulty copied decoder and was not modified.

Investigation controls:

- The first WPO-off/on recordings had different viewpoints and are INVALID.
  They are excluded from the conclusions below.
- Repeated controls reset to the agreed road pose and check spectator transform
  before and after each capture. Vegetation WPO on/off/on measured tree-region
  variation 1.716 / 0.815 / 1.724 with the old decoder. All 89 temporary
  vegetation component overrides were restored. No vegetation assets saved.
- Force-output velocity was also tested, then cleared by a fresh server restart.
- Material graph inspection found the shared M_VegetationMaster uses material
  attributes, so a missing direct MP_WORLD_POSITION_OFFSET input is NOT evidence
  of absent WPO. Its wind function reads CARLA's VisualTime collection value.
  This investigation does not establish whether previous-frame wind evaluation
  is correct under all animated weather/replay conditions.

With original vegetation settings and the corrected decoder, the tree ROI
measured 0.798 and 0.800 in two captures (about 54% below 1.724). Construction
measured 0.895 / 0.894 versus 0.879 before. These are regional temporal metrics,
not a claim that all perceived flicker is solved. Each before/after/repeat
recording contained 90 frames and zero duplicate pairs. A camera-motion clip
was recorded; moving traffic and varied weather remain unvalidated.

Validation: Unreal successfully compiled/loaded the modified shader on restart;
NGX created the 1280x720 DLAA feature without evaluation failures, and repeated
live captures completed. Diff whitespace checks passed. The server was left
with native DLAA active, normal vegetation and default velocity controls.
Evidence: .omc/scene-artifacts-2026-09-10/dlss-velocity, including a 2x nearest-
neighbor crop comparison of the trees (no temporal smoothing in the video).

NVIDIA's integration guidance independently identifies motion-vector correctness
as an important foliage diagnostic:
https://forums.developer.nvidia.com/t/ue5-1-dlss-buzzing-artifacts-on-foliage/249160
The actual mismatch and fix above are established from this engine's source
and local runtime tests, not inferred from that forum report.
