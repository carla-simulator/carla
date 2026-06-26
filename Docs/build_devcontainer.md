# CARLA Docker Dev Environment (UE5)

CARLA UE5 can be built inside a Docker container running Ubuntu 24.04 (default) or Ubuntu 22.04. This is useful when your host OS does not meet UE5's compilation requirements, or when you want to validate the build on a specific Ubuntu version without changing your host. The container provides the correct toolchain and dependencies while you keep all source code on the host.

## Prerequisites

- [Docker](https://docs.docker.com/engine/install/) installed and configured for non-root use
- [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html) for GPU support
- A compiled CARLA fork of Unreal Engine 5 on the host, with `CARLA_UNREAL_ENGINE_PATH` pointing to it

---

## Build the Docker image

From the CARLA root directory:

```sh
Util/Docker/build.sh
```

This builds two images:

1. `carla-base:ue5-24.04` — base image with all build dependencies
2. `carla-development:ue5-24.04` — development image with user matching your host UID/GID

Options:

```sh
Util/Docker/build.sh --force-rebuild                  # Rebuild from scratch (no cache)
Util/Docker/build.sh --user 1000:1000                 # Custom UID:GID
Util/Docker/build.sh --ubuntu-distro 24.04            # Specify Ubuntu version (default: 24.04, also supports 22.04)
```

### Build for a different Ubuntu version

The default is Ubuntu 24.04. To build the image (and the matching development container) on Ubuntu 22.04 instead, pass `--ubuntu-distro`:

```sh
Util/Docker/build.sh --ubuntu-distro 22.04
```

This produces `carla-base:ue5-22.04` and `carla-development:ue5-22.04`, with a separate `carla-development-ue5-22.04` volume so the two distros never share state. You can keep both images on the same host and switch between them by passing the matching `--ubuntu-distro` flag to `run.sh`.

---

## Run the container

```sh
export CARLA_UNREAL_ENGINE_PATH=/path/to/UnrealEngine5_carla
Util/Docker/run.sh --dev
```

This mounts:

| Host path | Container path | Description |
|-----------|---------------|-------------|
| `CARLA_UNREAL_ENGINE_PATH` | `/workspaces/unreal-engine` | Unreal Engine 5 |
| CARLA repository | `/workspaces/carla` | Working directory |
| `/tmp/.X11-unix` | `/tmp/.X11-unix` | X11 display forwarding |

The container starts in `/workspaces/carla` with GPU support and host networking enabled.

### Container workspace layout

Inside the container, `/workspaces/` contains your two main directories:

```
/workspaces/
├── carla/              ← Your CARLA repository (mounted from host)
└── unreal-engine/      ← Your UE5 build (mounted from host)
```

Both are **bind mounts** — they point directly to the directories on your host machine. Any changes you make inside the container (editing code, compiling, generating build artifacts) are immediately visible on the host, and vice versa. This means you can:

- **Edit code** on the host with your preferred editor or IDE, and compile inside the container.
- **Stop and restart** the container without losing any work — source code and build artifacts live on the host.
- **Share the same UE5 build** across multiple containers or workflows.

The container itself only provides the Ubuntu environment (24.04 by default, 22.04 if you passed `--ubuntu-distro 22.04` at build time), toolchain, and dependencies. Your actual project data never lives inside the container.

Use `--rebuild` to rebuild the image before running:

```sh
Util/Docker/run.sh --dev --rebuild
```

---

## Build Unreal Engine inside the container (first time only)

If your Unreal Engine 5 is not yet compiled, you can build it from inside the container. Since the UE5 directory is mounted from the host, the build artifacts persist after the container exits and you only need to do this once:

```sh
cd /workspaces/unreal-engine
./Setup.sh && ./GenerateProjectFiles.sh && make
```

After this completes, return to the CARLA working directory for development:

```sh
cd /workspaces/carla
```

---

## Download CARLA content (first time only)

Before compiling, you need to download the CARLA content assets (maps, meshes, textures, etc.). The `CarlaSetup.sh` script handles this automatically. Since the container already has all prerequisites installed, use `--skip-prerequisites` and `--interactive`:

```sh
bash CarlaSetup.sh --skip-prerequisites --interactive
```

The `--interactive` flag is needed because `CarlaSetup.sh` requires `GIT_LOCAL_CREDENTIALS` in unattended mode (used for cloning UE5 from Epic's private repo). Since UE5 is already mounted in the container, the credentials are not actually needed — `--interactive` makes the script warn instead of exiting.

The script will:

- Skip prerequisite installation (already in the Docker image).
- Skip the UE5 download (detects `CARLA_UNREAL_ENGINE_PATH` is set).
- Clone CARLA content from Bitbucket into `Unreal/CarlaUnreal/Content/Carla/` (large download, only needed once).
- Configure and build CARLA with cmake.

If you only want to download the content without building, you can clone it manually instead:

```sh
mkdir -p Unreal/CarlaUnreal/Content
git -C Unreal/CarlaUnreal/Content clone -b ue5-dev https://bitbucket.org/carla-simulator/carla-content.git Carla
```

Since the CARLA directory is mounted from the host, the downloaded content persists after the container exits.

---

## Build CARLA inside the container

Once inside the container, run the standard cmake build commands:

```sh
# Configure
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release -DENABLE_ROS2=ON --toolchain=/workspaces/carla/CMake/Toolchain.cmake

# Build
cmake --build Build

# Build and install the Python API
cmake --build Build --target carla-python-api-install

# Launch the Unreal Editor
cmake --build Build --target launch

# Build a distributable package
cmake --build Build --target package
```

---

## Generate Python API wheels for multiple Python versions

Both images bundle every supported interpreter, Python 3.8 through 3.14, each compiled from source and available as `python3.8` … `python3.14` (the distro's own interpreter remains the default `python3`). Wheel building itself is part of the CMake build, through the `carla-python-api-wheels` target and the `CARLA_PYTHON_API_VERSIONS` option, so the workflow is the same inside or outside the container; the container's only job is to provide the interpreters.

By default (`CARLA_PYTHON_API_VERSIONS` empty) the target builds a wheel for **only the current configured interpreter**, exactly like `carla-python-api`:

```sh
cmake --build Build --target carla-python-api-wheels
```

To build wheels for specific versions, set the option at configure time to a `;`- or `,`-separated list. The build then looks up each `python3.X` on the system and builds the ones it finds:

```sh
# Build wheels for a chosen set of interpreters.
# 1. Reconfigure with the desired versions (';' or ',' separated):
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release \
  --toolchain="$PWD/CMake/Toolchain.cmake" \
  -DCARLA_PYTHON_API_VERSIONS="3.10;3.11;3.12"

# 2. Build the target. Each version is located as 'python3.X' on PATH:
cmake --build Build --target carla-python-api-wheels

# 3. Collect the finished wheels:
ls Build/PythonAPI/dist/
#   carla-0.10.0-cp310-cp310-manylinux_2_35_x86_64.whl
#   carla-0.10.0-cp311-cp311-manylinux_2_35_x86_64.whl
#   carla-0.10.0-cp312-cp312-manylinux_2_35_x86_64.whl
```

To build the full supported range in one go, pass every version:

```sh
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release \
  --toolchain="$PWD/CMake/Toolchain.cmake" \
  -DCARLA_PYTHON_API_VERSIONS="3.8;3.9;3.10;3.11;3.12;3.13;3.14"
cmake --build Build --target carla-python-api-wheels
```

If a requested interpreter is not installed, the build prints an error for that version and continues with the rest; it stops with an error only if no wheel could be built at all (none of the requested interpreters are present, or the only one available is outside the supported 3.8–3.14 range). Finished wheels are collected in `Build/PythonAPI/dist/`, the same directory `carla-python-api` already uses for the single-interpreter wheel (which, through the bind mount, is the same `Build/PythonAPI/dist/` on your host). The `manylinux` tag follows the image's glibc: `manylinux_2_38` on 24.04, `manylinux_2_35` on 22.04.

The Python API is client side, so the wheel build needs neither the Unreal Engine editor nor the CARLA content. To build wheels without cloning the content repository, add `-DBUILD_CARLA_UNREAL=OFF` to the configure:

```sh
cmake -G Ninja -S . -B Build -DCMAKE_BUILD_TYPE=Release \
  --toolchain="$PWD/CMake/Toolchain.cmake" \
  -DBUILD_CARLA_UNREAL=OFF \
  -DCARLA_PYTHON_API_VERSIONS="3.8;3.9;3.10;3.11;3.12;3.13;3.14"
cmake --build Build --target carla-python-api-wheels
```

### Prerequisites for each target interpreter

Every version you list must be present **on the host as a usable build interpreter**, not just installed. For each `python3.X` you want a wheel for, you need three things:

1. **The interpreter** itself, reachable as `python3.X` on `PATH`.
2. **Its development package** (the `Python.h` headers and shared library), which the Boost.Python extension is compiled against. On Debian/Ubuntu that is `python3.X-dev`; an interpreter built from source via `make altinstall` already includes them.
3. **`build` and `numpy` installed into that interpreter.** The PEP 517 build runs in an isolated environment, so it pulls its own backend (`scikit-build-core`, `wheel`, `setuptools`) automatically; you only add `auditwheel` if you want the manylinux repair (without it the plain `linux_x86_64` wheel is kept).

Inside the dev container all three are already provided for Python 3.8–3.14, so nothing extra is needed. On a bare host you must provision each interpreter yourself, for example with the deadsnakes PPA:

```sh
# Example: add Python 3.11 as a build target on Ubuntu.
sudo add-apt-repository ppa:deadsnakes/ppa
sudo apt-get update
sudo apt-get install -y python3.11 python3.11-dev python3.11-venv

# Install the build requirements into that interpreter (auditwheel optional, for manylinux).
python3.11 -m pip install build numpy auditwheel
```

Repeat for every interpreter in `CARLA_PYTHON_API_VERSIONS`. A version whose interpreter, headers, or build requirements are missing fails only that version (you will see the error in the build log) and the rest still build.

By default each wheel is repaired to the most portable `manylinux` tag its symbols allow (`CARLA_PYTHON_API_WHEEL_MANYLINUX=ON`); the repair is best-effort, so on a host without `auditwheel` the plain wheel is kept instead. Set `CARLA_PYTHON_API_WHEEL_PLATFORM` to force a specific tag, or `CARLA_PYTHON_API_WHEEL_MANYLINUX=OFF` to skip the repair.

!!! note
    The current interpreter reuses the main build tree, but every other version is built in its own nested `Build/python-wheels/py<version>/` configure (the wheel links a version-specific Boost.Python target). A full 3.8 → 3.14 run rebuilds Boost.Python and `carla-client` once per non-current version and is therefore significantly slower than a single build. Build only the versions you need during development; reserve the full set for a release build.

---

## Running commands from additional terminals

While the container is running, you can execute commands inside it from any terminal on the host using `docker exec`. The container name follows the pattern `carla-development-ue5-${UBUNTU_DISTRO}`, so the default 24.04 build is named `carla-development-ue5-24.04`. If you launched with `--ubuntu-distro 22.04`, swap the suffix in every example below to `22.04`.

**Open an interactive shell** in the running container:

```sh
docker exec -it carla-development-ue5-24.04 bash
```

**Run a single command** without entering the container:

```sh
# Build the Python API from a second terminal while the editor is running
docker exec carla-development-ue5-24.04 cmake --build Build --target carla-python-api-install

# Build a package in the background
docker exec carla-development-ue5-24.04 cmake --build Build --target package

# Run a Python example against the running simulator
docker exec carla-development-ue5-24.04 python3 PythonAPI/examples/generate_traffic.py

# Check build configuration
docker exec carla-development-ue5-24.04 cmake --build Build --target help
```

This is useful for running parallel builds, launching Python scripts against a running simulator, or integrating with external tools (CI runners, editors, etc.) that need to invoke build commands inside the container.

---

## Using a VS Code Devcontainer

Create `.devcontainer/devcontainer.json` in your CARLA repository:

```json
{
    "name": "CARLA UE5 Dev",
    "image": "carla-development:ue5-24.04",

    "initializeCommand": "./Util/Docker/build.sh --ubuntu-distro 24.04",

    "updateRemoteUserUID": false,

    "customizations": {
      "vscode": {
        "settings": {
          "terminal.integrated.defaultProfile.linux": "bash"
        },
        "extensions": [
          "ms-vscode.cpptools"
        ]
      }
    },

    "runArgs": [
      "--rm",
      "--runtime", "nvidia",
      "--name", "carla-ue5-development-24.04",
      "--env", "NVIDIA_VISIBLE_DEVICES=all",
      "--env", "NVIDIA_DRIVER_CAPABILITIES=all",
      "--env", "CARLA_UNREAL_ENGINE_PATH=/workspaces/unreal-engine",
      "--env", "DISPLAY=${localEnv:DISPLAY}",
      "--volume", "/tmp/.X11-unix:/tmp/.X11-unix",
      "--volume", "${localEnv:CARLA_UNREAL_ENGINE_PATH}:/workspaces/unreal-engine",
      "--mount", "source=carla-development-ue5-24.04,target=/home/carla",
      "--gpus", "all"
    ]
}
```

To target Ubuntu 22.04 instead, replace every `24.04` suffix above with `22.04` (image tag, container name, mount source) and pass `--ubuntu-distro 22.04` in `initializeCommand`.

---

## Tips & Known Issues

1. **Do not run binaries on the host** after building inside the container. Internal paths like `/workspaces/unreal-engine` do not exist on your host. Use `cmake --build Build --target package` and run from the resulting files instead.

2. **Audio warnings** may appear because ALSA/PulseAudio are not configured in the container. These are harmless.

3. **Disk space**: The UE5 engine and CARLA build artifacts together require significant disk space. Ensure sufficient room on the volume containing both your UE5 and CARLA directories.
