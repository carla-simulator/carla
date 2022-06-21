# Customizing Maps: Procedural Buildings

- [__Procedural buildings__](#add-serial-meshes)
	- [Building structure](#building-structure)
	- [Structure modifications](#structure-modifications)
- [__Next steps__](#next-steps)

---

## Procedural buildings

The procedural building tool allows you to create rectangular buildings composed of different levels. Each level is built using a configurable array of meshes or a single blueprint. If an array of meshes is used, each mesh will be repeated along the level at random to provide variety. The meshes are created once and each repition will be an instance of that mesh. This improves performance of your map.

### Building structure

To get started on your building:

1. In the _Content Browser_ of the Unreal Engine editor, navigate to `Content/Carla/Blueprints/LevelDesign`.
2. Drag the `BP_Procedural_Building` into the scene.

In the _Details_ panel, you will see all the options available to customize your building. Every time a change is made here, the building will disappear from the scene view, as the key meshes are updated. Click on _Create Building_ to see the new result, or enable _Create automatically_ to avoid having to repeat this step.

The key meshes are pieces of the building's structure. They fall into four categories:

- __Base:__ The ground floor.
- __Body:__ The middle floors.
- __Top:__ The highest floor.
- __Roof:__ The roof that covers the top floor.

For each of them, except the __Roof__, there is a mesh to fill the center of the floor, and a __Corner__ mesh that will be placed on the sides of the floor. The following picture represents the global structure.

![bp_procedural_building_visual](../img/map_customization/BP_Procedural_Building_Visual.jpg)
<div style="text-align: right"><i>Visualization of the building structure.</i></div>

The __Base parameters__ set the dimensions.

- __Num Floors:__ Floors of the building. Repetitions of the __Body__ meshes.
- __Length X and Length Y:__ Length and breadth of the building. Repetitions of the central meshes for each side of the building.

![bp_procedural_building_full](../img/map_customization/BP_Procedural_Building_Full.jpg)
<div style="text-align: right"><i>Example of BP_Procedural_Building.</i></div>

### Structure modifications

There are some additional options to modify the general structure of the building.

- __Disable corners:__ If selected, no corner meshes will be used.
- __Use full blocks:__ If selected, the structure of the building will use only one mesh per floor. No corners nor repetitions will appear in each floor.
- __Doors:__ Meshes that appear in the ground floor, right in front of the central meshes. The amount of doors and their location can be set. `0` is the initial position, `1` the next base repetition, and so on.
- __Walls:__ Meshes that substitute one or more sides of the building. For example, a plane mesh can be used to paint one side of the building.

![bp_procedural_building_extras](../img/map_customization/BP_Procedural_Building_Extras.jpg)
<div style="text-align: right"><i>On the left, a building with no cornes and one door. <br> On the right, a building with a wall applied to one side of the building. The wall is a texture with no fire escape.</i></div>

---

## Next steps

Continue customizing your map using the tools and guides below:

- [Implement sub-levels in your map.](tuto_M_custom_layers.md)
- [Add and configure traffic lights and signs.](tuto_M_custom_add_tl.md)
- [Customize the road with the road painter tool.](tuto_M_custom_road_painter.md)
- [Customize the weather](tuto_M_custom_weather_landscape.md#weather-customization)
- [Customize the landscape with serial meshes.](tuto_M_custom_weather_landscape.md#add-serial-meshes)

Once you have finished with the customization, you can [generate the pedestrian navigation information](tuto_M_generate_pedestrian_navigation.md).

---

If you have any questions about the process, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
