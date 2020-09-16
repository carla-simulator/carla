# Create distribution packages for assets 

It is a common practice in CARLA to manage assets with standalone packages. Keeping them aside allows to reduce the size of the build. These asset packages can be easily imported into a CARLA package anytime. They also become really useful to easily distribute assets in an organized way. 

*   [__Export a package from the UE4 Editor__](#export-a-package-from-the-ue4-editor)  
*   [__Import assets into a CARLA package__](#import-assets-into-a-carla-package)  

---
## Export a package from the UE4 Editor

Once assets are imported into Unreal, users can generate a __standalone package__ for them. This will be used to distribute the content to CARLA packages such as 0.9.8.

To export packages, simply run the command below.

```sh
make package ARGS="--packages=Package1,Package2"
```

This will create a standalone package compressed in a `.tar.gz` file for each of the packages listed. The files will be saved in `Dist` folder on Linux, and `/Build/UE4Carla/` on Windows. 

!!! Note
    As an alternative, the [Docker method](tuto_A_add_map.md#via-docker) will create the standalone package without the need of having Unreal Engine in the system.  

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
    Standalone packages cannot be directly imported into a CARLA build. Follow the tutorials to import [props](tuto_A_add_props.md), [maps](tuto_A_add_map.md) or [vehicles](tuto_A_add_vehicle.md).

---

That sumps up how to create and use standalone packages in CARLA. If there is any unexpected issue, feel free to post in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>