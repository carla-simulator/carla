# Pedestrian navmesh — detail

Detail layer for the `debug-navmesh` skill.

## What the navmesh is

The navmesh is a precomputed walkable-surface graph pedestrians use to path-find.
It is built per map (a `.bin` alongside the map assets) during the map/import
pipeline, separate from the drivable road network the Waypoint API exposes. If it
is absent, `WalkerAIController.go_to_location(...)` has nothing to route on and
walkers stand still.

## The only programmatic handle

The Python API does not return navmesh geometry. The usable signal is:

- `world.get_random_location_from_navigation()` → a `carla.Location` sampled from
  the navmesh, or `None`/degenerate output when there is no navmesh.

So "visualise the navmesh" means "sample many valid points and draw them", and
"validate the navmesh" means "check that sampling yields many, spread-out points".
This skill does both. Related pedestrian controls live on the world:
`set_pedestrians_seed(n)` (reproducible sampling) and
`set_pedestrians_cross_factor(p)` (how often walkers cross roads).

## Reading the validation

- **0 valid points** → no navmesh. Hard failure for pedestrian navigation.
- **Few unique points** → a tiny or degenerate walkable area (bad import).
- **Broad coverage bounds** → healthy navmesh spanning the map's sidewalks.

Coverage bounds are the min/max of sampled x/y/z; the walkable span is their
extent in metres. Compare against the map size (map-waypoints `summary`) to judge
whether sidewalks across the whole town are walkable or only part of it.

## After a map import

The typical post-import check: load the map (load-map), run `validate`, and only
then spawn pedestrians. A PASS here isolates later "walkers don't move" problems
to the controller wiring rather than the map. Drawing (`sample`) needs a rendered
server; validation works headless.

## Relationship to other skills

- Drawing uses the same `world.debug` overlay as the debug-draw skill (headless
  `-nullrhi` shows nothing).
- Spawning/steering pedestrians and the `WalkerAIController` belong to the
  walker-spawning skill; this skill only inspects the surface they walk on.
