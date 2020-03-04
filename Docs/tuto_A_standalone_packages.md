# Creating standalone asset packages for distribution

*Please note that we will use the term *assets* for referring to **props** and also **maps**.*

The main objective for importing and exporting assets is to reduce the size of
the distribution build. This is possible since these assets will be imported as
independent packages that can be plugged in anytime inside Carla and also exported.

---
## How to import assets inside Unreal Engine

The first step is to create an empty folder inside the Carla `Import` folder and rename it with any
folder name desired. For simplifying this newly created folder structure, we recommend having
as many subfolders as maps to import and one single subfolder containing all the props to import.
Inside each subfolder, we will place all the files needed for importing.<br>
So basically, for a **map** subfolder, we will need to place the following files:

* The map itself in `.FBX` format.
* Optionally, the textures required by the map.
* Optionally, the `.xodr` OpenDrive file corresponding to that map.

And for the **props** folder, we will need the following files:

* The prop itself in `.FBX` format.
* Optionally, the textures required by the prop.

Additionally, we have to create a **JSON file inside the package** that will contain information
about its assets. The file extension must be `.json`. We recommend the JSON file to have the same
name as the package name in order to keep it organized.
**Please, keep in mind that the name of this file will used as the name of the distribution package**.

The content of this JSON file should be similar to the following:

```json
{
  "maps": [
    {
      "name": "MyTown01",
      "source": "./MapToImport01/MapToImport01.fbx",
      "use_carla_materials": true,
      "xodr": "./MapToImport01/MapToImport01.xodr"
    },
    {
      "name": "MyTown02",
      "source": "./MapToImport02/MapToImport02.fbx",
      "use_carla_materials": false,
      "xodr": "./MapToImport02/MapToImport02.xodr"
    }
  ],
  "props": [
    {
      "name": "MyProp01",
      "size": "medium",
      "source": "./AssetsToImport/PropToImport01/PropToImport01.fbx",
      "tag": "SemanticSegmentationTag01"
    },
    {
      "name": "MyProp02",
      "size": "small",
      "source": "./AssetsToImport/PropToImport02/PropToImport02.fbx",
      "tag": "SemanticSegmentationTag02"
    }
  ]
}
```

As you can observe in the JSON file content, we have defined a JSON array of **maps** and **props**.

Each item inside the **maps** array has the following parameters:

* **name**: The name of the map. It is possible to import the same map mesh but with a different
name and, if so, it will create separate mesh files inside Unreal for each different name.
* **source**: Source path of the map inside the package folder.
* **use_carla_materials**: If true, we will use Carla materials, otherwise, we will use RoadRunner materials.
* **xodr**: Path to the `.xodr` Opendrive file for that map.

And each item inside the **props** array has the following parameters:

* **name**: The name of the prop. It is possible to import the same prop mesh but with different
name and, if so, it will create separate mesh files inside Unreal for each different name.
* **source**: Source path of the prop inside the package folder.
* **size**: Size of the prop, possible values are:

    - `tiny`
    - `small`
    - `medium`
    - `big`
    - `huge`

- **tag**: Semantic segmentation tag. Possible values are:

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

To sum up, the `Import` folder should have this similar structure:

```
Import
|
├── Folder01
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
|   ├── MapToImport02
│   │   ├── MapToImport02.fbx
│   │   └── MapToImport02.xodr
|   ├── AssetsToImport
|   │   ├── PropToImport01
│   │   │   ├── PropToImport01_Diff.png
│   │   │   ├── PropToImport01_Norm.png
│   │   │   ├── PropToImport01_Spec.png
│   │   │   └── PropToImport01.fbx
│   │   ├── PropToImport02
│   │   │   └── PropToImport02.fbx
|   │   └── PropToImport03
│   │       └── PropToImport03.fbx
|   └── Package1.json
└── Folder02
    ├── AssetsToImport
    |   └── PropToImport04
    |       └── PropToImport04.fbx
    └── Package2.json
```

_Maps exported from **RoadRunner** are also supported for importing them inside_
_Carla. So, basically, once created your **RoadRunner** map, you just need to export it, take the_
_required files and place them following the structure listed above._

!!! note
    **RoadRunner's** imported assets will be classified for **semantic segmentation** based on
    the asset's name (`RoadNode`, `Terrain`, `MarkingNode`) and moved to `Roads`, `Terrain`
    and `RoadLines` respectively, under `Content/Carla/PackageName/Static`.<br>
    _If the process doesn't work due to different names or other issues, you can always move the assets_
    _manually, check this [`tutorial`][importtutorial]_ (_Section 3.2.1 - 6_).

[importtutorial]: tuto_A_map_creation.md#32-importing-from-the-files

Now we have everything ready for importing assets. To do so, you just need to run the command:

```sh
make import
```

This command will read the JSON file and take each asset and place it inside the Content
in Unreal Engine. Furthermore, it will create a `Package1.Package.json` file inside the package's
`Config` folder that will be used for **defining** its props in the Carla blueprint library,
**exposing** them in the `PythonAPI` and also for **exporting** those assets if needed.

_Packages with the same name will produce an error. Delete or rename the package before importing_
_a new one with the same name._

!!! note
    The imported map won't have collisions, so they should be generated manually. This
    [tutorial][collisionlink] (_Section 3.2.1 - 5_) shows how to do it.

[collisionlink]: how_to_make_a_new_map.md/#32-importing-from-the-files

---
## How to export assets

Once imported all the packages inside Unreal, users could also generate a **cooked package**
for each of them. This last step is important in order to have all packages ready to add for
distribution versions of Carla and for any supported platform. To export the packages,
simply run the command:

```sh
make package ARGS="--packages=Package1,Package2"
```

This command will create the distribution package compressed in a `.tar.gz` file for **each**
cooked package and place it in the Carla `Dist` folder. Finally, you could import these packages
in a Carla distribution by simply moving them in the Carla `Import` folder and executing
the `ImportAssets.sh` script.
