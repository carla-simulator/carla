# World data — detail

Detail layer for the `world-data` skill.

## actors: the resolver

`world.get_actors()` returns every actor; `.filter(pattern)` matches `type_id`
with wildcards. The skill then:

1. **filters by stable attribute** — `--filter` (type_id), `--role`
   (attributes['role_name']), `--color` (a vehicle's color attribute); hides
   traffic lights/signs/spectator unless `--all`.
2. **spatial predicate** — `--nearest` returns the single closest actor to
   `--near X,Y,Z` or `--near-id N`. This is a predicate, not an ordering.
3. **`--full`** shows every attribute so you can find a distinguishing field.

There is **no rank/order** among peer actors — do not index by position ("the
3rd"). The disambiguation contract for the whole toolkit is: narrow by attribute
(or take `--nearest`) until one remains, then use its **id**. It prints id,
type_id, role, color, location, speed (km/h) and distance (when a reference is
set) so a human or the agent can also eyeball it. `--limit` only truncates the
display; it is not a selection.

Related resolvers in sibling skills: **map-waypoints** `junctions --arms N`
(3-/4-way intersections, by distance to map centre) and lane waypoints;
**control-spectator**/**control-vehicle**/**telemetry** accept `--id` from here.

## snapshot

`world.get_snapshot()` → a `WorldSnapshot` with `.frame` and `.timestamp`
(`elapsed_seconds`, `delta_seconds`, `platform_timestamp`). Use it as the single
consistent time source; `snapshot.find(actor_id)` gives an `ActorSnapshot` with
frame-locked `get_transform/velocity/acceleration/angular_velocity` (the telemetry
skill relies on this).

## level bounding boxes

`world.get_level_bbs(CityObjectLabel)` returns the static/baked geometry's
bounding boxes for a label (e.g. every building or traffic sign) as
`carla.BoundingBox` (world `location`, `extent`, `rotation`). Labels are the same
`CityObjectLabel` set the toggle-env-objects skill lists. These are map geometry,
not actors — distinct from `actors` and from an actor's own `bounding_box`.

## raycast and ground projection

- `world.cast_ray(start, end)` → ordered list of `carla.LabelledPoint`
  (`.location`, `.label` = the `CityObjectLabel` of the surface hit) where the
  segment crosses geometry. Good for line-of-sight and "what's between A and B".
- `world.project_point(location, direction, search_distance)` → the first
  `LabelledPoint` hit from `location` along `direction` (this skill uses
  `(0,0,-1)` to drop to the ground), or `None`. Returns the surface point + label.

## Notes

- Distances are 3D straight-line metres; speed is the velocity magnitude.
- Large maps have many actors and huge level-bbox sets — use `--filter`/`--limit`.
- Actor ids are stable for the actor's lifetime but reused after a map reload;
  re-resolve after loading a new map.
