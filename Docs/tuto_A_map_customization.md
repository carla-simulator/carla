# Map customization tools

There are several tools provided by the CARLA team that allow users to edit maps at will from the Unreal Editor. This tutorial introduces the most relevant tools, according to their purpose. 

*   [__Serial meshes__](#add-serial-meshes)  
	*   [BP_RepSpline](#bp_repspline)  
	*   [BP_Spline](#bp_spline)  
	*   [BP_Wall](#bp_wall)  
	*   [BP_SplinePowerLine](#bp_splinepowerline)  
*   [__Procedural buildings__](#add-serial-meshes)  
	*   [Building structure](#building-structure)  
	*   [Structure modifications](#structure-modifications)  
*   [__Weather customization__](#weather-customization)  
	*   [BP_Weather](#bp_weather)  
	*   [BP_Sky](#bp_sky)  

!!! Important
    This tutorial only applies to users that work with a build from source, and have access to the Unreal Editor. 

---
## Add serial meshes

There is a series of blueprints in `Carla/Blueprints/LevelDesign` that are useful to add props aligned in one direction. All of them use a series of meshes, and a Bezier curve that establishes the path where the props are placed.  

There are differences between them, that make them fit specific purposes. However, the all work the same way. Only the parametrization presents differences.  

* __Initialize the series__. The blueprints need a __Static Mesh__ that will be repeated. Initially, only one element will appear, standing on the starting point of a Bezier curve with two nodes, beginning and ending.  
* __Define the path__. Press __Alt__ over one of the nodes, to create a new one and modify the curve. A new mesh will appear on every node of the curve, and the space between nodes will be filled with elements __separated by a distance__ measure. Adjust the curve using the weights on every node.  
* __Customize the pattern__. This is where the blueprints present differences between each other.  

!!! Warning
    New props will probably interfere with the mesh navigation. If necessary, rebuild that as explained [here](tuto_A_add_map.md#generate-pedestrian-navigation) after doing these changes.

### BP_RepSpline

The blueprint __BP_RepSpline__ adds __individual__ elements along the path defined by a Bezier curve. There are some specificic parameters that change the serialization.  

*   __Distance between__ — Set the distance between elements.  
*   __Offset rotation__ — Set a fixed rotation for the different axis.  
*   __Random rotation__ — Set a range of random rotations for the different axis.  
*   __Offset translation__ — Set a range of random locations along the different axis.  
*   __Max Number of Meshes__ — Set the maximum amount of elements that will be place between nodes of the curve.  
*   __World aligned ZY__ — If selected, the elements will be vertically aligned regarding the world axis.  
*   __EndPoint__ — If selected, an element will be added in the ending node of the curve.  
*   __Collision enabled__ — Set the type of collisions enabled for the meshes.  

![bp_repspline_pic](img/map_customization/BP_Repspline.jpg)
<div style="text-align: right"><i>BP_RepSpline example.</i></div>

### BP_Spline

The blueprint __BP_Spline__ adds __connected__ elements __strictly__ following the path defined by a Bezier curve. The mesh will be warped to fit the path created.  

*   __Gap distance__ — Add a separation between elements.  

![bp_spline_pic](img/map_customization/BP_Spline.jpg)
<div style="text-align: right"><i>BP_Spline example.</i></div>

### BP_Wall

The blueprint __BP_Wall__ adds __connected__ elements along the path defined by a Bezier curve. The mesh will not be warped to fit the curve, but the nodes will be respected.  

*   __Distance between__ — Set the distance between elements.  
*   __Vertically aligned__ — If selected, the elements will be vertically aligned regarding the world axis.  
*   __Scale offset__ — Scale the length of the mesh to round out the connection between elements.  

![bp_wall_pic](img/map_customization/BP_Wall.jpg)
<div style="text-align: right"><i>BP_Wall example.</i></div>

### BP_SplinePowerLine

The blueprint __BP_SplinePowerLine__ adds __electricity poles__ along the path defined by a Bezier curve, and __connects them with power lines__.  

This blueprint can be found in `Carla/Static/Pole`. This blueprint allows to set an __array of meshes__ to repeat, to provide variety.  

![bp_splinepowerline_pic](img/map_customization/BP_Splinepowerline.jpg)
<div style="text-align: right"><i>BP_SplinePowerLine example.</i></div>

The power line that connects the pole meshes can be customized.  

*   __Choose the mesh__ that will be used as wire.  
*   __Edit the tension__ value. If `0`, the power lines will be staight. The bigger the value, the looser the connection.  
*   __Set the sockets__. Sockets are empty meshes that represent the connection points of the power line. A wire is created form socket to socket between poles. The amount of sockets can be changed inside the pole meshes.  

![bp_powerline_socket_pic](img/map_customization/BP_Splinepowerline_Sockets.jpg)
<div style="text-align: right"><i>Visualization of the sockets for BP_SplinePowerLine.</i></div>

!!! Important
    The amount of sockets and their names should be consistent between poles. Otherwise, visualization issues may arise. 

---
## Procedural buildings

The blueprint __BP_Procedural_Building__ in `Content/Carla/Blueprints/LevelDesign` creates a realistic building using key meshes that are repeated along the structure. For each of them, the user can provide an array of meshes that will be used at random for variety. The meshes are only created once, and the repetitions will be instances of the same to save up costs.  

!!! Note
    Blueprints can be used instead of meshes, to allow more variety and customization for the building. Blueprints can use behaviour trees to set illumination inside the building, change the materials used, and much more.  

### Building structure

The key meshes will be updated everytime a change is made, and the building will disappear. Enable `Create automatically` or click on `Create Building` to see the new result. 

These key meshes can be percieved as pieces of the building's structure. They can be grouped in four categories.  

*   __Base__ — The ground floor of the building.  
*   __Body__ — The middle floors of the building.  
*   __Top__ — The highest floor of the building.  
*   __Roof__ — Additional mesh that used to fill the spaces in the middle of the top floor.  

For each of them, except the __Roof__, there is a mesh to fill the center of the floor, and a __Corner__ mesh that will be placed on the sides of the floor. The following picture represents the global structure. 

![bp_procedural_building_visual](img/map_customization/BP_Procedural_Building_Visual.jpg)
<div style="text-align: right"><i>Visualization of the building structure.</i></div>

The __Base parameters__ set the dimensions of the building.  

*   __Num Floors__ — Floors of the building. Repetitions of the __Body__ meshes.  
*   __Length X and Length Y__ — Area of the building. Repetitions of the central meshes for each side of the building.  

![bp_procedural_building_full](img/map_customization/BP_Procedural_Building_Full.jpg)
<div style="text-align: right"><i>Example of BP_Procedural_Building.</i></div>

### Structure modifications

There are some additional options to modify the general structure of the building.  

*   __Disable corners__ — If selected, no corner meshes will be used.  
*   __Use full blocks__ — If selected, the structure of the building will use only one mesh per floor. No corners nor repetitions will appear in each floor.  
*   __Doors__ — Meshes that appear in the ground floor, right in front of the central meshes. The amount of dloors and their location can be set. `0` is the initial position, `1` the next base repetition, and so on.  
*   __Walls__ — Meshes that substitute one or more sides of the building. For example, a plane mesh can be used to paint one side of the building. 

![bp_procedural_building_extras](img/map_customization/BP_Procedural_Building_Extras.jpg)
<div style="text-align: right"><i>On the left, a building with no cornes and one door. <br> On the right, a building with a wall applied to one side of the building. The wall is a texture with no fire escape.</i></div>

---
## Weather customization

The weather can be easily customized by the users in CARLA using the PythonAPI. However, there is some configuration that users can do in order to set the default weather for a map. The weather parameters available for configuration by the following blueprints, are the same accessible from the API. These are described [here](https://carla.readthedocs.io/en/latest/python_api/#carlaweatherparameters).  

### BP_Weather

This blueprint is loaded into the world when the simulation starts. It contains the default weather parameters for every map, and these can be modified at will.  

__1. Open the BP_Weather__ in `Content/Carla/Blueprints/Weather`.  

__2. Go to the Weather group__ in the blueprint.  

__3. Choose the desired town__ and modify the parameters.  

![bp_weather_pic](img/map_customization/BP_Weather.jpg)<br>
<div style="text-align: right">
<i>Array containing default weather parameters for every CARLA map. Town01 opened.</i></div>

### BP_Sky

This blueprint groups all the weather parameters. It can be loaded into the scene when there is no CARLA server running, and used to ea test different configurations before setting a new default weather.  

__1. Find the BP_Sky__ in `Content/Carla/Blueprints/Weather`.  

__2. Load the blueprint in the scene.__ Drag it into the scene view.  

__3. Edit the weather parameters.__ The weather in the scene will be updated accordingly.  

!!! Important
    If more than one blueprint is loaded into the scene, the weather will be duplicated with weird results, such as having two suns. 

---

That is all there is so far, regarding the different map customization tools available in CARLA.

Open CARLA and mess around for a while. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>