---
name: check-ue5-limitations
description: Tells you what CARLA on UE 5.5 (branch ue5-dev) cannot do that UE 5.8 (ue58-dev) can, so you do not attempt an unsupported workflow there — no Autoware integration, no ROS 2 middleware choice (FastDDS only), no DLSS or ray-traced-lens camera, no OFPA large-map mount so Town12/13/15 load black, and a few missing World/Actor methods. Also states what is identical, so the ue58 skills can be used directly for everything else. Use when working against a 5.5 checkout, when a ue58 skill's step has no counterpart, or when deciding whether a feature request is possible on 5.5 at all.
license: MIT
compatibility: Any Linux with bash. `list` needs nothing; `check` needs a ue5-dev checkout (CARLA_UE5_ROOT); `diff` additionally needs a ue58-dev one (CARLA_UE58_ROOT). VERIFIED by diffing real trees on 2026-08-28 — ue5-dev @ 0a5ce0d (2026-07-14) against ue58-dev @ 718efd7cc (2026-08-25). NOT verified at runtime: no 5.5 engine or build exists on this machine, so every claim here is a source/layout fact, never a measured behaviour.
metadata:
  group: ue5
  prerequisites: scripts/check_env.sh
  reference: references/gaps.md
---

# What UE 5.5 cannot do

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

**5.5 and 5.8 are the same CARLA line, not parallel products.**
`PythonAPI/carla/src/Sensor.cpp` is byte-identical between the branches,
`Actor.cpp` differs by 2 lines and `World.cpp` by 22. 5.8 is the later revision —
the one released as **CARLA 1.0** — so treat 5.5 as an earlier point release. It
still declares `0.10.0`, which is also what every ue58 build reported before that
release, so the version string does not separate the two: the branch does.

The practical consequence: **the `ue58` skills are the procedures for 5.5 too**,
minus the five gaps below. There is no ue5 mirror of each skill because there is
almost nothing to mirror.

## Instructions

```
Progress:
- [ ] Step 1: bash scripts/check_env.sh
- [ ] Step 2: list  — the gaps, and which ue58 skill each one breaks
- [ ] Step 3: check — confirm against the tree in front of you
- [ ] Step 4: then use the ue58 skill, minus the MISSING lines
```

```bash
source scripts/env.sh

bash scripts/gaps.sh list      # needs no checkout
bash scripts/gaps.sh check     # this tree's gap markers
bash scripts/gaps.sh diff      # measured, if a ue58 tree is also present
```

### The five gaps

| Gap | What is absent on 5.5 | What it breaks |
|---|---|---|
| **ROS 2 middleware** | `ros2/middleware/` (RMW abstraction, `QosProfile.h`, `ActiveMiddleware`) and the `CARLA_CYCLONEDDS_*` / `CARLA_ZENOH_C_*` options | FastDDS only — `--rmw cyclonedds` / `--rmw zenoh` do not exist ([[run-carla-ue58-server]]); [[add-ros-publisher]]'s `QosProfile.h` step does not apply |
| **Autoware** | both publishers, the control subscriber, `AutowareSteeringCompensation.h`, `sensor.other.autoware_gnss`, `sensor.other.vehicle_status` | [[run-autoware-ue58]] has **no 5.5 counterpart** |
| **DLSS + ray-traced lens** | `enable_dlss`, `dlss_screen_percentage`, `SceneCaptureCamera_RayTracedLens.cpp`, `CARLA_DLSS_SDK_PATH` | `sensor.camera.rt_lens` does not exist; [[create-sensor]]'s DLSS attributes are 5.8-only |
| **OFPA large-map mount** | `MountExternalPackageRoots` in `Carla.cpp` | **Town12 / Town13 / Town15 load with an empty World Partition and a black screen.** 5.8 absorbed this patch upstream; on 5.5 it is still a manual fix |
| **A few API additions** | `World.get_ego_spawn_points`, `World.set_publish_tf` / `get_publish_tf`, `World.spawn_custom_mesh`, `Actor.enable_constant_acceleration` / `disable_constant_acceleration` | those calls raise `AttributeError`; the rest of the python-api group applies unchanged |

### What is identical — every ue58 finding transfers

CMake-only build (no `Makefile`, `Util/Tools` not `Util/BuildTools`); all six
`package` targets; the `MapsToCook` default **including uncookable Town15**, so an
out-of-the-box package build fails on 5.5 too; the single-dash `-ros2` flag; the
actor factories and their `PropParameters.json` / `WalkerParameters.json` /
`VehicleParameters.json` registration; the entire CarlaTools header set.

The known defects are shared too, which means the ue58 notes apply verbatim:
`RecastBuilder` missing from `Util/DockerUtils/dist` (map-import navmesh silently
produces nothing), `Environment.sh` not setting `CARLA_BUILD_TOOLS_FOLDER` (so
`Import.sh` invokes `/Import.py` and exits 2), and the `#if 0 // @CARLAUE5` wheel
block that stops `GenerateNewVehicleBlueprint` producing a drivable vehicle.

## Examples

**Example 1: "run Autoware against my 5.5 build"**

`list` — gap 2. Not possible: none of the publishers, the subscriber or the two
sensors exist. Say so; do not adapt [[run-autoware-ue58]]'s steps, they have
nothing to bind to.

**Example 2: "start the server with cyclonedds"**

Gap 1. 5.5 has no middleware abstraction, so FastDDS is the only transport.
`-ros2` itself works.

**Example 3: "load Town12 and it's black"**

Gap 4, and it is the expected outcome on 5.5, not a broken install. The mount
patch is absent, so the World Partition comes up empty. Either apply the patch or
use a non-partitioned town.

**Example 4: "is this checkout 5.5 or 5.8?"**

`check`. It classifies by markers rather than by branch name — `ros2/middleware`,
the Autoware publishers, `MountExternalPackageRoots` — and reports the engine
branch the tree names for itself (`ue5-dev-carla` vs `ue58-dev-carla`).

**Example 5: "import a prop on 5.5"**

Nothing in gap 1-5 touches it: use [[import-carla-ue58-prop]] directly. Same for
walkers, packaging, and running a server.

## Troubleshooting

**Problem: `check` says the tree has 5.8 markers**
Cause: `CARLA_UE5_ROOT` points at a ue58-dev checkout.
Solution: use the ue58 skills directly — none of these gaps apply.

**Problem: a ue58 skill's step fails on 5.5 and it is not in the gap list**
Cause: the list covers *structural* differences found by diffing the trees; a
behavioural difference at runtime would not appear in it.
Solution: say the behaviour is unverified on 5.5 — no 5.5 build has ever been run
here — and investigate rather than assuming the gap list is exhaustive.

**Problem: `diff` refuses to run**
Cause: it needs both trees.
Solution: export `CARLA_UE58_ROOT` as well, or use `list` / `check`.

## Outputs

Nothing is written and nothing is started: all three modes are read-only.
`list` prints the gap catalogue, `check` reports which markers this tree has, and
`diff` prints a measured comparison of the ROS 2 layer, the CMake options, the
Python binding line counts and the declared versions.

Per-gap evidence — file paths, reference counts, and what each gap means for
which skill — is in [references/gaps.md](references/gaps.md).
