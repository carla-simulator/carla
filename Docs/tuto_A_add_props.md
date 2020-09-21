# Add new props

Props are the assets that populate the scene, besides the map, and the vehicles. That includes streetlights, buildings, trees, and much more. The simulator can ingest new props anytime in a simple process. This is really useful to create customized environments in a map. 

* [__Prepare the package__](#prepare-the-package)  
	*   [Create the folder structure](#create-the-folder-structure)  
	*   [Create the JSON description](#create-the-json-description)  
*   [__Ingestion in a CARLA package__](#ingestion-in-a-carla-package)  
*   [__Ingestion in a build from source__](#ingestion-in-a-build-from-source)  

---
## Prepare the package

### Create the folder structure

__1. Create a folder inside `carla/Import`.__ The name of the folder is not relevant.  

__2. Create the subfolders.__ There should be one general subfolder for all the props, and inside of it, as many subfolders as props to import. 

__3. Move the files of each prop to the corresponding subfolder.__ A prop subfolder will contain the `.fbx` mesh, and optionally, the textures required by it.  

For instance, an `Import` folder with two separate packages should have a structure similar to the one below.

```sh
Import
│
├── Package01
│   ├── Package01.json
│   └── Props
│       ├── Prop01
│       │   ├── Prop01_Diff.png
│       │   ├── Prop01_Norm.png
│       │   ├── Prop01_Spec.png
│       │   └── Prop01.fbx
│       └── Prop02
│           └── Prop02.fbx
└── Package02
    ├── Packag02.json
    └── Props
        └── Prop03
            └── Prop03.fbx
```

### Create the JSON description

Create a `.json` file in the root folder of the package. Name the file after the package. Note that this will be the distribution name. The content of the file will describe a JSON array of __maps__ and __props__ with basic information for each of them.  

__Maps__ are not part of this tutorial, so this definition will be empty. There is a specific tutorial to [__add a new map__](tuto_A_add_map.md).  

__Props__ need the following parameters.  

*   __name__ of the prop. This must be the same as the `.fbx`.  
*   __source__ path to the `.fbx`.  
*   __size__ estimation of the prop. The possible values are listed here.  
	*   `tiny`  
	*   `small`  
	*   `medium`  
	*   `big`  
	*   `huge`  
*   __tag__ value for the semantic segmentation. If the tag is misspelled, it will be read as `Unlabeled`. 
	*   `Bridge`
	*   `Building`
	*   `Dynamic`
	*   `Fence`
	*   `Ground`
	*   `GuardRail`
	*   `Other`
	*   `Pedestrian`
	*   `Pole`
	*   `RailTrack`
	*   `Road`
	*   `RoadLine`
	*   `SideWalk`
	*   `Sky`
	*   `Static`
	*   `Terrain`
	*   `TrafficLight`
	*   `TrafficSign`
	*   `Unlabeled`
	*   `Vegetation`
	*   `Vehicles`
	*   `Wall`
	*   `Water`

In the end, the `.json` should look similar to the one below.

```json
{
  "maps": [
  ],
  "props": [
    {
      "name": "MyProp01",
      "size": "medium",
      "source": "./Props/Prop01/Prop01.fbx",
      "tag": "SemanticSegmentationTag01"
    },
    {
      "name": "MyProp02",
      "size": "small",
      "source": "./Props/Prop02/Prop02.fbx",
      "tag": "SemanticSegmentationTag02"
    }
  ]
}
```
!!! Warning
    Packages with the same name will produce an error.  

---
## Ingestion in a CARLA package

This is the method used to ingest the props into a CARLA package such as CARLA 0.9.8.  

A Docker image of Unreal Engine will be created. It acts as a black box that automatically imports the package into the CARLA image, and generates a ditribution package. The Docker image takes 4h and 400GB to be built. However, this is only needed the first time. 

__1. Build a Docker image of Unreal Engine.__ Follow [these instructions](https://github.com/carla-simulator/carla/tree/master/Util/Docker) to build the image.  

__2. Run the script to cook the props.__ In the folder `~/carla/Util/Docker` there is a script that connects with the Docker image previously created, and makes the ingestion automatically. It only needs the path for the input and output files, and the name of the package to be ingested.  

```sh
python3 docker_tools.py --input ~/path_to_package --output ~/path_for_output_assets  --package=Package01
```

__3. Locate the package__. The Docker should have generated the package `Package01.tar.gz` in the output path. This is the standalone package for the assets. 

__4. Import the package into CARLA.__  

*   __On Windows__ extract the package in the `WindowsNoEditor` folder. 

*   __On Linux__ move the package to the `Import` folder, and run the script to import it. 

```sh
cd Util
./ImportAssets.sh
```

!!! Note
    There is an alternative on Linux. Move the package to the `Import` folder and run the script `Util/ImportAssets.sh` to extract the package.


---
## Ingestion in a build from source

This is the method to import the props into a CARLA build from source.  

The JSON file will be read to place the props inside the `Content` in Unreal Engine. Furthermore, it will create a `Package1.Package.json` file inside the package's `Config` folder. This will be used to define the props in the blueprint library, and expose them in the Python API. It will also be used if the package is exported as a [standalone package](tuto_A_create_standalone.md).

When everything is ready, run the command. 

```sh
make import
```

!!! Warning
    Make sure that the package is inside the `Import` folder in CARLA. 

---

That is all there is to know about the different ways to import new props into CARLA. If there are any doubts, feel free to post these in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>


