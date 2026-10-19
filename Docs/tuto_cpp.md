# C++ tutorial

This tutorial is a practical C++ workflow for CARLA users who want to build and run client-side logic without relying on Python wrappers.

- [__Scope__](#scope)
- [__Prerequisites__](#prerequisites)
- [__Build and run the C++ example client__](#build-and-run-the-c-example-client)
- [__Use the CARLA C++ API in another project__](#use-the-carla-c-api-in-another-project)
- [__Batch commands__](#batch-commands)
- [__World snapshots__](#world-snapshots)
- [__Recorder and replay__](#recorder-and-replay)
- [__Navigation agents in C++__](#navigation-agents-in-c)

---
## Scope

Covered here:

- Connect and control the simulator from C++
- Reuse the C++ client in another project
- Spawn/destroy actors with batch commands
- Read world snapshots
- Record and replay sessions

Not covered here:

- Unreal plugin authoring

---
## Prerequisites

- CARLA [built from source](build_linux_ue5.md) on Linux or [on Windows](build_windows_ue5.md), or a [packaged release](start_quickstart.md)
- A CARLA server running (`localhost:2000` by default)
- The C++ client target available from this repository's build

---
## Build and run the C++ example client

Build:

```sh
cmake --build Build --target carla-example-client
```

Run (default endpoint):

```sh
./Build/Examples/carla-example-client
```

Run (custom endpoint):

```sh
./Build/Examples/carla-example-client 192.168.1.100 3000
```

What this example demonstrates:

1. `carla::client::Client` connection
2. Map/world loading
3. Vehicle spawn and control
4. Spectator movement
5. Actor cleanup

---
## Use the CARLA C++ API in another project

For external integration, use the same client entrypoints shown in `Examples/CppClient`.

Minimal shape:

```cpp
#include <carla/client/Client.h>

int main() {
  carla::client::Client client("localhost", 2000);
  client.SetTimeout(std::chrono::seconds(10));
  auto world = client.GetWorld();
  // Integrate world access, actors, sensors with your own architecture.
}
```

### Linking against an installed CARLA

CARLA ships a `find_package(Carla CONFIG REQUIRED)` integration. Build CARLA
once with the C++ client enabled, install it to a prefix, then consume from
any external CMake project:

```sh
# in CARLA's source tree, with the same toolchain the normal build uses
cmake -G Ninja -S . -B Build --toolchain=$PWD/CMake/Toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CARLA_CLIENT=ON -DBUILD_PYTHON_API=OFF -DBUILD_CARLA_UNREAL=OFF
cmake --build Build --target carla-client -j
cmake --install Build --prefix /opt/carla
```

The install carries CARLA's own targets; Boost and zlib are external, so the consuming project has to find them itself.

In your own project's `CMakeLists.txt`:

```cmake
find_package(Carla CONFIG REQUIRED)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE Carla::carla-client)
```

```sh
cmake -B build -DCMAKE_PREFIX_PATH=/opt/carla
cmake --build build -j
```

A complete end-to-end demo project lives at
[`Examples/CppClient/Standalone/`](https://github.com/carla-simulator/carla/tree/ue58-dev/Examples/CppClient/Standalone)
— covers install, configure, build, and run.

---
## Batch commands

Use batch commands when applying many actor operations in one tick.

```cpp
#include <carla/rpc/Command.h>

std::vector<carla::rpc::Command> cmds;
for (const auto &spawn : spawn_points) {
  auto description = blueprint.MakeActorDescription();
  cmds.emplace_back(carla::rpc::Command::SpawnActor(description, spawn));
}
auto responses = client.ApplyBatchSync(cmds, true);
```

Destroy in batch:

```cpp
std::vector<carla::rpc::Command> destroy_cmds;
for (auto id : actor_ids) {
  destroy_cmds.emplace_back(carla::rpc::Command::DestroyActor(id));
}
client.ApplyBatchSync(destroy_cmds, true);
```

---
## World snapshots

Snapshots give a frame-consistent view of actor transforms and velocities.

```cpp
auto snapshot = world.WaitForTick(std::chrono::seconds(1));
for (auto &actor_snapshot : snapshot) {
  auto transform = actor_snapshot.transform;
  auto velocity = actor_snapshot.velocity;
  (void)transform;
  (void)velocity;
}
```

---
## Recorder and replay

Record:

```cpp
client.StartRecorder("session.log");
// ... run simulation logic ...
client.StopRecorder();
```

Replay:

```cpp
client.ReplayFile("session.log", 0.0, 0.0, 0u, false);
```

---
## Navigation agents in C++

The navigation agents are available in C++ as well as in Python, under [`LibCarla/source/carla/agents/navigation/`](https://github.com/carla-simulator/carla/tree/ue58-dev/LibCarla/source/carla/agents/navigation): `BasicAgent`, `BehaviorAgent` and `ConstantVelocityAgent`, the same classes [the Python agents](adv_agents.md) expose.

```cpp
#include <carla/agents/navigation/BasicAgent.h>

carla::agents::navigation::BasicAgent agent(vehicle, 20.0f);  // target speed in km/h
agent.SetDestination(destination);
while (!agent.Done()) {
  vehicle->ApplyControl(agent.RunStep());
  world.Tick(std::chrono::seconds(10));
}
```

`BasicAgent::Options` carries the same knobs as the Python `opt_dict`: `ignore_traffic_lights`, `ignore_vehicles`, `sampling_resolution`, `max_brake` and the rest.

For traffic that does not need a planner, register the vehicle with the Traffic Manager instead, exactly as a Python client would.

---
## See also

- [C++ reference](ref_cpp.md)
