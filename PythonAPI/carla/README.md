This is the Python package for the CARLA Python API used for controlling and communicating-with [CARLA](https://carla.org) - The open-source simulator for autonomous driving research.

This package allows you to control the CARLA simulator and retrieve simulation data through the Python API. For instance, you can control any actor (vehicle, pedestrian, traffic light, etc...) in the simulation, attach sensors to vehicles, and read the sensor data etc.

For more information, refer to the [Guide for getting started with CARLA](https://carla.readthedocs.io/en/latest/getting_started/).

When building this package from a CARLA source checkout, use the CMake targets
`carla-python-api` or `carla-python-api-install`. They build the native client
extension with the PEP 517 `scikit-build-core` backend. Keep `ENABLE_PEP517=ON`
(the default); the setuptools-only `ENABLE_PEP517=OFF` path is unsupported and
is rejected during configuration.
