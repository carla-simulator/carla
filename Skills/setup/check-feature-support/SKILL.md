---
name: check-feature-support
description: Answers "can CARLA do X here, and is there a vetted procedure for it" before you promise anything, and detects client/server/tree version skew — a support matrix over the whole collection (covered by a skill / works but undocumented / present but untested / broken or removed), an index of the features that are verified BROKEN on 0.10.0 (gbuffers crash the server, map layers do nothing, Landmark.waypoint is None, standalone asset packages are gone), and a probe that reports what this machine's build flags and running server actually offer. Use when the user asks whether CARLA supports something, when a feature has no skill, when a UE4 tutorial's steps do not exist, or before improvising a procedure.
license: MIT
compatibility: Any Linux with bash. NO CARLA required — `matrix` and `broken` answer offline; `probe` reports more when CARLA_UE58_ROOT/CARLA_UE5_ROOT/CARLA_UE4_ROOT/CARLA_TARGET is set, and more still against a running server with an importable `carla`. Every claim was measured or read from source on ue58-dev HEAD 718efd7cc (engine 5.8.0, CARLA 0.10.0) and the 0.9.16 UE4 tree; re-verify after upgrading either.
metadata:
  group: setup
  prerequisites: scripts/check_env.sh
  reference: references/matrix.md
---

# Is this feature usable, and is there a procedure for it?

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

The failure this prevents is specific: **an agent asked to use an uncovered CARLA
feature reconstructs a plausible procedure from UE4-era documentation, and it is
wrong.** Every row in this skill's `[broken]` list used to be a working, well
documented UE4 workflow. Reciting those steps on ue58-dev produces a silent
no-op, an empty result, or a crashed server.

So answer from this skill instead of from memory, and when the answer is "no
vetted procedure exists", say that and offer to investigate.

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh   (no blockers — it reports probe depth)
- [ ] Step 2: matrix — find the feature's status
- [ ] Step 3: broken — read this BEFORE promising anything on 0.10.0
- [ ] Step 4: probe — confirm against this machine's build and server
- [ ] Step 5: version — only if a measurement looks impossible (skew check)
```

```bash
source scripts/env.sh

