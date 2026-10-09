---
name: package-leaderboard-agent
description: Packages an agent for CARLA Leaderboard submission — builds the docker image with make_docker.sh (plain or ROS variant), checks the four required roots and the egg layout it assumes, sets the agent/track/routes environment inside the image, and verifies the image can import the stack and load the agent before submission. Use when the user asks to "build the leaderboard docker image", "submit to the leaderboard", "package my agent", or make_docker.sh fails.
license: MIT
compatibility: Linux with docker. Needs CARLA_ROOT, SCENARIO_RUNNER_ROOT, LEADERBOARD_ROOT and TEAM_CODE_ROOT set, plus CARLA_ROS_BRIDGE_ROOT for the ROS image. make_docker.sh requires eggs in $CARLA_ROOT/PythonAPI/carla/dist — a wheel-only CARLA tree fails.
metadata:
  group: leaderboard
  prerequisites: scripts/check_env.sh
  reference: references/submission.md
---

# Package an agent for submission

> **Paths.** `scripts/…` and `references/…` below are relative to the
> directory holding this SKILL.md. Your working directory is the user's
> project, not that directory, so prefix them with its absolute path or the
> command is not found.

The leaderboard evaluates a **docker image**, not a repository. `make_docker.sh`
assembles one from four trees plus your code:

```
$CARLA_ROOT/PythonAPI   ->  /workspace/CARLA/PythonAPI
$SCENARIO_RUNNER_ROOT   ->  /workspace/scenario_runner
$LEADERBOARD_ROOT       ->  /workspace/leaderboard
$TEAM_CODE_ROOT         ->  /workspace/team_code
```

and bakes `PYTHONPATH`, `TEAM_AGENT`, `CHALLENGE_TRACK_CODENAME` and the routes into
the image.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh) — the four roots and the egg layout
- [ ] Step 2: Confirm the agent runs locally first (run-leaderboard-evaluation)
- [ ] Step 3: Build the image
- [ ] Step 4: Verify the image imports the stack and loads the agent
- [ ] Step 5: Submit
```

### Step 1: The egg trap

`make_docker.sh` does this unconditionally:

```bash
cp -fr ${CARLA_ROOT}/PythonAPI .lbtmp
mv .lbtmp/PythonAPI/carla/dist/carla*-py2*.egg .lbtmp/PythonAPI/carla/dist/carla-leaderboard-py2.7.egg
mv .lbtmp/PythonAPI/carla/dist/carla*-py3*.egg .lbtmp/PythonAPI/carla/dist/carla-leaderboard-py3x.egg
```

so it **fails if `$CARLA_ROOT/PythonAPI/carla/dist/` has no `.egg` files** — which is
the case for a pip/wheel-only install. `check_env.sh` reports this before the build
wastes time. Get a CARLA tree that ships eggs (a release tarball or a source build),
or add the wheel to the image yourself.

The py2.7 rename is legacy; a modern release ships only a py3 egg, so the py2 `mv`
fails harmlessly and the script continues because it is not run under `set -e`.

### Step 3: Build

```bash
source scripts/env.sh

