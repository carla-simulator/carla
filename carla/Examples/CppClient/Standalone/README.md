# Standalone consumer demo

Demonstrates how to consume LibCarla's C++ client API from a project that
lives **outside** the CARLA source tree, via `find_package(Carla)`. Closes
[carla-simulator/carla#6091](https://github.com/carla-simulator/carla/issues/6091).

## Step 1 — Install CARLA

Build CARLA with the C++ client enabled, then install to a prefix of your
choice:

```bash
cd /path/to/carla
cmake -B Build -DBUILD_CARLA_CLIENT=ON -DBUILD_PYTHON_API=OFF
cmake --build Build --target carla-client -j
cmake --install Build --prefix /opt/carla
```

This installs:

```
/opt/carla/include/carla/...                 # public C++ headers
/opt/carla/lib/libcarla-client.a             # the static library
/opt/carla/lib/cmake/Carla/CarlaConfig.cmake # find_package() entry point
/opt/carla/lib/cmake/Carla/CarlaConfigVersion.cmake
```

## Step 2 — Build the consumer project

```bash
cd Examples/CppClient/Standalone
cmake -B build -DCMAKE_PREFIX_PATH=/opt/carla
cmake --build build -j
```

`find_package(Carla CONFIG REQUIRED)` resolves against `/opt/carla/lib/cmake/Carla/`.

## Step 3 — Run

A CARLA simulator must be reachable on `localhost:2000`:

```bash
/path/to/CARLA_<version>/CarlaUE4.sh
```

Then in another terminal:

```bash
./build/my-carla-app
# Connected to CARLA 0.10.0
# Available maps:
#   /Game/Carla/Maps/Town01
#   /Game/Carla/Maps/Town02
#   ...
```

## Notes

The consumer needs the same toolchain (C++20-capable compiler, matching
libstdc++ ABI) that CARLA itself was built with. On Ubuntu 24.04 the
default GCC 13 toolchain is sufficient.

For development inside the CARLA tree, the `add_subdirectory(...)`
pattern is also supported — see `Examples/CppClient/` for the in-tree
example.
