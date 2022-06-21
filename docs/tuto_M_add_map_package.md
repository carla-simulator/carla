# Ingesting Maps in a CARLA package

This section describes the process of ingesting maps into __a package (binary) version of CARLA__. If you are using a version of CARLA that has been built from source to ingest maps then follow the guidelines [here][source_ingest] instead.

This process is only available for Linux systems. The import process involves running a Docker image of Unreal Engine to import the relevant files and then export them as a standalone package which can then be configured to be used in CARLA. The Docker image takes around 4h and 600-700 GB to be built. This is only needed the first time the image is built.

- [__Before you begin__](#before-you-begin)
- [__Map ingestion in a CARLA package__](#map-ingestion-in-a-carla-package)

---

## Before you begin

- You will need to fulfill the following system requirements:
    - 64-bit version of [Docker](https://docs.docker.com/engine/install/) in Ubuntu 16.04+
    - Minimum 8GB of RAM
    - Minimum 700 GB available disk space for building container images
    - [Git](https://git-scm.com/downloads) version control
- Ensure you are using a package (binary) version of CARLA. If you are using a version of CARLA that has been built from source to ingest maps then follow the guidelines [here][source_ingest] instead.
- You should have at least two files, `<mapName>.xodr` and `<mapName>.fbx` that have been [generated][rr_generate_map] from a map editor such as RoadRunner. 
- These files should have the same value for `<mapName>` in order to be recognised as the same map.


[source_ingest]: tuto_M_add_map_source.md
[import_map_package]: tuto_M_add_map_package.md
[rr_generate_map]: tuto_M_generate_map.md

---
## Map ingestion in a CARLA package

__1.__ CARLA provides all the utilities to build Unreal Engine in a Docker image and to compile CARLA using that image. The tools are found in the source code available in GitHub. Clone the repository using the following command:

```sh
    git clone https://github.com/carla-simulator/carla
```

__2.__ Build the Docker image of Unreal Engine by following [these instructions](https://github.com/carla-simulator/carla/tree/master/Util/Docker). 

__3.__ Create an `input_folder`.  This is where you will put the files to be imported. Docker will automatically create a `.json` file describing the package folder structure. Change permissions on the `input_folder` for this to be created successfully:

```sh
    #Go to the parent folder, where the input folder is contained
    chmod 777 input_folder
```

> !!! Note
    This is not necessary if the package is [prepared manually](tuto_M_manual_map_package.md), and contains a `.json` file already. 

__4.__ Create an `output_folder`. This is where the Docker image will write the output files after it has cooked the map. 

__5.__ Navigate to `~/carla/Util/Docker`. This is where the ingestion script is located. The script requires the path for the `input_folder` and `output_folder` and the name of the package to be ingested. If a `.json` file is provided, the name of that file is the package name, if no `.json` is provided, the name must be `map_package`:

```sh
    python3 docker_tools.py --input ~/path_to_input_folder --output ~/path_to_output_folder --packages map_package
```

> !!! Warning
    If the argument `--packages map_package` is not provided, the Docker image will make a package of CARLA. 

__6.__ The package will be generated in the `output_folder` as `<map_package>.tar.gz`. This is the standalone package that is now ready to be imported into CARLA. Move the package to the `Import` folder in the CARLA root directory (of the package/binary version where you will be using the map), and run the following script from the root directory to import it: 

```sh
        ./ImportAssets.sh
```

__7.__ To run a simulation with the new map, run CARLA and then change the map using the `config.py` file:

```sh
    cd PythonAPI/util
    python3 config.py --map <mapName>
```
<br>

---

Your map is now ready to run simulations in CARLA. If you have any questions about the process then you can ask in the [forum](https://github.com/carla-simulator/carla/discussions) or you can try running some of our [example scripts](https://github.com/carla-simulator/carla/tree/master/PythonAPI/examples) on your new map to test it out.