# Traffic lights — detail

Detail layer for the `control-traffic-lights` skill.

## The actors

Traffic lights are `traffic.traffic_light` actors — `world.get_actors().filter(
"traffic.traffic_light")`. Each is a `carla.TrafficLight`:

- **state**: `get_state()` / `set_state(TrafficLightState)`; states are `Green`,
  `Yellow`, `Red`, `Off`, `Unknown`.
- **freeze**: `freeze(bool)` per light, or `world.freeze_all_traffic_lights(bool)`
  for all — holds the current state indefinitely instead of cycling.
- **timing**: `set_green_time/set_yellow_time/set_red_time(seconds)` and the
  matching getters; `get_elapsed_time()` for the current phase.
- **groups**: lights at one intersection form a group —
  `get_group_traffic_lights()`, `reset_group()`; a group cycles so exactly one arm
  is green at a time. `get_pole_index()` and `get_opendrive_id()` identify them.
- **affected lanes**: `get_affected_lane_waypoints()`, `get_stop_waypoints()`.

`world.reset_all_traffic_lights()` restores normal cycling everywhere.

## Finding the right lights

- **all**: filter `traffic.traffic_light`.
- **by junction**: `world.get_traffic_lights_in_junction(junction_id)` — get the
  junction id from the map-waypoints skill (`junctions`).
- **from a waypoint**: `world.get_traffic_lights_from_waypoint(wp, distance)`.
- **nearest to a point**: min by distance (this skill's `--near`).

## set vs freeze

`set --state` changes the **current** state, but the light keeps cycling, so it
will move on after its phase — good for a momentary change. To **hold** a state,
`freeze on` (optionally `--state green` to force all green first, the classic
free-flow/"green wave" setup). `freeze off` or `reset` resumes cycling.

## Relationship to other skills

- **control-traffic** (Traffic Manager) decides whether autopilot *vehicles* obey
  or ignore lights (`ignore-lights`); this skill sets the *lights*. To guarantee
  free flow you can both `freeze on --state green` here and/or ignore lights there.
- Junction ids and locations come from **map-waypoints**; the ego reacts to lights
  when driven by the **navigate-to** agent.
