# Ingesting Maps in CARLA Built From Source

This section describes the process of ingesting maps into __CARLA that has been built from source__. If you are using a package (binary) version of CARLA to ingest maps then follow the guidelines [here][package_ingest] instead.

[package_ingest]: add_map_package.md

- [Before you begin](#before-you-begin)
- [Map ingestion](#map-ingestion)
    - [Customize the map](#customize-the-map)
    - [Generate pedestrian navigation](#generate-pedestrian-navigation)

---

## Before you begin

- Ensure you are using a version of CARLA that has been built from source. If you are using a packaged (binary) version CARLA, follow the tutorial [here][import_map_package].
- You should have at least two files, `<mapName>.xodr` and `<mapName>.fbx` that have been [generated][rr_generate_map] from a map editor such as RoadRunner. 
- These files should have the same value for `<mapName>` in order to be recognised as the same map.

[import_map_package]: add_map_package.md
[rr_generate_map]: generate_map_roadrunner.md

---
## Map ingestion

1. Place the map files to be imported in the `Import` folder found in the CARLA root directory.

2. Run the command below to ingest the files:

        make import ARGS="--package <package_name> --no-carla-materials"

    __Note that there are two parameter flags that can be set__:

    - `--package <package_name>` specifies the name of the package. By default, this is set to `map_package`. Two packages cannot have the same name, so using the default value will lead to errors on a subsequent ingestion. __It is highly recommended to change the name of the package__. 
    - `--no-carla-materials` specifies that you do not want to use the default CARLA materials (road textures etc).

### Customize the map

Before generating the pedestrian navigation, you may want to customize the map in the editor. Props such as trees, streetlights or grass zones can be added. Customization should be completed before generating the pedestrian navigation in order to avoid interference or collisions between the two. Otherwise it may be required to generate the pedestrian navigation a second time.  

* __Create new spawning points__. These spawning points will be used in scripts such as `spawn_npc.py`:
    - In the editor, go to the `Modes` panel and search for "spawn" in the `Search Classes` search bar.
    - Choose the type of spawning point you would like to create and drag it on to the map.
    - Place the spawn point about 0.5/1m above the ground to prevent collisions with the road.
*   __Add crosswalk meshes to crosswalks defined in the `.xodr` file.__ Crosswalks defined inside the `.xodr` file remain in the logic of the map, but are not visible. For each of them, create a plane mesh that extends a bit over both sidewalks connected. __Place it overlapping the ground, and disable its physics and rendering__. 
* __Generate new crosswalks__ by changing the name of the mesh to `Road_Crosswalk` or `Roads_Crosswalk`. Avoid doing this if the crosswalk is already defined the `.xodr` file. This will lead to duplication. 

![ue_crosswalks](../img/ue_crosswalks.jpg)  



### Generate pedestrian navigation

__1.__ To prevent the map being too large to export, select the __BP_Sky object__ and add a tag `NoExport` to it. If you have any other particularly large meshes that are not involved in the pedestrian navigation, add the `NoExport` tag to them as well. 

![ue_skybox_no_export](../img/ue_noexport.png) 

__2.__ Check the name of the meshes. By default, pedestrians will be able to walk over sidewalks, crosswalks, and grass (with minor influence over the rest).  

*   Sidewalk = `Road_Sidewalk` or `Roads_Sidewalk`.  
*   Crosswalk = `Road_Crosswalk` or `Roads_Crosswalk`.  
*   Grass = `Road_Grass` or `Roads_Grass`.  

![ue_meshes](../img/ue_meshes.jpg) 

__3.__ Name these planes following the conventional format of `Road_Crosswalk_<mapName>` or `Roads_Crosswalk_<mapName>`. 

__4.__ Press `ctrl + A` to select everything and export the map. `File` -> `Carla Exporter`. A `<mapName>.obj` file will be created in `Unreal/CarlaUE4/Saved`.

__5.__ Move the `<mapName>.obj` and the `<mapName>.xodr` to `Util/DockerUtils/dist`.  

__6.__ Run the following command to generate the navigation file:  

*   __Windows__ 
```sh
build.bat <mapName> # <mapName> has no extension
```
*   __Linux__
```sh
./build.sh <mapName> # <mapName> has no extension
```

__7.__ A `<mapName>.bin` file will be created. Move this file to the `Nav` folder of the package that contains the map.  

---

Your map is now ready to run simulations in CARLA. 