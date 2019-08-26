# Traffic Manager

## Installing Traffic Manager

First of all, make sure that CARLA has already been installed, following the instructions on the documentation, and that you have built the different modules:

```

make launch
make PythonAPI
make package
make LibCarla

```

Then, after downloading the carla repository and extracting it, on the CMakeLists.txt file in ${CARLA_ROOT}/TrafficManager/ you must set an environment variable, LIBCARLA_LOCATION, to where your build of LibCarla resides. After this, you can do the following to build the traffic manager:

```

cd ${CARLA_ROOT}/TrafficManager/
mkdir build
cd build
cmake..
make

```

## Using Traffic Manager

Navigate back to build folder and run the compiled executable.


```

cd ${CARLA_ROOT}/TrafficManager/build/
./traffic_manager

```