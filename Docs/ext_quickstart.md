# Quickstart extras

The following sections contain additional options and details for installing and running a packaged version of the CARLA simulator.

* __[Command-line options](#command-line-options)__  
* __[Config script](#config-script)__
* __[Import additional assets](#import-additional-assets)__  

---

## Command-line options

There are some configuration options available when launching CARLA and they can be used as follows:

```sh
    ./CarlaUE4.sh -carla-rpc-port=3000
```

* `--ros2` - Launch CARLA with the native ROS2 connector enabled
* `-carla-rpc-port=N` - Listen for client connections at port `N`. Streaming port is set to `N+1` by default.  
* `-carla-streaming-port=N` - Specify the port for sensor data streaming. Use 0 to get a random unused port. The second port will be automatically set to `N+1`.  
* `-quality-level={Low,Epic}` - Change graphics quality level. Find out more in [rendering options](adv_rendering_options.md).  
* __[List of Unreal Engine 4 command-line arguments][ue4clilink].__ - There are a lot of options provided by Unreal Engine however not all of these are available in CARLA.  

[ue4clilink]: https://docs.unrealengine.com/en-US/Programming/Basics/CommandLineArguments

--- 

## Config script

The script [`PythonAPI/util/config.py`][config] provides more configuration options and should be run when the server has been started:

[config]: https://github.com/carla-simulator/carla/blob/master/PythonAPI/util/config.py

```sh
./config.py --no-rendering      # Disable rendering
./config.py --map Town05        # Change map
./config.py --weather ClearNoon # Change weather

./config.py --help # Check all the available configuration options
```

---

## Import additional assets 

Each release has it's own additional package of extra assets and maps. This additional package includes the maps __Town06__ and __Town07__. These are stored separately to reduce the size of the build, so they can only be imported after the main package has been installed. 

__1.__ [Download](https://github.com/carla-simulator/carla/blob/master/Docs/download.md) the appropriate package for your desired version of CARLA.

__2.__ Extract the package:

- __On Linux__:

    - move the package to the _Import_ folder and run the following script to extract the contents:  

```sh
        cd path/to/carla/root

        ./ImportAssets.sh
```

- __On Windows__:

    - Extract the contents directly in the root folder. 

---