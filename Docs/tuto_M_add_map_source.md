# Ingesting Maps in CARLA Built From Source

This section describes the process of ingesting maps into __CARLA that has been built from source__. If you are using a package (binary) version of CARLA to ingest maps then follow the guidelines [here][package_ingest] instead.

The ingestion process involves importing the relevant map files by compiling them into a package. This package can then be opened in the Unreal Engine editor and customized before generating the pedestrian navigation file and finally adding it to the package. 

[package_ingest]: tuto_M_add_map_package.md

- [__Before you begin__](#before-you-begin)
- [__Map ingestion__](#map-ingestion)
    - [Customize the map](#customize-the-map)
    - [Generate pedestrian navigation](#generate-pedestrian-navigation)

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
    make import ARGS="--package <package_name> --no-carla-materials"
```

__Note that there are two parameter flags that can be set__:

- `--package <package_name>` specifies the name of the package. By default, this is set to `map_package`. Two packages cannot have the same name, so using the default value will lead to errors on a subsequent ingestion. __It is highly recommended to change the name of the package__. 
- `--no-carla-materials` specifies that you do not want to use the default CARLA materials (road textures etc). You will use the RoadRunner materials instead. This flag is only required if you are not providing your own [`.json` file](tuto_M_custom_map_overview.md#ingest-the-map-in-carla). Any value in the `.json` file will override this flag.




Your map is now ready to run simulations in CARLA. If you have any questions about the process then you can ask in the [forum](https://forum.carla.org/) or you can try running some of our [example scripts](https://github.com/carla-simulator/carla/tree/master/PythonAPI/examples) on your new map to test it out.