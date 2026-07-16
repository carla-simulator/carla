# author_map_sky.py

Authors daytime native lighting into a CARLA map that ships a rig-less, dark
lighting setup.

## Problem

Some CARLA 0.10 maps (e.g. `Town04_Opt`) are lit at runtime only by their own
native `DirectionalLight` + `SkyLight` actors, with no `SkyAtmosphere` in the
level. That combination renders as dim dusk: the sun is present but there is
no scattering sky, and the `SkyLight` is a dark baked cubemap rather than a
captured-scene light. Through the CARLA RGB sensor (as opposed to the
interactive viewport) this reads as a near-black frame, because the sensor's
auto-exposure does not compensate enough for a scene this dark (see the
gotcha below).

## What it does

`author_map_sky.py` runs inside the UE editor's embedded Python
(`PythonScriptPlugin`), headless, and upgrades a map's *native* lighting rig
in place:

1. Keeps the map's native `DirectionalLight` as the runtime sun (no BP
   transplant), ensuring `atmosphere_sun_light=True`; optionally sets its
   intensity via `SUN_INTENSITY`.
2. Adds a `SkyAtmosphere` actor if none is present, driven by that
   `DirectionalLight` — this is the single biggest missing piece and is what
   produces a physically bright scattering sky.
3. Switches the native `SkyLight` to real-time captured-scene mode
   (`source_type=SLS_CAPTURED_SCENE`, `real_time_capture=True`,
   `mobility=MOVABLE`) so it captures ambient light from the now-bright
   atmosphere instead of a dark baked cubemap, at a tunable intensity scale.
4. Optionally adds an unbound histogram `PostProcessVolume` for the
   interactive game viewport only (`ADD_PP=1`; off by default) — this has no
   effect on CARLA sensor captures, which override their own exposure.

An earlier approach — transplanting the `BP_Carla_Sky` blueprint rig from
`Town10HD_Opt` into the target map — was abandoned: CARLA renders `Town04_Opt`
via its own native standalone `DirectionalLight`, not a placed `BP_Carla_Sky`
actor, so the transplanted rig never actually drove the scene's lighting.
Only the native-lighting family of modes below ships.

## Modes

| MODE | Purpose |
|---|---|
| `list-native` | Inspect a map's native `DirectionalLight`/`SkyLight`/`SkyAtmosphere`/`PostProcessVolume` actors and log their key properties. Read-only, no save. |
| `apply-native` | Author daytime native lighting into the target map (steps 1-4 above) and save it. |

All configuration is passed via environment variables — UE's
`-run=pythonscript` argument passing is unreliable, so env vars are used
instead of CLI flags:

```
MODE               list-native | apply-native
TARGET_MAP         /Game path of the map to author (default Town04_Opt)
# apply-native only:
ADD_ATMOSPHERE     add a SkyAtmosphere if none present (default 1)
SKYLIGHT_CAPTURED  switch the native SkyLight to real-time captured scene (default 1)
SKYLIGHT_INTENSITY captured-SkyLight intensity scale (default 1.0)
SUN_INTENSITY      set the native DirectionalLight intensity (default: leave as-is)
ADD_PP             add an unbound histogram PostProcessVolume (default 0; viewport only)
PP_BIAS            AutoExposureBias for that PostProcessVolume (default 1.2)
```

`ADD_PP=1` is a no-op when the map already has a `PostProcessVolume`; that case
is logged (`PostProcessVolume already present; not adding another`) rather than
silently ignored. A malformed numeric env value (e.g. `SKYLIGHT_INTENSITY=1,0`)
is logged and falls back to the default instead of crashing the run.

### Failure reporting

`apply-native` always saves (so a partial result is inspectable), but it makes
incompleteness loud: any write that fails or silently no-ops (writes are
verified by read-back), a requested step whose target actor is missing (e.g. no
`SkyLight` while `SKYLIGHT_CAPTURED=1` — it may be stranded in an unloaded
streamed sub-level), or a malformed numeric env value all count toward a final
`WARNING: authoring incomplete -- N write(s) failed` line. Treat that WARNING as
"re-check before trusting the map", even though `saved ...` also printed.

