# Ingesting Maps in CARLA Built From Source

This section describes the process of ingesting maps into __CARLA that has been built from source__. If you are using a package (binary) version of CARLA to ingest maps then follow the guidelines [here][package_ingest] instead.

The ingestion process involves importing the relevant map files by compiling them into a package. This package can then be opened in the Unreal Engine editor and customized before generating the pedestrian navigation file and finally adding it to the package. 

[package_ingest]: add_map_package.md

- [__Before you begin__](#before-you-begin)
- [__Map ingestion__](#map-ingestion)
    - [Customize the map](#customize-the-map)
    - [Generate pedestrian navigation](#generate-pedestrian-navigation)

---

## Before you begin

- Ensure you are using a version of CARLA that has been built from source. If you are using a packaged (binary) version of CARLA then follow the tutorial [here][import_map_package].
- You should have at least two files, `<mapName>.xodr` and `<mapName>.fbx` that have been [generated][rr_generate_map] from a map editor such as RoadRunner. 
- These files should have the same value for `<mapName>` in order to be recognised as the same map.
- You can ingest multiple maps into the same package. Each map should have a unique name.

[import_map_package]: add_map_package.md
[rr_generate_map]: generate_map_roadrunner.md

---
## Map ingestion

__1.__ Place the map files to be imported in the `Import` folder found in the CARLA root directory.

__2.__ Run the command below to ingest the files:

```sh
    make import ARGS="--package <package_name> --no-carla-materials"
```

__Note that there are two parameter flags that can be set__:

- `--package <package_name>` specifies the name of the package. By default, this is set to `map_package`. Two packages cannot have the same name, so using the default value will lead to errors on a subsequent ingestion. __It is highly recommended to change the name of the package__. 
- `--no-carla-materials` specifies that you do not want to use the default CARLA materials (road textures etc). You will use the RoadRunner materials instead. This flag is only required if you are not providing your own [`.json` file](../tuto_A_add_map_overview.md#ingest-the-map-in-carla). Any value in the `.json` file will override this flag.

### Customize the map

Before generating the pedestrian navigation, you may want to customize the map in the editor. Props such as trees, streetlights or grass zones can be added. Customization should be completed before generating the pedestrian navigation in order to avoid interference or collisions between the two, resulting in the need to generate the pedestrian navigation a second time.  

* __Create new spawning points__. These spawning points will be used in scripts such as [`spawn_npc.py`](https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/spawn_npc.py):
    - In the editor, go to the `Modes` panel and search for "spawn" in the `Search Classes` search bar.
    - Choose the type of spawning point you would like to create and drag it on to the map.
    - Place the spawn point about 0.5-1m above the ground to prevent collisions with the road.
* __Generate new crosswalks__. Avoid doing this if the crosswalk is already defined the `.xodr` file as this will lead to duplication:
    - Create a plane mesh that extends a bit over two sidewalks that you want to connect. 
    - Place the mesh overlapping the ground and disable it's physics and rendering. 
    - Change the name of the mesh to `Road_Crosswalk` or `Roads_Crosswalk`.  

![ue_crosswalks](../img/ue_crosswalks.jpg)  



### Generate pedestrian navigation

__1.__ To prevent the map being too large to export, select the __BP_Sky object__ and add a tag `NoExport` to it. If you have any other particularly large meshes that are not involved in the pedestrian navigation, add the `NoExport` tag to them as well. 

![ue_skybox_no_export](../img/ue_noexport.png) 

__2.__ Double check your mesh names. Mesh names should start with any of the appropriate formats listed below in order to be recognized as areas where pedestrians can walk. By default, pedestrians will be able to walk over sidewalks, crosswalks, and grass (with minor influence over the rest):  

*   Sidewalk = `Road_Sidewalk` or `Roads_Sidewalk` 
*   Crosswalk = `Road_Crosswalk` or `Roads_Crosswalk` 
*   Grass = `Road_Grass` or `Roads_Grass`

![ue_meshes](../img/ue_meshes.jpg) 

__3.__ Press `ctrl + A` to select everything and export the map by selecting `File` -> `Carla Exporter`. A `<mapName>.obj` file will be created in `Unreal/CarlaUE4/Saved`.

__4.__ Move the `<mapName>.obj` and the `<mapName>.xodr` to `Util/DockerUtils/dist`.  

__5.__ Run the following command to generate the navigation file:  

*   __Windows__ 
```sh
build.bat <mapName> # <mapName> has no extension
```
*   __Linux__
```sh
./build.sh <mapName> # <mapName> has no extension
```

__6.__ A `<mapName>.bin` file will be created. This file contains the information for pedestrian navigation on your map. Move this file to the `Nav` folder of the package that contains the map.  

---

Your map is now ready to run simulations in CARLA. If you have any questions about the process then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions/) or you can try running some of our [example scripts](https://github.com/carla-simulator/carla/tree/master/PythonAPI/examples) on your new map to test it out.