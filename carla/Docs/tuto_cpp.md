# C++ tutorial

This tutorial is a practical C++ workflow for CARLA users who want to build and run client-side logic without relying on Python wrappers.

## Scope

Covered here:

- Connect and control the simulator from C++
- Reuse the C++ client in another project
- Spawn/destroy actors with batch commands
- Read world snapshots
- Record and replay sessions

Not covered here:

- Unreal plugin authoring

## Prerequisites

- CARLA built or installed
- CARLA server running (`localhost:2000` by default)
- C++ client target available from this repository build

## 1) Build and run the C++ example client

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

## 2) Use CARLA C++ API in another project

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

```bash
# in CARLA's source tree
cmake -B Build -DBUILD_CARLA_CLIENT=ON -DBUILD_PYTHON_API=OFF
cmake --build Build --target carla-client -j
cmake --install Build --prefix /opt/carla
```

In your own project's `CMakeLists.txt`:

```cmake
find_package(Carla CONFIG REQUIRED)
add_executable(myapp main.cpp)
target_link_libraries(myapp PRIVATE Carla::carla-client)
```

```bash
cmake -B build -DCMAKE_PREFIX_PATH=/opt/carla
cmake --build build -j
```

A complete end-to-end demo project lives at
[`Examples/CppClient/Standalone/`](https://github.com/carla-simulator/carla/tree/master/Examples/CppClient/Standalone)
— covers install, configure, build, and run.

## 3) Batch commands (spawn/destroy efficiently)

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

## 4) World snapshots

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

## 5) Recorder and replay

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

## Parity note

The C++ API is first-class for low-level simulation control. The high-level navigation helpers `BasicAgent`, `BehaviorAgent`, and `ConstantVelocityAgent` are now available in C++ under [LibCarla/source/carla/agents/navigation/](../LibCarla/source/carla/agents/navigation/).

Typical C++ approach today:

- High-level navigation via the `BasicAgent` / `BehaviorAgent` C++ classes
- Traffic Manager autopilot for baseline behavior
- Custom route/planning/control modules for explicit ego behavior

## See also

- [C++ reference](ref_cpp.md)
- [C++ API parity roadmap](cpp_api_parity.md)
