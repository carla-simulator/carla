# Manual package preparation

A map package follows a certain folder structure and must contain a `.json` file describing that structure. Our automatic map import processes create this `.json` file automatically, but you also have the option to prepare it yourself. Including your own `.json` file will overwrite any arguments passed to the `make import` command.

- [__Standard Maps__](#standard-maps)
    - [Create the folder structure for the standard maps](#create-the-folder-structure-for-the-standard-maps)
    - [Create the JSON description for the standard maps](#create-the-json-description-for-the-standard-maps)
- [__Large Maps__](#large-maps)
    - [Create the folder structure for the large maps](#create-the-folder-structure-for-the-large-maps)
    - [Create the JSON description for the large maps](#create-the-json-description-for-the-large-maps)

---

## Standard maps
### Create the folder structure for the standard maps

__1. Create a folder inside `carla/Import`.__ The name of the folder is not important.  

__2. Create different subfolders__ for each map to be imported.

__3. Move the files of each map to the corresponding subfolder.__ A subfolder will contain a specific set of elements: 

-   The mesh of the map in a `.fbx` file.  
-   The OpenDRIVE definition in a `.xodr` file.  
-   Optionally, the textures required by the asset.  

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

---

### Create the JSON description for the standard maps

Create a `.json` file in the root folder of the package. Name the file after the package. Note that this will be the distribution name. The content of the file will describe a JSON array of __maps__ and __props__ with basic information for each of them.  

__Maps__ need the following parameters:  

- __name__ of the map. This must be the same as the `.fbx` and `.xodr` files.  
- __source__ path to the `.fbx` file.  
- __use_carla_materials__. If __True__, the map will use CARLA materials. Otherwise, it will use RoadRunner materials.  
- __xodr__ Path to the `.xodr` file.  

__Props__ are not part of this tutorial. The field will be left empty. There is another tutorial on how to [add new props](tuto_A_add_props.md).  

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

If you have any questions about the process, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>