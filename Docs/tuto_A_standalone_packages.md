# Standalone package for assets

It is a common practice in CARLA to manage assets with independent packages. Keeping them aside from the main CARLA build allows to reduce the size of the build itself. These packages can be easily ingested in CARLA anytime, and easily distributed. 

---
## Export a package

Once a package is imported into Unreal, users could also generate a __coocked packge__ for it. This is important in order to have the package ready for
distribution.

To export packages, simply run the command below.

```sh
make package ARGS="--packages=Package1,Package2"
```

This will create a distribution package compressed in a `.tar.gz` file for each of the packages listed. The files will be saved in `PythonAPI/dist` folder. 

!!! Note
    After using the Docker import method, the distribution package will be generated automatically.  

---
## Import a distribution package

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