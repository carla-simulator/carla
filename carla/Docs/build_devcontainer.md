# CARLA Docker Dev Environment (UE5)

CARLA UE5 can be built inside a Docker container running Ubuntu 22.04. This is useful when your host OS (e.g., Ubuntu 24.04) does not meet UE5's compilation requirements. The container provides the correct toolchain and dependencies while you keep all source code on the host.

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

1. `carla-base:ue5-22.04` — base image with all build dependencies
2. `carla-development:ue5-22.04` — development image with user matching your host UID/GID

Options:

```sh
Util/Docker/build.sh --force-rebuild                  # Rebuild from scratch (no cache)
Util/Docker/build.sh --user 1000:1000                 # Custom UID:GID
Util/Docker/build.sh --ubuntu-distro 22.04            # Specify Ubuntu version (default: 22.04)
```

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

The container itself only provides the Ubuntu 22.04 environment, toolchain, and dependencies. Your actual project data never lives inside the container.

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

## Running commands from additional terminals

While the container is running, you can execute commands inside it from any terminal on the host using `docker exec`. The container name is `carla-development-ue5-22.04`.

**Open an interactive shell** in the running container:

```sh
docker exec -it carla-development-ue5-22.04 bash
```

**Run a single command** without entering the container:

```sh
# Build the Python API from a second terminal while the editor is running
docker exec carla-development-ue5-22.04 cmake --build Build --target carla-python-api-install

# Build a package in the background
docker exec carla-development-ue5-22.04 cmake --build Build --target package

# Run a Python example against the running simulator
docker exec carla-development-ue5-22.04 python3 PythonAPI/examples/generate_traffic.py

# Check build configuration
docker exec carla-development-ue5-22.04 cmake --build Build --target help
```

This is useful for running parallel builds, launching Python scripts against a running simulator, or integrating with external tools (CI runners, editors, etc.) that need to invoke build commands inside the container.

---

## Using a VS Code Devcontainer

Create `.devcontainer/devcontainer.json` in your CARLA repository:

```json
{
    "name": "CARLA UE5 Dev",
    "image": "carla-development:ue5-22.04",

    "initializeCommand": "./Util/Docker/build.sh --ubuntu-distro 22.04",

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
      "--name", "carla-ue5-development-22.04",
      "--env", "NVIDIA_VISIBLE_DEVICES=all",
      "--env", "NVIDIA_DRIVER_CAPABILITIES=all",
      "--env", "CARLA_UNREAL_ENGINE_PATH=/workspaces/unreal-engine",
      "--env", "DISPLAY=${localEnv:DISPLAY}",
      "--volume", "/tmp/.X11-unix:/tmp/.X11-unix",
      "--volume", "${localEnv:CARLA_UNREAL_ENGINE_PATH}:/workspaces/unreal-engine",
      "--mount", "source=carla-development-ue5-22.04,target=/home/carla",
      "--gpus", "all"
    ]
}
```

---

## Tips & Known Issues

1. **Do not run binaries on the host** after building inside the container. Internal paths like `/workspaces/unreal-engine` do not exist on your host. Use `cmake --build Build --target package` and run from the resulting files instead.

2. **Audio warnings** may appear because ALSA/PulseAudio are not configured in the container. These are harmless.

3. **Disk space**: The UE5 engine and CARLA build artifacts together require significant disk space. Ensure sufficient room on the volume containing both your UE5 and CARLA directories.
