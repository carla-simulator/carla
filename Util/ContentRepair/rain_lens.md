# Procedural camera rain

`M_LensRain` replaces `M_screenDrops` as the RGB camera precipitation pass.
The shader source is `Plugins/Carla/Shaders/RainLens.ush`, under the Unreal
project. `build_rain_lens.py` creates the material graph, exposing its artistic
controls and wiring in the existing weather collection. The previous material
and textures remain available for reference.

## What changed

The old material uses repeated scrolling normal/mask textures and a radius-10
Gaussian blur with nested loops: 441 scene samples in the blur alone. Its final
blend divides the weather collection's already-normalized `Precipitation`
value by 100 again, reducing the visible effect dramatically.

The new effect reads that 0–1 value directly. Rain increases droplet density.
Independent, staggered cells generate small resting beads and larger drops
that form, dwell, run downward, and fade with a short wet trail. Aspect-correct
coordinates preserve droplet proportions. Animation uses Unreal game time,
without accumulated per-frame state.

Refraction samples the actual scene underneath each droplet. A five-tap local
filter softens the refracted image, and the uncovered image stays unchanged.
The cinematic preset increases coverage and bead size, blends the refracted
image at 68% strength, and removes the dark rim multiplier. Lens water adds no
white color or full-frame blur. The shader makes five
scene lookups plus the original color input, rather than the old nested blur.
This is a shader sample-count comparison, not a measured end-to-end speedup.

