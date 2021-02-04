# Add a new map

find out more about adding maps

1. [Roadrunner](tuto_A_add_map/generate_map_roadrunner.md)
2. [Source](tuto_A_add_map/add_map_source.md)
3. [Package](tuto_A_add_map/add_map_package.md)
4. [Deprecated ways](tuto_A_add_map/add_map_deprecated.md)


































Users can create their own maps, and run CARLA using these. The creation of the map object is quite independent from CARLA. Nonetheless, the process to ingest it has been refined to be automatic. Thus, the new map can be used in CARLA almost out-of-the-box.  

*   [__Introduction__](#introduction)  
*   [__Create a map with RoadRunner__](#create-a-map-with-roadrunner)  
	*   [Export from RoadRunner](#export-from-roadrunner)  
*   [__Map ingestion in a CARLA package__](#map-ingestion-in-a-carla-package)  
*   [__Map ingestion in a build from source__](#map-ingestion-in-a-build-from-source)  
	*   [Modify pedestrian navigation](#modify-pedestrian-navigation)  
*   [__Deprecated ways to import a map__](#deprecated-ways-to-import-a-map)  

---


---
## Introduction

RoadRunner is the recommended software to create a map due to its simplicity. Some basic steps on how to do it are provided in [the next section](#create-a-map-with-roadrunner). The resulting map should consist of a `.fbx` and a `.xodr` with the mesh and road network informtion respectively.  

The process of the map ingestion has been simplified to minimize the users' intervention. For said reason, there are certains steps have been automatized.  

*   __Package `.json` file and folder structure__. Normally packages need a certain folder structure and a `.json` file describing them to be imported. However, as regards the map ingestion, this can be created automatically during the process. 
*   __Traffic signs and traffic lights.__ The simulator will generate the traffic lights, stops, and yields automatically when running. These will be creatd according to their `.xodr` definition. The rest of landmarks present in the road map will not be physically on scene, but they can be queried using the API.  
*   __Pedestrian navigation.__ The ingestion will generate a `.bin` file describing the pedestrian navigation. It is based on the sidewalks and crosswalks that appear in the OpenDRIVE map. This can only be modified if working in a build from source.  

!!! Important
    If a map contains additional elements besides the `.fbx` and `.xodr`, the package has to be [prepared manually](#prepare-the-package-manually).

The map ingestion process differs, depending if the package is destined to be in a CARLA package (e.g., 0.9.9) or a build from source. 

There are other ways to import a map into CARLA, which are now deprecated. They require the user to manually set the map ready. Nonetheless, as they may be useful for specific cases when the user wants to customize a specific setting, they are listed in the [last section](#deprecated-ways-to-import-a-map) of this tutorial.  

---

---

That comprises the process to create and import a new map into CARLA. If during the process any doubts arise, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>