# Create distribution packages for assets 

It is a common practice in CARLA to manage assets with standalone packages. Keeping them aside allows to reduce the size of the build. These asset packages can be easily imported into a CARLA package anytime. They also become really useful to easily distribute assets in an organized way. 

- [__Export a package from the UE4 Editor__](#export-a-package-from-the-ue4-editor)  
- [__Export a package using Docker__](#export-a-package-using-docker)
- [__Import assets into a CARLA package__](#import-assets-into-a-carla-package)  

---
## Export a package in a CARLA build from source

Once assets are imported into Unreal, users can generate a __standalone package__ for them. This will be used to distribute the content to CARLA packages such as 0.9.8.

To export packages, simply run the command below.

```sh
make package ARGS="--packages=Package1,Package2"
```

This will create a standalone package compressed in a `.tar.gz` file for each of the packages listed. The files will be saved in `Dist` folder on Linux, and `/Build/UE4Carla/` on Windows. 

---

## Export a package using Docker

Unreal Engine and CARLA can be built in a Docker image which can then be used to create a package or export assets for use in a package.

To create the Docker image, follow the tutorial [here](build_docker_unreal.md).

When you have the image ready:

1. Navigate to `Util/Docker`.
2. Create a CARLA package or prepare assets for use in a package by running one of the following commands:

```sh
# To create a standalone package
./docker_tools.py --output /output/path

#To cook assets to be consumed in a CARLA package
./docker_tools.py --input /assets/to/import/path --output /output/path --packages PkgeName1,PkgeName2
```

---
## Import assets into a CARLA package

A standalone package is contained in a `.tar.gz` file. The way this is extracted depends on the platform.  

*   __On Windows__ extract the compressed file in the main root CARLA folder.  
*   __On Linux__ move the compressed file to the `Import` folder and run the following script.  

```sh
cd Import
./ImportAssets.sh
```

!!! Note
    Standalone packages cannot be directly imported into a CARLA build. Follow the tutorials to import [props](tuto_A_add_props.md), [maps](tuto_M_custom_map_overview.md) or [vehicles](tuto_A_add_vehicle.md).

---

That sumps up how to create and use standalone packages in CARLA. If there is any unexpected issue, feel free to post in the forum. 

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>