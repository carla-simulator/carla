# CARLA Documentation

Welcome to the CARLA documentation. 

This home page contains an index with a brief description of the different sections in the documentation. Feel free to read in whatever order preferred. In any case, here are a few suggestions for newcomers.  

* __Install CARLA.__ Either follow the [Quick start installation](start_quickstart.md) to get a CARLA release or [make the build](build_linux.md) for a desired platform.  
* __Start using CARLA.__ The section titled [First steps](core_concepts.md) is an introduction to the most important concepts.  
* __Check the API.__ there is a handy [Python API reference](python_api.md) to look up the classes and methods available.  

The CARLA forum is available to post any doubts or suggestions that may arise during the reading.  
<div class="build-buttons">
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</div>

!!! Important
    This documentation refers to CARLA 0.9.0 or later. To read about previous versions, check the [stable branch](https://carla.readthedocs.io/en/stable/).

---

## Getting started
  <p style="padding-left:30px;line-height:1.8">
    [__Introduction__](start_introduction.md)
        — What to expect from CARLA.  
    [__Quick start__](start_quickstart.md)
        — Get the CARLA releases.  
  </p>

## Building CARLA
  <p style="padding-left:30px;line-height:1.8">
    [__Linux build__](build_linux.md)
        — Make the build on Linux.  
    [__Windows build__](build_windows.md)
        — Make the build on Windows.  
    [__Update CARLA__](build_update.md)
        — Get up to date with the latest content.  
    [__Build system__](build_system.md)
        — Learn about the build and how it is made.  
    [__Running in a Docker__](build_docker.md)
        — Run CARLA using a container solution.  
    [__F.A.Q.__](build_faq.md)
        — Some of the most frequent installation issues.  
  </p>

## First steps
  <p style="padding-left:30px;line-height:1.8">
    [__Core concepts__](core_concepts.md)
        — Overview of the basic concepts in CARLA.  
    [__1st. World and client__](core_world.md)
        — Manage and access the simulation.  
    [__2nd. Actors and blueprints__](core_actors.md)
        — Learn about actors and how to handle them.  
    [__3rd. Maps and navigation__](core_map.md)
        — Discover the different maps and how do vehicles move around.  
    [__4th. Sensors and data__](core_sensors.md)
        — Retrieve simulation data using sensors.  
    </p>

## Advanced steps
  <p style="padding-left:30px;line-height:1.8">
    [__Recorder__](adv_recorder.md)
        — Register the events in a simulation and play it again.  
    [__Rendering options__](adv_rendering_options.md)
        — From quality settings to no-render or off-screen modes.  
    [__Synchrony and time-step__](adv_synchrony_timestep.md)
        — Client-server communication and simulation time.  
    [__Traffic Manager__](adv_traffic_manager.md)
        — Simulate urban traffic by setting vehicles to autopilot mode.  
    </p>

## References
  <p style="padding-left:30px;line-height:1.8">
    [__Python API reference__](python_api.md)
        — Classes and methods in the Python API.  
    [__Code recipes__](ref_code_recipes.md)
        — Some code fragments commonly used.  
    [__Blueprint library__](bp_library.md)
        — Blueprints provided to spawn actors.  
    [__C++ reference__](ref_cpp.md)
        — Classes and methods in CARLA C++.  
    [__Recorder binary file format__](ref_recorder_binary_file_format.md)
        — Detailed explanation of the recorder file format.  
    [__Sensors reference__](ref_sensors.md)
        — Everything about sensors and the data they retrieve.  

## ROS bridge
  <p style="padding-left:30px;line-height:1.8">
    [__ROS bridge installation__](ros_installation.md)
        — The different ways to install the ROS bridge.  
    [__CARLA messages reference__](ros_msgs.md)
        — Contains explanations and fields for every type of CARLA message available in ROS.  
    [__Launchfiles reference__](ros_launchs.md)
        — Lists the launchfiles and nodes provided, and the topics being consumed and published.  
    </p>

## Tutorials — General
  <p style="padding-left:30px;line-height:1.8">
    [__Add friction triggers__](tuto_G_add_friction_triggers.md)
        — Define dynamic box triggers for wheels.  
    [__Control vehicle physics__](tuto_G_control_vehicle_physics.md)
        — Set runtime changes on a vehicle physics.  
    [__Control walker skeletons__](tuto_G_control_walker_skeletons.md)
        — Animate walkers using skeletons.  
    </p>

## Tutorials — Assets
  <p style="padding-left:30px;line-height:1.8">
    [__Import new assets__](tuto_A_import_assets.md)
        — Use personal assets in CARLA.  
    [__Map creation__](tuto_A_map_creation.md)
        — Create a new map following simple guidelines.  
    [__Map customization__](tuto_A_map_customization.md)
        — Edit an existing map.  
    [__Standalone asset packages__](tuto_A_standalone_packages.md)
        — Import assets into UE and set them for package distribution.  
    [__Use Epic's Automotive materials__](tuto_A_epic_automotive_materials.md)
        — Apply Epic's set of Automotive materials to vehicles.  
    [__Vehicle modelling__](tuto_A_vehicle_modelling.md)
        — Create a new vehicle for CARLA.  
    </p>

## Tutorials — Developers
  <p style="padding-left:30px;line-height:1.8">
    [__Contribute with new assets__](tuto_D_contribute_assets.md)
        — Add new content to CARLA.  
    [__Create a sensor__](tuto_D_create_sensor.md)
        — Develop a new sensor to be used in CARLA.  
    [__Make a release__](tuto_D_make_release.md)
        — For developers who want to publish a release.  
    [__Generate pedestrian navigation__](tuto_D_generate_pedestrian_navigation.md)
        — Obtain the information needed for walkers to move around.  
    </p>

## Contributing
  <p style="padding-left:30px;line-height:1.8">
    [__Contribution guidelines__](cont_contribution_guidelines.md)
        — The different ways to contribute to CARLA.  
    [__Code of conduct__](cont_code_of_conduct.md)
        — Standard rights and duties for contributors.  
    [__Coding standard__](cont_coding_standard.md)
        — Guidelines to write proper code.  
    [__Documentation standard__](cont_doc_standard.md)
        — Guidelines to write proper documentation.  
    </p>