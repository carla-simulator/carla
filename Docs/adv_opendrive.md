# OpenDRIVE standalone mode

This feature allows users to ingest any OpenDRIVE file as a CARLA map out-of-the-box. In order to do so, the simulator will automatically generate a road mesh for actors to navigate through.  

*	[__Overview__](#overview)  
*	[__Run a standalone map__](#run-a-standalone-map)  
*	[__Mesh generation__](#mesh-generation)  

---
## Overview

This mode runs a full simulation using only an OpenDRIVE file, without the need of any additional gemotires or assets. To this end, the simulator takes an OpenDRIVE file and procedurally creates a temporal 3D mesh to run the simulation with.  

The resulting mesh describes the road definition in a minimalistic manner. All the elements will correspond with the OpenDRIVE file, but besides that, there will be only void. In order to prevent vehicles from falling off the road, two measures have been taken. 

*	Lanes are a bit wider at junctions, where the flow of vehicles is most complex.  
*	Visible walls are created at the boundaries of the road, to act as a last safety measure. 

Traffic lights, stops and yields will be generated on the fly, as happens when using any map. Pedestrians will navigate over the sidewalks and crosswalks that appear in the map. All of these elements, and every detail on the road, are based on the OpenDRIVE file. As the standalone mode uses the `.xodr` directly, any issues in it will propagate to the simulation. This could be an issue especially at junctions, where many lanes are mixed.

*	OpenDRIVE maps created with RoadRunner present some issues related to tilted junction ??
*	Lateral slope for roads is not supported yet ??
*	The sidewalks height is currently hardcoded. This ensures collisions with them are detected even if the map definition does not include this field ??
*	What .xml ??

!!! Important
    It is especially important to double check the OpenDRIVE file. Any issues in it will propagate when running the simulation.

![opendrive_standalone](img/opendrive_standalone.png)

---
## Run a standalone map

In order to test this feature, the `config.py` script in `PythonAPI/util/` has a new argument, `-x` or `--xodr-path`. This argument contains a string with the path to the `.xodr` file. The script calls a client method, [`client.generate_opendrive_world()`](python_api.md#carla.Client.generate_opendrive_world). This will block the simulation, as [`load_world()`](python_api.md#carla.World.load_world) would do, until the new map is active.

```sh
python3 config.py -x path/to/some/file.xodr
```

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
