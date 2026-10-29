# CARLA Leaderboard

The [CARLA Autonomous Driving Leaderboard](https://leaderboard.carla.org/) is an open platform to evaluate how well an autonomous driving agent drives in realistic traffic, in a way that is fair and reproducible, so that different approaches can be compared. The agent starts at a point and has to reach a destination, guided by a route given as GPS coordinates, map coordinates or route instructions. The routes cross freeways, urban areas, residential districts and rural roads, by day, at sunset, in rain, fog and at night, and on the way the agent meets traffic scenarios taken from the NHTSA pre-crash typology: merging and changing lanes, junctions and roundabouts, traffic lights and signs, yielding to emergency vehicles, and pedestrians and cyclists.

The Leaderboard runs on [ScenarioRunner](core_traffic.md#scenariorunner-and-openscenario): the evaluator drives your agent through each route and triggers the scenarios along it. This page summarises how it works; the [Leaderboard site](https://leaderboard.carla.org/) has the full rules, the guides and the results.

- [__Versions__](#versions)
- [__Tracks and sensors__](#tracks-and-sensors)
- [__Metrics__](#metrics)
- [__Writing an agent__](#writing-an-agent)
- [__Running the evaluation and submitting__](#running-the-evaluation-and-submitting)

!!! agent "Ask your agent"
    *"Install the Leaderboard, write an agent with a front camera and a GNSS, run it on the first three routes and explain the infractions."*

    Skills: `install-leaderboard`, `write-leaderboard-agent`, `run-leaderboard-evaluation`, `read-leaderboard-results`, `create-leaderboard-route`, `package-leaderboard-agent`. [Connect your agent](agent_skills.md#connect-your-agent).

---
## Versions

| Version | Notes |
| --- | --- |
| __Leaderboard 2.1__ (current) | Since March 2025. The infraction penalty is linear instead of exponential, so stopping early in a route no longer pays off. Same training, validation and test routes and scenarios as 2.0; scores are not comparable with 2.0. |
| __Leaderboard 2.0__ | Still supported. |
| __Leaderboard 1.0__ | Still supported. |

Each version has its own Leaderboard and ScenarioRunner branches (`leaderboard-2.1`, `leaderboard-2.0`, `leaderboard-1.0`) and is evaluated on the CARLA version given in its "Get started" guide on the Leaderboard site.

---
## Tracks and sensors

| Track | Sensors allowed |
| --- | --- |
| __SENSORS__ | RGB cameras (up to 8), LIDAR (up to 2), RADAR (up to 4), GNSS, IMU and speedometer (one each) |
| __MAP__ | The SENSORS track plus the OpenDRIVE map of the town as a pseudo-sensor, given as a string the agent parses itself |

Each track has a __qualifier__ on an unseen map with simpler scenarios, with fewer sensors (4 cameras, 2 radars, 1 lidar). A submission to a main track is accepted only after a high enough score in its qualifier.

---
## Metrics

* __Route completion__: the percentage of the route the agent completes.
* __Infraction penalty__: starts at 1.0 and goes down with every infraction, as 1 / (1 + Σ coefficient × number of infractions).
* __Driving score__: route completion × infraction penalty, per route, up to 100. The global scores are the averages over all the routes.

| Infraction | Coefficient |
| --- | --- |
| Collision with a pedestrian | 1.0 |
| Collision with a vehicle | 0.70 |
| Collision with a static element | 0.60 |
| Running a red light | 0.40 |
| Failing to yield to an emergency vehicle | 0.40 |
| Scenario timeout | 0.40 |
| Failing to keep a minimum speed | up to 0.40 |
| Running a stop sign | 0.25 |

Driving off-road does not add a penalty: the off-road part of the route is not counted as completed. A route stops early when the agent deviates more than 30 m from it, does not move for 180 simulation seconds, the client and server do not communicate for 60 seconds, the route takes too long, or the agent raises an error.

---
## Writing an agent

An agent is a Python class deriving from `AutonomousAgent` (`leaderboard.autoagents.autonomous_agent`), or from `ROS1Agent` / `ROS2Agent` for a ROS-based agent:

* `get_entry_point()` returns the name of the agent class.
* `setup(path_to_conf_file)` runs at the start of every route and sets `self.track` to `Track.SENSORS` or `Track.MAP`.
* `sensors()` returns the sensor suite, a list of dictionaries with the type, id, position and attributes of each sensor, within the limits of the track.
* `run_step(input_data, timestamp)` runs every step with the sensor data and returns a [`carla.VehicleControl`](python_api.md#carla.VehicleControl).
* `destroy()` runs at the end of every route.

---
## Running the evaluation and submitting

Locally, `run_leaderboard.sh` in the Leaderboard repository calls `leaderboard/leaderboard_evaluator.py` with the routes, the number of repetitions, your agent and its configuration, and writes the results to a JSON checkpoint file, which can resume a run that crashed.

To appear in the Leaderboard, build your agent as a Docker image with `scripts/make_docker.sh` from the Leaderboard repository and push it to the CARLA Leaderboard challenge on [EvalAI](https://eval.ai/) with `evalai push <image>:<tag> --phase <phase>`. Submissions are evaluated in the cloud, for free, with a monthly allowance of 20 per team. The CARLA AD Challenge, the yearly competition on top of the Leaderboard, did not run in 2025; the Leaderboard itself stays open.

<div class="build-buttons">
<p>
<a href="https://leaderboard.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA Leaderboard">
Go to the CARLA Leaderboard</a>
</p>
</div>
