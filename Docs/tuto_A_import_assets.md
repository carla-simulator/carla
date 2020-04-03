
# Import new assets

Assets in CARLA is an umbrella term that includes both maps, and props. The simulator can ingest new assets anytime in a simple process. This not only includes different distribution packages, but also assets developed by the users. 

* [__Prepare the package__](#prepare-the-package)  
	*   [Create the folder structure](#create-the-folder-structure)  
	*   [Create the JSON description](#create-the-json-description)  
* [__Import into CARLA__](#import-into-carla)  
	*   [Via Docker](#via-docker)  
	*   [Via terminal](#via-terminal)  

---
## Prepare the package

### Create the folder structure

__1. Create a folder inside `carla/Import`.__ The name of the folder is not relevant.  

__2. Create different subfolders__ for each maps to import, and one subfolder for the rest of props.  
&nbsp;&nbsp;&nbsp;__2.1__ Inside the props subfolder, create as many subfolders as props to import. 

__3. Move the files of each asset to the corresponding subfolder.__ A subfolder will contain a specific set of elements.  

*   The mesh of the asset in a `.fbx`.  
*   If the asset is a map, the OpenDRIVE definition in a `.xodr`.  
*   Optionally, the textures required by the asset.  


For instance, an `Import` folder with two separate packages should have a structure similar to the one below.

```sh
Import
│
├── Package01
│   ├── Package01.json
│   ├── Map01
│   │   ├── Asphalt1_Diff.png
│   │   ├── Asphalt1_Norm.png
│   │   ├── Asphalt1_Spec.png
│   │   ├── Grass1_Diff.png
│   │   ├── Grass1_Norm.png
│   │   ├── Grass1_Spec.png
│   │   ├── LaneMarking1_Diff.png
│   │   ├── LaneMarking1_Norm.png
│   │   ├── LaneMarking1_Spec.png
│   │   ├── Map01.fbx
│   │   └── Map01.xodr
│   ├── MapToImport02
│   │   ├── MapToImport02.fbx
│   │   └── MapToImport02.xodr
│   └── Props
│       ├── Prop01
│       │   ├── PropToImport01_Diff.png
│       │   ├── PropToImport01_Norm.png
│       │   ├── PropToImport01_Spec.png
│       │   └── Prop01.fbx
│       └── Prop02
│           └── Prop02.fbx
└── Package02
    ├── Packag02.json
    └── Props
        └── Prop03
            └── PropToImport03.fbx
```

### Create the JSON description

Create a `.json` file in the root folder of the package. Name the file after the package. Note that this will be the distribution name. The content of the file will describe a JSON array of __maps__ and __props__ with basic information for each of them.  

__Maps__ need the following parameters.  

* __name__ of the map. This must be the same as the `.fbx` and `.xodr` files.  
* __source__ path to the `.fbx`.  
* __use_carla_materials__. If __True__, the map will use CARLA materials. Otherwise, it will use RoadRunner materials.  
* __xodr__ Path to the `.xodr`.  

__Props__ need the following parameters.  

*   __name__ of the prop. This must be the same as the `.fbx`.  
*   __source__ path to the `.fbx`.  
*   __size__ estimation of the prop. The possible values are listed here.  
	*   `tiny`  
	*   `small`  
	*   `medium`  
	*   `big`  
	*   `huge`  
*   __tag__ value for the semantic segmentation. If the tag is misspelled, it will be read as `None`. 
	*   `None`
	*   `Buildings`
	*   `Fences`
	*   `Other`
	*   `Pedestrians`
	*   `Poles`
	*   `RoadLines`
	*   `Roads`
	*   `Sidewalks`
	*   `TrafficSigns`
	*   `Vegetation`
	*   `Vehicles`
	*   `Walls`

In the end, the `.json` should look similar to the one below.

```json
{
  "maps": [
    {
      "name": "MyTown01",
      "source": "./MapToImport01/MyTown01.fbx",
      "use_carla_materials": true,
      "xodr": "./MapToImport01/MyTown01.xodr"
    },
    {
      "name": "MyTown02",
      "source": "./MapToImport02/MyTown02.fbx",
      "use_carla_materials": false,
      "xodr": "./MapToImport02/MyTown02.xodr"
    }
  ],
  "props": [
    {
      "name": "MyProp01",
      "size": "medium",
      "source": "./AssetsToImport/PropToImport01/MyProp01.fbx",
      "tag": "SemanticSegmentationTag01"
    },
    {
      "name": "MyProp02",
      "size": "small",
      "source": "./AssetsToImport/PropToImport02/MyProp02.fbx",
      "tag": "SemanticSegmentationTag02"
    }
  ]
}
```
---
## Import into CARLA

!!! Warning
    Packages with the same name will produce an error.  

### Via Docker

This is the recommended option. The package will be ingested from start to finish. That means that not only will the package be imported the same way the terminal method would do. 
A [standalone package](tuto_A_standalone_packages.md) will be exported, to facilitate distribution. Besides that, this method presents remarkable advantages for new maps.  

* __Pedestrian navigation__ will be generated automatically.  
* __Traffic lights and traffic lights__ will be created when running the simulation using the information in the `.xodr`.  

That means that importing assets, specially maps, will become and out-of-the-box process.  

__1. Build a Docker image of Unreal Engine.__ Follow [these instructions](https://github.com/carla-simulator/carla/tree/master/Util/Docker). 

__2. Run the script to cook the map.__ In the folder `~/carla/Util/Docker` there is a script that connects with the Docker image previously created, and makes the ingestion automatically. It only needs the path for the input and output files.  

```sh
python docker_tools.py --input ~/path_to_package --output ~/path_for_output_assets
```

__3. Check the output files__. They should be in `Content/Carla/type_of_asset`. Move them in case they are not already there. 

!!! Note
    The Docker image takes 4h and 400GB to be built. However, this is only needed the first time. 

### Via terminal 

This option will read the JSON file, and place the assets inside the `Content` in Unreal Engine. Furthermore, it will create a `Package1.Package.json` file inside the package's
`Config` folder. This will be used to define the props in the blueprint library, expose them in the Python API, and also to export them in a [standalone package](tuto_A_standalone_packages.md) if needed.

When everything is ready, run the command. 

```sh
make import
```

Not using the Docker has some disadvantages when importing maps. There will be a few things to do after the import.  

#### Generate map collisions

This is mandatory, otherwise, pedestrians and vehicles will fall into the abyss.

* Select the meshes meant to have colliders.
* Right-click `Asset Actions > Bulk Edit via Property Matrix...`.

  ![ue_selectmesh_collision](img/ue_selectmesh_collision.png)

* Search for _collision_ in Property's Matrix search box.
* Change `Collision complexity` from `Project Default` to `Use Complex Collision As Simple`.

  ![ue_collision_complexity](img/ue_collision_complexity.png)

* Go to `File > Save All`.

#### Generate pedestrian navigation

In order to prepare the map for pedestrian navigation, a `.bin` file must be generated. This needs. some settings to be done.  

__1.__ Select the __Skybox object__ and add a tag `NoExport` to it. Otherwise, the map will not be exported, as the size would be too big. 

![ue_skybox_no_export](img/ue_noexport.png) 

__2.__ Check the name of the meshes. By default, pedestrians will be able to walk over sidewalks, crosswalks, and grass (with minor influence over the rest).

![ue_meshes](img/ue_meshes.png) 

__3.__ Crosswalks have to be manually created. For each of them, create a plane mesh that extends a bit over both sidewalks connected. __Place it overlapping the ground, and disable its physics and rendering__. 

![ue_crosswalks](img/ue_crosswalks.png) 

__4.__ Name these planes following the common format `Road_Crosswalk_mapname`. 

__5.__ Press `G` to deselect everything, and export the map. `File > Export CARLA...`.  
__6.__ Run RecastDemo `./RecastDemo`.  

  * Select `Solo Mesh` from the `Sample` parameter's box.
  * Select the _mapname.obj_ file from the `Input Mesh` parameter's box.
![recast_example](img/recast_example.png)

__7.__ Click on the `Build` button.  
__8.__ Once the build has finished, click on the `Save` button.  
__9.__ Change the **filename** of the binary file generated at `RecastDemo/Bin` to `mapname.bin`.  
__10.__ Drag the _mapname.bin_ file into the `Nav` folder under `Content/Carla/Maps`.  