Scene sampling converts viewport UV to post-process input UV and clamps to
the input view's bilinear bounds. This follows the distinction documented in
[Epic's post-process material guide](https://dev.epicgames.com/documentation/unreal-engine/post-process-materials-in-unreal-engine).
It avoids sampling neighboring viewports or stretching the rendered scene
when the view and scene-buffer sizes differ.

`AWeather` also removes the rain material from existing cameras when rain
stops. Previously it only removed it from its own active-material map, leaving
the old pass attached to the cameras. Dust uses the same removal correction.
Other camera blendables are preserved.

## Art controls

Scalar parameters in the material's **Lens Rain** group:

| Parameter | Default | Purpose |
| --- | ---: | --- |
| LensCoverage | 1 | Multiply weather-driven bead density; 0 clears the lens |
| LensDropScale | 1.2 | Droplet size relative to image height |
| LensRefraction | 0.75 | Strength of local optical distortion |
| LensSoftness | 1.75 | Softness inside the droplets |
| LensSeed | 0 | Deterministic spatial variation |

These are an artistic lens approximation, not a fluid simulation: droplets do
not collide/coalesce or respond to camera acceleration, lens shelter, or
wipers. The default seed is shared; material instances can vary the pattern.
The world-space rain particles and wet-road material are separate effects.

## Cinematic falling streaks

The old `M_Rain` used unlit, translucent sprites with a fixed dark color
(approximately 0.059, 0.065, 0.068), alpha 0.2, and index-of-refraction varying
from 1 to 1.05. Those sprites attenuated bright backgrounds into dark lines
and refracted entire building edges while falling.

`build_rain_streaks.py` rewires the existing material to use a narrow Gaussian
cross-section with soft ends, distance fade, and depth fade at surfaces.
Additive compositing represents cinematic rain highlights without subtracting
the scene into black streaks. Surface refraction is neutralized; lens droplets
continue to provide local refraction. `StreakBrightness` (0.18) and
`StreakOpacity` (0.65, multiplied by particle alpha) are exposed for art tuning.
Inverse eye adaptation stabilizes the artistic streak brightness across
exposure changes. This is deliberately a cinematic highlight model, not a
lighting-dependent physical simulation; it can remain visible in dark scenes.
The particle system itself is unchanged.

Actual rain appearance varies with illumination, viewing angle, and exposure;
rain is not universally white. See [Garg and Nayar's rain-streak research](https://www.cs.columbia.edu/CAVE/publications/pdfs/Garg_TOG06.pdf).

## Verification and preview

### Flicker investigation (2026-09-10)

The default camera profile applied `sharpen: 3`. This strongly amplified
temporal noise on building edges and wet-road reflections, including in dry
weather. `Config/PostProcess/Default.json` now uses 0.3. Temporal AA, shadows,
Nanite, ambient occlusion, and Lumen remain enabled; diagnostic console
overrides are not part of the fix. This changes cameras using the Default
profile, including dry-weather cameras; custom profiles retain their settings.

At a fixed Town10HD_Opt spawn-0 camera, 1280×720, 20 simulation frames/s,
100% rain, 180 warmup/capture ticks with the last 40 consecutive frames
measured, mean absolute RGB difference between adjacent frames fell from
1.360 to 0.650 on the 0–255 scale (52%). The façade region fell from 2.736
to 1.307, and the road region from 1.246 to 0.592. These measurements include
legitimate rain animation; they measure image variation, not a pure flicker
score. Separate recordings have different droplet animation phases.

Switching TSR to TAA or disabling Nanite, shadows, or Lumen reflections did
not eliminate the surface shimmer. Overlapping geometry has not been
established as its cause. Lower sharpening addresses a confirmed amplifier,
not all remaining temporal instability. The legacy wet-surface ripple
animation and reflection noise still warrant investigation before final rain
intensity tuning. No changes to rain intensity were made during this check.

- Built `CarlaUnrealEditor Linux Development` successfully.
- Rendered dry, 20%, 85%, night rain, and a return to dry with one RGB camera.
- Rendered cameras spawned during rain at 1280×720, 800×600, and 720×1280.
- Ran `test_rain_transitions.py`: 0→20→85→0→100→0 gives exactly one correctly
  weighted rain blendable when wet and none when dry, while preserving an
  unrelated user material.
- Checked the rendered shader logs for compilation failures.

Run the editor test with:

```sh
UnrealEditor-Cmd CarlaUnreal.uproject -run=pythonscript \
  -script=/absolute/path/to/Util/ContentRepair/test_rain_transitions.py \
  -nullrhi -unattended -nosound
```

Start a dedicated server and preview its RGB sensor output with:

```sh
python PythonAPI/examples/rain_lens_demo.py --port 4654
```

The preview replaces that server's map. Keys 1–4 select dry/light/heavy/storm;
N toggles day/night; Space toggles cycling; Esc closes the preview. The preview
shows a sensor feed. The native weather integration also applies lens droplets
to the spectator viewport through a camera modifier.

Ship the code/shader and the new `M_LensRain.uasset` together. Restart the
editor/server after updating; packaged builds must cook the new material.


## Native falling-rain integration

`AWeather` owns the spectator rain volume and manages one volume for each RGB
camera, adopting the legacy blueprint emitter when present. Each sensor volume uses Unreal's owner visibility and capture-only
flags; the spectator volume is hidden from scene captures. CARLA's capture
component identifies its sensor actor as the view owner, so colocated sensors
see independent rain without stacking each other's particles. Sensor destruction
and world teardown release their components. Teleports clear old particles.
There are no town names or placed rain actors in this runtime path.

Use the existing API; no extra Python API build or weather fields are needed:

```python
weather = world.get_weather()
weather.precipitation = 85  # 0 clears droplets and falling particles
weather.wind_intensity = 35  # sideways rain motion, also existing foliage wind
world.set_weather(weather)
```

Precipitation maps linearly to spawn rate (4000 particles/s at 85%). The legacy
`PS_Rain` velocity distribution expects normalized Z=1 for downward motion;
negative Z clamps to zero. Runtime material overrides use
brightness=1.2, opacity=0.65 and scale=2, softening the earlier visibility preview. Wetness and precipitation deposits
remain independently controlled by their existing weather fields.

Main-view lens droplets use a camera modifier; RGB droplets retain their
per-sensor blendables. This avoids applying an unbound lens pass to unrelated
captures. Runtime writes the lens precipitation collection directly, independent
of the town sky blueprint. The native integration requires restarting the server.


Validation of the native integration (2026-09-10): rebuilt the Carla module;
rendered 0 → 20 → 85 → 0 precipitation in Town03_Opt and Town10HD_Opt with
separated RGB cameras, then spawned a third camera during rain at the first
camera's position. Inspected emitter ownership/visibility in both towns:
one emitter per RGB sensor, capture-only and owner-only; the spectator emitter
is excluded from captures. Heavy rain produced approximately 4200 live particles
per volume, with no additional legacy emitter. Dry viewport inspection confirms
both lens and falling-rain passes disappear (Cascade retains an internal particle
count after completion; inactive components do not render it).

Final tuning reduces brightness from 3 to 1.2, opacity from 1 to 0.65, and
spawn rate from 6000 to 4000 at 85% precipitation. Droplet tuning is unchanged.
Rainy foliage flicker remains a separate, unresolved issue. Runtime is independent
of town names; these two towns were tested, not every installed map. Validation
used a development server; a packaged build was not tested.

Evidence: `.omc/scene-artifacts-2026-09-10/rain-native/` in the workspace root.
Repeat the API capture test with `test_rain_runtime.py --port 4654 --town
Town03_Opt --load`; it restores weather and spectator pose within the selected
world on exit, but does not restore the previously loaded map.
