---
name: convert-geocoordinates
description: Converts between CARLA locations (X, Y, Z in metres) and geographic coordinates (latitude, longitude, altitude) on a running CARLA server, through the map's own OpenDRIVE projection or a PROJ string you give — single points, an actor's position, or a CSV of points, each checked by a round trip; reads the map's georeference and projection; and snaps a lat/lon to the nearest driving lane to tell a point on the road from one off the map. Use when the user asks "where is this lat/lon in CARLA", "what are the GPS coordinates of the ego", "convert these GNSS points to the map", "which projection does this map use", or a GNSS reading does not land where expected.
license: MIT
compatibility: Any OS with the CARLA PythonAPI installed for the active interpreter and a reachable, already-running CARLA server. Needs the geocoordinate API of CARLA 1.0 (ue58-dev); `--proj` needs the carla.GeoProjection* classes. Verified against ue58-dev ccc52d24d.
metadata:
  group: python-api
  prerequisites: scripts/check_env.sh
  reference: references/geocoordinates.md
---

# Convert geographic coordinates

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

`carla.Map` converts with the projection in the `<geoReference>` of the map's OpenDRIVE header. The GNSS sensor uses the same projection, so this skill's `to-geo` gives the GNSS reading for a location, without the sensor's noise.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Read the map's georeference (info)
- [ ] Step 3: Convert (to-geo / to-world), and check the round trip and --snap
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

### Step 2: Read the georeference

```bash
source scripts/env.sh
python3 scripts/geo.py info
```

It prints the georeference, the projection class and its parameters, and the raw `<geoReference>` string. The projection objects do not expose the map's ellipsoid, so read it from that string.

Most CARLA towns sit at latitude 0, longitude 0. Only these have a real-world origin: Town11, Town15 and Eixample (Barcelona), and Town13 (Ohio). Mine_01 is a few metres from 0,0.

### Step 3: Convert

```bash
python3 scripts/geo.py to-geo --at=10,-20,0.5                     # CARLA location -> lat/lon/alt
python3 scripts/geo.py to-geo --actor 137                         # an actor's current position
python3 scripts/geo.py to-world --at 41.5016,2.1038 --snap        # lat/lon -> location + nearest lane
python3 scripts/geo.py to-world --file points.csv --snap          # one LAT,LON[,ALT] per row
python3 scripts/geo.py to-geo --at=10,10,1 --proj '+proj=utm +zone=31 +ellps=WGS84'   # another projection
```

Use the `=` form for values that start with a minus sign. Output is CSV on stdout, so it can be redirected to a file.

Every row ends with `roundtrip_m`, the distance after converting back, which should be under a millimetre. With `--snap`, `to-world` adds the nearest driving lane and the distance to it, and warns on stderr when that distance exceeds `--max-offset` (50 m). A large distance means the point is off the map, or the coordinates came from another projection.

## Gotchas

- **Y points south.** CARLA's frame is left-handed: +X is east, +Y is south. A point at +Y has a lower latitude than the origin.
- **Altitude is Z.** CARLA applies no geoid, so `+geoidgrids` in the header is ignored and the altitude is the location's Z.
- **`--proj` uses CARLA's defaults, not PROJ's.** The script builds the projection the way CARLA parses a header: WGS84 when no ellipsoid is named, the ellipsoid parameters in the order `+a`, `+b`, `+f`, `+rf`. Building `carla.GeoEllipsoid()` by hand gives a sphere instead (see the reference).
- **Unsupported projections.** CARLA implements `tmerc`, `utm`, `merc` and `lcc` only; the script refuses anything else rather than fall back silently as the server does.

## Examples

**Example 1: where is this GPS fix on the map**

User says: "a GNSS log says 41.5016, 2.1038 — where is that on Town15?"

`to-world --at 41.5016,2.1038 --snap` gives the location and the lane it is on.

**Example 2: ego in lat/lon**

User says: "give me the ego's GPS coordinates"

Resolve the id with world-data (`actors --role hero`), then `to-geo --actor <id>`.

**Example 3: a route given as lat/lon**

User says: "convert these 200 waypoints from my CSV"

`to-world --file route.csv --snap > route_carla.csv`, then check that no row warned about its lane distance.

## Troubleshooting

**Problem: every point lands thousands of kilometres away**
Cause: the coordinates are real-world but the map's origin is 0,0 (most towns), or the reverse.
Solution: read `info`; convert with `--proj` for the projection the data came from.

**Problem: `roundtrip_m` is not near zero**
Cause: a projection parameter out of its valid range, such as a UTM zone far from the point.
Solution: check the PROJ string against `info`.

## Outputs

CSV on stdout. Nothing in the world changes.

Projection tables, the ellipsoid list and the measured checks are in [references/geocoordinates.md](references/geocoordinates.md).
