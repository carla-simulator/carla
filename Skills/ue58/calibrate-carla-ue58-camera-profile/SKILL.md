---
name: calibrate-carla-ue58-camera-profile
description: Creates or recalibrates a CARLA camera profile (post_process_profile, the JSON presets in Content/Carla/Config/PostProcess) from reference photos — frames of a driving dataset (nuScenes, KITTI, Cityscapes, your own fleet camera) or any photographic look (a 1950s sepia print, black and white, a warm consumer camera). Measures the references (black and white level, mid-tone, colour cast, colour left after toning, grain, vignette), then fits exposure, contrast, black lift, saturation, colour toning, grain and vignette on a running server by rendering street views with the candidate profile and comparing, until the frames match. Use when the user asks to "make a camera profile that looks like these images", "calibrate the camera to my dataset", "match the RGB sensor to KITTI/nuScenes frames", "create a vintage / sepia / black-and-white camera", or to check how far a profile is from real frames.
license: MIT
compatibility: Linux with a CARLA 1.0 / ue58-dev server running on this machine (editor -game from a ue58-dev tree, or a package) and the carla Python client with numpy and Pillow. The profile file is written next to the server, so the server must be local. VERIFIED on ue58-dev 94f2985b4 (editor -game, Town10HD_Opt): a 1950s sepia look from 9 references (test profile, not shipped) (cost 189 -> 0.57 in 12 renders), and a nuScenes front-camera fit from AutomotiveHDR (cost 14.5 -> 0.72). The package path (a loose JSON next to the .pak, as Docs/core_camera_presets.md describes) is not covered by those runs.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/calibration.md
---

# Calibrate a camera profile from reference photos (UE 5.8)

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

A camera profile is `Content/Carla/Config/PostProcess/<Name>.json`, the Unreal
post-process settings of the camera. A sensor picks it with
`post_process_profile=<Name>`, and the server reads the file **every time such a
camera spawns**: writing the file and respawning the camera is enough, no restart.
This skill closes the loop that a person does by eye in the editor:

```
reference photos --imgmetrics--> target medians
        |                                     \
  base profile + parameters --write JSON--> server --camera at N street views--> frames --imgmetrics--> medians
        ^                                                                                       |
        +-------------- damped Gauss-Newton on (frame medians - target medians) ----------------+
```

## Instructions

```
Progress:
- [ ] Step 1: Server running, prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Collect and check the references (imgmetrics.py)
- [ ] Step 3: Choose base, camera geometry, weather and parameters
- [ ] Step 4: Calibrate (calibrate.py); read report.md and look at sheet.jpg
- [ ] Step 5: Show it (capture.py or manual_control.py) and hand over the JSON
```

### Step 1: Server and prerequisites

The server must run on this machine, from the tree or package the profile goes
into. Ask the user which one when it is not recorded (`CARLA_UE58_ROOT` for a
ue58-dev tree, `CARLA_PACKAGE_ROOT` for a package); never guess, several
checkouts on one machine is normal. Start it with [[run-carla-ue58-server]] if
none is up, then:

```bash
export CARLA_UE58_ROOT=/path/to/ue58-dev        # or CARLA_PACKAGE_ROOT=/path/to/package
bash scripts/check_env.sh
```

### Step 2: References

Put the reference images in one directory. What makes a good set:

- **5 to 30 images of the same look**, street-level outdoor scenes like the
  views CARLA renders (a street, buildings, road). Portraits, interiors, maps
  and aerials skew the statistics.
- **One condition per set.** For a dataset camera, one set per weather/time of
  day (sunny, overcast, night) and one calibration each, with `--weather` to
  match. Never mix day and night in one set.
- Frames, captions and scanner margins are fine: the metrics ignore 6% of
  each side. Larger borders: crop them first.
