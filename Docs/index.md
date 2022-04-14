# CARLA Documentation

Welcome to the CARLA documentation.

This home page contains an index with a brief description of the different sections in the documentation. Feel free to read in whatever order preferred. In any case, here are a few suggestions for newcomers.

* __Install CARLA.__ Either follow the [Quick start installation](start_quickstart.md) to get a CARLA release or [make the build](build_carla.md) for a desired platform.
* __Start using CARLA.__ The section titled [Foundations](foundations.md) is an introduction to the most important concepts and the [first steps tutorial](tuto_first_steps.md) shows you how to get started.
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
[__First steps__](tuto_G_getting_started.md) — Taking the first steps in CARLA.  
[__Building CARLA__](build_carla.md) — How to build CARLA from source.

## CARLA components
[__Foundations__](core_concepts.md) — Overview of the fundamental building blocks of CARLA.    
[__Actors__](core_actors.md) — Learn about actors and how to handle them.  
[__Maps__](core_map.md) — Discover the different maps and how do vehicles move around.  
[__Sensors and data__](core_sensors.md) — Retrieve simulation data using sensors.  
[__Traffic__](ts_traffic_simulation_overview.md) — An overview of the different options available to populate your scenes with traffic.  
[__3rd party integrations__](3rd_party_integrations.md) — Integrations with 3rd party applications and libraries.   
[__Development__](development_tutorials.md) — Information on how to develop custom features for CARLA.  
[__Custom assets__](custom_assets_tutorials.md) — Information on how to develop custom assets 

## Resources
[__Blueprint library__](bp_library.md) — Blueprints provided to spawn actors.   
[__Python API__](python_api.md) — Classes and methods in the Python API.   
[__C++ reference__](ref_cpp.md) — Classes and methods in CARLA C++.    

## CARLA ecosystem

[__ROS bridge documentation__](ros_documentation.md) — Brief overview of the ROS bridge and a link to the full documentation  
[__MathWorks__](large_map_roadrunner.md) — Overview of creating a map in RoadRunner.    
[__SUMO__](adv_sumo.md) — Details of the co-simulation feature with SUMO.   
[__Scenic__](tuto_G_scenic.md) — How to use Scenic with CARLA to generate scenarios.    
[__Chrono__](tuto_G_chrono.md) — Details of the Chrono physics simulation integration with CARLA.   
[__OpenDrive__](adv_opendrive.md) — Details of the OpenDrive support in CARLA.  
[__PTV-Vissim__](adv_ptv.md) — Details of the co-simulation feature with PTV-Vissim.    
[__RSS__](adv_rss.md) — Details of the Responsibility Sensitive Safety library integration with CARLA.  
[__AWS__](tuto_G_rllib_integration) — Details of using RLlib to run CARLA as a distributed application on Amazon Web Services.  
[__ANSYS__](ecosys_ansys.md) — Brief overview of how the Ansys Real Time Radar Model was integrated into CARLA.  
[__carlaviz — web visualizer__](plugins_carlaviz.md) — Plugin that listens the simulation and shows the scene and some simulation data in a web browser.

## Contributing to CARLA
[__Guidelines__](cont_contribution_guidelines.md) — Guidelines on contributing to the development of the CARLA simulator and its ecosystem.   
[__Coding standards__](cont_coding_standard.md) — Details on the best coding practices when contributing to CARLA development.  
[__Documentation standard__](cont_doc_standard.md) — Details on the documentation standards for CARLA docs.     

## Tutorials

There are numerous tutorials covering CARLA features with code and guidelines for varied use cases. Please check the [tutorials page](tutorials.md) for help with your work.

## Extended documentation

The pages above cover most of the core concepts and features of CARLA. There is additional documentation in the [extended documentation](ext_docs.md) section covering advanced features in more depth.