# CARLA Documentation

Welcome to the CARLA documentation.

This home page contains an index with a brief description of the different sections in the documentation. Feel free to read in whatever order preferred. In any case, here are a few suggestions for newcomers.

* __Install CARLA.__ Either follow the [Quick start installation](start_quickstart.md) to get a CARLA release or [make the build](build_linux.md) for a desired platform.
* __Start using CARLA.__ The section titled [First steps](core_concepts.md) is an introduction to the most important concepts.
* __Check the API.__ there is a handy [Python API reference](python_api.md) to look up the classes and methods available.

The CARLA forum is available to post any doubts or suggestions that may arise during the reading.
<div class="build-buttons">
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the latest CARLA release">
CARLA forum</a>
</div>

<br>

!!! Warning
    __Change the docs version to fit the CARLA version you are using__. Use the panel in the bottom-right side of this window to change to previous versions. __The _Latest_ version points to documentation in the `dev` branch__ which may refer to features currently in development and __not available__ in any packaged version of CARLA, as well general documentation improvements. ![docs_version_panel](img/docs_version_panel.jpg)

---

## Getting started

[__Introduction__](start_introduction.md) — What to expect from CARLA.  
[__Quick start package installation__](start_quickstart.md) — Get the CARLA releases. 
[__First steps__](tuto_G_getting_started.md) - Taking the first steps in CARLA.  


## CARLA components
[__Foundations__](core_concepts.md) — Overview of the fundamental building blocks of CARLA.    
[__Actors__](core_actors.md) — Learn about actors and how to handle them.  
[__Maps__](core_map.md) — Discover the different maps and how do vehicles move around.  
[__Sensors and data__](core_sensors.md) — Retrieve simulation data using sensors.  
[__Traffic__](ts_traffic_simulation_overview.md) — An overview of the different options available to populate your scenes with traffic  
 
## Tutorials
[__Traffic__](tuto_G_traffic_manager.md) - Use the traffic manager to create and control NPC traffic in your simulation.
[__Rendering__](tuto_G_texture_streaming.md) - Change textures during runtime through the API.
[__Sensors__](tuto_G_instance_segmentation_sensor.md) - Use the instance segmentation camera sensor.
[__Actors__](tuto_G_pedestrian_bones.md) - Retrieve 3D ground truth pedestrian skeleton through the API
[__Maps__](tuto_M_custom_map_overview.md) — An overview of the process and options involved in adding a custom, standard sized map  
[__Integrations__](tuto_G_chrono.md) — Use the Chrono integration to simulation physics  
[__Custom assets__](tuto_A_add_vehicle.md) — Prepare a vehicle to be used in CARLA.  
[__Development__](tuto_D_contribute_assets.md) — Add new content to CARLA.  

## Resources
[__Blueprint library__](bp_library.md) — Blueprints provided to spawn actors.
[__Python API__](python_api.md) — Classes and methods in the Python API.   
[__C++ reference__](ref_cpp.md) — Classes and methods in CARLA C++.    

## 3rd Party Integrations
[__ROS bridge documentation__](ros_documentation.md) — Brief overview of the ROS bridge and a link to the full documentation  
[__carlaviz — web visualizer__](plugins_carlaviz.md) — Plugin that listens the simulation and shows the scene and some simulation data in a web browser.  