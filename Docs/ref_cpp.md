# C++ reference

CARLA's C++ code is documented with [Doxygen](https://www.doxygen.nl/), from the comments in the headers.

!!! Warning
    The copy hosted at <http://carla.org/Doxygen/html/index.html> was generated from the UE 4.26 line and still describes `CarlaUE4`. For this branch, generate it locally with the steps below.

---
## Generate the documentation

Doxygen needs [Graphviz](https://www.graphviz.org/) for the diagrams.

```sh
sudo apt-get install doxygen graphviz
```

From the repository root, where the `Doxyfile` lives:

```sh
doxygen
```

The result is written to `Doxygen/html/`. Open `Doxygen/html/index.html` in a browser.

---
## What is covered

| Path | Contents |
| --- | --- |
| `LibCarla/source/carla` | The client and server library: the C++ client API, the RPC types, the geometry, the sensor serialization and the Traffic Manager. |
| `Unreal/CarlaUnreal/Plugins/Carla/Source` | The Carla plugin: the actors, the sensors and the server side of the simulation. |
| `Unreal/CarlaUnreal/Source` | The Unreal project itself. |

For using the C++ client rather than reading its internals, start with the [C++ tutorial](tuto_cpp.md).
