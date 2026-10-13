# CARLA / Unreal integration

## Reference implementation and branch compatibility

The Model 3 workflow lives in `Util/VehicleAuthoring/TeslaModel3/`:

- Source preparation: `fetch_source.py`, `prepare_candidate.py`, `enhance_wheels.py`, `upgrade_surfaces.py`, `upgrade_details.py`, `upgrade_optics.py`.
- Export/import: `export_parts.py`, `import_unreal.py`, `build_lods.py`.
- Verification: `validate_assets.py`, `validate_surface_assets.py`, `validate_runtime.py`, `validate_cameras.py`, `validate_collisions.py`, `capture_upgrade.py`.

Read scripts before execution: they contain Model 3-specific selections and can overwrite its assets. Adapt into a distinct vehicle authoring directory and namespace. Reuse the design, not its absolute coordinates, EV gearing, masses, or material gains.

Known baseline: CARLA `ue58-dev` code commit `e278569b85e835770ce4ea94cad8858e652c56d5`, content `ue58-dev-carla` commit `8b34717330184ff70c4ff7893676f0dfb5f389d4`. These identify the original implementation, not a requirement to reset branches. Verify support in the target checkout before using its opt-in tags.

Run Unreal Python commandlets with an absolute `.uproject` and script path, using the target engine's `-run=pythonscript -script=... -unattended -nullrhi` support. Build changed native helpers before importing. Confirm package saves actually reach disk. Reimport may preserve existing Blueprints and meshes; material updates do not prove component attachments were regenerated. Make deliberate, scoped regeneration decisions.

## Rig and physics

Verify exported axes, units, transforms, wheel bones/pivots, wheel radius, and chassis dimensions against the target pipeline. Do not blindly apply a coordinate convention twice.

The reference architecture uses a hidden skeletal physics scaffold with modular visible meshes. Hidden bones must still refresh. Explicitly assign and save the vehicle's intended physics asset on the skeletal mesh; an inherited template physics asset previously survived authoring and produced unstable collisions. Validate the assignment after fresh load.

After programmatically changing collision geometry, invalidate/rebuild cooked Chaos collision and save the owning packages. The reference native helper in `USDImporterWidget.cpp` was corrected for this and package filenames. Visual wheel components use `NoCollision` and `auto_weld=False`; Chaos wheel contact supplies tire interaction. These settings belong to this architecture, not every possible vehicle rig.

Check door pivots, simple collision, API ordering, and constraint lifecycle. CARLA's reference code tears down door constraints before welding to avoid Chaos self-joints. Collision response depends on shapes, masses, transforms, contacts, constraints, and timesteps; do not mask launch-on-impact bugs with arbitrary damping.

Configure plausible dynamics for the actual vehicle. Separate stability evidence from validated manufacturer dynamics. Reuse tests, not the Model 3's numerical tuning.

## Opt-in modular support

Verify these names in the current native code and importer:

| Feature | Reference convention |
| --- | --- |
| Paint actor tag | `Carla.StaticBodyworkPaint` |
| Painted mesh slot / MID parameter | `Bodywork_Mat` / `Base Color` |
| Caliper actor tag | `Carla.ModularWheelCalipers` |
| Caliper component tags | `Carla.Caliper.0` through `.3`, FL/FR/RL/RR |
| Physical light component tags | `Carla.Light.<group>`; inspect exact group names in code |

Catalog registration is under content `Config/VehicleParameters.json`; per-class lamp defaults are in `Config/Lights/Defaults.json`. Merge only the relevant definition. The reference light visibility path depends on saved defaults, so inspect that path as well as component tags. Keep existing vehicle behavior intact when modifying shared native code.

## Lighting: verify topology before gain

Treat visible lamp emission and actual road/cabin light sources separately. Place emitters outside blocking surfaces, orient cones correctly, and remove/disable inherited duplicate emitters. Use keyword arguments for Unreal Rotator fields to avoid positional-order mistakes.

Map Position, Low Beam, High Beam, Brake, Reverse, Fog, Interior, and left/right indicators deliberately. Do not map a roof lamp to High Beam or a daytime strip to Fog because a donor material did. Verify appropriate simultaneous states too.

Source meshes can contain coplanar opaque indicator/reverse faces. Inspect exported triangles, material slots, occlusion, and runtime overrides before increasing emission. For a genuinely shared surface, one shader can select white reverse or amber indicator with explicit priority; this is a design option based on actual lamp construction, not a rule for every car.

**Check shader connection return values and saved graph connectivity.** In the reference UE version, Saturate/OneMinus destination pins were unnamed (`''`); using `'Input'` silently left the graph disconnected. Large emission increases did not fix the dark indicators. Query/verify the actual API and node pins; merely finding a scalar parameter does not prove it reaches Emissive Color. Reload the material and check upstream connections, then visually verify both sides in CARLA. Model 3 emission gains are exposure-dependent examples, not portable photometry.
