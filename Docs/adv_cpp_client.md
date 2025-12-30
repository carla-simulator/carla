# C++ client example

To build the C++ client example you will need `make` installed. Before building a C++ client, you will need to build CARLA, follow the relevant [build instructions](build_carla.md) for your platform. 

Navigate to the `Examples/CppClient` folder in the CARLA repository and open a terminal. You will find a Makefile in this directory. To build and run it in Linux execute `make run` at the command prompt. In Windows, create a file named `CMakeLists.txt` in the same directory and add the contents in [this file](cpp_client_cmake_windows.md), then run `cmake`.

This C++ example will connect to the server, spawn a vehicle and apply a command to the vehicle before destroying it and terminating. 

### Include the relevant header files

For this example, we will be using several different CARLA classes, so we need to include the relevant header files from the CARLA library and include any standard libraries we will use:

```cpp
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>

#include <carla/client/ActorBlueprint.h>
#include <carla/client/BlueprintLibrary.h>
#include <carla/client/Client.h>
#include <carla/client/Map.h>
#include <carla/client/Sensor.h>
#include <carla/client/TimeoutException.h>
#include <carla/client/World.h>
#include <carla/geom/Transform.h>
#include <carla/image/ImageIO.h>
#include <carla/image/ImageView.h>
#include <carla/sensor/data/Image.h>

```

### Connecting the C++ client to the server

Include `carla/client/Client.h` and then connect the client:

```cpp
...
#include <carla/client/Client.h>
...
int main(int argc, const char *argv[]) {

    std::string host;
    uint16_t port;
    std::tie(host, port) = ParseArguments(argc, argv);
    ...
    // Connect the client to the server
    auto client = cc::Client(host, port);
    client.SetTimeout(40s);
```

### Load a map

Now let's load a randomly chosen map:

```cpp
// Initialize random number generator
std::mt19937_64 rng((std::random_device())());
...
auto town_name = RandomChoice(client.GetAvailableMaps(), rng);
std::cout << "Loading world: " << town_name << std::endl;
auto world = client.LoadWorld(town_name);
```

### Spawn a randomly chosen vehicle

Next we will fetch the blueprint library, filter for vehicles and choose a random vehicle blueprint:

```cpp
auto blueprint_library = world.GetBlueprintLibrary();
auto vehicles = blueprint_library->Filter("vehicle");
auto blueprint = RandomChoice(*vehicles, rng);
```

Now we need to find a location to spawn the vehicle from a spawn point in the map. We will get a pointer reference to the map object and then choose a random spawn point (ensure you have initialized the random number generator):

```cpp
auto map = world.GetMap();
auto transform = RandomChoice(map->GetRecommendedSpawnPoints(), rng);
```

Now we have the blueprint and spawn location, we can now spawn the vehicle using the `world.SpawnActor(...)` method:

```cpp
auto actor = world.SpawnActor(blueprint, transform);
std::cout << "Spawned " << actor->GetDisplayId() << '\n';
// Retrieve a pointer to the vehicle object
auto vehicle = boost::static_pointer_cast<cc::Vehicle>(actor);
```

### Apply a control

Let's now apply some control to the vehicle to move it using the `ApplyControl(...)` method:

```cpp
cc::Vehicle::Control control;
control.throttle = 1.0f;
vehicle->ApplyControl(control);
```

Now we will relocate the spectator so that we can see our newly spawned vehicle in the map:

```cpp
auto spectator = world.GetSpectator();
// Adjust the transform to look 
transform.location += 32.0f * transform.GetForwardVector();
transform.location.z += 2.0f;
transform.rotation.yaw += 180.0f;
transform.rotation.pitch = -15.0f;
// Now set the spectator transform
spectator->SetTransform(transform);
```

We'll also sleep the process for 10 seconds to observe the simulation shortly, before the client closes:


```cpp
std::this_thread::sleep_for(10s);

```

If you wish to keep the client open while other commands are executed, create a game loop. Now you have loaded a map and spawned a vehicle. To further explore the C++ API [build the Doxygen documentation](ref_cpp.md#c-documentation) and open it in a browser. 

To build the C++ client in another location outside of the CARLA repository, edit the first 5 lines of the Makefile to reference the correct locations for the `/build` directory and the CARLA build location:

```make
CARLADIR=$(CURDIR)/../..
BUILDDIR=$(CURDIR)/build
BINDIR=$(CURDIR)/bin
INSTALLDIR=$(CURDIR)/libcarla-install
TOOLCHAIN=$(CURDIR)/ToolChain.cmake
```

