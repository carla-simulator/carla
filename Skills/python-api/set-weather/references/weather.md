# Weather — detail and natural-language mapping

Detail layer for the `set-weather` skill. The point of this file is to convert a
plain-language weather request into **exact** `WeatherParameters` — first by
matching a built-in preset, then by overriding fields from the vocabulary tables.

## Contents

- WeatherParameters fields
- Preset matrix (condition x time)
- NL → fields vocabulary
- Time of day → sun angle
- Conversion workflow
- Gotchas

## WeatherParameters fields

14 fields (0.9.16). "0-100" fields are percentages the skill clamps on `set`.

| Field | Range | Meaning |
|---|---|---|
| `cloudiness` | 0-100 | cloud cover; also gates whether rain looks right |
| `precipitation` | 0-100 | falling-rain intensity |
| `precipitation_deposits` | 0-100 | standing water / puddles on the ground |
| `wind_intensity` | 0-100 | wind; bends rain and moves clouds |
| `wetness` | 0-100 | surface/road wetness (mostly visible at night presets) |
| `fog_density` | 0-100 | fog thickness |
| `fog_distance` | m ≥ 0 | distance where fog starts; smaller = fog closer/denser |
| `fog_falloff` | ≥ 0 | fog density decay with height; higher = fog hugs the ground |
| `sun_altitude_angle` | -90..90 | sun height; <0 is below the horizon (night) |
| `sun_azimuth_angle` | 0..360 | sun compass direction (0 N, 90 E, 180 S, 270 W) |
| `scattering_intensity` | ≥ 0 | overall light scattering (advanced) |
| `mie_scattering_scale` | ≥ 0 | haze/aerosol scattering (advanced) |
| `rayleigh_scattering_scale` | ≥ 0 | sky-blue scattering (advanced) |
| `dust_storm` | 0-100 | dust-storm intensity |

## Field names in plain language

Each field is addressable on its own — a request that names a single quantity
("raise the sun", "more puddles", "how far you can see in the fog") maps to one
field via `set --<field>`, no preset involved. Everyday phrasings:

| Field | Plain-language phrasings |
|---|---|
| `sun_altitude_angle` | sun height/altitude/elevation, how high/low the sun is, how high the sun sits, raise/lower the sun, time of day |
| `sun_azimuth_angle` | sun direction/bearing, where the sun is, sun in the east/west, which way the sun faces |
| `cloudiness` | clouds, cloud cover, how overcast/grey the sky is |
| `precipitation` | rain, rainfall, how hard it's raining |
| `precipitation_deposits` | puddles, standing water, water on the road/ground, how wet the ground is |
| `wetness` | wet look/sheen, surface or road wetness, damp-looking |
| `wind_intensity` | wind, how windy, gusts, breeze, gale |
| `fog_density` | fog, mist, haze thickness, how foggy |
| `fog_distance` | how far you can see (in fog), visibility distance, where the fog starts |
| `fog_falloff` | how the fog hangs, fog hugging the ground, ground fog vs high fog |
| `dust_storm` | dust, sand/dust storm, dustiness |
| `scattering_intensity` | overall light scattering / haze glow (advanced) |
| `mie_scattering_scale` | aerosol/pollution haze (advanced) |
| `rayleigh_scattering_scale` | sky blueness (advanced) |

**Relative requests** ("raise the sun a bit", "make it windier", "less rain")
adjust one field up or down: `show` the current value first, then `set` the
field to the new number (the reference tables give sensible target values). A
bare `set --<field>` without `--base` changes only that field and leaves the rest.

## Preset matrix (condition x time)

CARLA ships 23 presets = {condition} x {Noon, Sunset, Night} (+ `DustStorm`,
`Default`). A standard NL request usually **is** one of these — apply it with
`preset` for exactly these values. Key fields (time sets `sun_altitude`: Noon 45,
Sunset 15, Night -90):

| Condition | cloud | precip | deposits | wind | wetness | fog_density |
|---|---|---|---|---|---|---|
| Clear | 5 | 0 | 0 | 10 | 0 | 2 (noon) / 60 (night) |
| Cloudy | 60 | 0 | 0 | 10 | 0 | 3 |
| Wet | 5 | 0 | 50 | 10 | 0/60* | 3 |
| WetCloudy | 60 | 0 | 50 | 10 | 0/60* | 3 |
| SoftRain | 20/60* | 30 | 50 | 30 | 0/60* | 3 |
| MidRainy | 60/80* | 60 | 60 | 60 | 0/80* | 3 |
| HardRain | 100 | 100 | 90 | 100 | 0/100* | 7 |
| DustStorm | 100 | 0 | 0 | 100 | 0 | 2 (+ dust_storm 100) |

`*` night variants carry higher `wetness` and `fog_density` (60) than their
noon/sunset counterparts. Full names: `Clear|Cloudy|Wet|WetCloudy|SoftRain` +
`Noon|Sunset|Night`, `MidRainyNoon|MidRainSunset|MidRainyNight`, `HardRain` +
`Noon|Sunset|Night`, `DustStorm`, `Default` (server default, all -1 = "unset").

## NL → fields vocabulary

When the request is not a plain preset (e.g. "foggy afternoon", "light rain,
windy"), start from the nearest preset and override these:

**Sky / clouds** (`cloudiness`): clear/sunny 5-15 · partly cloudy 30-40 ·
cloudy 60 · overcast/grey 85-100.

**Rain** (`precipitation`): none 0 · drizzle/light 20-30 · moderate/steady 50-60 ·
heavy/downpour/torrential 90-100. Pair with `precipitation_deposits` (ground
water): dry 0 · damp 30 · wet 50 · flooded 80-90 — puddles lag the rain, so a
"just stopped raining" scene is `precipitation` low but `deposits` high.

**Fog** (`fog_density` + `fog_distance` + `fog_falloff`): none 0 · light/misty
10-20 (distance ~60-75, falloff ~0.2) · moderate 40 (distance ~25) · heavy/thick
70-100 (distance ~8-10, falloff ~1.0, fog hugs ground).

**Wind** (`wind_intensity`): calm 0-10 · breezy 30 · windy 60 · strong/gale 100.

**Dust** (`dust_storm`): light 30 · heavy 100 (usually with `wind_intensity` high
and `cloudiness` high).

## Time of day → sun angle

`sun_altitude_angle` drives day/night; keep it consistent with the request.

| Phrase | sun_altitude_angle |
|---|---|
| solar noon / sun overhead | 90 |
| midday / daytime (CARLA "Noon" preset) | 45 |
| afternoon / morning | 30-45 |
| golden hour / low sun | 10 |
| sunset / sunrise / dusk / dawn | 0-15 (preset "Sunset" = 15) |
| twilight | -10 |
| evening | -20 |
| night | -60 to -90 (presets use -90) |
| midnight | -90 |

Direction, if asked (`sun_azimuth_angle`): sunrise ≈ 90 (E), sunset ≈ 270 (W).

### Putting the sun somewhere relative to the camera

The compass values above cannot answer "put the sun behind me" or "on my
right" on their own, because nothing here says how compass north relates to
CARLA's yaw. Measured on 0.10.0 (UE 5.8), against the spectator:

    sun_azimuth_angle = <camera yaw> + 180 + <offset>

with the offset in the same sense as yaw: `0` puts the sun down the view axis,
`+90` puts it to the camera's right, `-90` to its left, `180` behind. Verified
twice: a spectator at yaw 0 with azimuth 270 has the sun on its right, and a
camera at yaw 25 with azimuth 180 sees the sun ahead and slightly left.

A small offset rather than `0` is usually what is wanted for a shot: the sun
dead centre floods the frame with flare instead of lighting the subject.

## Conversion workflow

1. **Is it a standard condition x time?** → `preset <Name>`. ("heavy rain at
   sunset" → `HardRainSunset`; "clear night" → `ClearNight`.)
2. **Close to a preset but tweaked?** → `set --base <Name>` then override only the
   differing fields. ("hard rain at sunset but foggier" →
   `set --base HardRainSunset --fog-density 40`.)
3. **No close preset?** → `set` from the vocabulary tables, choosing the sun angle
   from the time-of-day table. ("misty grey afternoon, roads still wet" →
   `set --cloudiness 90 --fog-density 20 --fog-distance 60 --precipitation-deposits 50 --wetness 40 --sun-altitude-angle 40`.)
4. Always read the `VERIFY` block back and confirm the numbers match the words.

## Gotchas

- **`Default` is not a look.** Its fields are `-1` ("unset"), handing control back
  to the map's own default weather — not a neutral clear day.
- **Rain needs clouds.** High `precipitation` with low `cloudiness` looks wrong;
  the rain presets all raise cloudiness too. Match them.
- **Puddles lag rain.** `precipitation` (falling) and `precipitation_deposits`
  (standing) are independent — set both for a coherent wet scene.
- **Night ≠ dark preset only.** A "night" request is primarily
  `sun_altitude_angle` ≤ -60; the night presets also carry more fog and wetness.
- **Weather is per-world and transient.** A map load resets it; re-apply after
  loading (see the `load-map` skill). For gradual change over time, step the
  fields across successive `set` calls (CARLA's `dynamic_weather.py` pattern).
