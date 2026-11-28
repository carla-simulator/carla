---
name: build-carla-ue58-cpp-client
description: Builds and runs C++ clients for CARLA on UE 5.8 from a ue58-dev build tree — the in-tree examples (carla-example-client, carla-agent-demo with the C++ BehaviorAgent), or your own out-of-tree CMake project linked against an installed libcarla-client through find_package(Carla), with the vendored Boost/rpclib/Recast/libpng/zlib wiring the installed package leaves out. Use when the user asks to "use the C++ API", "build the C++ client", "write a C++ program that talks to CARLA", "link LibCarla into my project", or "run the C++ agent demo".
license: MIT
compatibility: Linux with a ue58-dev checkout configured by build-carla-ue58 (BUILD_CARLA_CLIENT=ON; BUILD_EXAMPLES=ON for the examples) and the CarlaUnreal UE 5.8 engine, whose clang and libc++ are the toolchain. Running a client needs a server of the same commit. Verified on ue58-dev ccc52d24d against a game-mode server.
metadata:
  group: ue58
  requires: build-carla-ue58
  prerequisites: scripts/check_env.sh
  reference: references/cpp-client.md
---

# Build a CARLA C++ client (UE 5.8)

> **Paths.** `scripts/…`, `references/…` and `assets/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

Two routes, and the second needs one file the docs do not mention:

| Route | What you get | Use it when |
|---|---|---|
| In-tree examples | `carla-example-client`, `carla-agent-demo` in `Build/<preset>/Examples/` | trying the C++ API, or a driveability check of the C++ agents |
| Out-of-tree project | your own executable linked to an installed `libcarla-client.a` | integrating CARLA into another codebase |

**The installed package is not enough on its own.** `find_package(Carla)` gives you `libcarla-client.a` and CARLA's headers, but not Boost, rpclib, Recast, libpng or zlib, which those headers include and the archive needs at link time. Following `Examples/CppClient/Standalone/README.md` as written fails at compile time with:

```
/usr/include/boost/asio/detail/type_traits.hpp:89:12: error: no member named 'result_of' in namespace 'std'
carla/MsgPack.h:11:10: fatal error: 'rpc/msgpack.hpp' file not found
```

The system Boost is picked up instead of CARLA's, and rpclib is not found at all. [assets/project/CarlaDeps.cmake](assets/project/CarlaDeps.cmake) adds the vendored copies from the build tree to the imported target; the scaffolded project includes it.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2: Build and run the in-tree examples, or
- [ ] Step 3: Install libcarla-client, scaffold, build and run your own project
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

A FAIL on the build tree means `build-carla-ue58` has not configured it. Set `CARLA_PRESET` if the tree is not `Build/Release`.

### Step 2: In-tree examples

```bash
bash scripts/cpp_client.sh examples                     # builds both, checks each binary exists and is over 1 MB
bash scripts/cpp_client.sh example localhost 2000       # loads a RANDOM map, spawns and destroys one vehicle
bash scripts/cpp_client.sh agent-demo localhost 2000 Town10HD_Opt normal
```

`example` changes the server's map: it calls `LoadWorld` on a random entry of `GetAvailableMaps()`. Do not point it at a server someone else is using. `agent-demo` drives a `BehaviorAgent` (`cautious`, `normal` or `aggressive`) to a far spawn point and destroys the vehicle at the end; on Town10HD_Opt it took 29692 steps.

### Step 3: Your own project

```bash
bash scripts/cpp_client.sh install /opt/carla           # libcarla-client.a, headers, lib/cmake/Carla/
bash scripts/cpp_client.sh new ~/my-carla-client        # CMakeLists.txt, CarlaDeps.cmake, main.cpp
bash scripts/cpp_client.sh build ~/my-carla-client /opt/carla
bash scripts/cpp_client.sh run ~/my-carla-client localhost 2000
```

`install` builds the `carla-client` target first, then installs only `Build/<preset>/LibCarla`, not the whole tree. `new` refuses to overwrite an existing `CMakeLists.txt`. `build` configures with CARLA's `CMake/Toolchain.cmake` and passes `CARLA_BUILD_DIR` to `CarlaDeps.cmake`. The template `main.cpp` prints the client and server versions, the map, the spawn-point count and the actor count.

A working run looks like:

```
client 1.0-ccc52d24d, server 1.0-ccc52d24d
map Carla/Maps/Town10HD_Opt, 155 spawn points, 24 actors
```

## Gotchas

- **Use CARLA's toolchain.** The archive is built with the engine's clang against libc++ (`-stdlib=libc++`). A project built with the system GCC and libstdc++ cannot link it. `CMake/Toolchain.cmake` needs `CARLA_UNREAL_ENGINE_PATH`.
- **Keep the build tree.** `CarlaDeps.cmake` points into `Build/<preset>/_deps`; deleting or reconfiguring that tree breaks the consumer. Rebuild the consumer after rebuilding CARLA.
- **Same commit on both ends.** A client from another commit prints `WARNING: Version mismatch detected` and may fail mid-call. A `-dirty` suffix on the client version only means the checkout had uncommitted edits when `Version.h` was generated.
- **Do not build while another build runs.** `install` and `examples` run `cmake --build` on `Build/<preset>`; two ninja processes in one tree corrupt its state.

## Outputs

The example binaries in `Build/<preset>/Examples/`, an install prefix with `lib/libcarla-client.a`, `include/carla/` and `lib/cmake/Carla/`, and your project's `build/my-carla-client`. Running a client changes the server only as the program does: `example` loads a map, `agent-demo` spawns and destroys a vehicle.

Detail (what each example does, the C++ API entry points, batch commands, the agents) in [references/cpp-client.md](references/cpp-client.md).
