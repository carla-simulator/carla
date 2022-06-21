# Ingesting Maps in CARLA Built From Source

This section describes the process of ingesting maps into __CARLA that has been built from source__. If you are using a package (binary) version of CARLA to ingest maps then follow the guidelines [here][package_ingest] instead.

The ingestion process involves importing the relevant map files by compiling them into a package. This package can then be opened in the Unreal Engine editor and customized before generating the pedestrian navigation file and finally adding it to the package. 

[package_ingest]: tuto_M_add_map_package.md

- [__Before you begin__](#before-you-begin)
- [__Map ingestion__](#map-ingestion)
- [__Next steps__](#next-steps)

---

## Before you begin

- Ensure you are using a version of CARLA that has been built from source. If you are using a packaged (binary) version of CARLA then follow the tutorial [here][import_map_package].
- You should have at least two files, `<mapName>.xodr` and `<mapName>.fbx` that have been [generated][rr_generate_map] from a map editor such as RoadRunner. 
- These files should have the same value for `<mapName>` in order to be recognised as the same map.
- You can ingest multiple maps into the same package. Each map should have a unique name.

[import_map_package]: tuto_M_add_map_package.md
[rr_generate_map]: tuto_M_generate_map.md

---
## Map ingestion

__1.__ Place the map files to be imported in the `Import` folder found in the CARLA root directory.

__2.__ Run the command below to ingest the files:

```sh
make import
```

__Note that there are two optional parameter flags that can be set__:

- `--package=<package_name>` specifies the name of the package. By default, this is set to `map_package`. Two packages cannot have the same name, so using the default value will lead to errors on a subsequent ingestion. __It is highly recommended to change the name of the package__. Use this flag by running the command:

```sh
make import  ARGS="--package=<package_name>"
```

- `--no-carla-materials` specifies that you do not want to use the default CARLA materials (road textures etc). You will use the RoadRunner materials instead. This flag is __only required if you are not__ providing your own [`.json` file](tuto_M_manual_map_package.md). Any value in the `.json` file will override this flag. Use this flag by running the command:

```sh
make import  ARGS="--no-carla-materials"
```

A folder will be created in `Unreal/CarlaUE4/Content` with the name of your map package. It will contain config files, overdrive information, static asset information and navigation information.

---

## Next steps

You will now be able to open your map in the Unreal Editor and run simulations. From here, you will be able to customize the map and generate the pedestrian navigation data. We recommend generating the pedestrian navigation after all customization has finished, so there is no chance of obstacles blocking the pedestrian paths.

CARLA provides several tools and guides to help with the customization of your maps:

- [Implement sub-levels in your map.](tuto_M_custom_layers.md)
- [Add and configure traffic lights and signs.](tuto_M_custom_add_tl.md)
- [Add buildings with the procedural building tool.](tuto_M_custom_buildings.md)
- [Customize the road with the road painter tool.](tuto_M_custom_road_painter.md)
- [Customize the weather](tuto_M_custom_weather_landscape.md#weather-customization)
- [Customize the landscape with serial meshes.](tuto_M_custom_weather_landscape.md#add-serial-meshes)

Once you have finished with the customization, you can [generate the pedestrian navigation information](tuto_M_generate_pedestrian_navigation.md).

---

If you have any questions about the process, then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions).