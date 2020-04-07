# Create standalone packages 

It is a common practice in CARLA to manage assets with independent packages. Keeping them aside from the main CARLA build allows to reduce the size of the build itself. These packages can be easily imported into CARLA anytime. They also become really useful to easily distribute assets in an organized way. 

* [__Export a package__](#export-a-package)  
* [__Import a distribution package__](#import-a-distribution-package)  

---
## Export a package

Once a package is imported into Unreal, users can  generate a __coocked packge__ for it. This is what will be used to distribute the content.

To export packages, simply run the command below.

```sh
make package ARGS="--packages=Package1,Package2"
```

This will create a distribution package compressed in a `.tar.gz` file for each of the packages listed. The files will be saved in `Dist` folder on Linux, and `/Build/UE4Carla/` on Windows. 

!!! Note
    If the assets have been imported [using Docker](tuto_A_add_map.md#via-docker), the distribution package be generated automatically.  

---
## Import a distribution package

Distribution packages are contained in `.tar.gz` files. In order to import these into CARLA, just extract these into the main root CARLA folder. The assets will be automatically storage in the corresponding path.  

To get the package into a CARLA distribution, move the compressed file to the `Import` folder and run the following script.  

```sh
cd Import
./ImportAssets.sh
```

---

That sumps up the management of asset packages in CARLA. If there is any unexpected issue, feel free to post in the forum. 

<div class="build-buttons">
<p>
<a href="https://forum.carla.org/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>