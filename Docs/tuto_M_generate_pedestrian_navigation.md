# How to generate the pedestrian navigation info

To allow pedestrians to navigate a map, you will need to generate a pedestrian navigation file. This guide details what meshes to use and how to generate the file.

- [__Before you begin__](#before-you-begin)
- [__Pedestrian navigable meshes__](#pedestrian-navigable-meshes)
- [__Generate the pedestrian navigation__](#generate-the-pedestrian-navigation)

---

## Before you begin

Before generating the pedestrian navigation, you may want to customize the map in the editor. Props such as trees, streetlights or grass zones can be added. Customization should be completed before generating the pedestrian navigation in order to avoid interference or collisions between the two, resulting in the need to generate the pedestrian navigation a second time.  

* __Create new spawning points__. These spawning points will be used in scripts such as [`spawn_npc.py`](https://github.com/carla-simulator/carla/blob/master/PythonAPI/examples/spawn_npc.py):
    - In the editor, go to the `Modes` panel and search for "spawn" in the `Search Classes` search bar.
    - Choose the type of spawning point you would like to create and drag it on to the map.
    - Place the spawn point about 0.5-1m above the ground to prevent collisions with the road.
* __Generate new crosswalks__. Avoid doing this if the crosswalk is already defined the `.xodr` file as this will lead to duplication:
    - Create a plane mesh that extends a bit over two sidewalks that you want to connect. 
    - Place the mesh overlapping the ground and disable it's physics and rendering. 
    - Change the name of the mesh to `Road_Crosswalk` or `Roads_Crosswalk`.  

![ue_crosswalks](img/ue_crosswalks.jpg)  

---

## Pedestrian navigable meshes

Pedestrians can only navigate specific meshes. You need to name the meshes you want to include in pedestrian navigation according to the nomenclature in the table below:

| Type | Name includes | Description |
|------|------------|-------------|
| Ground | `Road_Sidewalk` or `Roads_Sidewalk` | Pedestrians will walk over these meshes freely. |
| Crosswalk  | `Road_Crosswalk` or `Roads_Crosswalk` | Pedestrians will walk over these meshes as a second option if no ground is found. |
| Grass | `Road_Grass` or `Roads_Grass` | Pedestrians won't walk on this mesh unless you specify a percentage of them to do so. |
| Road | `Road_Road` or `Roads_Road` <br> `Road_Curb` or `Roads_Curb` <br> `Road_Gutter` or `Roads_Gutter` <br> `Road_Marking` or `Roads_Marking` | Pedestrians will only cross roads through these meshes. |

<br>

---

## Generate the pedestrian navigation

__1.__ To prevent the map being too large to export, select the __BP_Sky object__ and add a tag `NoExport` to it. If you have any other particularly large meshes that are not involved in the pedestrian navigation, add the `NoExport` tag to them as well. 

![ue_skybox_no_export](img/ue_noexport.png) 

__2.__ Double check your mesh names. Mesh names should start with any of the appropriate formats listed below in order to be recognized as areas where pedestrians can walk. By default, pedestrians will be able to walk over sidewalks, crosswalks, and grass (with minor influence over the rest):  

*   Sidewalk = `Road_Sidewalk` or `Roads_Sidewalk` 
*   Crosswalk = `Road_Crosswalk` or `Roads_Crosswalk` 
*   Grass = `Road_Grass` or `Roads_Grass`

![ue_meshes](img/ue_meshes.jpg) 

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
