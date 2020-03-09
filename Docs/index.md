# CARLA Documentation

Welcome to the CARLA documentation. 

This page contains the index with a brief explanation on the different sections for clarity. 
Feel free to explore the documentation on your own, however, here are a few tips for newcomers:

* __Install CARLA__: visit the [Quickstart installation](start_quickstart.md) to get the CARLA releases or make the build for a desired platform. 
* __Start using CARLA__: there is a section titled [First steps](core_concepts.md), highly recommended for newcomers.
* __Doubts on the API__: there is a handy [Python API reference](python_api.md) to consult classes and methods.

Besides that, there is also the CARLA forum where the community gathers to share issues, suggestions and solutions: 
<div class="build-buttons">
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</div>

!!! Important
    This is documentation for CARLA 0.9.0 or later. Previous documentation is in the [stable branch](https://carla.readthedocs.io/en/stable/).

---

## Getting started
  <p style="padding-left:30px;line-height:1.8">
    <a href="../start_introduction"><b>
    Introduction
    </b></a>
        — Capabilities and intentions behind the project.  
    <a href="../start_quickstart"><b>
    Quickstart installation
    </b></a>
        — Get the CARLA releases.  
  </p>

## Building CARLA
  <p style="padding-left:30px;line-height:1.8">
    <a href="../build_linux"><b>
    Linux build
    </b></a>
        — Make the build on Linux.  
    <a href="../build_windows"><b>
    Windows build
    </b></a>
        — Make the build on Windows.  
    <a href="../build_update"><b>
    Update CARLA
    </b></a>
        — Get up to date with the latest content.  
    <a href="../build_system"><b>
    Build system
    </b></a>
        — Learn about the build and how it is made.  
    <a href="../build_docker"><b>
    Running in a Docker
    </b></a>
        — Run CARLA using a container solution.  
    <a href="../build_faq"><b>
    F.A.Q.
    </b></a>
        — Some of the most frequent issues for newcomers.  
  </p>

## First steps
  <p style="padding-left:30px;line-height:1.8">
    <a href="../core_concepts"><b>
    Core concepts
    </b></a>
        — Overview of the basic concepts in CARLA.  
    <a href="../core_world"><b>
    1st. World and client
    </b></a>
        — Manage and access the simulation.  
    <a href="../core_actors"><b>
    2nd. Actors and blueprints
    </b></a>
        — Learn about actors and how to handle them.  
    <a href="../core_map"><b>
    3rd. Maps and navigation
    </b></a>
        — Discover the different maps and how to move around.  
    <a href="../core_sensors"><b> 
    4th. Sensors and data
    </b></a>
        — Retrieve simulation data using sensors.  

## Advanced steps
  <p style="padding-left:30px;line-height:1.8">
    <a href="../adv_recorder"><b>
    Recorder
    </b></a>
        — Store all the events in a simulation a play it again.  
    <a href="../adv_rendering_options"><b>
    Rendering options
    </b></a>
        — Different settings, from quality to no-render or off-screen runs.  
    <a href="../adv_synchrony_timestep"><b>
    Synchrony and time-step
    </b></a>
        — Client-server communication and simulation time.  
    <a href="../adv_traffic_manager"><b>
    Traffic Manager
    </b></a>
        — Feature to handle autopilot vehicles and emulate traffic.  

## References
  <p style="padding-left:30px;line-height:1.8">
    <a href="../python_api"><b>
    Python API reference
    </b></a>
        — Classes and methods in the Python API.  
    <a href="../ref_code_recipes"><b>
    Code recipes
    </b></a>
        — Code fragments commonly used.  
    <a href="../bp_library"><b>
    Blueprint library
    </b></a>
        — Blueprints provided to spawn actors.  
    <a href="../ref_cpp"><b>
    C++ reference
    </b></a>
        — Classes and methods in CARLA C++.  
    <a href="../ref_recorder_binary_file_format"><b>
    Recorder binary file format
    </b></a>
        — Detailed explanation of the recorder file format.  
    <a href="../ref_sensors"><b>
    Sensors reference
    </b></a>
        — Everything about sensors and the data they retrieve.  

## ROS bridge
  <p style="padding-left:30px;line-height:1.8">
    <a href="../ros_installation"><b>
    ROS bridge installation
    </b></a>
        — How to install the ROS bridge's package or repository.  
    <a href="../ros_msgs"><b>
    CARLA messages reference
    </b></a>
        — Contains explanations and fields for every type of CARLA message available in ROS.  
    <a href="../ros_launchs"><b>
    Launchfiles reference
    </b></a>
        — Explains the launchfiles provided, its nodes, and the topics that are being consumed and published.  

## Tutorials — General
  <p style="padding-left:30px;line-height:1.8">
    <a href="../tuto_G_add_friction_triggers"><b>
    Add friction triggers
    </b></a>
        — Define dynamic box triggers for wheels.  
    <a href="../tuto_G_control_vehicle_physics"><b>
    Control vehicle physics
    </b></a>
        — Set runtime changes on a vehicle physics.  
    <a href="../tuto_G_control_walker_skeletons"><b>
    Control walker skeletons
    </b></a>
        — Skeleton and animation for walkers explained.  

## Tutorials — Assets
  <p style="padding-left:30px;line-height:1.8">
    <a href="../tuto_A_import_assets"><b>
    Import new assets
    </b></a>
        — Use personal assets in CARLA.  
    <a href="../tuto_A_map_creation"><b>
    Map creation 
    </b></a>
        — Guidelines to create a new map.  
    <a href="../tuto_A_map_customization"><b>
    Map customization
    </b></a>
        — Edit an existing map.  
    <a href="../tuto_A_standalone_packages"><b>
    Standalone asset packages 
    </b></a>
        — Import assets into Unreal Engine and prepare them for package distribution.  
    <a href="../tuto_A_epic_automotive_materials"><b>
    Use Epic's Automotive materials
    </b></a>
        — Apply Epic's set of Automotive materials to vehicles for a more realistic painting.
    <a href="../tuto_A_vehicle_modelling"><b>  
    Vehicle modelling
    </b></a>
        — Guidelines to create a new vehicle for CARLA.  

## Tutorials — Developers
  <p style="padding-left:30px;line-height:1.8">
    <a href="../tuto_D_contribute_assets"><b>
    Contribute with new assets
    </b></a>
        — Add new content to CARLA.  
    <a href="../tuto_D_create_sensor"><b>
    Create a sensor
    </b></a>
        — The basics on how to add a new sensor to CARLA.  
    <a href="../tuto_D_make_release"><b>
    Make a release
    </b></a>
        — For developers who want to publish a release.  
    <a href="../tuto_D_generate_pedestrian_navigation"><b>
    Generate pedestrian navigation
    </b></a>
        — Generate the information needed for walkers to navigate a map.  

## Contributing
  <p style="padding-left:30px;line-height:1.8">
    <a href="../cont_contribution_guidelines"><b>
    Contribution guidelines
    </b></a>
        — The different ways to contribute to CARLA.  
    <a href="../cont_code_of_conduct"><b>
    Code of conduct
    </b></a>
        — Some standards for CARLA, rights and duties for contributors.  
    <a href="../cont_coding_standard"><b>
    Coding standard
    </b></a>
        — Guidelines to write proper code.  
    <a href="../cont_doc_standard"><b>
    Documentation standard
    </b></a>
        — Guidelines to write proper documentation.  