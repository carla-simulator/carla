# Geometry and material review

## Reference and proportions

Use the selected year/trim consistently; nearby model years can have different bumpers, lamps, wheels, and interiors. Compare matched views against references, not memory. Resolve overall length, height, wheelbase, track, wheel diameter, and overhangs before small details. Record whether dimensions include mirrors. Treat photo-derived geometry as an approximation, not OEM CAD.

Inspect reflections with broad light sources at grazing angles. Pinched highlights can come from topology, duplicated shells, normals, or transforms; subdivision alone does not identify the cause. Preserve good custom normals and avoid destructive blanket remeshing. Use bevels where real edges catch light, at plausible physical scale.

## Wheels and tires

Separate rotating tire/rim/rotor geometry from calipers, which should follow suspension and front-wheel steering without spinning. Check both sides for mirrored orientation, wheel pivots, radius, axle position, and clearance at full steering and suspension travel. Export straight wheels; posed review copies can bake a steering error into the asset.

Model silhouette-critical sidewall curvature, shoulder, tread channels, bead/rim transition, and appropriate valve/fastener detail. Use normal/roughness maps for finer rubber grain and small wear rather than spending geometry everywhere. Keep embossed lettering shallow and fitted to the sidewall; flat or overly deep text looks attached to the tire.

**Whole-wheel percentage decimation is dangerous.** A dense spoke mesh and sparse tire mesh have different needs. In the Model 3, decimation destroyed the tire's round silhouette while preserving excessive spoke detail, producing tank-like/pointed wheels. Build wheel LODs with protected tire rings, separate component budgets, and silhouette review at every transition. Retaining LOD0 can diagnose/fix the immediate artifact but is a documented performance compromise, not a fleet-ready endpoint. The reference Model 3 retained 324,782 triangles per wheel; do not copy that as a target budget.

## Surfaces and small construction details

- Paint: clean normals, appropriate base color, physically plausible roughness and clear coat. Verify CARLA color overrides affect every intended panel.
- Metal: replace photographed/baked reflections with neutral physical material inputs. Inspect rims, brake discs, chrome, and reflector bowls under changed lighting.
- Glass: inspect thickness, normals, transparency, and overlapping shells. Redundant solidify modifiers on already closed glazing can produce doubled surfaces. Preserve legitimate thickness rather than universally deleting shells.
- Lamps: model lens depth, reflector bowls, guides, and partitions. Check daytime appearance and nighttime emission separately.
- Fittings: panel gaps, handles, seals, camera housings, mirror edges, and trim need coherent scale and contact. Fit seals to actual boundaries; avoid dangling lower segments from indiscriminate edge extraction.
- Cabin: differentiate plastic, upholstery, headliner, metal, glass, and display surfaces. Fit stitching to the surface; avoid oversized threads. Keep displays dark when appropriate and explicit about static artwork versus simulated displays.
- Underside: add visible arch liners and undertray where exposed by camera/steering, checking intersections and apparent floating surfaces.

Blender/OpenGL normal maps commonly require green-channel inversion for Unreal. Verify the convention with a lit sample; import normals as non-sRGB normal maps and roughness as linear data. Use original or appropriately licensed texture content.

Review front/rear three-quarter, true side, low wheel, steered wheel, cabin, door-open, and lamp close-ups as relevant. Compare default CARLA camera grading with a neutral review profile when grading might hide a material defect. Do not silently alter global grading to make one asset look better.
