# Deprecated ways to import a map

This section explains methods that were used to import maps in previous versions of CARLA. These methods are now deprecated but explained here for those who are using older versions of CARLA. 


!!! Warning
    __These importing methods are deprecated.__ Click [here][ingest_overview] find the latest and recommended methods to ingest a map.  

[ingest_overview]: ../tuto_A_add_map_overview.md

- [__Manual package preparation__](#manual-package-preparation)
- [__RoadRunner plugin import__](#roadrunner-plugin-import)
- [__Manual import__](#manual-import)
- [__Set traffic and pedestrian behaviour__](#set-traffic-and-pedestrian-behaviour)

---

## Manual package preparation

A map package needs to follow a certain folder structure and must contain a `.json` file describing that folder structure. These steps can be saved under certain circumstances, but doing it manually will always work. 
#### Create the folder structure

__1. Create a folder inside `carla/Import`.__ The name of the folder is not important.  

__2. Create different subfolders__ for each map to be imported.

__3. Move the files of each map to the corresponding subfolder.__ A subfolder will contain a specific set of elements: 

*   The mesh of the map in a `.fbx` file.  
*   The OpenDRIVE definition in a `.xodr` file.  
*   Optionally, the textures required by the asset.  

For instance, an `Import` folder with one package containing two maps should have a structure similar to the one below.

```sh
Import
│
└── Package01
  ├── Package01.json
  ├── Map01
  │   ├── Asphalt1_Diff.jpg
  │   ├── Asphalt1_Norm.jpg
  │   ├── Asphalt1_Spec.jpg
  │   ├── Grass1_Diff.jpg
  │   ├── Grass1_Norm.jpg
  │   ├── Grass1_Spec.jpg
  │   ├── LaneMarking1_Diff.jpg
  │   ├── LaneMarking1_Norm.jpg
  │   ├── LaneMarking1_Spec.jpg
  │   ├── Map01.fbx
  │   └── Map01.xodr
  └── Map02
      └── Map02.fbx
```

#### Create the JSON description

Create a `.json` file in the root folder of the package. Name the file after the package. Note that this will be the distribution name. The content of the file will describe a JSON array of __maps__ and __props__ with basic information for each of them.  

__Maps__ need the following parameters:  

* __name__ of the map. This must be the same as the `.fbx` and `.xodr` files.  
* __source__ path to the `.fbx` file.  
* __use_carla_materials__. If __True__, the map will use CARLA materials. Otherwise, it will use RoadRunner materials.  
* __xodr__ Path to the `.xodr` file.  

__Props__ are not part of this tutorial. The field will be left empty. There is another tutorial on how to [add new props](../tuto_A_add_props.md).  

The resulting `.json` file should resemble the following:

```json
{
  "maps": [
    {
      "name": "Map01",
      "source": "./Map01/Map01.fbx",
      "use_carla_materials": true,
      "xodr": "./Map01/Map01.xodr"
    },
    {
      "name": "Map02",
      "source": "./Map02/Map02.fbx",
      "use_carla_materials": false,
      "xodr": "./Map02/Map02.xodr"
    }
  ],
  "props": [
  ]
}
```
</details>
<br>

---

## RoadRunner plugin import

This software provides specific plugins for CARLA. Get those and follow some simple steps to get the map.  

#### Plugin installation

These plugins will set everything ready to be used in CARLA. It makes the import process simpler. 

__1. Locate the plugins__ in RoadRunner's installation folder  
`/usr/bin/VectorZero/Tools/Unreal/Plugins`.

__2. Copy those folders__ to the CarlaUE4 plugins directory `/carla/Unreal/CarlaUE4/Plugins/`.

__3. Rebuild the plugin__ following the instructions below.  

*   __a) Rebuild on Windows.__  
	* Right-click the `.uproject` file and `Generate Visual Studio project files`.  
	* Open the project and build the plugins.  

*   __b) Rebuild on Linux.__  
	* Run the following command.  
```sh
> UE4_ROOT/GenerateProjectFiles.sh -project="carla/Unreal/CarlaUE4/CarlaUE4.uproject" -game -engine
```

__4. Restart Unreal Engine.__ Make sure the checkbox is on for both plugins `Edit > Plugins`. 

![rr_ue_plugins](../img/rr-ue4_plugins.jpg)

### Import map

__1. Import the `mapname.fbx` file__ to a new folder under `/Content/Carla/Maps` with the `Import` button.  

![ue_import](../img/ue_import_mapname.jpg)

__2. Set `Scene > Hierarchy Type`__ to _Create One Blueprint Asset_ (selected by default).  
__3. Set `Static Meshes > Normal Import Method`__ to _Import Normals_.  

![ue_import_options](../img/ue_import_options.jpg)

__4. Click `Import`.__  
__5. Save the current level__ `File` -> `Save Current As...` -> `<mapname>`.  

The new map should now appear next to the others in the Unreal Engine _Content Browser_.

![ue_level_content](../img/ue_level_content.jpg)
</details>

!!! Note
    The tags for semantic segmentation will be assigned by the name of the asset. And the asset moved to the corresponding folder in `Content/Carla/PackageName/Static`. To change these, move them manually after importing. 

---

## Manual import 

This is the generic way to import maps into Unreal Engine using any _.fbx_ and _.xodr_ files. As there is no plugin to ease the process, there are many settings to be done before the map is available in CARLA.  

__1. Create a new level__ with the **Map** name in Unreal `Add New > Level` under `Content/Carla/Maps`.  
__2. Copy the illumination folder and its content__ from the BaseMap `Content/Carla/Maps/BaseMap`, and paste it in the new level. Otherwise, the map will be dark.

![ue_illumination](../img/ue_illumination.jpg)

#### Import binaries

__1. Import the `mapname.fbx` file__ to a new folder under `/Content/Carla/Maps` with the `Import` button. __Make sure the following options are unchecked:__

*   Auto Generate Collision  
*   Combine Meshes  
*   Force Front xAxis  
*   Normal Import Method - _To import normals_  

__2. Check the following options:__  

*   Convert Scene Unit
*   _To import materials and textures:_
    *   Material Import Method - _To create new materials_
    *   Import Textures

![ue_import_file](../img/ue_import_file.jpg)

__3. Check that the static meshes have appeared__ in the chosen folder.

__4. Drag the meshes__ into the level.

![ue_meshes](../img/ue_drag_meshes.jpg)

__5. Center the meshes at point (0,0,0)__ when Unreal finishes loading. 

![Transform_Map](../img/transform.jpg)

__6. Generate collisions__. Otherwise, pedestrians and vehicles will fall into the abyss.

*   Select the meshes meant to have colliders.
*   Right-click `Asset Actions > Bulk Edit via Property Matrix...`.

  ![ue_selectmesh_collision](../img/ue_selectmesh_collision.jpg)

*   Search for _collision_ in Property's Matrix search box.
*   Change `Collision complexity` from `Project Default` to `Use Complex Collision As Simple`.

  ![ue_collision_complexity](../img/ue_collision_complexity.jpg)

*   Go to `File > Save All`.

__7. Move the static meshes__ from `Content/Carla/Maps/mapfolder` to the corresponding `Carla/Static` subsequent folder. This will be meaningful for the semantic segmentation ground truth.  

  *   `Terrain/mapname`
  *   `Road/mapname`
  *   `RoadLines/mapname`

```sh
Content
└── Carla
    ├── Blueprints
    ├── Config
    ├── Exported Maps
    ├── HDMaps
    ├── Maps
    └── Static
        ├── Terrain
        │   └── mapname
        │       └── Static Meshes
        │
        ├── Road
        │   └── mapname
        │       └── Static Meshes
        │
        ├── RoadLines  
        |   └── mapname
        |       └── Static Meshes
        └── Sidewalks  
            └── mapname
                └── Static Meshes
```

![ue__semantic_segmentation](../img/ue_ssgt.jpg)

#### Import OpenDRIVE files

__1. Copy the `.xodr` file__ inside the `Content/Carla/Maps/OpenDrive` folder.  
__2. Open the Unreal level.__ Drag the _Open Drive Actor_ inside the level. It will read the level's name. Search the Opendrive file with the same name and load it.

![ue_opendrive_actor](../img/ue_opendrive_actor.jpg)

</details>
<br>

---

## Set traffic and pedestrian behaviour

This software provides specific plugins for CARLA. Get those and follow some simple steps to get the map.  

#### Set traffic behavior

Once everything is loaded into the level, it is time to create traffic behavior.  

__1. Click on the _Open Drive Actor_.__  
__2. Check the following boxes in the same order.__  

*   Add Spawners.  
*   _(Optional for more spawn points)_ On Intersections.  
*   Generate Routes.  

This will generate a series of _RoutePlanner_ and _VehicleSpawnPoint_ actors. These are used for vehicle spawning and navigation.

#### Traffic lights and signs

Traffic lights and signs must be placed all over the map.

__1. Drag traffic light/sign actors__ into the level and place them.  
__2. Adjust the [`trigger volume`][triggerlink]__ for each of them. This will determine their area of influence.  

[triggerlink]: ../python_api.md#carla.TrafficSign.trigger_volume

![ue_trafficlight](../img/ue_trafficlight.jpg)

__3. In junctions, drag a traffic light group actor__ into the level. Assign to it all the traffic lights involved and configure their timing. Make sure to understand [how traffic lights work](http://127.0.0.1:8000/core_actors/#traffic-signs-and-traffic-lights).  

![ue_tl_group](../img/ue_tl_group.jpg)

__4. Test traffic light timing and traffic trigger volumes.__ This may need trial and error to fit perfectly.

![ue_tlsigns_example](../img/ue_tlsigns_example.jpg)

> _Example: Traffic Signs, Traffic lights and Turn based stop._

---
#### Add pedestrian navigation

In order to prepare the map for pedestrian navigation, there are some settings to be done before exporting it.  

__1.__ Select the __Skybox object__ and add a tag `NoExport` to it. Otherwise, the map will not be exported, as the size would be too big. Any geometry that is not involved or interfering in the pedestrian navigation can be tagged also as `NoExport`.

![ue_skybox_no_export](../img/ue_noexport.jpg) 

__2.__ Check the name of the meshes. By default, pedestrians will be able to walk over sidewalks, crosswalks, and grass (with minor influence over the rest).

![ue_meshes](../img/ue_meshes.jpg) 

__3.__ Crosswalks have to be manually created. For each of them, create a plane mesh that extends a bit over both sidewalks connected. __Place it overlapping the ground, and disable its physics and rendering__. 

![ue_crosswalks](../img/ue_crosswalks.jpg) 

__4.__ Name these planes following the common format `Road_Crosswalk_mapname`. 

__5.__ Press `G` to deselect everything, and export the map. `File > Export CARLA...`.  
__6.__ Run RecastDemo `./RecastDemo`.  

  * Select `Solo Mesh` from the `Sample` parameter's box.
  * Select the `mapname.obj` file from the `Input Mesh` parameter's box.
![recast_example](../img/recast_example.jpg)

__7.__ Click on the `Build` button.  
__8.__ Once the build has finished, click on the `Save` button.  
__9.__ Change the **filename** of the binary file generated at `RecastDemo/Bin` to `mapname.bin`.  
__10.__ Drag the _mapname.bin_ file into the `Nav` folder under `Content/Carla/Maps`.  
