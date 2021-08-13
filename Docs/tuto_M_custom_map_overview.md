# Add a new map

Users of CARLA can create custom maps and use them to run simulations. There are several ways to import custom maps in CARLA. The method to be used will depend on whether you are using a packaged version of CARLA or a version built from source. This section gives an overview of what you need to start the process, the different options available to import, and customization and pedestrian navigation tools available.

- [__Overview__](#overview)
- [__Generation__](#generation)
- [__Importation__](#importation)
- [__Customization__](#customization)
- [__Generate pedestrian navigation__](#generate-pedestrian-navigation)

---

## Overview

Using custom maps in CARLA involves four main processes:

1. Generation
2. Importation
3. Customization
4. Pedestrian Navigation

Read on further for additional general information on each process.

---

## Generation

CARLA requires map geometry information in `.fbx` format and OpenDRIVE information in `.xodr` format. The current recommended software to generate these files is RoadRunner.

__[This guide](tuto_M_generate_map.md) explains how to use RoadRunner to generate the map information.__

---

## Importation

There are several ways to import your map into CARLA. 

If you are using a __package version__ of CARLA, you will import your map using Docker. This option is only available in Linux, and you will not have the ability to customize the map using the Unreal Editor. __You will find the guide [here](tuto_M_add_map_package.md).__

If you are using a __source build__ version of CARLA, there are three methods available to import your map:

1. Using the automatic `make import` process (recommended). __You will find the guide [here](tuto_M_add_map_source.md).__
2. Using the RoadRunner plugin. __You will find the guide [here](tuto_M_add_map_alternative.md#roadrunner-plugin-import).__
3. Manually importing the map into Unreal Engine. __You will find the guide [here](tuto_M_add_map_alternative.md#manual-import).__

The following video explains some of the methods available to import maps into CARLA:

<iframe width="560" height="315" src="https://www.youtube.com/embed/mHiUUZ4xC9o" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

<br>

---

## Customization

As well as hundreds of static meshes ready to be added to the landscape, CARLA provides several tools and guides to help you customize your map:

- __Add sub-levels:__ Sub-levels will allow multiple people to work on the same map at the same time. They will also allow you to toggle layers of your map with the Python API, just like the CARLA layered maps. __You will find the guide [here](tuto_M_custom_layers.md).__
- __Set default weather:__ Experiment with different weather presets, and when you find the right combination, set the default weather for your map. __You will find the guide [here](tuto_M_custom_weather_landscape.md#weather-customization).__
- __Populate landscape:__ Use blueprints to populate the landscape with repeating meshes such as street lights, power lines, and walls. __You will find the guide [here](tuto_M_custom_weather_landscape.md#add-serial-meshes).__
- __Paint the roads:__ Paint the roads with a master material that blends different textures. Add decals and meshes such as fallen leaves, cracks, or manholes. __You will find the guide [here](tuto_M_custom_road_painter.md).__
- __Add procedural buildings:__ Add buildings with a custom size, amount of floors, and variable mesh combinations using the procedural building blueprint. __You will find the guide [here](tuto_M_custom_buildings.md).__
- __Add traffic lights and signs:__ Add traffic lights and signs and configure their area of influence. Group traffic lights at junctions. __You will find the guide [here](tuto_M_custom_add_tl.md).__

---

## Generate pedestrian navigation

For pedestrians to be spawned and navigate the map, you need to generate the pedestrian navigation information using the tool provided by CARLA. Pedestrian navigation should be generated after you complete the customization of your map so that obstacles are not created over the top of navigation paths. __You can find the guide [here](tuto_M_generate_pedestrian_navigation.md).__

---

If you have any questions about the above process, feel free to post these in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>