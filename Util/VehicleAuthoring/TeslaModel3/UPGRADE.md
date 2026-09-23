# Fidelity upgrade acceptance — 2026-09-11

Reviewed master: `artifacts/tesla-model3/source/tesla-model3-optics-upgrade.blend`.
The source, exported parts and imported CARLA vehicle include the surface,
construction, tire and optical revisions. Reproduction commands are in README.md.
Evidence paths below are relative to `artifacts/tesla-model3`.

## Scope and evidence

- [x] **Body surfaces.** Refined normals on fourteen painted meshes, preserving
  panel positions and gaps. Strip-light comparisons informed the correction;
  blanket subdivision and Laplacian fairing were rejected. Evidence:
  `audit/detail-upgrade.json`, `renders/body-strips-normal-filter.png` and
  `renders/upgrade-carla-front.png`. Paint retains CARLA color control and a
  connected clear-coat graph, checked by `validate_surface_assets.py`.
- [x] **Optical assemblies.** Six curved reflector bowls, two refined outer
  reflectors, original flute normals and five thickened rear light guides.
  Removed the baked chrome photograph in the headlight diffuse texture. Rear
  emission stays red at night. Evidence: `audit/optics-candidate.json`,
  `renders/upgrade-carla-headlight.png`, `renders/upgrade-carla-rear-on.png`.
- [x] **Wheels and tires.** Modeled tread channels, sidewall ribs and embossed
  lettering; subtler rubber lettering, metallic rims/discs and independent
  calipers. Rim clear coat is present in the saved Unreal material. Evidence:
  `renders/upgrade-carla-steered.png`, `audit/surface-asset-validation.json`.
- [x] **Glass and seals.** Removed redundant Solidify shells from seven already
  closed glazing meshes. Added upper gaskets following the actual glass boundary;
  corrected a dangling lower segment found during the open-door review. Evidence:
  `audit/surface-upgrade.json`, `renders/upgrade-carla-glass.png`,
  `renders/upgrade-carla-door.png`.
- [x] **Fittings and gaps.** Twenty metallic fitting meshes have small manufactured
  bevels; repeater camera lenses sit inside their existing garnishes. Preserved
  panel separation and door articulation. Evidence: `audit/detail-upgrade.json`,
  `renders/detail-fittings.png` and the open-door CARLA capture.
- [x] **Interior materials and detail.** Separate leather, plastic and headliner
  grain/roughness, surface-fitted steering stitches, reflective mirror material,
  and a dark-glass emissive display. Evidence: `renders/detail-stitching.png`,
  `renders/upgrade-carla-cabin.png`, `renders/upgrade-carla-mirror.png`, and fresh
  texture/material checks in `audit/surface-asset-validation.json`.
- [x] **Wheel wells and underside.** Four arch liners with fasteners and a formed
  undertray with joints. Lowered/thinned the undertray after an overlap appeared
  in review. Evidence: `renders/detail-underbody.png`,
  `renders/upgrade-carla-steered.png` and the detail-generation report.
- [x] **CARLA integration.** Final motion/braking, steering/caliper and all-door
  checks passed; all five impact cases passed. The refreshed twelve-view capture
  exited cleanly. Evidence: `audit/final-upgrade-checks.log`, `audit/runtime.json`,
  `audit/collision-regression-final-upgrade.json`, `audit/upgrade-camera-review.json`.
  Saved physics/LOD checks passed in `audit/rear-channel-saved-checks.log`; final material checks passed in `audit/rear-final-materials.log`.

## Lighting controls

The roof lamp now uses Interior instead of High Beam. Upper white position strips
have independent materials instead of sharing Fog with the lower lamps. Visible
headlamp emission uses the maximum of low beam and twice high beam. Seven tagged
lights provide low/high/fog road illumination and interior illumination. The
superseded template low beams are hidden in game. Native code supports Interior
and `Carla.Light.<group>` visibility control through the saved-defaults path.

Ten isolated API states were captured from four fixed views; reported vehicle
states matched every request. A subsequent filtered capture verified the final
beam angles/intensities. High beam leaves the roof lamp off; Interior lights it.
Low beam illuminates the near road; high beam reaches farther; Fog has a separate
lower emitter. Evidence: `audit/light-state-camera-review.json`,
`audit/light-state-image-metrics.json`, `audit/beam-camera-review.log`, and
`renders/upgrade-carla-audit-*.png`. Native build: `audit/native-light-groups-build.log`.

The capture tool drains restored-weather callbacks before client teardown. It also
settles steering and avoids issuing requests for filtered-out light states.

## Limits

This is a photograph-guided art upgrade, not OEM CAD or measured lamp photometry.
The display remains static artwork, and mirrors use the renderer's reflections
rather than dedicated camera targets. There are 1,964,987 LOD0 triangles across
fifteen exported parts. Each wheel retains 324,782 triangles at every distance;
other parts have four LODs. Purpose-built wheel LODs and fleet-scale performance
optimization remain separate work. The EV dynamics are approximations; the impact
suite verifies the tested scenarios, not factory crash behavior.

## Rear-channel correction

The source had overlapping reverse/indicator faces. These now share one surface
per side. A second fault in the generated graph left Saturate and OneMinus inputs
disconnected: their Unreal pins are unnamed, not `Input`. The importer asserts
both connections, and saved-material validation checks them. The final amber
emission is (250, 45, 0.75) before the vehicle's intensity multiplier.

Both selected-side amber and opposite-side off were visually checked in
`audit/rear-accepted-camera.log`; white reverse on both sides was checked in
`audit/rear-graph-camera.log`. Saved graph checks passed in
`audit/rear-final-materials.log`. Earlier failed captures are diagnostic history,
not acceptance evidence. Final motion, twelve-view camera capture and all five collision cases passed and exited cleanly (`audit/final-upgrade-checks.log`).
