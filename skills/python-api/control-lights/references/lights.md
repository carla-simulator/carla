# Light sources — detail

Detail layer for the `control-lights` skill.

## LightManager

`world.get_lightmanager()` → `carla.LightManager`, the handle for every light
source baked into the map. Key calls (most take a list of `carla.Light`):

- `get_all_lights(LightGroup)` → the lights in a group (`LightGroup.NONE` = all).
- `turn_on(lights)` / `turn_off(lights)`.
- `set_color(lights, carla.Color)` / `set_colors(lights, colors)`.
- `set_intensity(lights, float)` / `set_intensities(...)`.
- `set_active(lights, bools)`, `set_light_group(...)`, `set_light_state(...)`.
- `set_day_night_cycle(bool)` — when on, the sim switches lights with the sun.

Each `carla.Light` also self-controls: `turn_on/off`, `set_color`, `set_intensity`,
`is_on`, `color`, `intensity`, `location`, `light_group`.

## Light groups

`carla.LightGroup`: `Street` (street lamps), `Building` (window/facade lights),
`Vehicle` (car head/tail lights as light sources), `Other`, and `NONE` which this
skill exposes as `all`. Group availability varies by map — some towns have rich
building lights, others almost none.

## What this is NOT

- **toggle-env-objects** hides the lamp-post *mesh* (geometry + collision); this
  toggles the *light it emits*. You can have a visible lamp that is off, or light
  with the post hidden.
- **set-weather** moves the *sun* (`sun_altitude_angle`) — global daylight. Street
  lights matter at night (negative sun altitude).
- **control-vehicle `lights`** sets one vehicle's light *state* flags; the Vehicle
  light *group* here is the emitted illumination side.

## Units and tips

Colour is `carla.Color(r, g, b)` 0-255. Intensity is an emissive value — street
lamps read well around 1000-3000; higher blows out, lower is dim. For a night
scene: set the sun below the horizon (set-weather), then `on --group street` and
`on --group building`. `day-night on` automates this against the sun position.
Effects require a rendered server (headless `-nullrhi` shows nothing).
