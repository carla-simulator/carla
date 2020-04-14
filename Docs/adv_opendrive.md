# OpenDRIVE standalone mode

This feature allows users to ingest any OpenDRIVE file as a CARLA map out-of-the-box. In order to do so, the simulator will automatically generate a road mesh for actors to navigate through.  

*   [__Overview__](#overview)  
*   [__Run a standalone map__](#run-a-standalone-map)  
*   [__Mesh generation__](#mesh-generation)  

---
## Overview

This mode runs a full simulation using only an OpenDRIVE file, without the need of any additional gemotires or assets. To this end, the simulator takes an OpenDRIVE file and procedurally creates a temporal 3D mesh to run the simulation with.  

The resulting mesh describes the road definition in a minimalistic manner. All the elements will correspond with the OpenDRIVE file, but besides that, there will be only void. In order to prevent vehicles from falling off the road, two measures have been taken. 

*   Lanes are a bit wider at junctions, where the flow of vehicles is most complex.  
*   Visible walls are created at the boundaries of the road, to act as a last safety measure. 

Traffic lights, stops and yields will be generated on the fly, as happens when using any map. Pedestrians will navigate over the sidewalks and crosswalks that appear in the map. All of these elements, and every detail on the road, are based on the OpenDRIVE file. As the standalone mode uses the `.xodr` directly, any issues in it will propagate to the simulation. This could be an issue especially at junctions, where many lanes are mixed.

*   OpenDRIVE maps created with RoadRunner present some issues related to tilted junction ??
*   Lateral slope for roads is not supported yet ??
*   The sidewalks height is currently hardcoded. This ensures collisions with them are detected even if the map definition does not include this field ??
*   What .xml ??

!!! Important
    It is especially important to double check the OpenDRIVE file. Any issues in it will propagate when running the simulation.

![opendrive_standalone](img/opendrive_standalone.png)

---
## Run a standalone map

In order to open an OpenDRIVE file, the Python API includes now the method [`client.generate_opendrive_world()`](python_api.md#carla.Client.generate_opendrive_world). This will block the simulation until the new one is ready. The method needs for two parameters.  

*   __`opendrive`__ is the content of the OpenDRIVE file parsed as a string.  
*   __`parameters`__ is a [carla.OpendriveGenerationParameters](python_api.md#carla.OpendriveGenerationParameters) containing settings for the generation of the mesh. 
	*   `vertex_distance` is the distance between the vertices of the mesh. --> The bigger the more inaccuracies?? The smallest, the more detailed but heavier ??
	*   `max_road_length` 
	*   `wall_height` states the height of the additional walls created on the boundaries of the road. These prevent vehicles from falling to the void.  
	*   `additional_width` is an small width increment applied to the junction lanes. This is a safety measure to prevent vehicles from falling.  
	*   `smooth_junctions` if __True__, the mesh at junctions will be smoothed, to prevent some inaccuracies that may occur when various lanes meet.  --> And why can I choose to disable them?? 
	*   `enable_mesh_visibility` if __True__, the mesh will be rendered. --> If False? Vehicles rendered, no rendering mode or what?  


In order to easily test this feature, the `config.py` script in `PythonAPI/util/` has a new argument, `-x` or `--xodr-path`. This argument contains a string with the path to the `.xodr` file. If the mesh is generated with this script, the parameters used will always be the default ones. 
```sh
python3 config.py -x path/to/some/file.xodr
```

!!! Important
    __[carla.OpendriveGenerationParameters](python_api.md#carla.OpendriveGenerationParameters)__ uses __content of the OpenDRIVE file parsed as string__. On the contrary, __`config.py`__ script needs __the path to the `.xodr` file__.

---
## Mesh generation

The generation of the mesh is the key element of this mode. The feature can only be successful if the resulting mesh is smooth and fits its definition perfectly. For that reason, this step is constantly being improved. In the last iterations, junctions have been polished to avoid inaccuracies that occur, especially where uneven lanes joined.  

Besides that, instead of creating the whole map as a unique mesh, different fragments are created. Working smaller prevents unexpected issues. Also, by dividing the mesh, not all of it has to be rendered at a time. This is a step towards a larger goal, where the feature will be able to generate huge maps.


---

That covers all there is to know so far, regarding the OpenDRIVE standalone mode. Take the chance and use any OpenDRIVE map to test it in CARLA.  

Doubts and suggestions in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
