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

---
## Import a standalone package

To get the package into a CARLA distribution, move the compressed file to the `Import` folder and run the following script.  

```sh
cd Import
./ImportAssets.sh
```