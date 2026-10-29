# CARLA Documentation

CARLA is an open-source simulator for autonomous driving research and development. It renders cities, highways and rural roads on Unreal Engine 5.8, with vehicles, pedestrians, traffic lights and weather, and gives you full control of them from Python or C++. Mount cameras, lidars, radars and other sensors on a vehicle and get their data together with the ground truth; populate the world with traffic; script the situations you want to test; and connect a complete autonomous driving stack in closed loop.

CARLA is used to train and validate perception models, to generate labelled datasets, to test planning and control, and to evaluate full driving stacks in scenarios that would be dangerous or impossible to stage on a real road. It is free, open source and backed by a large community. If you want to know more before installing anything, read the [Introduction](start_introduction.md).

!!! Note
    This is the documentation of __CARLA 1.0__, on Unreal Engine 5.8. The previous versions of CARLA, on Unreal Engine 4 (0.9.x), have [their own documentation](https://carla.readthedocs.io/en/latest/); they are now obsolete and will be deprecated soon.

!!! agent "Ask your agent"
    Most of what these pages describe can be done by an AI coding agent: installing and building CARLA, running servers, spawning traffic and sensors, importing maps and vehicles, and writing and running scenarios. Look for boxes like this one, with the prompt to give your agent and the skills it follows. [Connect your agent](agent_skills.md#connect-your-agent).

Where to start:

* __Install CARLA:__ follow the [Quick start with the CARLA binary](start_quickstart.md) to run a CARLA release, or [build CARLA from source](build_carla.md) to work with the Unreal Engine editor.
* __Start using CARLA:__ the [First steps](start_first_steps.md) tutorial shows how to create a simulation, and [Foundations](core_foundations.md) introduces the most important concepts.
* __Check the API:__ the [Python API reference](python_api.md) lists every class and method.

CARLA's GitHub Discussions are the place to ask questions and connect with other users.
<div class="build-buttons">
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to CARLA's GitHub Discussions">
GitHub Discussions</a>
</div>

<br>

---
## Getting started

- [__Introduction__](start_introduction.md): What to expect from CARLA.
- [__Quick start with the CARLA binary__](start_quickstart.md): Download a CARLA release and run it.
- [__Download__](download.md): Releases, nightly builds and Docker images.
- [__First steps__](start_first_steps.md): Taking the first steps in CARLA with the Python API.
- [__Building CARLA__](build_carla.md): How to build CARLA from source on Linux and Windows.

---
## Agentic CARLA

- [__Agent skills__](agent_skills.md): Let your AI coding agent install, build, run and drive CARLA for you, following 62 procedures tested by the CARLA team, and how to connect it.

---
## CARLA topics

- [__Foundations__](core_foundations.md): The world and the client, coordinate conventions, actors and blueprints, maps and navigation.
- [__Sensor simulation__](core_rendering_overview.md): The renderers available, from Unreal Engine to NVIDIA NuRec and Cosmos, and the [sensors](core_sensors.md) that read the simulation.
- [__Scenarios__](core_traffic.md): Traffic Manager, ScenarioRunner, Scenic, Inverted AI and the CARLA Leaderboard.
- [__AV stacks__](av_stacks.md): Complete autonomous driving stacks in closed loop: Autoware and NVIDIA Alpamayo 2 Super.

---
## Tutorials

- [__Tutorials__](tutorials.md): Tutorials covering CARLA's functionality.
- [__C++ tutorial__](tuto_cpp.md): The C++ client workflow, batch commands, snapshots and replay.

---
## Content authoring

- [__Custom assets__](content_authoring_overview.md): Bring your own props, vehicles and materials into CARLA.
- [__Vehicles__](content_authoring_vehicles.md): Prepare a vehicle, from the model to the blueprint.
- [__Content packs__](content_authoring_packs.md): Ship maps, props and vehicles as a package of their own.

---
## Resources

- [__Python API__](python_api.md): Classes and methods in the Python API.
- [__C++ reference__](ref_cpp.md): Classes and methods in CARLA C++.
- [__Sensors reference__](ref_sensors.md): Every sensor, its attributes and its output.
- [__Blueprint library__](bp_library.md): Every blueprint that can be spawned.
- [__Catalogue__](catalogue.md): CARLA's maps, vehicles, pedestrians and props.

---
## CARLA ecosystem

- [__RoadRunner__](roadrunner.md): MathWorks' editor for the road network of a map.
- [__NVIDIA__](nvidia.md): Neural reconstruction (NuRec), Cosmos and DLSS.
- [__ROS 2__](ros2_native.md): The native ROS 2 interface.

---
## Contributing to CARLA

- [__Guidelines__](cont_contribution_guidelines.md): How to contribute to CARLA and its ecosystem.
- [__Code of conduct__](cont_code_of_conduct.md): The standards of behaviour in the CARLA community.
- [__Coding standard__](cont_coding_standard.md): The coding conventions CARLA follows.
- [__Documentation standard__](cont_doc_standard.md): How these pages are written.
- [__Development__](dev_overview.md): Add sensors and semantic tags, update the content, run the tests and make a release.
