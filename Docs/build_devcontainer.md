# CARLA Docker development environment

CARLA UE5 can be built inside a Docker container running Ubuntu 22.04. This is useful when your host OS does not meet UE5's compilation requirements. The container provides the correct toolchain and dependencies while you keep all source code on the host.

- [__Prerequisites__](#prerequisites)
- [__Build the Docker image__](#build-the-docker-image)
- [__Run the container__](#run-the-container)
- [__Build Unreal Engine inside the container (first time only)__](#build-unreal-engine-inside-the-container-first-time-only)
- [__Download CARLA content (first time only)__](#download-carla-content-first-time-only)
- [__Build CARLA inside the container__](#build-carla-inside-the-container)
- [__Running commands from additional terminals__](#running-commands-from-additional-terminals)
- [__Using a VS Code Devcontainer__](#using-a-vs-code-devcontainer)
- [__Tips and known issues__](#tips-and-known-issues)

## Prerequisites

- [Docker](https://docs.docker.com/engine/install/) installed and configured for non-root use
- [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html) for GPU support, registered as a Docker runtime
- A clone of the CARLA fork of Unreal Engine 5 on the host, branch `ue58-dev-carla` of [CarlaUnreal/UnrealEngine](https://github.com/CarlaUnreal/UnrealEngine), with `CARLA_UNREAL_ENGINE_PATH` pointing to it. It can be compiled on the host or [inside the container](#build-unreal-engine-inside-the-container-first-time-only).

`run.sh` starts the container with the NVIDIA runtime. If `docker info | grep Runtimes` does not list `nvidia`, register it and restart Docker:

```sh
sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker
```

---

## Build the Docker image

From the CARLA root directory:

```sh
Util/Docker/build.sh
```

This builds two images:

1. `carla-base:ue5-22.04` — base image with all build dependencies
2. `carla-development:ue5-22.04` — development image with the user `carla`, matching your host UID/GID so that files it writes to the mounted directories keep your ownership. The user has password-less `sudo`.

Options:

```sh
Util/Docker/build.sh --force-rebuild                  # Rebuild from scratch (no cache)
Util/Docker/build.sh --user 1000:1000                 # Custom UID:GID
Util/Docker/build.sh --ubuntu-distro 22.04            # Specify Ubuntu version (default: 22.04)
Util/Docker/build.sh --docker-gid 999                 # GID of the host's docker group
```

The Ubuntu version is part of the image, volume and container names (`ue5-<distro>`). If you build for a version other than 22.04, pass the same `--ubuntu-distro` to `run.sh --dev` and use that name wherever this page shows `ue5-22.04`.

!!! Warning
    `--force-rebuild` also deletes the `carla-development-ue5-22.04` volume, which holds `/home/carla`: the DLSS SDK, the `DLSS_SDK` export in `.bashrc` and the Python packages installed with pip. Stop the container and close any devcontainer first, since a volume in use is not deleted and `build.sh` does not report it. Run `bash CarlaSetup.sh --skip-prerequisites --interactive` again afterwards.

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
| `/var/run/docker.sock` | `/var/run/docker.sock` | The host's Docker daemon, so the container can run Docker itself |
| named volume `carla-development-ue5-22.04` | `/home/carla` | The container user's home: `.bashrc`, pip packages and the DLSS SDK survive restarts |

The container starts in `/workspaces/carla` with GPU support and host networking enabled.

The CARLA repository and Unreal Engine 5 are **bind mounts** — they point directly to the directories on your host machine. Any changes you make inside the container (editing code, compiling, generating build artifacts) are immediately visible on the host, and vice versa. This means you can:

- **Edit code** on the host with your preferred editor or IDE, and compile inside the container.
- **Stop and restart** the container without losing any work — source code and build artifacts live on the host.
- **Share the same UE5 build** across multiple containers or workflows.

Use `--rebuild` to rebuild the image before running:

```sh
Util/Docker/run.sh --dev --rebuild
```

---

## Build Unreal Engine inside the container (first time only)

If your Unreal Engine 5 is not yet compiled, you can build it from inside the container:

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
- Fetch the NVIDIA DLSS SDK into `~/SDKs/DLSS`, inside the `/home/carla` volume, and export `DLSS_SDK` in `~/.bashrc`. The CARLA build requires it unless `DLSS_SDK=disabled` is exported before the first configure (on an existing `Build/`, reconfigure with `-DCARLA_DLSS_SDK_PATH=disabled`), and the script stops if it cannot fetch it.
- Clone CARLA content from Bitbucket into `Unreal/CarlaUnreal/Content/Carla/` (large download, only needed once).
- Configure and build CARLA with cmake, then build and install the Python API. The editor is only launched if you add `--launch`.

If you only want to download the content without building, you can clone it manually instead:

```sh
mkdir -p Unreal/CarlaUnreal/Content
git -C Unreal/CarlaUnreal/Content clone -b ue58-dev-carla https://bitbucket.org/carla-simulator/carla-content.git Carla
```

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

# Start a Zen server, needed by the package target (second terminal)
/workspaces/unreal-engine/Engine/Binaries/Linux/zenserver --port 8558

# Build a distributable package
cmake --build Build --target package
```

`launch` and `zenserver` stay in the foreground, so run each from its own shell, opened with `docker exec -it` as shown in [Running commands from additional terminals](#running-commands-from-additional-terminals).

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

# Install the Python client first, then run an example against the simulator
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
      "--net", "host",
      "--name", "carla-development-ue5-22.04",
      "--workdir", "/workspaces/carla",
      "--env", "NVIDIA_VISIBLE_DEVICES=all",
      "--env", "NVIDIA_DRIVER_CAPABILITIES=all",
      "--env", "CARLA_UNREAL_ENGINE_PATH=/workspaces/unreal-engine",
      "--env", "DISPLAY=${localEnv:DISPLAY}",
      "--volume", "/tmp/.X11-unix:/tmp/.X11-unix",
      "--volume", "/var/run/docker.sock:/var/run/docker.sock",
      "--volume", "${localEnv:CARLA_UNREAL_ENGINE_PATH}:/workspaces/unreal-engine",
      "--mount", "source=carla-development-ue5-22.04,target=/home/carla",
      "--gpus", "all"
    ],

    "workspaceMount": "source=${localWorkspaceFolder},target=/workspaces/carla,type=bind",
    "workspaceFolder": "/workspaces/carla"
}
```

The container name has to match the one the `docker exec` commands above use, `carla-development-ue5-22.04`, which is also what `run.sh` creates.

---

## Tips and known issues

1. **Do not run binaries on the host** after building inside the container. Internal paths like `/workspaces/unreal-engine` do not exist on your host. Use `cmake --build Build --target package` and run from the resulting files instead.

2. **Audio warnings** may appear because ALSA/PulseAudio are not configured in the container. These are harmless.

3. **Disk space**: The UE5 engine and CARLA build artifacts together require significant disk space. Ensure sufficient room on the volume containing both your UE5 and CARLA directories.