export TEAM_CODE_ROOT=~/team_code
bash scripts/package_agent.sh build                    # -> leaderboard-user
bash scripts/package_agent.sh build --ros noetic       # -> leaderboard-user:ros-noetic
```

The wrapper checks the roots, warns about the egg layout and about copying a huge
`TEAM_CODE_ROOT` (model weights inflate the image fast), then calls the repo's
`make_docker.sh`. Supported ROS distros are exactly `melodic`, `noetic`, `foxy` —
anything else prints the usage and exits.

Set the agent-specific values in the image by editing the block in
`scripts/Dockerfile.master`:

```dockerfile
ENV TEAM_AGENT ${TEAM_CODE_ROOT}/my_agent.py
# ENV TEAM_CONFIG ${TEAM_CODE_ROOT}/config.json
ENV CHALLENGE_TRACK_CODENAME SENSORS
```

### Step 4: Verify before submitting

```bash
bash scripts/package_agent.sh verify                   # imports + agent class inside the image
bash scripts/package_agent.sh shell                    # interactive poke around
```

`verify` runs the image and checks, inside it: `carla`, `agents`, `srunner` and
`leaderboard` all import; `TEAM_AGENT` exists and defines the class the evaluator
will derive from its filename; and the declared track is valid. That is the whole
class of failures that otherwise comes back as a rejected submission hours later.

A full local dry run is better still — same image, your own server:

```bash
bash scripts/package_agent.sh run --routes-subset 0
```

### Step 5: Submit

Submission is through the leaderboard website
(<https://leaderboard.carla.org/submit/>): push the image to a registry and provide
the tag. Read the current instructions there — the mechanics (registry, quotas,
allowed track) change between challenge rounds and are not in the repository.

## Examples

**Example 1: "build the docker image for my agent"**

`export TEAM_CODE_ROOT=~/team_code`, `check_env.sh`, edit the Dockerfile's
`TEAM_AGENT`, `build`, then `verify`.

**Example 2: "make_docker.sh fails on the mv of the egg"**

Your CARLA is wheel-only. Use a release tarball for `CARLA_ROOT`, or copy the
wheel into the image and install it in the Dockerfile instead of relying on the egg.

**Example 3: "my ROS agent needs the bridge"**

`export CARLA_ROS_BRIDGE_ROOT=~/ros-bridge` (cloned with
`--recurse-submodules -b leaderboard-2.1`), then `build --ros noetic`, which uses
`Dockerfile.ros` and copies the bridge into the image.

**Example 4: "the image builds but the submission is rejected"**

Almost always sensors or the agent class. `verify` catches both;
[[write-leaderboard-agent]] `validate` catches the sensor budget specifically.

## Troubleshooting

**Problem: `Error $CARLA_ROOT is empty. Set $CARLA_ROOT as an environment variable first.`**
Cause: `make_docker.sh` checks all four roots up front (and `CARLA_ROS_BRIDGE_ROOT`
when `-r` is given).
Solution: export them; `source scripts/env.sh` does the first three.

**Problem: `mv: cannot stat '.lbtmp/PythonAPI/carla/dist/carla*-py3*.egg'`**
Cause: no egg in the CARLA tree.
Solution: see Step 1.

**Problem: the image is tens of gigabytes**
Cause: `TEAM_CODE_ROOT` is copied wholesale — datasets, checkpoints, `.git`.
Solution: keep only what the agent needs at runtime; `check_env.sh` reports the
directory size before you build.

**Problem: `python3: can't open file '/workspace/leaderboard/leaderboard/leaderboard_evaluator.py'`**
Cause: `LEADERBOARD_ROOT` pointed somewhere that is not a leaderboard checkout, so
the copy landed wrong.
Solution: `check_env.sh` validates each root by looking for its marker file.

**Problem: imports work locally but not in the image**
Cause: the image's `PYTHONPATH` uses `carla-leaderboard-py3x.egg`, a *renamed* copy
of your egg. If your local setup used a wheel, the image is using a different client.
Solution: `verify`, and make the local and image clients the same.

**Problem: the ROS image builds but the stack never starts**
Cause: `ROS_DISTRO` mismatch with what your launch files expect, or the bridge was
cloned without submodules.
Solution: re-clone with `--recurse-submodules`; build with the matching distro.

## Outputs

A docker image (`leaderboard-user`, or `leaderboard-user:ros-<distro>`) containing
CARLA's PythonAPI, the paired scenario_runner and leaderboard, your team code and
the environment the evaluator expects. `verify` reports whether the image can
actually import the stack and load the agent.

Dockerfile contents, the environment it bakes in, and what the submission expects
are in [references/submission.md](references/submission.md).
