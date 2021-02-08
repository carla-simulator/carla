# Ingesting Maps in a CARLA package

This section describes the process of ingesting maps into __a package (binary) version of CARLA__. If you are using a version of CARLA that has been built from source to ingest maps then follow the guidelines [here][source_ingest] instead.

It will run a Docker image of Unreal Engine to import the files, and export them as a standalone package. The Docker image takes around 4h and 600-700 GB to be built. However, this is only needed the first time.

## Before you begin

- Ensure you are using a package (binary) version of CARLA. If you are using a version of CARLA that has been built from source to ingest maps then follow the guidelines [here][source_ingest] instead.
- You should have at least two files, `<mapName>.xodr` and `<mapName>.fbx` that have been [generated][rr_generate_map] from a map editor such as RoadRunner. 
- These files should have the same value for `<mapName>` in order to be recognised as the same map.
- You will need to fulfill the following system requirements:
    - 64-bit version of Docker in Ubuntu 16.04+
    - Minimum 8GB of RAM
    - Minimum 600 GB available disk space for building container images

[source_ingest]: add_map_source.md
[import_map_package]: add_map_package.md
[rr_generate_map]: generate_map_roadrunner.md

---
## Map ingestion in a CARLA package

__1.__ CARLA provides all the utilities to build Unreal Engine in a Docker image and to compile CARLA using that image. The tools are found in the source code available in GitHub. Clone the repository using the following command:

```sh
    git clone https://github.com/carla-simulator/carla
```

__2.__ Build the Docker image of Unreal Engine by following [these instructions](https://github.com/carla-simulator/carla/tree/master/Util/Docker). 

__3.__ Create an `input_folder`.  This is where you will put the files to be imported. If there is no `.json` file provided there, then Docker will create one. Change permissions on the `input_folder` for this to be successful:

```sh
    #Go to the parent folder, where the input folder is contained
    chmod 777 input_folder
```

> !!! Note
    This is not necessary if the package is [prepared manually](add_map_deprecated.md#manual-package-preparation), and contains a `.json` file. 

__4.__ Create an `output_folder`. this is where the Docker image will write the output files after it has cooked the map. 

__5.__ Navigate to `~/carla/Util/Docker`. This is where the ingestion script is located. The script requires the path for the `input_folder` and `output_folder` and the name of the package to be ingested. If a `.json` file is provided, the name of that file is the package name, if no `.json` is provided, the name must be `map_package`:

```sh
    python3 docker_tools.py --input ~/path_to_input_folder --output ~/path_to_output_folder --packages map_package
```

> !!! Warning
    If the argument `--packages map_package` is not provided, the Docker image will make a package of CARLA. 

__6.__ The package will be generated in the `output_folder` as `<map_package>.tar.gz`. This is the standalone package that is now ready to be imported into CARLA:

> *   __On Windows__ extract the package to the `WindowsNoEditor` folder. 

> *   __On Linux__ move the package to the `Import` folder in the CARLA root directory (package/binary version), and run the script from the root directory to import it: 

```sh
        ./ImportAssets.sh
```

__7.__ Two packages cannot have the same name in CARLA so you may need to change the name of the package folder. Go to `CarlaUE4/Content` and change the package name appropriately.

__8.__ To run a simulation with the new map, run CARLA and then change the map using the `config.py` file:

```sh
    cd PythonAPI/util
    python3 config.py --map <mapName>
```

---

Your map is now ready to run simulations in CARLA. If you have any questions about the process then you can ask in the forum.

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>