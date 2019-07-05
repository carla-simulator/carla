<h1>Importing and exporting maps and props for distribution builds</h1>

*Please note that we will use the term *assets* for refering to **props** and also **maps**.* 

The main objective for importing and exporting assets is to reduce the size of the distribution build. This is possible since these assets will be imported as independent packages that can be plugged in anytime inside Carla and also exported. 

<h4>How to import assets inside Unreal Engine</h4>

The first step is to create an empty folder inside the Carla `Import` folder and rename it with the name of the package desired. For simplifying the package folder structure,we recommend to have as many subfolders as maps to import and one single subfolder containing all the props to import. Inside each subfolder, we will place all the files needed for importing. 
So basically, for a **map** subfolder, we will need to place the following files:
- The map itself in `.FBX` format.
- Optionally, the textures required by the map.
- Optionally, the `.xodr` OpenDrive file corresponding to that map.

And for the **props** folder, we will need the following files:
- The prop itself in `.FBX` format.
- Optionally, the textures required by the prop.

Therefore, the package folder should have this similar structure:
```
Import
|
├── Package1
|   ├── MapToImport01
│   │   ├── Asphalt1_Diff.png
│   │   ├── Asphalt1_Norm.png
│   │   ├── Asphalt1_Spec.png
│   │   ├── Grass1_Diff.png
│   │   ├── Grass1_Norm.png
│   │   ├── Grass1_Spec.png
│   │   ├── LaneMarking1_Diff.png
│   │   ├── LaneMarking1_Norm.png
│   │   ├── LaneMarking1_Spec.png
│   │   ├── MapToImport01.fbx
│   │   └── MapToImport01.xodr
|   └── MapToImport02
│   │   ├── MapToImport02.fbx
│   │   └── MapToImport02.xodr
|   └── AssetsToImport
|       ├── PropToImport01
│       │   ├── PropToImport01_Diff.png
│       │   ├── PropToImport01_Norm.png
│       │   ├── PropToImport01_Spec.png
│       │   └── PropToImport01.fbx
│       ├── PropToImport02
│       │   └── PropToImport02.fbx
|       └── PropToImport03
│           └── PropToImport03.fbx
└── Package2
    └── AssetsToImport
        └── PropToImport04
            └── PropToImport04.fbx
```

Once set this project structure, a JSON file needs to be created for each package that will contain necessary information about the assets inside it. The format of this file must be `.json`. We recommend for the JSON file to have the same name as the package name in order to keep it organized.

The content of this JSON file should be similar to the following:
```json
{
  "maps": [{
      "name": "MyTown01",
      "source": "./Town01/Town01.fbx",
      "use_carla_materials": true,
      "xodr": "./Town01/Town01.xodr"
    },
    {
      "name": "MyTown02",
      "source": "./Town02/Town02.fbx",
      "use_carla_materials": true,
      "xodr": "./Town02/Town02.xodr"
    }
  ],
  "props": [{
      "name": "MyProp01",
      "size": "medium",
      "source": "./MyProp01Mesh.fbx",
      "tag": "SegmentationTag01"
    },
    {
      "name": "MyProp02",
      "size": "small",
      "source": "MyProp02Mesh.fbx",
      "tag": "SegmentationTag02"
    }
  ]
}
```

As you can observe in the JSON file content, we have defined a JSON array of **maps** and **props**.

Each item inside the **maps** array has the following parameters:
- *name*: The name of the map. It is possible to import the same map mesh but with different name and, if so, it will create separate mesh files inside Unreal for each different name.
- *source*: Source path of the map inside the package folder.
- *use_carla_materials*: If true, we will use Carla materials, otherwise, we will use RoadRunner materials.
- *xodr*: Path to the `.xodr` Opendrive file for that map.

And each item inside the **props** array has the following parameters:
- *name*: The name of the prop. It is possible to import the same prop mesh but with different name and, if so, it will create separate mesh files inside Unreal for each different name.
- *source*: Source path of the prop inside the package folder.
- *size*: Size of the prop, possible values are: 
    - `tiny`
    - `small`
    - `medium`
    - `big`
    - `huge`
- *tag*: Semantic segmentation tag. Possible values are:
    - `None`
    - `Buildings`
    - `Fences`
    - `Other`
    - `Pedestrians`
    - `Poles`
    - `RoadLines`
    - `Roads`
    - `Sidewalks`
    - `TrafficSigns`
    - `Vegetation`
    - `Vehicles`
    - `Walls`
    Note that if the tag is not spelled correctly, it will interpret it as `None` value by default.

Now we have everything ready for importing the packages. To do so, you just need to run the command:
```sh
make import
```
This command will read the JSON file and take each asset and place it inside the Content in Unreal Engine. Furthermore, it will create a `Package1.Package.json` file inside the recently imported package folder that will be used for **defining** its props in the blueprint library, **exposing** them in the `PythonAPI` and also for **exporting** those assets if needed. If a package was already imported before, it will overwrite it.


<h4>How to export assets</h4>

Once imported all the packages inside Unreal, users could also generate a **cooked package** for each of them. This last step is important in order to have a packages ready to add for distribution versions of Carla and for any supported platform. To export the packages, simply run the command:

```sh
make package ARGS="--packages=Package1,Package2"
```

This command will create the distribution packages compressed in a `.tar.gz` file for each cooked package and place it in the Carla `Dist` folder. Finally, you could use these distribution packages in the Carla distribution packages by simply moving them in the Carla `Import` folder and executing the `ImportAssets.sh` script.