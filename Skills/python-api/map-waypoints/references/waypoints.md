# Map / waypoints — detail

Detail layer for the `map-waypoints` skill: the API, how the rundown is computed,
and how to turn spatial language into a specific element.

## Contents

- The API surface
- How `summary` is computed
- Junctions and arm counting
- Turning language into an element
- Waypoint navigation
- Gotchas

## The API surface

`world.get_map()` → `carla.Map`:

- `get_topology()` → list of `(waypoint_a, waypoint_b)` pairs: the directed road
  graph, one pair per connected lane segment (segment endpoints, not dense).
- `generate_waypoints(distance)` → waypoints every `distance` m along every
  driving lane; the basis for map-wide stats.
- `get_waypoint(location, project_to_road=True, lane_type=Driving)` → nearest
  waypoint (projected onto a road by default).
- `get_waypoint_xodr(road_id, lane_id, s)` → the waypoint at an OpenDRIVE address.
- `get_crosswalks()`, `get_spawn_points()`, `get_all_landmarks()`.

`carla.Waypoint`: `road_id`, `section_id`, `lane_id`, `s`, `transform`,
`is_junction`, `junction_id`, `get_junction()`, `lane_type`, `lane_width`,
`lane_change`, `left_lane_marking`/`right_lane_marking`, `get_left_lane()`/
`get_right_lane()`, and navigation `next(d)` / `previous(d)` /
`next_until_lane_end(d)` / `previous_until_lane_start(d)` (all return lists — a
fork returns several).

`carla.Junction`: `id`, `bounding_box` (centre `Location` + half-`extent`),
`get_waypoints(lane_type)` → list of `(entry_wp, exit_wp)` pairs, one per path
through the junction.

## How `summary` is computed

From `generate_waypoints(STAT_STEP=3.0)`:

- **roads** = distinct `road_id`; **lanes** = distinct `(road_id, lane_id)`.
- **dominant lane count** = the modal lanes-per-road.
- **lane length** ≈ `count * STAT_STEP` (one sample per lane per step).
- **extent / area** = min/max of sampled x,y.
- **junctions** from the topology (deduped by `junction_id`), with an **arm
  histogram** (four-way / three-way / other).
- **density** = junctions per km² → "dense, city-like" / "moderate" / "sparse".

The RUNDOWN line assembles these into one paragraph you can relay directly.

## Junctions and arm counting

Arms = the number of distinct `road_id`s among a junction's **entry** waypoints
(`get_waypoints(Driving)`). A clean crossroads → 4, a T-junction → 3. Ramps,
merges and split lanes can nudge the count, so treat it as approximate and
confirm with `--draw`.

## Turning language into an element

`junctions` prints, per junction: `id`, `arms`, `centre (x,y)`, `size`,
`distance from map centre`, and an 8-way `bearing`. Map centre is the midpoint of
the map extent. Resolve phrases by filtering this list:

| Phrase | Filter |
|---|---|
| "the 4-way junction" | `arms == 4` |
| "in the middle / central" | smallest distance-from-centre |
| "the northern / eastern one" | bearing `N` / `E` (map XY frame) |
| "the biggest junction" | largest `size` |
| "near (x, y)" | smallest distance to that point (`waypoint --at`) |

**Bearing is in the map's XY frame, not true compass** — CARLA uses a
left-handed frame and towns are not aligned to north. Use bearings for relative
placement ("the one to the east side"), and prefer distance-from-centre for
"middle". When in doubt, `--draw` and confirm visually.

## Waypoint navigation

`next(d)` / `previous(d)` step along the lane by `d` metres and return a **list**
(more than one at a fork/junction); `navigate` takes the first branch each step.
`get_left_lane()` / `get_right_lane()` cross to adjacent lanes (respecting
`lane_change` and lane-type changes — the neighbour may be a sidewalk or shoulder).
`next_until_lane_end(d)` / `previous_until_lane_start(d)` walk to the lane's end.

## Gotchas

- **Topology is sparse.** `get_topology()` gives segment endpoints, not a dense
  centre-line; densify with `generate_waypoints` or `next()`.
- **Large maps are heavy.** Town11/12/13 produce very many waypoints; `summary`
  still runs but takes longer. Raise `STAT_STEP` for speed.
- **Drawing needs rendering.** Overlays are invisible on a headless `-nullrhi`
  server (see debug-draw).
- **`lane_id` sign** encodes side/direction (negative vs positive around the road
  reference line); pair with `road_id` and `s` for a unique address.
