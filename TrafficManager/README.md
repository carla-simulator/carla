# Traffic Manager

## Installing Traffic Manager

First of all, make sure that CARLA has already been installed (you can check the instructions on the documentation) and that you have built the following modules:

```
make LibCarla
make TrafficManager
```

## Using Traffic Manager

Navigate to the build folder and run the compiled executable.

```
cd ${CARLA_ROOT}/TrafficManager/build/
./traffic_manager -n <NUMBER_OF_VEHICLES>
```
