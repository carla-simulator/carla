# Add a new map

Users of CARLA can create custom maps and use them to run simulations. There are several ways to import custom maps in CARLA. The method to be used will depend on if the map is destined to be used in a packaged version of CARLA or in a version built from source. This section serves as a guide to direct you to the correct method.

- [__Overview__](#overview)
	- [Export from RoadRunner](#export-from-roadrunner)
	- [Ingest the map in CARLA](#ingest-the-map-in-carla)
- [__Summary__](#summary)

---

## Overview

Below is a summary of the steps in the ingestion process or you can follow the links directly to find each relevant guide:

1. [__Export from RoadRunner__](tuto_A_add_map/generate_map_roadrunner.md)
2. [__Import into source build version of CARLA__](tuto_A_add_map/add_map_source.md)
3. [__Import into package version of CARLA__](tuto_A_add_map/add_map_package.md)

You can also watch the following video to see an explanation of the various methods:

<iframe width="560" height="315" src="https://www.youtube.com/embed/mHiUUZ4xC9o" frameborder="0" allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

<br>
### Export from RoadRunner

RoadRunner is the recommended software to create a map due to its simplicity. We provide a [__guide__](tuto_A_add_map/generate_map_roadrunner.md) on how to export maps made in RoadRunner so they are ready for import into CARLA.

### Ingest the map in CARLA

This is where the route splits in two:

 - If you are using CARLA built from source, follow the guide [__here__](tuto_A_add_map/add_map_source.md).
 - If you are using a packaged (binary) version of CARLA, follow the guide [__here__](tuto_A_add_map/add_map_package.md).

Regardless of the method used, there are some common themes involved in the ingestion process:
	
- __Package `.json` file and folder structure__. Map packages have a particular folder structure and this structure is described in a `.json` file. This file is automatically created during the import process if it is not provided by the user. 
- __Traffic signs and traffic lights.__ The simulator will generate the traffic lights, stops and yields automatically when running. These will be created according to their `.xodr` definition. Any other landmarks present in the map will not be physically on scene, but they can be queried using the API.  
*   __Pedestrian navigation.__ The ingestion process will generate a `.bin` file describing the pedestrian navigation. It is based on the sidewalks and crosswalks that appear in the `.xodr` definition. This can only be modified if working in a build from source.  

---

## Summary

If you have any questions about the process to create and import a new map into CARLA, feel free to post these in the [forum](https://forum.carla.org/). 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>