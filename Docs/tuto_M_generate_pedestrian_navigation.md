# Generate Pedestrian Navigation

To allow pedestrians to navigate a map, you will need to generate a pedestrian navigation file. This guide details what meshes to use and how to generate the file.

- [__Before you begin__](#before-you-begin)
- [__Pedestrian navigable meshes__](#pedestrian-navigable-meshes)
- [__Optional pedestrian navigation options__](#optional-pedestrian-navigation-options)
- [__Generate the pedestrian navigation__](#generate-the-pedestrian-navigation)

---

## Before you begin

Map customization (adding buildings, painting the road, adding landscape features, etc.) should be completed before generating the pedestrian navigation in order to avoid interference or collisions between the two, resulting in the need to generate the pedestrian navigation a second time.

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

## Optional pedestrian navigation options

The following step is not necessary for generating a pedestrian navigation, but allows you to customize pedestrian activity to a certain extent.

- __Generate new crosswalks__.

Avoid doing this if the crosswalk is already defined the `.xodr` file as this will lead to duplication:

1. Create a plane mesh that extends a bit over two sidewalks that you want to connect.
2. Place the mesh overlapping the ground and disable it's physics and rendering.
3. Change the name of the mesh to `Road_Crosswalk` or `Roads_Crosswalk`.

![ue_crosswalks](../img/ue_crosswalks.jpg)  

---
## Generate the pedestrian navigation

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

__7.__ Test the pedestrian navigation by starting a simulation and running the example script `generate_traffic.py` in `PythonAPI/examples`.

---

If you have any questions about the process, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>