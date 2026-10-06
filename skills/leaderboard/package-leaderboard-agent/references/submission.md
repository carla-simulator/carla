# Submission image: what make_docker.sh builds

## The script

`$LEADERBOARD_ROOT/scripts/make_docker.sh`:

1. refuses to run unless `CARLA_ROOT`, `SCENARIO_RUNNER_ROOT`, `LEADERBOARD_ROOT`
   and `TEAM_CODE_ROOT` are set (plus `CARLA_ROS_BRIDGE_ROOT` with `-r`),
2. makes `.lbtmp/` and copies each tree into it,
3. **renames the eggs**:
   `carla*-py3*.egg` → `carla-leaderboard-py3x.egg` (and a legacy py2.7 rename),
4. deletes `.git` from the scenario_runner and leaderboard copies,
5. copies `scripts/agent_entrypoint.sh`,
6. `docker build -t leaderboard-user -f scripts/Dockerfile.master .lbtmp`
   (or `-t leaderboard-user:ros-$DISTRO -f scripts/Dockerfile.ros` with
   `--build-arg ROS_DISTRO=`),
7. removes `.lbtmp/`.

Accepted ROS distros: `melodic`, `noetic`, `foxy`. Anything else prints usage.

The egg rename is the fragile step: with no `.egg` in
`$CARLA_ROOT/PythonAPI/carla/dist/` the `mv` fails, the script keeps going (no
`set -e` around it), and the image ends up with a `PYTHONPATH` entry pointing at a
file that does not exist.

## Dockerfile.master

```dockerfile
FROM nvidia/cuda:11.7.1-cudnn8-devel-ubuntu20.04
# locales, build-essential, cmake, git, python3-dev, python3-pip
# miniconda + a `python37` env (numpy networkx scipy six requests)

ENV CARLA_ROOT           "/workspace/CARLA"
ENV SCENARIO_RUNNER_ROOT "/workspace/scenario_runner"
ENV LEADERBOARD_ROOT     "/workspace/leaderboard"
ENV TEAM_CODE_ROOT       "/workspace/team_code"

COPY PythonAPI      ${CARLA_ROOT}/PythonAPI
COPY scenario_runner ${SCENARIO_RUNNER_ROOT}
COPY leaderboard    ${LEADERBOARD_ROOT}
COPY team_code      ${TEAM_CODE_ROOT}

RUN pip3 install -r ${SCENARIO_RUNNER_ROOT}/requirements.txt
RUN pip3 install -r ${LEADERBOARD_ROOT}/requirements.txt

ENV PYTHONPATH "${CARLA_ROOT}/PythonAPI/carla/dist/carla-leaderboard-py3x.egg":\
"${SCENARIO_RUNNER_ROOT}":"${CARLA_ROOT}/PythonAPI/carla":"${LEADERBOARD_ROOT}":\
"${TEAM_CODE_ROOT}":${PYTHONPATH}

#### AGENT SPECIFIC — edit these ####
ENV TEAM_AGENT ${TEAM_CODE_ROOT}/npc_agent.py
# ENV TEAM_CONFIG ${TEAM_CODE_ROOT}/YOUR_CONFIG_FILE
ENV CHALLENGE_TRACK_CODENAME SENSORS
#####################################

ENV ROUTES ${LEADERBOARD_ROOT}/data/routes_training.xml
ENV REPETITIONS 1
ENV CHECKPOINT_ENDPOINT /workspace/results/results.json
ENV DEBUG_CHALLENGE 0
CMD ["/bin/bash"]
```

Worth knowing:

- The three lines between the AGENT SPECIFIC markers are the ones you must edit.
  Nothing in `make_docker.sh` reads your `TEAM_AGENT` env var — it is baked from the
  Dockerfile.
- **The conda `python37` env is created but never used** for the requirements: the
  `pip3 install`s go to the system Python 3.8. The conda env is on `PATH` ahead of
  it, which is a latent inconsistency — if you rely on a specific interpreter, set
  it explicitly in your own layer.
- `PYTHONPATH` includes `TEAM_CODE_ROOT`, so your agent can import its own package
  by name.
- The base image is CUDA 11.7 / Ubuntu 20.04. A newer PyTorch may want a newer
  CUDA; change the `FROM` if so.
- `agent_entrypoint.sh` sources `${HOME}/agent_sources.sh` and then `exec "$@"`.
  Create that file in your own layer if your stack needs sourcing (a ROS overlay,
  a conda activate).

## Dockerfile.ros

Same shape plus a ROS distro layer and `carla_ros_bridge` copied to
`/workspace/carla_ros_bridge`, built with `--build-arg ROS_DISTRO`. Clone the bridge
with submodules and the matching branch:

```bash
git clone --recurse-submodules -b leaderboard-2.1 --single-branch \
    https://github.com/carla-simulator/ros-bridge
```

## Image hygiene

`TEAM_CODE_ROOT` is copied wholesale — including `.git`, datasets and checkpoints.
Keep a separate directory holding only what the agent loads at runtime, or add a
`.dockerignore` (note `make_docker.sh` copies with `cp -fr` **before** the build, so
a `.dockerignore` in `TEAM_CODE_ROOT` does not help; prune the directory itself).

## Submission

The mechanics live on <https://leaderboard.carla.org/submit/> and change between
challenge rounds: which registry, image size limits, the track you are entering, and
the evaluation quota. Read that page rather than relying on anything cached here.

What does not change:

- the image must run the agent with **no network access** and no extra downloads,
- the sensor configuration is validated at the start of every route
  ([[write-leaderboard-agent]]),
- only `entry_status: Finished` is eligible — one crashed route invalidates the
  entry,
- the version that scores your submission is the leaderboard's, not your local
  checkout's, so build against the branch matching the round you enter
  (2.1 since March 2025).

## Local dry run

The closest thing to a submission you can test yourself:

```bash
# a server on the host
bash package_agent.sh run --routes-subset 0
```

`--network host` lets the containerised evaluator reach a server running outside.
This exercises exactly the image that would be submitted, which catches the
"works in my checkout, not in the image" class of failures — a different `carla`
client (wheel locally, renamed egg in the image) being the most common.
