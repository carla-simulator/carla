---
name: set-weather
description: Reads and sets the weather on a running CARLA server via get_weather/set_weather, and turns natural-language weather requests ("heavy rain at sunset", "light fog at night", "clear noon", "dust storm") into exact WeatherParameters — cloudiness, precipitation, puddles, wind, fog, sun position, wetness, dust. Use when the user asks to "set/change the weather", "make it rainy/foggy/sunny/night", "apply a weather preset", or describes any sky/lighting condition.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Does NOT need UE4_ROOT or a source checkout. Tested against CARLA 0.9.16.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/weather.md
---

# Set CARLA weather

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

Set the weather on a **running** server from a client, and read it back to
confirm. The deliverable is verified server state: `set_weather` returns nothing,
so every change is re-read into a `VERIFY` block.

The job that matters here is **turning words into the right numbers.** CARLA has
14 weather fields and 23 tuned presets; a plain request maps to exact values, not
a guess. The mapping is authoritative and lives in
[references/weather.md](references/weather.md) — read it before composing a
`set`. The rule of thumb:

- A request naming **one quantity** ("raise the sun", "how high the sun sits",
  "more puddles", "make it windier", "thicker fog") maps to a **single field** →
  `set --<field>`. The plain-language→field table in the reference names the
  everyday phrasings for each of the 14 fields. Relative wording ("a bit higher",
  "less rain") → `show` the current value, then `set` the field to the new one.
- A standard **condition × time** ("heavy rain at sunset") **is a preset** →
  apply it directly.
- A tweak on a standard look → start from that preset and override the few
  differing fields (`set --base <preset> ...`).
- Anything else → compose fields from the reference's vocabulary tables, picking
  the sun angle from its time-of-day table.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh), clear FAILs
- [ ] Step 2: Map the request -> preset or field values (references/weather.md)
- [ ] Step 3: Apply (preset / set --base / set)
- [ ] Step 4: Verify the VERIFY block matches the words
```

Commands need `CARLA_HOST`/`CARLA_PORT` from `scripts/env.sh` (defaults
`127.0.0.1:2000`). Prefix with `source scripts/env.sh` or export them.

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

FAILs only on a missing `carla` module or an unreachable server.

### Step 2: Map the words to parameters

Open [references/weather.md](references/weather.md). Decide whether the request
is a preset, a preset-plus-tweak, or a from-scratch composition. Do **not**
invent field values — use the tables so the result is reproducible.

### Step 3: Apply

```bash
source scripts/env.sh

# see what maps to what
python3 scripts/weather.py show
python3 scripts/weather.py list-presets

# a standard look = a preset
python3 scripts/weather.py preset HardRainSunset
python3 scripts/weather.py preset ClearNight

# a preset with a tweak (foggier hard rain at sunset)
python3 scripts/weather.py set --base HardRainSunset --fog-density 40

# a from-scratch composition (misty grey afternoon, roads still wet)
python3 scripts/weather.py set \
    --cloudiness 90 --fog-density 20 --fog-distance 60 \
    --precipitation-deposits 50 --wetness 40 --sun-altitude-angle 40
```

`set` takes a flag for every field (`--cloudiness`, `--precipitation`,
`--precipitation-deposits`, `--wind-intensity`, `--fog-density`, `--fog-distance`,
`--fog-falloff`, `--wetness`, `--sun-altitude-angle`, `--sun-azimuth-angle`,
`--dust-storm`, and the three scattering scales). 0-100 fields are clamped.
Without `--base` it overrides the **current** weather, leaving unspecified fields
untouched.

### Step 4: Verify

The `VERIFY` block prints all 14 fields. Confirm they match the request: e.g.
"heavy rain at sunset" ⇒ `precipitation = 100`, `precipitation_deposits = 90`,
`cloudiness = 100`, `sun_altitude_angle = 15`.

## Examples

**Example 1: a preset request**

User says: "make it rain hard at sunset"

`python3 scripts/weather.py preset HardRainSunset`. VERIFY: precipitation 100,
deposits 90, cloudiness 100, sun_altitude 15. Done.

**Example 1b: a single-field request**

User says: "the sun's too low, raise it higher"

`show` (say `sun_altitude_angle = 15`), then `set --sun-altitude-angle 60`.
Only the sun height changes; clouds/rain/fog stay as they were.

**Example 2: night + fog, not an exact preset**

User says: "foggy night, calm"

Nearest is `ClearNight` (already fog 60 at night); if they want thicker fog:
`set --base ClearNight --fog-density 90 --fog-distance 10 --wind-intensity 5`.
VERIFY: sun_altitude -90, fog_density 90.

**Example 3: from-scratch daytime scene**

User says: "overcast afternoon just after rain — wet roads, no rain falling"

`set --cloudiness 90 --precipitation 0 --precipitation-deposits 70 --wetness 50
--sun-altitude-angle 40`. Puddles high, rain zero — the "just stopped" look.

**Example 4: keep weather across a map change**

User says: "set hard rain, then switch to Town02"

`preset HardRainNoon`, then the [`load-map`](../load-map/SKILL.md) skill — weather
resets on load, so re-apply the preset after loading.

## Troubleshooting

**Problem: I set rain but it looks wrong / no clouds**
Cause: high `precipitation` with low `cloudiness`.
Solution: raise cloudiness too (presets do — `HardRain*` sets 100).

**Problem: "wet" scene but roads look dry**
Cause: only `precipitation` set; standing water is `precipitation_deposits`.
Solution: set `precipitation-deposits` (and `wetness`) as well.

**Problem: `preset Default` didn't give a clear day**
Cause: `Default` fields are `-1` = "unset" — it defers to the map's own default.
Solution: use `ClearNoon` (or the intended named preset) for a defined look.

**Problem: weather reset after loading a map**
Cause: weather is per-world; a load resets it.
Solution: re-apply after `load-map` (re-run the preset/set).

## Outputs

Server state, not a file: the running world now renders the requested weather.
The `VERIFY` block is the record of the applied parameters.

The field semantics, the full preset matrix, and the natural-language vocabulary
tables are in [references/weather.md](references/weather.md).
