# Model 3 validation — 2026-09-11

Tested in a dedicated CARLA UE5.8 editor server on Town10HD_Opt, RPC 4674. The original Eixample server on 4654 was not modified or restarted. Native Carla and CarlaTools builds succeeded. All authoring scripts parse, content JSON parses, and git diff whitespace checks pass.

## Functional evidence

- API: `vehicle.tesla.model3` successfully spawns and settles on the road.
- Sensor bounding box: 4.694 × 2.088 × 1.443 m, including mirrors.
- 35% throttle for four seconds: 6.61 m travelled, 12.09 km/h maximum.
- Full braking: 0.00 km/h after three seconds.
- Front wheel steering: 14.70°; front caliper: 14.70°.
- Maximum caliper pitch/roll during straight driving: 0.00°; calipers remain independent of wheel spin.
- All four doors exercised individually and together without a Chaos assertion.
- Existing Dodge Charger: three all-door open/close cycles passed; existing Audi TT spawn/cleanup also passed (its catalog does not advertise dynamic doors).
- RGB captures verify paint variation, lamp/indicator response and cabin assembly. Semantic capture contains 170235 car-class pixels; actor label is 14 (Car).
- 15 independent mesh parts; wheels retain LOD0, other parts have four LODs; 1,964,987 LOD0 triangles including the surface, construction and optical revisions.

Final scripts exited successfully. Earlier failed integration attempts remain only in the local audit directory for diagnosis; `validation.json` records the successful checks.

## Visual review and provenance

Reference photographs and Tesla body dimensions are linked in SOURCES.md. Reviewed front, rear, side, cabin, open-door, steered-wheel, night and rain captures. The selected base is Ameer Studio's licensed model, reworked here; it is not newly modeled from scratch or verified against OEM CAD.

`VehicleReview` is an opt-in neutral camera profile. Default CARLA grading uses reduced saturation, gamma adjustments and +1.2 EV exposure bias; default-profile comparison captures are retained locally. The default profile itself is unchanged.

Working artifacts: `artifacts/tesla-model3/source/tesla-model3-optics-upgrade.blend`, `renders/`, `export/`, and `audit/` in the workspace. Reproduction commands are in README.md.

## Limits

This is an integrated art candidate for close visual review. Geometry was adjusted from photographs and nominal dimensions; it is not scan-verified. The dynamics are a single-speed EV approximation, without validated Tesla tire/torque/suspension data. Lighting responds to CARLA states but is not photometrically calibrated to factory lamps. The screen is static artwork; mirrors have no dedicated rear-view camera render target. Hood/trunk are separate meshes but only the four doors are articulated. LODs are generated and functional; fleet-scale performance and every LOD transition have not been exhaustively benchmarked.

## Wheel silhouette and collision corrections

Fresh-process asset checks confirm the saved rig references `PH_Model3`, rather than the inherited USD template physics asset. The importer now persists that assignment, invalidates/rebuilds cooked Chaos collision after geometry changes, and enables chassis collision notifications. The skeletal scaffold is excluded from rendering, but its bones always refresh. Visual wheels do not block collision or auto-weld.

Removed whole-wheel automatic decimation: it distorted the low-density tire surface despite the much denser rim. Full wheel geometry now remains at all viewing distances. Matching side-view captures show the restored round silhouette. This increases distant-wheel rendering cost; purpose-built wheel LODs remain an optimization task.

Five controlled impact tests passed: rear impacts at 30/60 Hz, a side impact, and fixed-obstacle impacts initialized at 36 and 72 km/h. Every case produced vehicle collision events; maximum upward displacement was 0.043 m across the suite. Driving, braking, steering, caliper motion and all door operations passed again after the corrections. These tests do not reproduce the user's exact unidentified collision or establish OEM crash dynamics.

The local live demo now advances at a fixed 1/30 s with physics substeps, so rendering stalls do not change simulation time steps.

## Tire and wheel surface detail revision

The detail source is `source/tesla-model3-wheel-detail.blend`; the turned-wheel inspection copy is `source/tesla-model3-tire-review.blend`. The revision replaces texture-only tread and pre-lit disc surfaces with modeled channels, lateral cuts, rounded sidewalls, moulded rings and curved embossed lettering. Rubber uses new 2048px normal/roughness maps. The disc uses a circular vented mesh and independent metallic roughness, with valve stems and corrected caliper paint lettering. Source spoke geometry is retained; its baked-lighting albedo is replaced with a neutral silver material.

Manufacturer close-up photographs were inspected; lettering and tread are authored approximations. High-detail wheel geometry remains unreduced at distance; fleet-scale rendering cost has not been validated for this revision. Physics assets, anchors and collision settings pass the fresh-load asset regression.

CARLA RGB close-ups of straight and steered wheels were inspected after reimport: tread, sidewall lettering and material maps are present; calipers follow steering. The earlier impact suite was not rerun for this visual-only revision.

## Surface, construction and lighting upgrade

The current master includes refined body normals/clear coat, corrected glazing
volumes, upper seals, bevelled fittings, camera lenses, arch liners/undertray,
upholstery grain, fitted steering stitches and a dark-glass emissive LCD. Curved
headlight reflectors and original flute normals replace baked chrome imagery.
Rear guide emission retains red at night. See UPGRADE.md for item-by-item evidence.

The final saved revision passed native compilation, fresh physics/material/LOD
checks, motion/braking/steering/all-door checks, twelve-view RGB capture and all
five impact regressions. These latest results supersede the earlier visual-only
revision's note about not repeating impacts. Maximum rise remains 0.043 m.

Forty isolated light-state captures verified API state round-trips. Interior now
controls the roof lamp, High Beam controls the headlamp emitters, and Position is
separate from Fog. Seven tagged components provide beam/cabin illumination; the
template low beams are disabled. Final road/roof comparisons verified beam reach
and that High Beam does not activate the roof lamp. Beam strengths and patterns
are renderer-calibrated approximations. The physics and fleet-performance limits
above still apply.

Final rear-light acceptance: both rear indicators are visibly amber on the
selected side, opposite-side off, and reverse is white. Unnamed shader pins
are now connected with checked return values and saved-graph assertions. See
`UPGRADE.md` for the final capture and material evidence. All three final
integration tools exit 0; all five impact cases pass after the last body import.