## Invocation

Both modes run through the `-run=pythonscript` commandlet. The commandlet's
async shader compiler (enabled via `-AllowCommandletRendering`, needed so the
`SkyAtmosphere`/`SkyLight` actually compute) can crash a worker thread on
shutdown *after* the save has already completed — judge success by the
`saved ...` log line (and a follow-up `list-native` check), not the process
exit code.

Headless UE still needs an `X` display to initialize its RHI, even though
nothing is shown; point `DISPLAY` at any running X server.

Inspect (read-only):

```bash
export CARLA_UNREAL_ENGINE_PATH=/path/to/UnrealEngine
CARLA_ROOT=/path/to/carla
UE="$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux"
PROJ="$CARLA_ROOT/Unreal/CarlaUnreal/CarlaUnreal.uproject"
MODE=list-native TARGET_MAP=/Game/Carla/Maps/Town04_Opt DISPLAY=:0 \
  timeout 600 "$UE/UnrealEditor" "$PROJ" -run=pythonscript \
  -script="$CARLA_ROOT/Util/Tools/author_map_sky.py" \
  -unattended -nosplash -stdout -FullStdOutLogOutput -AllowCommandletRendering
```

Author (mutates and saves the map — verified daytime recipe for
`Town04_Opt`):

```bash
export CARLA_UNREAL_ENGINE_PATH=/path/to/UnrealEngine
CARLA_ROOT=/path/to/carla
UE="$CARLA_UNREAL_ENGINE_PATH/Engine/Binaries/Linux"
PROJ="$CARLA_ROOT/Unreal/CarlaUnreal/CarlaUnreal.uproject"
MODE=apply-native TARGET_MAP=/Game/Carla/Maps/Town04_Opt \
  SKYLIGHT_INTENSITY=1.0 SUN_INTENSITY=20000 DISPLAY=:0 \
  timeout 600 "$UE/UnrealEditor" "$PROJ" -run=pythonscript \
  -script="$CARLA_ROOT/Util/Tools/author_map_sky.py" \
  -unattended -nosplash -stdout -FullStdOutLogOutput -AllowCommandletRendering
```

Filter the log for `author_map_sky` to see just this tool's output, e.g.
append `2>&1 | grep -i author_map_sky`.

## Verification

Boot the map headless in game mode with rendering enabled but off-screen (not
`-run=pythonscript`, which is a UI-less editor commandlet — verification
needs the actual runtime/game path a sensor uses):

```bash
"$UE/UnrealEditor" "$PROJ" -game -RenderOffScreen -nosound
```

Then run a luminance probe against the running simulation — a small CARLA
client that spawns a vehicle + RGB sensor, grabs a settled frame, and reports:

- `mean_y` / `roi_mean_y` — full-frame / region-of-interest mean luminance,
- `roi_contrast` — p95-p50 of ROI luminance (lane-marking legibility proxy),
- `blown_frac` — fraction of ROI pixels at luminance >= 250 (blowout).

A correctly authored daytime frame should land at `roi_mean_y` roughly in the
90s-100s (comparable to `Town10HD_Opt`, which ships its own working rig) with
`blown_frac` at or near 0, versus `roi_mean_y` ~10 (near-black) on the
unauthored map.

## Gotcha: the RGB sensor's auto-exposure has a fixed floor

The CARLA RGB sensor clamps its own auto-exposure adaptation to a fixed
luminance window (approximately `[10, 12]` cd/m²) regardless of scene
brightness. A `PostProcessVolume` exposure lever (`AutoExposureBias`,
histogram min/max, etc.) only affects the *interactive game viewport* — it is
**inert on sensor output**, because the sensor overrides its own exposure
settings independent of any placed `PostProcessVolume`. This is why
`author_map_sky.py`'s `ADD_PP` option is off by default and documented as
viewport-only.

The only way to make a CARLA sensor capture render as daytime is to make the
*scene itself* physically brighter — the levers this tool applies (a
scattering `SkyAtmosphere`, a real-time captured `SkyLight` for bright
ambient, and enough `DirectionalLight`/sun illuminance) — rather than trying
to compensate after the fact with post-process exposure.
