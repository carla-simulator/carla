
## Map ingestion in a CARLA package

This is the recommended method to import a map into a CARLA package. It will run a Docker image of Unreal Engine to import the files, and export them as a standalone package. The Docker image takes 4h and 400GB to be built. However, this is only needed the first time.

__1. Build a Docker image of Unreal Engine.__ Follow [these instructions](https://github.com/carla-simulator/carla/tree/master/Util/Docker) to build the image. 

__2. Change permissions on the input folder.__ If no `.json` file is provided, the Docker will try to create it on the input folder. To be successful, said folder must have all permissions enabled for others.

```sh
#Go to the parent folder, where the input folder is contained
chmod 777 input_folder
```

!!! Note
    This is not necessary if the package is [prepared manually](#prepare-the-package-manually), and contains a `.json` file. 

__2. Run the script to cook the map.__ In the folder `~/carla/Util/Docker` there is a script that connects with the Docker image previously created, and makes the ingestion automatically. It only needs the path for the input and output files, and the name of the package to be ingested. If no `.json` is provided, the name must be `map_package`.

```sh
python3 docker_tools.py --input ~/path_to_input_folder --output ~/path_to_output_folder --packages map_package
```
!!! Warning
    If the argument `--package <package_name>` is not provided, the Docker will make a package of CARLA. 

__3. Locate the package__. The Docker should have generated the package `map_package.tar.gz` in the output path. This is the standalone package for the assets. 

__4. Import the package into CARLA.__  

*   __On Windows__ extract the package in the `WindowsNoEditor` folder. 

*   __On Linux__ move the package to the `Import` folder, and run the script to import it. 

```sh
cd Util
./ImportAssets.sh
```

__5. Change the name of the package folder__. Two packages cannot have the same name in CARLA. Go to `Content` and find the package. Change the name if necessary, to use one that identifies it.

---