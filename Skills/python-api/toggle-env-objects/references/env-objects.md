# Environment objects — detail

Detail layer for the `toggle-env-objects` skill.

## API

- `world.get_environment_objects(object_type=CityObjectLabel.Any)` → list of
  `carla.EnvironmentObject`, each with `id`, `name` (asset instance name),
  `transform`, `bounding_box`, `type` (its `CityObjectLabel`).
- `world.enable_environment_objects(ids, enable)` → show (`True`) or hide
  (`False`) the objects whose ids are in the iterable. Affects **rendering and
  collision** together. Returns nothing.

These are static, map-baked assets, distinct from spawned actors
(`world.get_actors()`), traffic (traffic lights/signs as actors), and debug
overlays. Toggling is per-world state and is rebuilt on a map load/reload.

## CityObjectLabel taxonomy

Valid labels (0.9.16): `Any`, `NONE`, `Buildings`, `Vegetation`, `Poles`,
`Fences`, `Walls`, `TrafficSigns`, `TrafficLight`, `GuardRail`, `Bridge`,
`RailTrack`, `RoadLines`, `Roads`, `Sidewalks`, `Ground`, `Terrain`, `Water`,
`Sky`, `Static`, `Dynamic`, `Other`, plus vehicle/person semantic classes
(`Car`, `Truck`, `Bus`, `Motorcycle`, `Bicycle`, `Rider`, `Train`,
`Pedestrians`).

Natural-language → label:

| Request | Label |
|---|---|
| buildings / houses | `Buildings` |
| trees / bushes / plants / greenery | `Vegetation` |
| poles / lamp posts / street lights | `Poles` |
| traffic signs | `TrafficSigns` |
| fences | `Fences` |
| walls | `Walls` |
| guard rails / crash barriers | `GuardRail` |
| bridges | `Bridge` |
| sidewalks / pavement | `Sidewalks` |
| road markings / lane lines | `RoadLines` |

The vehicle/person labels apply to *baked* props of that class, not to actors you
spawn. If an expected asset is missing from a label, it may be tagged `Static` or
`Other` — use `list` (no label) for the whole-map by-type summary and re-target.

## Name filtering

`--name` is a case-insensitive substring match on `EnvironmentObject.name` (the
asset instance name, e.g. `BP_StreetLight_3`). Use it to toggle a subset within a
label — e.g. only the lamp poles among all `Poles`.

## Behaviour notes

- **Rendering + collision.** A disabled building is both invisible and
  non-colliding — vehicles/sensors pass through. Useful for clean captures or to
  open sightlines.
- **Whole-set operations.** You pass a set of ids; disabling a label hides every
  object of that label unless you narrow with `--name`/`--limit`.
- **No read-back.** `enable_environment_objects` returns nothing and there is no
  per-object "enabled" flag to query, so verify visually on a rendered server; the
  skill reports the count acted on.
- **Resets on reload.** Re-apply after a `load-map` (the world is rebuilt).
