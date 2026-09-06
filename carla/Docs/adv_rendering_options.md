# Rendering options

This guide details the different rendering options available in CARLA, including quality levels, no-rendering mode and off-screen mode.

- [__Graphics quality__](#graphics-quality)
	- [Quality levels](#quality-levels)
	- [Tier comparison](#tier-comparison)
	- [Render features disabled across every tier](#render-features-disabled-across-every-tier)
- [__Off-screen rendering mode__](#off-screen-rendering-mode)
	- [Setting off-screen mode](#setting-off-screen-mode)


!!! Important
    Some of the command options below are not equivalent in the CARLA packaged releases. Read the [Command line options](start_quickstart.md#command-line-options) section to learn more about this.

---
## Graphics quality

### Quality levels

CARLA ships with four graphics-quality tiers selected by the `-quality-level=<Tier>` command-line flag. The tier name is case-sensitive PascalCase: `Low`, `Medium`, `High`, `Epic`. When the flag is omitted, the simulator falls back to the built-in default tier `Epic`. An unrecognised value (typo, wrong casing) falls back to the platform default without applying any Carla tier and the engine log emits a warning naming the bad value.

```sh
./CarlaUnreal.sh -quality-level=Epic
./CarlaUnreal.sh -quality-level=High
./CarlaUnreal.sh -quality-level=Medium
./CarlaUnreal.sh -quality-level=Low

# When there is not a flag, Epic is the default.
./CarlaUnreal.sh
```

Each tier sets a coherent mix of memory budgets, foliage density, scalability bucket levels, and selected render-pipeline knobs. The character of each tier, from richest to cheapest:

* __Epic__: engine "Epic" preset across every axis with no overrides. Full anisotropy, full mesh LODs, full view distance, sharper Lumen reflections (downsample 1, two bounces). Same memory budget as High.
* __High__: engine "Epic" preset everywhere, except six render axes (view distance, mesh LOD bias, anisotropy, virtual-texture anisotropy, Lumen reflection downsample, Lumen reflection max bounces) pulled down at `ECVF_SetByDeviceProfile` priority. Memory pools at the Epic ramp. "Epic minus" intended for hardware that can afford rich rendering but wants a smaller view bubble and cheaper reflections.
* __Medium__: engine "Medium" preset across every scalability axis, with a moderate memory budget. Foliage and grass at full density. Engine-typical mid-range look.
* __Low__: full-resolution rendering with sharp textures, view distance, anti-aliasing, and shading kept high. Dynamic shadows are disabled and Lumen reflections are disabled; Lumen global illumination is kept at engine "Medium" level so the shadowed side of vehicles and distant buildings still receive diffuse bounce light, avoiding crushed-dark surfaces. The scene renders flat-lit with no cast shadows but with proper indirect fill. Foliage and grass are removed (`DensityScale=0`). A `TemporalAA` sharpening preset and motion-blur-off are applied for the HD-readable feel. Compact memory footprint.

The flag is identical on Windows and Linux. Inside the UE editor, scalability is governed by the editor's own `Settings/Engine Scalability Settings` menu rather than this flag.

#### Epic mode

![Epic mode screenshot](img/epic_rendering.png)
*Epic quality mode screenshot*

#### Low mode

![Low mode screenshot](img/low_quality_rendering.png)
*Low quality mode screenshot*

### Tier comparison

Values quoted verbatim from the per-tier CVar set in the CARLA selector module. Entries marked *default* inherit the engine default and are not modified by the tier.

| Aspect | Epic | High | Medium | Low |
|---|---|---|---|---|
| `r.Streaming.PoolSize` (MB) | 4000 | 4000 | 3000 | 3000 |
| `r.SkinCache.SceneMemoryLimitInMB` | 1024 | 1024 | 512 | 256 |
| `r.Nanite.Streaming.PoolSize` | 512 | 512 | 256 | 128 |
| `r.LumenScene.SurfaceCache.AtlasSize` | 4096 | 4096 | *default* | 2048 |
| `r.Shadow.Virtual.MaxPhysicalPages` | 8192 | 8192 | 4096 | 2048 |
| `r.Shadow.DistanceScale` | 2.0 | 2.0 | 1.5 | *default* |
| `r.ReflectionCaptureResolution` | 512 | 512 | 256 | 128 |
| `foliage.DensityScale` / `grass.DensityScale` | 1.0 | 1.0 | 1.0 | 0 |
| `Foliage.DitheredLOD` | 1 | 1 | *default* | *default* |
| `r.CustomDepth` | 3 | 3 | *default (1)* | *default (1)* |
| `r.DefaultFeature.AntiAliasing` | 1 (FXAA) | 1 (FXAA) | *default* | *default* |
| `r.Lumen.Reflections.MaxRoughnessToTrace` | 0.2 | 0.2 | *default* | *default* |
| `r.Lumen.Reflections.DownsampleFactor` | 1 | 2 | *(bucket)* | *(bucket)* |
| `r.Lumen.Reflections.MaxBounces` | 2 | 1 | *(bucket)* | *(bucket)* |
| `r.ViewDistanceScale` | 1.0 | 0.8 | *(bucket)* | *(bucket)* |
| `r.SkeletalMeshLODBias` | 0 | 1 | *(bucket)* | *(bucket)* |
| `r.MaxAnisotropy` / `r.VT.MaxAnisotropy` | *(bucket = 8)* | 4 | *(bucket)* | *(bucket)* |
| `r.DefaultFeature.MotionBlur` | *default* | *default* | *default* | 0 (off) |
| TAA sharpening preset | *default* | *default* | *default* | enabled |
| `sg.ShadowQuality` | 3 | 3 | 2 | 0 (no shadows) |
| `sg.GlobalIlluminationQuality` | 3 | 3 | 2 | 2 |
| `sg.ReflectionQuality` | 3 | 3 | 2 | 0 |
| `sg.PostProcessQuality` | 3 | 3 | 2 | 1 |
| `sg.EffectsQuality` | 3 | 3 | 2 | 1 |
| `sg.FoliageQuality` | 3 | 3 | 2 | 1 |
| `sg.TextureQuality` | 3 | 3 | 2 | 3 |
| `sg.ViewDistanceQuality` | 3 | 3 | 2 | 3 |
| `sg.AntiAliasingQuality` | 3 | 3 | 2 | 3 |
| `sg.ShadingQuality` | 3 | 3 | 2 | 3 |
| `sg.ResolutionQuality` | 100 | 100 | 100 | 100 |

Rows marked *(bucket)* take their value from the `[GroupName@N]` section in `DefaultScalability.ini` selected by the corresponding `sg.*Quality` row.

### Render features disabled across every tier

The following render-pipeline features are disabled at the project level for every tier, because their combined activation has not been validated stable on all target GPUs. Each tier renders with Software Lumen for GI plus reflections, and Virtual Shadow Maps for shadows.

| CVar | Project default | Effect when enabled |
|---|---|---|
| `r.Lumen.HardwareRayTracing` | False | Switches Lumen GI + reflections from compute-only paths to hardware RT cores. Sharper glossy reflections, accurate secondary bounces. |
| `r.RayTracing.Shadows` | False | Ray-traced shadows with physically accurate soft penumbras. Replaces Virtual Shadow Maps for the run. |
| `r.RayTracing.ForceAllRayTracingEffects` | 0 | Force-enables every RT-capable effect in the project. High BVH cost; only enable with HW-RT-capable GPUs. |
| `r.MegaLights.EnableForProject` | 0 | UE 5.5 MegaLights; many-light renderer for stylized / dense-light scenes. |
| `r.PathTracing` | False | Offline-quality unbiased path-traced rendering. Not real-time. |

Enabling any of these requires editing `Config/DefaultEngine.ini` (or the corresponding tier code in `CarlaDeviceProfileSelectorModule.cpp`) and rebuilding the simulator. The packaged Shipping binary strips the engine's `-execcmds=` command-line parser, so runtime overrides via that flag are not available in distributed CARLA releases; only project-side configuration takes effect.

The persisted `GameUserSettings.ini` does not need to be deleted when switching between tiers. The selector module re-applies the active tier's scalability after `UGameUserSettings::ApplyNonResolutionSettings` runs, so cross-run scalability state cannot shadow the current tier.

<br>

---

## Off-screen rendering mode

The off-screen rendering mode enables the CARLA simulator to run without the spectator. This would be the appropriate mode to use for rendering in a headless configuration, for example on a cloud virtual machine instance. Data from any cameras and other sensors in the simulation will still be rendered to disk or transmitted to other applications, but no rendering data will be passed to the screen. 

### Setting off-screen mode

To start CARLA in off-screen mode, run the following command:

```sh
./CarlaUnreal.sh -RenderOffScreen
```

---

