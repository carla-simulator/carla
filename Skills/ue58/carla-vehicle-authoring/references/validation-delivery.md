# Acceptance and delivery

## Choose checks by changed behavior

A full new vehicle needs asset, visual, runtime, and collision coverage. A narrow material correction needs relevant saved-material and camera checks; broaden testing when shared code, physics, or new evidence warrants it.

Record the source hash, code/content revisions, engine build, map, simulation timestep/substeps, camera profile, and tested asset identity. Preserve readable logs and representative images. Separate earlier failed experiments from final acceptance evidence.

## Saved assets

In a fresh Unreal process, verify scale/bounds, part and material assignments, intended physics asset, collision shapes, wheel anchors, LOD configuration, texture color spaces, and any critical shader connections. Inspect the final imported version rather than an earlier in-memory version. A commandlet exit code alone cannot prove visual quality.

## Runtime

Use a dedicated server with discovered/configurable ports. Track and destroy only test-owned actors and restore altered world settings. Stop sensor listeners before destroying sensors; account for in-flight callbacks before client teardown. The Model 3 tools explicitly drain restored-weather callbacks because teardown otherwise crashed after successful metrics. Inspect current implementations rather than assuming an arbitrary sleep fixes it.

For a complete vehicle, verify:

- Catalog discovery, spawn, settle, ground contact, body/sensor bounds, and semantic classification.
- Acceleration, braking, steering, wheel spin, suspension, caliper alignment without spin, and moving clearance.
- **Every spawn point, not one.** In synchronous mode, spawn the vehicle at each of the map's spawn points (Town10HD_Opt has 155) with a `sensor.other.collision` attached. Let it settle for 20 ticks, apply throttle 0.3 for 60 ticks, and record the distance moved and any `static.road` events. A healthy car moves at every point and reports no road contact. Run a stock car (Lincoln MKZ) as the baseline. One or two hand-picked spawn points missed the Model 3's sweep problem, which showed at 10 of 155.
- **A fleet under Traffic Manager.** Spawn about 40 of the vehicle on autopilot for 60 s, several seeds, and count `static.road` collision events, stuck vehicles (under 20 m travelled) and top speed against the same run with a stock car. Single-vehicle braking tests did not reproduce the chassis scrape the fleet run found; it needed TM braking combined with body roll at road seams.
- Every supported door separately and together, including close and actor destruction.
- API paint variation across all body panels.
- Isolated lamp states plus relevant combinations, from both rear sides and front. Confirm state round-trips **and** visible pixels; test road illumination separately from glowing lenses. Check temporal behavior where indicators blink.

When shared native behavior changes, include a small representative existing-vehicle regression.

For collision changes/new integration, perform controlled rear, side, and fixed-obstacle impacts at representative speeds and 30/60 Hz with valid substepping. Confirm contact events and record peak speed, upward displacement, and final state. Judge unexplained energy gain against setup-specific expectations; the reference Model 3's 0.043 m maximum rise is historical evidence, not a universal threshold or crash-model certification.

## Visual and performance evidence

Capture matched overview and detail views with repeatable camera transforms, resolution, weather, and exposure. Inspect images directly. Include moving/steered wheels and LOD transitions; one static beauty shot misses the original wheel failure. Use default sensor grading as well as optional neutral review grading when material judgments depend on exposure/color treatment.

Test representative fleet density if broad traffic use is part of the request. Record triangle/material/texture cost and observed rendering performance. A high-detail hero vehicle with unreduced wheels is not automatically suitable for a dense fleet.

## Deliverables and repository pairing

Keep the editable, unposed Blender master with the vehicle's content assets where repository conventions permit. Include exported/integrated assets, provenance/license attribution, reproducible authoring code, and concise validation/limitations. Reference images used only for inspection need not be distributed. The Model 3 example stores its master at content `Static/Car/4Wheeled/TeslaModel3UE58/Source/TeslaModel3.blend`.

Before authorized commit/push, inspect status in both code and nested content repositories; stage only task changes. Check Git LFS rules and tracked payloads, including text files if the content repo applies LFS broadly. Pair code and content revisions in documentation. Verify remote tips and LFS upload success after pushing. Do not include unrelated captures/videos or large working artifacts merely because they exist locally.

Report what is actually demonstrated: reference-adjusted versus CAD-verified geometry, approximate versus measured dynamics/photometry, articulated versus static closures, static screens versus functional displays, and any unresolved fleet/LOD costs. Never label a vehicle production-ready solely because the import or runtime script passed.