bash scripts/support.sh matrix     # the table (works with no CARLA)
bash scripts/support.sh broken     # verified-broken index, with evidence
bash scripts/support.sh probe      # this machine: flags, tree, live server
bash scripts/support.sh version    # the identity tuple + skew warnings
```

### `version`: am I reasoning about the thing I am talking to?

"Which CARLA is this" has five answers that can disagree — installed client,
server, source tree, content branch, engine — and a disagreement is silently
wrong rather than loudly wrong. `version` prints all five plus the build flags,
and warns on the two skews that cause wasted debugging:

- **client ≠ server.** CARLA usually still connects, then misbehaves in ways that
  look like your own bug.
- **installed client older than the tree HEAD, or than a wheel built from it.**
  You read a source change that the client you are calling does not contain. This
  is not hypothetical: it happened while these skills were being written, and it
  is why one API was briefly reported as removed when it had only moved class.

Note what `version` deliberately does **not** claim: that matching versions mean
matching behaviour. `ENABLE_ROS2`, `ENABLE_RSS` and `ENABLE_OSM2ODR` are
compile-time, and the map-layer breakage is a property of the *content* branch —
two servers reporting `0.10.0` can differ completely. Use `matrix` for behaviour;
use `version` to establish that your measurements are even about the right
target.

### Engine lines, and the version number

UE 5.8 (`ue58-dev`) is the line that continues, and it is the one released as
**CARLA 1.0**. UE 5.5 (`ue5-dev`) is an *earlier revision of the same line*, not
a fork: `PythonAPI/carla/src/Sensor.cpp` is byte-identical between them. So the
verdicts below apply to both engines unless a row says otherwise, and the five
features 5.5 lacks are catalogued in [[check-ue5-limitations]].

**On the version string.** Every measurement here was taken on a pre-release
ue58 build, which reports `0.10.0` — that is the number `version` will print on
those, and the number 5.5 reports today. Read `0.10.0` below as naming *the UE5
line*: 1.0 renamed it, it did not change the behaviour underneath. Where a
verdict is genuinely engine-specific the row says so.

### The four verdicts, and what each obliges you to do

| Verdict | Meaning | What to do |
|---|---|---|
| `[skill]` | a vetted procedure exists here | invoke that skill; do not improvise |
| `[works]` | verified working, no skill | use the API directly, say it is unvetted |
| `[untested]` | in the build, never exercised here | say so, offer to investigate, **do not write steps** |
| `[broken]` | verified broken or removed on 0.10.0 | say so, name the alternative if one exists |

### What `probe` adds that the matrix cannot

Two features are gated by **compile-time** flags that no runtime check reveals,
and every `-D` option must be repeated on each re-configure — so the CMake cache
is the only honest source:

- `ENABLE_RSS` — `sensor.other.rss` appears in the blueprint library either way,
  and cannot function without it. On the tree measured here it is **OFF**.
- `ENABLE_OSM2ODR` — the OSM→OpenDRIVE half of digital twins. **OFF** here.
- `ENABLE_ROS2` — no native ROS 2 interface at all without it, and it cannot be
  switched on at runtime ([[run-autoware-ue58]], [[build-carla-ue58]]).

`probe` also reports tree-visible facts that decide whether a documented
procedure can work at all: whether `Co-Simulation/` exists (it does not on
ue58-dev), whether `RecastBuilder` sits in `Util/DockerUtils/dist` (it does not,
which is why map-import navmesh generation silently produces nothing), whether
the Autoware integration is shipped, and whether the multi-GPU sources are there.

## Examples

**Example 1: "grab the GBuffer normals from the camera"**

`broken` first. On 0.10.0 `listen_to_gbuffer()` asserts on
`Stream.has_value()` and takes the server down, while the whole API surface still
exists. Tell the user it crashes the simulator; offer
`sensor.camera.normals` instead, which is a real sensor.

**Example 2: "hide the buildings so I can see the road network"**

`matrix` shows map layers as `[broken]`. `enable_environment_objects` is the
mechanism that works — that is [[toggle-env-objects]], a `[skill]` row.

**Example 3: "set up SUMO co-simulation"**

`[broken]`: ue58-dev has no `Co-Simulation/` directory at all, while UE4 ships
Sumo, PTV-Vissim, Chrono and Carsim bridges. Do not port the UE4 instructions
blind — say the tooling is absent and ask whether they want it investigated.

**Example 4: "can I use the RSS sensor?"**

`probe`. The blueprint exists; if `ENABLE_RSS=OFF` the honest answer is "not in
this build — it needs a rebuild with `-DENABLE_RSS=ON`, and nobody here has
exercised it afterwards".

**Example 5: "add a vehicle to ue58"**

`[untested]` — **to be done; there is no skill for it in this version.** The
pipeline is known and gets as far as a vehicle that registers, spawns and reports
four wheels in `get_physics_control()`; it does not move, because the physics asset
does not survive an editor restart on the skeletal mesh. Deferred to a later
version rather than shipped half-working.

So: say it is not supported yet, offer to investigate, and do **not** adapt
[[import-carla-ue58-walker]]'s procedure. Walkers bind to a *shared* skeleton;
vehicles need a per-vehicle skeleton plus a persisted physics asset, which is the
one step still unsolved.

## Troubleshooting

**Problem: the matrix disagrees with the CARLA docs**
Cause: the public docs describe the UE4 tree.
Solution: trust `probe` and the evidence in
[references/matrix.md](references/matrix.md) — they were taken from the tree in
front of you. Docs are the weaker source here.

**Problem: `probe` reports nothing about build flags**
Cause: no root variable set, or the tree is unconfigured, or `CARLA_PRESET`
names a preset with no cache.
Solution: export `CARLA_UE58_ROOT` (or `CARLA_UE4_ROOT`), and set
`CARLA_PRESET` to a preset you have actually configured.

**Problem: a feature is missing from the matrix**
Cause: the matrix covers what has been examined; CARLA is larger than that.
Solution: say it is unexamined — which is not the same as unsupported — and
offer to check the tree. Do not extrapolate from a neighbouring row.

**Problem: a measurement contradicts the source you just read**
Cause: version skew — most often an installed client older than the tree.
Solution: `version`. On the machine these skills were written on it caught a
wheel built one day *after* the installed client, which is exactly how a moved
API gets misreported as a removed one.

**Problem: this skill is stale after an upgrade**
Cause: every verdict is stamped to `ue58-dev` HEAD `718efd7cc` / 0.10.0.
Solution: re-run the measurements in
[references/matrix.md](references/matrix.md); each row records how it was
established so it can be re-checked cheaply.

## Outputs

Nothing is written and nothing is started: all four modes are read-only.
`matrix` and `broken` print text; `probe` prints what this machine's variables,
CMake cache, tree layout and running server report; `version` prints the identity
tuple and exits non-zero for nothing — read its MISMATCH lines, they are advisory
but they invalidate measurements.

Per-feature evidence — file paths, line numbers, the measurements behind each
`[broken]` row, and what a future skill would have to verify to promote a row —
is in [references/matrix.md](references/matrix.md).
