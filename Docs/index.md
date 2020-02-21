<h1>CARLA Documentation</h1>

Welcome to the CARLA documentation. 

This page contains the index with a brief explanation on the different sections for clarity. 
Feel free to explore the documentation on your own, however, here are a few tips for newcomers:

* __Install CARLA__: visit the [Quickstart installation](../dev/quickstart) to get the CARLA releases or make the build for a desired platform. 
* __Start using CARLA__: there is a section titled [First steps](../core_concepts), highly recommended for newcomers.
* __Doubts on the API__: there is a handy [Python API reference](../python_api) to consult classes and methods.

Besides that, there is also the CARLA forum where the community gathers to share issues, suggestions and solutions: 
<div class="build-buttons">
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</div>

!!! important
    This is documentation for CARLA 0.9.0 or later. Previous documentation is in the [stable branch](https://carla.readthedocs.io/en/stable/).
---------------

  <h3>Getting started</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../getting_started/introduction"><b>
    Introduction
    </b></a>
        — Capabilities and intentions behind the project.  
    <a href="../getting_started/quickstart"><b>
    Quickstart installation
    </b></a>
        — Get the CARLA releases. 
  </p>

  <h3>Building CARLA</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../how_to_build_on_linux"><b>
    Linux build
    </b></a>
        — Make the build on Linux.  
    <a href="../how_to_build_on_windows"><b>
    Windows build
    </b></a>
        — Make the build on Windows.  
    <a href="../update_carla"><b>
    Update CARLA
    </b></a>
        — Get up to date with the latest content.  
    <a href="../dev/build_system"><b>
    Build system
    </b></a>
        — Learn about the build and how it is made.  
    <a href="../carla_docker"><b>
    Running in a Docker
    </b></a>
        — Run CARLA using a container solution.  
    <a href="../faq"><b>
    F.A.Q.
    </b></a>
        — Some of the most frequent issues for newcomers. 
  </p>

  <h3>First steps</h3>
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
    (broken) 4th. Sensors and data
    </b></a>
        — Retrieve simulation data using sensors.  

 <h3>Advanced steps</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../recorder_and_playback"><b>
    Recorder
    </b></a>
        — Store all the events in a simulation a play it again.  
    <a href="../rendering_options"><b>
    Rendering options
    </b></a>
        — Different settings, from quality to no-render or off-screen runs.  
    <a href="../simulation_time_and_synchrony"><b>
    Synchrony and time-step
    </b></a>
        — Client-server communication and simulation time.  
    <a href="../traffic_manager"><b>
    (broken) Traffic manager
    </b></a>
        — Feature to handle autopilot vehicles and emulate traffic.  

<h3>References</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../python_api"><b>
    Python API reference
    </b></a>
        — Classes and methods in the Python API.  
    <a href="../python_cookbook"><b>
    Code recipes
    </b></a>
        — Code fragments commonly used.  
    <a href="../bp_library"><b>
    Blueprint library
    </b></a>
        — Blueprints provided to spawn actors.  
    <a href="../cpp_reference"><b>
    C++ reference
    </b></a>
        — Classes and methods in CARLA C++.  
    <a href="../recorder_binary_file_format"><b>
    Recorder binary file format
    </b></a>
        — Detailed explanation of the recorder file format.  
    <a href="../ref_sensors"><b>
    (broken) Sensors reference
    </b></a>
        — Everything about sensors and the data they retrieve.  

<h3>Tutorials — General</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../how_to_add_friction_triggers"><b>
    Add friction triggers
    </b></a>
        — Define dynamic box triggers for wheels.  
    <a href="../how_to_control_vehicle_physics"><b>
    Control vehicle physics
    </b></a>
        — Set runtime changes on a vehicle physics.  
    <a href="../walker_bone_control"><b>
    Control walker skeletons
    </b></a>
        — Skeleton and animation for walkers explained.  

<h3>Tutorials — Assets</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../dev/how_to_upgrade_content"><b>
    Contribute with new assets
    </b></a>
        — Add new content to CARLA.  
    <a href="../how_to_add_assets"><b>
    Import new assets
    </b></a>
        — Use personal assets in CARLA.  
    <a href="../dev/map_customization"><b>
    Map customization
    </b></a>
        — Edit an existing map.  
    <a href="../how_to_make_a_new_map"><b>
    Map creation 
    </b></a>
        — Guidelines to create a new map.  
    <a href="../asset_packages_for_dist"><b>
    Standalone asset packages 
    </b></a>
        — Import assets into Unreal Engine and prepare them for package distribution.  
    <a href="../epic_automotive_materials"><b>
    Use Automotive materials
    </b></a>
        — Apply Epic's set of Automotive materials to vehicles for a more realistic painting.
    <a href="../how_to_model_vehicles"><b>
    Vehicle modelling
    </b></a>
        — Guidelines to create a new vehicle for CARLA.  

<h3>Tutorials — Developers</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../dev/how_to_add_a_new_sensor"><b>
    Create a sensor
    </b></a>
        — The basics on how to add a new sensor to CARLA.  
    <a href="../dev/how_to_make_a_release"><b>
    Make a release
    </b></a>
        — For developers who want to publish a release.  
    <a href="../bp_library"><b>
    Pedestrian navigation physics
    </b></a>
        — Generate the information needed for walkers to navigate a map.  

<h3>Contributing</h3>
  <p style="padding-left:30px;line-height:1.8">
    <a href="../CONTRIBUTING"><b>
    General guidelines
    </b></a>
        — The different ways to contribute to CARLA.  
    <a href="../coding_standard"><b>
    Coding standard
    </b></a>
        — Guidelines to write proper code.  
    <a href="../doc_standard"><b>
    Documentation standard
    </b></a>
        — Guidelines to write proper documentation.  
    <a href="../CODE_OF_CONDUCT"><b>
    Code of conduct
    </b></a>
        — Some standards for CARLA, rights and duties for contributors.  