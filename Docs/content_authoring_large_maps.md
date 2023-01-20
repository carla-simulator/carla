# Creating Large Maps for CARLA

Large Maps (like Towns 11 and 12) operate in a different way to standard maps (like Town 10) in CARLA. The map is divided up into tiles, which are subdivisions of the map, that are normally set to be about 1 to 2 km in size. The tiles divide up the map such that only the parts of the map that are needed, i.e. the parts near the ego vehicle and in view of its sensors, are loaded into graphics memory such that the map can be rendered efficiently. The unneeded tiles are dormant and ready to be loaded when required. Normally the tiles next to the tile in the location of the ego vehicle are loaded, but more tiles can be loaded if necessary. This behaviour can be modified in the settings when CARLA is launched.

# Create a Large Map in RoadRunner

RoadRunner is the recommended software to create large maps to be imported into CARLA. This guide outlines how to use RoadRunner for creating Large Maps. 

- [__Build a large map in RoadRunner__](#build-a-large-map-in-roadrunner)
- [__Export a large map in RoadRunner__](#export-a-large-map-in-roadrunner)
- [__Next steps__](#next-steps)
---

## Build a large map in RoadRunner

The specifics of how to build a large map in RoadRunner go beyond the scope of this guide, however, there are video tutorials available in the [RoadRunner documentation][rr_tutorials].

If you are building a large map with elevation, the recommended largest size of the map is 20km by 20km. Maps larger than this may cause RoadRunner to crash on export.

[rr_tutorials]: https://www.mathworks.com/support/search.html?fq=asset_type_name:video%20category:roadrunner/index&page=1&s_tid=CRUX_topnav

---

## Export a large map in RoadRunner

Below is a basic guideline to export your custom large map from RoadRunner.

[exportlink]: https://www.mathworks.com/help/roadrunner/ug/Exporting-to-CARLA.html

Once you have made your map in RoadRunner you will be able to export it. Be aware that __the road layout cannot be modified after it has been exported.__ Before exporting, ensure that:

- The map is centered at (0,0) to ensure the map can be visualized correctly in Unreal Engine.
- The map definition is correct.
- The map validation is correct, paying close attention to connections and geometries.


>>>>![CheckGeometry](../img/check_geometry.jpg)

Once the map is ready, click on the `OpenDRIVE Preview Tool` button to visualize the OpenDRIVE road network and give everything one last check.

>>>>![checkopen](../img/check_open.jpg)

!!! note
    _OpenDrive Preview Tool_ makes it easier to test the integrity of the map. If there are any errors with junctions, click on `Maneuver Tool`, and `Rebuild Maneuver Roads`.

Make sure the full map is selected for export by clicking on the [_World settings tool_](https://www.mathworks.com/help/roadrunner/ref/worldsettingstool.html) and dragging the edges of the blue boundary box to encompass the full area you would like to export. when it's ready, click on _Apply World Changes_.

![world_bounds_settings](img/rr_world_settings.png)

When you are ready to export:

__1.__ Export the `.fbx`:

  - In the main toolbar, select `File` -> `Export` -> `Firebox (.fbx)`

__2.__ In the window that pops up:

>- Check the following options:
    - _Split by Segmentation_: Divides the mesh by semantic segmentation and imroves pedestrian navigation.
    - _Power of Two Texture Dimensions_: Improves performance.
    - _Embed Textures_: Ensures textures are embedded in the mesh.
    - _Export to Tiles_: Choose the size of the tiles. The maximum size that can be used by CARLA is 2000 x 2000.
    - _Export Individual Tiles_: Generates the individual tiles needed for streaming large maps in CARLA.

>>>>>>![export_large_map_fbx](../img/large_map_export_fbx.png)

__3.__ Export the `.xodr`:

  - In the main toolbar, select `File` -> `Export` -> `OpendDRIVE (.xodr)`

!!! Warning
    Make sure that the `.xodr` and the `.fbx` files have the same name.

---

Now you've created your Large Map in Roadrunner, you are ready to import it into CARLA.


# Import/Package a Large Map

Large maps generated in RoadRunner can be imported into the source build of CARLA and packaged for distribution and usage in a CARLA standalone package. The process is very simlar to that of standard maps with the addition of specific nomenclature for tiles and batch importing.

- [__Files and folders__](#files-and-folders)
- [__Create the JSON description (Optional)__](#create-the-json-description-optional)
- [__Making the import__](#making-the-import)
- [__Package a large map__](#package-a-large-map)

---

## Files and folders

All files to be imported should be placed in the `Import` folder of the root CARLA directory. These files should include:

- The mesh of the map in multiple `.fbx` files representing different tiles of the map.
- The OpenDRIVE definition in a single `.xodr` file.

!!! Warning
    You cannot import large maps and standard maps at the same time.

The naming convention of map tiles is very important. Each map tile should be named according to the following convention:

```
<mapName>_Tile_<x-coordinate>_<y-coordinate>.fbx
```

Be aware that a more positive __y coordinate__ refers to a tile lower on the y-axis. For example,`Map01_Tile_0_1` would sit just below `Map01_Tile_0_0`. 

>>>>>>>>![map_tiles](../img/map_tiles.png)

A resulting `Import` folder with a package containing a large map made of four tiles should have a structure similar to the one below:

```sh
Import
│
└── Package01
  ├── Package01.json
  ├── Map01_Tile_0_0.fbx
  ├── Map01_Tile_0_1.fbx
  ├── Map01_Tile_1_0.fbx
  ├── Map01_Tile_1_1.fbx
  └── Map01.xodr

```

!!! Note
    The `package.json` file is not strictly necessary. If there is no `package.json` file created, the automated import process will create one. Find out more about to structure your own `package.json` in the next section.

---

## Create the JSON description (Optional)

The `.json` description is created automatically during the import process, but there is also the option to create one manually. An existing `.json` description will override any values passed as arguments in the import process.

The `.json` file should be created in the root folder of the package. The file name will be the package distribution name. The content of the file describes a JSON array of __maps__ and __props__ with basic information for each one.

__Maps__ need the following parameters:

- __name:__ Name of the map. This must be the same as the `.fbx` and `.xodr` files.
- __xodr:__ Path to the `.xodr` file.
- __use_carla_materials:__ If __True__, the map will use CARLA materials. Otherwise, it will use RoadRunner materials.
- __tile_size:__ The size of the tiles. Default value is 2000 (2kmx2km).
- __tiles:__ A list of the `.fbx` tile files that make up the entire map.

__Props__ are not part of this tutorial. Please see [this](tuto_A_add_props.md) tutorial for how to add new props.

The resulting `.json` file should resemble the following:

```json
{
  "maps": [
      {
        "name": "Map01",
        "xodr": "./Map01.xodr",
        "use_carla_materials": true,
        "tile_size": 2000,
        "tiles": [ 
        "./Map01_Tile_0_0.fbx",
        "./Map01_Tile_0_1.fbx",
        "./Map01_Tile_1_0.fbx",
        "./Map01_Tile_1_1.fbx"
        ]
      }
  ],
  "props": []
}
```
</details>
<br>

---

## Making the import

When all files have been placed in the `Import` folder, run the following command in the root CARLA folder:

```sh
make import
```

Depending on your system, Unreal Engine may consume too much memory to be able to import all files at once. You can choose to import the files in batches of MB by running the command:

```sh
make import ARGS="--batch-size=200"
```

Two more flags exist for the `make import` command:

- `--package=<package_name>` specifies the name of the package. By default, this is set to `map_package`. Two packages cannot have the same name, so using the default value will lead to errors on a subsequent ingestion. __It is highly recommended to change the name of the package__. Use this flag by running the command:

```sh
make import  ARGS="--package=<package_name>"
```

- `--no-carla-materials` specifies that you do not want to use the default CARLA materials (road textures etc). You will use the RoadRunner materials instead. This flag is __only required if you are not__ providing your own [`.json` file](tuto_M_manual_map_package.md). Any value in the `.json` file will override this flag. Use this flag by running the command:

```sh
make import  ARGS="--no-carla-materials"
```

All files will be imported and prepared to be used in the Unreal Editor. The map package will be created in `Unreal/CarlaUE4/Content`. A base map tile, `<mapName>`, will be created as a streaming level for all the tiles. The base tile will contain the sky, weather, and large map actors and will be ready for use in a simulation.

!!! Note
    It is currently not recommended to use the customization tools provided for standard maps in the Unreal Editor, e.g., road painter, procedural buildings, etc.

---

## Package a large map

To package your large map so it can be used in the CARLA standalone package, run the following command:

```sh
make package ARGS="--packages=<mapPackage>"
```

This will create a standalone package compressed in a `.tar.gz` file. The files will be saved in the `Dist` folder on Linux, and `/Build/UE4Carla/` on Windows. They can then be distributed and packaged to use in standalone CARLA packages.

---

If you have any questions about the large map import and packaging process, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>


