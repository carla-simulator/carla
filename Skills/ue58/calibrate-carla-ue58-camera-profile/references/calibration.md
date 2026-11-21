# Calibration reference

## Metrics (`scripts/imgmetrics.py`)

Every image loses 6% of each side (frames, captions) and is resized to 960 px
wide, so references and CARLA frames of any size are measured at the same scale.
All values are on the display-referred image, in CIE L\*a\*b\*.

| Metric | Meaning | Typical values |
| --- | --- | --- |
| `L_p05`, `L_p50`, `L_p95` | 5th, 50th, 95th percentile of L\* (0–100): black level, mid-tone, white level | modern camera 5 / 45 / 95; faded print 15–30 / 60 / 85 |
| `a_mid`, `b_mid` | mean a\* and b\* of mid-tones (25 < L\* < 85): the colour cast | neutral 0 / 0; sepia 1.5 / 11; cool camera b\* -3 |
| `chroma_sd` | spread of a\*/b\* after removing the mean a\*/b\* of each 5-L\* band: scene colour left once toning is removed | toned or black-and-white print 1–3; colour photo 6–20 |
| `grain` | 20th percentile of the local std of the high-passed L\* over 16 px blocks: noise in flat areas | 0.5–1.5 |
| `sharpness` | mean L\* gradient (report only) | CARLA 4–5; old prints 3–4 |
| `vignette` | centre L\* minus bottom-corner L\* (scene dependent) | 0–20 |

The medians over a set are the targets. One image off the median by much more
than the others is an outlier: drop it.

## Parameters (`scripts/calibrate.py`)

Unreal grades each pixel in linear light, before the filmic tone curve, in this
order: white balance, **saturation, contrast, gamma, gain, offset**, then the tone
curve. The skill fits:

| Parameter | Field | Moves | Measured effect (Town10HD_Opt, Cinematic base, ClearNoon) |
| --- | --- | --- | --- |
| `ev` | `autoExposureBias` | everything up/down | +0.4 EV: L_p50 +6.5, L_p95 +4.4, L_p05 +3.8 |
| `contrast` | `colorContrast.w` | spread around 18% grey | +0.12: L_p05 -3, L_p95 +2.5 |
| `lift` | `colorOffset.w` | black level (after the gain) | +0.006: L_p05 +2.8 |
| `sat` | `colorSaturation.w` | scene colour | +0.12: chroma_sd +1.7 |
| `ta` | `colorGain.xyz` along (0.5, -0.5, 0) | a\* (magenta–green) | +0.08: a_mid +3 |
| `tb` | `colorGain.xyz` along (0.35, 0.35, -0.7) | b\* (yellow–blue) | +0.08: b_mid +2.8 |
| `grain` | `filmGrainIntensity` | grain | +1.0: grain +0.5 (invisible below ~0.3) |
| `vignette` | `vignetteIntensity` | corners | +0.2: vignette +7.5, L_p50 -5 |

The gain is renormalised to keep luminance (exposure is `ev`'s job) and is
applied **after** saturation, which is why a profile with saturation near 0 can
still be toned. `WhiteTemp` and `SceneColorTint` act before the saturation and
are left alone.

Any other field can be pinned with `--fixed field=value` (case-insensitive;
`field.x` for one component; the value is JSON): `sharpen=0`,
`bloomIntensity=0.8`, `motionBlurAmount=0`, `filmGrainIntensity=0.5`.

## The fit

Residual per metric = (CARLA median - reference median) / scale, with scale 3 L\*
for the levels, 1 for a\*/b\* and chroma, 0.2 for grain, 3 for vignette. Cost = sum
of squared residuals. The first render is the base; then one render per parameter
moved by its step gives the Jacobian (finite differences); then damped
Gauss-Newton (Levenberg–Marquardt, Broyden update of the Jacobian after every
render) for up to `--iters` renders or until the cost is under `--tol`. The best
render wins, not the last.

Each render spawns one camera with the profile, visits the views (spawn points 60 m
apart, raised to 1.7 m, looking along the lane, `--seed` picks them) and waits
`--warmup` ticks (60 × 0.05 s) per view for the auto exposure to settle, in
synchronous mode. Fixed views (`--views-file`) make two runs comparable.

## Making more references

When there are only one or two examples of a look:

1. Find black-and-white street photographs of the same era and kind of scene,
   ideally public domain (Wikimedia Commons: search "street 1955", "city street
   1952 photograph", "sepia street 1950"; keep CC0 / public domain / no
   restrictions, note author and licence).
2. Tone them with the example's colour per brightness:

   ```bash
   python3 scripts/tone_references.py example.jpg refs/ bw/*.jpg
   ```

3. Put the example(s), any real toned photos found, and the toned copies in one
   directory and check it with `imgmetrics.py`.

The toned copies borrow the example's colour but keep their own exposure and
contrast, so they add variety where it matters for the levels and keep the
toning consistent.

## Verified runs

ue58-dev 94f2985b4, editor -game, Town10HD_Opt, ClearNoon, 4 views, RTX 3090.

**Vintage50s** (sepia street photographs, 1950s look; a test profile, not shipped). 9 references: 1 example
(Madrid, 1950s), 3 real sepia prints from Commons, 5 black-and-white Commons
photos toned from the example. Base `Cinematic`, 1440x1080, fov 50,
`--fit ev,contrast,lift,sat,ta,tb,vignette --fixed sharpen=0 filmGrainIntensity=0.5`.
Cost 188.5 -> 0.57 in 12 renders (~4 min):

| | L_p05 | L_p50 | L_p95 | a_mid | b_mid | chroma_sd | vignette |
| --- | --- | --- | --- | --- | --- | --- | --- |
| references | 11.75 | 66.11 | 92.33 | 1.64 | 11.03 | 1.62 | 16.40 |
| CARLA | 10.5 | 65.1 | 92.7 | 1.6 | 10.9 | 1.1 | 16.6 |

Result: ev 1.80, contrast 1.00, lift -0.028, saturation 0.097, ta 0.112,
tb 0.303, vignette 0.91. Without the 6% crop, two plate borders dragged L_p05 down
and the fit made the image too contrasty; with `grain` in the fit it went to 0.

**nuScenes front camera** (dataset case). 32 sunny CAM_FRONT frames, base
`AutomotiveHDR`, 1600x900, fov 70, `--fit ev,contrast,lift,sat,ta,tb`. The shipped
preset was tuned on nuScenes, so it starts close: cost 14.5 -> 0.72 in 10 renders
(ev -0.67, contrast 1.47, lift 0.030, saturation 0.77, slightly cooler). The
dataset frames are softer (sharpness 1.9 against 3.8) and that stays: see the
softness gotcha.
