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

---

# Extended documentation

Below you will find in depth documentation  and tutorials on the many extensive features of CARLA 

## Advanced concepts
 
[__Recorder__](adv_recorder.md) — Register the events in a simulation and play it again.  
[__Rendering options__](adv_rendering_options.md) — From quality settings to no-render or off-screen modes.  
[__Synchrony and time-step__](adv_synchrony_timestep.md) — Client-server communication and simulation time.  
[__Benchmarking Performance__](adv_benchmarking.md) — Perform benchmarking using our prepared script.  
[__CARLA Agents__](adv_agents.md) — Agents scripts allow single vehicles to roam the map or drive to a set destination.  

## Traffic Simulation

[__ Traffic Simulation Overview__](ts_traffic_simulation_overview.md) — An overview of the different options available to populate your scenes with traffic  
[__Traffic Manager__](adv_traffic_manager.md) — Simulate urban traffic by setting vehicles to autopilot mode.  

## References

[__Recorder binary file format__](ref_recorder_binary_file_format.md) — Detailed explanation of the recorder file format.  
[__Sensors reference__](ref_sensors.md) — Everything about sensors and the data they retrieve.  

## Custom Maps

[__Overview of custom maps in CARLA__](tuto_M_custom_map_overview.md) — An overview of the process and options involved in adding a custom, standard sized map  
[__Create a map in RoadRunner__](tuto_M_generate_map.md) — How to generate a customs, standard sized map in RoadRunner  
[__ Import map in CARLA package__](tuto_M_add_map_package.md) How to import a map in a CARLA package  
[__Import map in CARLA source build__](tuto_M_add_map_source.md) — How to import a map in CARLA built from source  
[__Alternative ways to import maps__](tuto_M_add_map_alternative.md) — Alternative methods to import maps  
[__ Manually prepare map package__](tuto_M_manual_map_package.md) — How to prepare a map for manual import  
[__Customizing maps: Layered maps__](tuto_M_custom_layers.md) — How to create sub-layers in your custom map  
[__ Customizing maps: Traffic lights and signs__](tuto_M_custom_add_tl.md) — How to add traffic lights and signs to your custom map  
[__ Customizing maps: Road painter__](tuto_M_custom_road_painter.md) — How to use the road painter tool to change the apearance of the road  
[__Customizing Maps: Procedural Buildings__](tuto_M_custom_buildings.md) — Populate your custom map with buildings  
[__ Customizing maps: Weather and landscape__](tuto_M_custom_weather_landscape.md) — Create the weather profile for your custom map and populate the landscape  
[__Generate pedestrian navigation__](tuto_M_generate_pedestrian_navigation.md) — Obtain the information needed for walkers to move around.  

## Large Maps

[__Large maps overview__](large_map_overview.md) — An explanation of how large maps work in CARLA  
[__Create a Large Map in RoadRunner__](large_map_roadrunner.md) — How to create a large map in RoadRunner  
[__Import/Package a Large Map__](large_map_import.md) — How to import a large map  

## Tutorials — General

[__Add friction triggers__](tuto_G_add_friction_triggers.md) — Define dynamic box triggers for wheels.  
[__Control vehicle physics__](tuto_G_control_vehicle_physics.md) — Set runtime changes on a vehicle physics.  
[__Control walker skeletons__](tuto_G_control_walker_skeletons.md) — Animate walkers using skeletons.  
[__Generate maps with OpenStreetMap__](tuto_G_openstreetmap.md) — Use OpenStreetMap to generate maps for use in simulations.  
[__Retrieve simulation data__](tuto_G_retrieve_data.md) — A step by step guide to properly gather data using the recorder.  
[__CarSim Integration__](tuto_G_carsim_integration.md) — Tutorial on how to run a simulation using the CarSim vehicle dynamics engine.  
[__RLlib Integration__](tuto_G_rllib_integration.md) — Find out how to run your own experiment using the RLlib library.  
[__Chrono Integration__](tuto_G_chrono.md) — Use the Chrono integration to simulation physics  
[__Build Unreal Engine and CARLA in Docker__](build_docker_unreal.md) — Build Unreal Engine and CARLA in Docker  

## Tutorials — Assets

[__Add a new vehicle__](tuto_A_add_vehicle.md) — Prepare a vehicle to be used in CARLA.  
[__Add new props__](tuto_A_add_props.md) — Import additional props into CARLA.  
[__Create standalone packages__](tuto_A_create_standalone.md) — Generate and handle standalone packages for assets.  
[__Material customization__](tuto_A_material_customization.md) — Edit vehicle and building materials.  

## Tutorials — Developers

[__How to upgrade content__](tuto_D_contribute_assets.md) — Add new content to CARLA.  
[__Create a sensor__](tuto_D_create_sensor.md) — Develop a new sensor to be used in CARLA.  
[__Create semantic tags__](tuto_D_create_semantic_tags.md) — Define customized tags for semantic segmentation.  
[__Customize vehicle suspension__](tuto_D_customize_vehicle_suspension.md) — Modify the suspension system of a vehicle.  
[__Generate detailed colliders__](tuto_D_generate_colliders.md) — Create detailed colliders for vehicles.  
[__Make a release__](tuto_D_make_release.md) — How to make a release of CARLA