# Traffic Manager

## Installing Traffic Manager

First of all, make sure that CARLA has already been installed, following the instructions on the documentation, and that you have built the different modules:

```

make launch
make PythonAPI
make package
make LibCarla

```

Then, after downloading the scenario runner repository and extracting it, on the CMakeLists.txt file in ${TRAFFIC_MANAGER}/source you must set an environment variable, LIBCARLA_LOCATION, to where your build of LibCarla resides. After this, you can do the following to build the traffic manager:

```

mkdir build
cd build
cmake..
make

```

## Using Traffic Manager

You need to modify spawn_npc.py in the ${CARLA_ROOT}/PythonAPI/examples folder and set the autopilot to False (on line 121) before running it. Then, start the CARLA server using ./CarlaUE4.sh in a terminal and spawn some vehicles on another terminal using:

``` python spawn_npc.py --safe ```

Navigate back to ${TRAFFIC_MANAGER}/source/build and run the compiled executable.

``` ./traffic_manager ```