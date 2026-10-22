# Tutorials

Step-by-step guides that solve one problem each, with the code to run. The sections of this documentation cover the subjects in full; these are the worked examples.

- [__Python API__](#python-api)
- [__Sensors__](#sensors)
- [__Traffic__](#traffic)
- [__C++__](#c)
- [__Where the rest lives__](#where-the-rest-lives)
- [__Video tutorials__](#video-tutorials)

---
## Python API

- [__Control vehicle physics__](tuto_vehicle_physics.md) — Read a vehicle's physics control, change it at runtime, and watch its telemetry.
- [__Control walker skeletons__](tuto_walker_skeletons.md) — Pose and animate a pedestrian bone by bone.
- [__Texture streaming__](tuto_texture_streaming.md) — Replace the textures of map objects while the simulation runs.
- [__Add friction triggers__](tuto_friction_triggers.md) — Define box triggers that change the friction under a vehicle's wheels.

---
## Sensors

- [__Bounding boxes__](tuto_bounding_boxes.md) — Project the 3D boxes of vehicles, pedestrians and map objects into a camera image.
- [__Pedestrian bones__](tuto_pedestrian_bones.md) — Project a pedestrian's skeleton into the camera plane, the way a dataset annotation would.

---
## Traffic

- [__Traffic Manager__](tuto_traffic_manager.md) — Populate a map with traffic, shape how it behaves and send it along set routes.

---
## C++

- [__C++ tutorial__](tuto_cpp.md) — The client workflow from C++: connect, spawn with batch commands, read snapshots, record and replay, and drive with the navigation agents.

---
## Where the rest lives

Much of what used to be a tutorial is now a section of its own, written as a sequence rather than a single page:

| Subject | Where |
| --- | --- |
| First steps with the API | [First steps](start_first_steps.md) |
| Making maps, vehicles, props and materials | [Content authoring](content_authoring_overview.md) |
| Shipping what you made | [Content packs](content_authoring_packs.md) |
| Adding a semantic class for your assets | [Create semantic tags](core_sensors_semantic_tags.md) |
| Working on CARLA itself | [Development](dev_overview.md) |

---
## Video tutorials

- [__Fundamentals__](https://www.youtube.com/watch?v=pONr1R1dy88) — The fundamental concepts of CARLA, and a first script.
- [__An in depth look at CARLA's sensors__](https://www.youtube.com/watch?v=om8klsBj4rc) — What each sensor produces and how to use it.