- For a **look** with too few examples (one old photo), more can be made: take
  black-and-white street photos (public domain, e.g. Wikimedia Commons) and tone
  them with the colour of the example as a function of brightness. See
  [references/calibration.md](references/calibration.md#making-more-references).

Check the set before using it:

```bash
python3 scripts/imgmetrics.py REFS_DIR
```

Read the table: every row should resemble the median. Drop outliers (one
night frame in a day set, a colour photo in a monochrome set, one very washed-out
print) rather than letting them pull the median.

### Step 3: Choose what to fit

| Choice | Dataset camera | Photographic look |
| --- | --- | --- |
| `--base` | `AutomotiveHDR` (machine-vision camera, sensor noise, fast AE) | `Cinematic` (photo camera, filmic, no sensor noise) |
| `--size`, `--fov` | the dataset camera's (table in `Docs/core_camera_presets.md`), e.g. nuScenes `1600x900`, `70` | the references' aspect, e.g. `1440x1080`, `50` |
| `--weather` | the condition of the set, e.g. `ClearNoon`, `CloudyNoon` | the light in the photos, usually `ClearNoon` |
| `--fit` | `ev,contrast,lift,sat,ta,tb` | `ev,contrast,lift,sat,ta,tb,vignette` |
| `--fixed` | nothing | `sharpen=0`, and grain set by hand: `filmGrainIntensity=0.5` |

What each parameter is and the metric it moves is in the header of
`scripts/calibrate.py`. Keep **grain** out of `--fit` unless the references are
as sharp as CARLA: the grain metric also sees fine scene detail, CARLA frames
are sharper than old prints and the fit drives the grain to zero. Set it with
`--fixed filmGrainIntensity=…` instead. Keep **vignette** out for sets that are
not street views (it compares corners to centre, which depends on the scene).

### Step 4: Calibrate

```bash
python3 scripts/calibrate.py --refs REFS_DIR --name NewProfile --base Cinematic \
  --weather ClearNoon --size 1440x1080 --fov 50 \
  --fit ev,contrast,lift,sat,ta,tb,vignette --fixed sharpen=0 filmGrainIntensity=0.5 \
  --out calib_NewProfile
```

One render (4 views) takes 10–20 s; a run is about 1 + (number of parameters) +
up to `--iters` renders, 3–5 minutes. It prints one line per render with the cost
(sum of squared residuals, each residual in the units of `TARGETS` in
`calibrate.py`: 3 L* for levels, 1 for colour) and stops when the cost drops below
`--tol` (1.0). Under ~2 the frames match the references on every metric.

At the end the best profile stays in the server's profile directory and the work
directory holds `NewProfile.json`, `report.md` (reference vs CARLA per metric),
`calibration.json` (every render) and `sheet.jpg` (references on top, CARLA frames
below). **Look at `sheet.jpg`** before reporting success: matching numbers with
the wrong look means the references or the parameter choice are off (Gotchas).

### Step 5: Show and hand over

```bash
python3 scripts/capture.py NewProfile Cinematic --views 4 --out compare   # side by side, with metrics
python3 PythonAPI/examples/manual_control.py --post-process-profile NewProfile  # drive with it
```

`capture.py PROFILE --set field=value …` renders a temporary variant without
touching the profile, to see what one field does.

A trial run leaves its profile in the server's directory too: delete
`<Name>.json` there when it was only a test.

Tell the user where the JSON is. In a source tree it is a new file in the
Content repository (`Unreal/CarlaUnreal/Content/Carla`): shipping it with CARLA
is a Content commit, plus a row in `Docs/core_camera_presets.md`. In a package it
is a loose file next to the `.pak`; copy it to other packages the same way.

## Gotchas

**Problem: colours of the scene survive in a monochrome or sepia profile**
Cause: the colour metric was saturated by the toning, or a colourful set kept the
saturation high. `chroma_sd` measures colour *after* removing the brightness-
dependent toning, so a toned print scores ~1–3 and a colour photo 6–20.
Solution: check the references' `chroma_sd`; if one colour image hides in the
set, drop it. `sat` near 0.1 is normal for sepia.

**Problem: toning disappears when saturation goes to 0**
Cause: tinting through `WhiteTemp` or `SceneColorTint` happens before the
saturation in Unreal's grading. Solution: nothing to do with this skill: it tones
through `colorGain` (parameters `ta`, `tb`), which is applied after the
saturation. Do not add `WhiteTemp` to `--fixed` for toning.

**Problem: the cost goes down, then jumps up and the run ends high**
Cause: too few iterations, or one reference far from the rest. Solution: rerun
with `--iters 10`; look at the `imgmetrics.py` table of the set for an outlier.

**Problem: the black level is way off on one reference**
Cause: a black plate border or caption bar wider than 6% of the side.
Solution: crop the image (or raise `CROP` in `imgmetrics.py` for the run).

**Problem: frames are softer/sharper than the references and nothing changes it**
Cause: a JSON profile cannot blur. Optical softness comes from the
`M_SensorNoise` material instances, which are edited in the editor. Solution:
accept it, or render at a lower resolution, or reference an existing
`MI_SensorNoise_<Preset>` in `weightedBlendables` with `--fixed` (colour noise:
not for monochrome looks).

**Problem: other clients misbehave during the run**
Cause: the capture switches the world to synchronous mode (restored at the end).
Solution: run the calibration on a server nobody else is driving.

**Problem: a new profile is not picked up in a package**
Cause: the file went into the tree but the server runs a package, or the other
way round. Solution: `--root` / `CARLA_PACKAGE_ROOT` must name what the server
runs from; the file goes to `<package>/CarlaUnreal/Content/Carla/Config/PostProcess/`.

## Reference

- [references/calibration.md](references/calibration.md): the metrics, how each
  parameter moves them, the fit, making more references, and the numbers of the
  verified runs.
- `references/base/`: `Cinematic.json` and `AutomotiveHDR.json`, the bases a
  package cannot read from its `.pak`.
- `Docs/core_camera_presets.md`: the shipped presets, their camera geometry and
  the editor workflow ([[run-carla-ue58-server]] starts the server).
