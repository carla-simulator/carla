# CARLA Unreal Engine 5 Documentation

!!! Note
    These pages document the Unreal Engine 5 version of CARLA, built from the `ue58-dev` branch on Unreal Engine 5.8. The Unreal Engine 4.26 version has [its own documentation](https://carla.readthedocs.io/en/latest/), and the two differ in features, blueprint ids and build procedure.

This home page is an index of the documentation. Read it in any order; for a newcomer, the suggestions below are a good path.

* __Install CARLA:__ Either follow the [Quick Start Installation](start_quickstart.md) to get a CARLA release or [build CARLA from source](build_carla.md) for a desired platform.
* __Start using CARLA:__ The [Foundations](core_foundations.md) section is an introduction to the most important concepts in CARLA and the [First Steps tutorial](start_first_steps.md) shows you how to get started creating simulations.
* __Check the API:__ there is a handy [Python API reference](python_api.md) to look up the classes and methods available.

CARLA's GitHub Discussions are the place to ask questions and connect with other users.
<div class="build-buttons">
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to CARLA's GitHub Discussions">
GitHub Discussions</a>
</div>

<br>

---

## Getting started

- [__Introduction__](start_introduction.md) — What to expect from CARLA.
- [__Quick start package installation__](start_quickstart.md) — Install the CARLA package and get started.
- [__Download__](download.md) — Releases, nightly builds and Docker images.
- [__First steps__](start_first_steps.md) — Taking the first steps in CARLA with the Python API.
- [__Building CARLA__](build_carla.md) — How to build CARLA from source on Linux and Windows.

---
## CARLA components

- [__Foundations__](core_foundations.md) — Overview of the fundamental building blocks of CARLA.
- [__Actors__](core_actors.md) — Learn about actors and how to handle them.
- [__Maps__](core_map.md) — The maps, and how vehicles move around them.
- [__Sensors and data__](core_sensors.md) — Retrieve simulation data using sensors.
- [__Coordinate conventions__](core_coordinate_conventions.md) — CARLA's frame, and how to convert to a right-handed one.
- [__Traffic__](core_traffic.md) — The options available to populate a scene with traffic.

---
## Content authoring

- [__Custom assets__](content_authoring_overview.md) — Bring your own props, vehicles and materials into CARLA.
- [__Vehicles__](content_authoring_vehicles.md) — Prepare a vehicle, from the model to the blueprint.
- [__Content packs__](content_authoring_packs.md) — Ship maps, props and vehicles as a package of their own.
- [__OpenDRIVE standalone mode__](core_opendrive.md) — Drive on a road network with no map around it.

---
## Resources

- [__Python API__](python_api.md) — Classes and methods in the Python API.
- [__Sensors reference__](ref_sensors.md) — Every sensor, its attributes and its output.
- [__Catalogue__](catalogue.md) — CARLA's maps, vehicles, pedestrians and props.
- [__Blueprint library__](bp_library.md) — Every blueprint that can be spawned.
- [__Tutorials__](tutorials.md) — Tutorials covering CARLA's functionality.
- [__C++ tutorial__](tuto_cpp.md) — The C++ client workflow, batch commands, snapshots and replay.
- [__C++ reference__](ref_cpp.md) — Classes and methods in CARLA C++.
- [__Agent skills__](agent_skills.md) — Step-by-step procedures for AI coding agents working in this repository.

---
## CARLA ecosystem

- [__RoadRunner__](roadrunner.md) — MathWorks' editor for the road network of a map.
- [__NVIDIA__](nvidia.md) — Alpamayo 2 Super, neural reconstruction (NuRec), Cosmos and DLSS.
- [__ROS 2__](ros2_native.md) — The native ROS 2 interface.
- [__Scenic__](scenic.md) — Generate scenarios with Scenic.
- [__Inverted AI__](inverted_ai.md) — Inverted AI's generative traffic simulation.

---
## Contributing to CARLA
- [__Guidelines__](cont_contribution_guidelines.md) — How to contribute to CARLA and its ecosystem.
- [__Coding standards__](cont_coding_standard.md) — The coding conventions CARLA follows.
- [__Documentation standard__](cont_doc_standard.md) — How these pages are written.
