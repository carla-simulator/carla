# Customizing maps: Weather and Landscape

CARLA provides several blueprints to help ease the creation of default weather settings for your maps and to populate the lanscape with serial meshes such as street lights, power lines, etc.

This guide will explain where each one of these blueprints are located and how to use and configure them.

- [__Weather customization__](#weather-customization)
	- [BP_Sky](#bp_sky)
	- [BP_Weather](#bp_weather)
- [__Serial meshes__](#add-serial-meshes)
	- [BP_RepSpline](#bp_repspline)
	- [BP_Spline](#bp_spline)
	- [BP_Wall](#bp_wall)
	- [BP_SplinePoweLine](#bp_splinepoweline)
- [__Next steps__](#next-steps)

!!! Important
    This tutorial only applies to users that work with a build from source, and have access to the Unreal Editor.

---

## Weather customization

This section explains how to experiment with different weather parameters before setting your map's default weather, and once you are happy with the settings, how to configure the default weather parameters for your map.

### BP_Sky

The `BP_Sky` blueprint is neccessary to bring light and weather to your map. It can also be used to test different weather configurations before deciding on your default weather parameters.

It is likely the `BP_Sky` blueprint will already be loaded in your map. If not you can add it by dragging it into the scene from `Content/Carla/Blueprints/Weather`.

To try out different weather parameters, go to the _Details_ panel of the `BP_Sky` actor, and play with the values in the _Parameters_ section.

!!! Important
    If more than one `BP_Sky` blueprint is loaded into the scene, the weather will be duplicated with undesirable results, e.g, having two suns.

### BP_Weather

The default weather for your map is defined in the `BP_Weather` blueprint. This blueprint allows you to set the same parameters as are available through the Python API. These parameters are described [here](https://carla.readthedocs.io/en/latest/python_api/#carlaweatherparameters).

To set the default weather for your map:

__1. Open the `BP_Weather` blueprint.__ 

In the _Content Browser_, navigate to `Content/Carla/Blueprints/Weather` and double-click on `BP_Weather`.

__2. Add your town.__

In the _Details_ panel of the `BP_Weather` window, go to the _Weather_ section and add your town to the _Default Weathers_ array.

__3. Configure your default weather parameters.__

For each weather parameter, set your desired value. When you are finished, press _Compile_ then _Save_ and close.

>>>>>![bp_weather_pic](../img/map_customization/BP_Weather.jpg)<br>
<div style="text-align: right">
<i>Array containing default weather parameters for every CARLA map. Town01 opened.</i></div>
<br>

---

## Add serial meshes

There are four blueprints available to add props aligned in one direction, e.g., walls, powerlines, street lights. These blueprints use a series of meshes distributed along a Bezier curve. Each one is initialized in the same way:

__1. Initialize the series.__

Drag the blueprint into the scene. You will see one element standing at the starting point of a Bezier curve with two nodes marking the beginning and ending.

__2. Define the path.__

Select the direction arrow of the element and press __Alt__ while dragging the element in the direction you want to go. This will create a new element which can be used to define the curve. As you drag, a new mesh will appear either on every node of the curve or every time you press `Alt` while dragging, depending on the blueprint.

__3. Customize the pattern.__

The following sections will describe the different customization parameters available to each blueprint.

### BP_RepSpline

The `BP_RepSpline` blueprint is found in `Carla/Blueprints/LevelDesign`. It is used to add __individual__ elements along a path defined by a Bezier curve.

The serialization is customized via the following values:

- _Distance between_ — Set the distance between elements.
- _Offset rotation_ — Set a fixed rotation for the different axis.
- _Random rotation_ — Set a range of random rotations for the different axis.
- _Offset translation_ — Set a range of random locations along the different axis.
- _Max Number of Meshes_ — Set the maximum amount of elements that will be place between nodes of the curve.
- _World aligned ZY_ — If selected, the elements will be vertically aligned regarding the world axis.
- _EndPoint_ — If selected, an element will be added at the end node of the curve.
- _Collision enabled_ — Set the type of collisions enabled for the meshes.

![bp_repspline_pic](../img/map_customization/BP_Repspline.jpg)
<div style="text-align: right"><i>BP_RepSpline example.</i></div>

### BP_Spline

The `BP_Spline` blueprint is found in `Carla/Blueprints/LevelDesign`. It adds __connected__ elements that __strictly__ follow a path defined by a Bezier curve. The mesh will be warped to fit the path created.

The blueprint can be customized using the following value:

- _Gap distance_ — Add a separation between elements.  

![bp_spline_pic](../img/map_customization/BP_Spline.jpg)
<div style="text-align: right"><i>BP_Spline example.</i></div>

### BP_Wall

The `BP_Wall` blueprint is found in `Carla/Blueprints/LevelDesign`. It adds __connected__ elements along a path defined by a Bezier curve. The mesh will not be warped to fit the curve, but the nodes will be respected.

- _Distance between_ — Set the distance between elements.
- _Vertically aligned_ — If selected, the elements will be vertically aligned regarding the world axis.
- _Scale offset_ — Scale the length of the mesh to round out the connection between elements.

![bp_wall_pic](../img/map_customization/BP_Wall.jpg)
<div style="text-align: right"><i>BP_Wall example.</i></div>

### BP_SplinePoweLine

The __BP_SplinePoweLine__ blueprint is found in `Carla/Static/Pole/PoweLine`. It adds __electricity poles__ along a path defined by a Bezier curve and __connects them with power lines__.

To provide variety, you can provide the blueprint with an array of powerline meshes to populate the path. To do this:

1. Double-click the __BP_SplinePoweLine__ blueprint in the _Content Browser_.
2. In the _Details_ panel, go to the _Default_ section.
3. Expand the _Array Meshes_ and add to or change it according to your needs.
4. Press _Compile_, then save and close the window.

![bp_splinepowerline_pic](../img/map_customization/BP_Splinepowerline.jpg)
<div style="text-align: right"><i>BP_SplinePowerLine example.</i></div>

To alter the line tension of the power lines:

1. Select the blueprint actor in the editor scene and go to the _Details_ panel.
2. Go to the _Default_ section.
3. Adjust the value in _Tension_. `0` indicates that the lines will be straight.

To increase the amount of wires:

1. In the _Content Browser_, double-click on one of the pole meshes.
2. Go to the _Socket Manager_ panel.
3. Configure existing sockets or add new ones by clicking _Create Socket_. Sockets are empty meshes that represent the connection points of the power line. A wire is created form socket to socket between poles.

![bp_powerline_socket_pic](../img/map_customization/BP_Splinepowerline_Sockets.jpg)
<div style="text-align: right"><i>Visualization of the sockets for BP_SplinePowerLine.</i></div>
<br>

!!! Important
    The amount of sockets and their names should be consistent between poles. Otherwise, visualization issues may arise. 

---

## Next steps

Continue customizing your map using the tools and guides below:

- [Implement sub-levels in your map.](tuto_M_custom_layers.md)
- [Add and configure traffic lights and signs.](tuto_M_custom_add_tl.md)
- [Add buildings with the procedural building tool.](tuto_M_custom_buildings.md)
- [Customize the road with the road painter tool.](tuto_M_custom_road_painter.md)
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