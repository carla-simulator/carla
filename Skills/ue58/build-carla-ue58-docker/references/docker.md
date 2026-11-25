# Docker detail

## The images

| Image | Built from | Size here | Holds |
|---|---|---|---|
| release, e.g. `carla-local:<hash>` | the package's `Dockerfile` (a copy of `Util/Docker/Release.Dockerfile`, written by `Unreal/Package/CopyCarlaAdditionalFiles.cmake`) | 17.6 GB on disk (8.1 GB content) | Ubuntu 22.04, SDL2, Vulkan loader, libomp, the whole package under `/workspace`, user `carla`; `CMD bash CarlaUnreal.sh` |
| `carla-base:ue5-22.04` | `Util/Docker/Base.Dockerfile` | 1.8 GB | build toolchain: build-essential, ninja, CMake 3.28.3 in `/opt`, git-lfs, Python with the repo's `requirements.txt` |
| `carla-development:ue5-22.04` | `Util/Docker/Development.Dockerfile` | 5.3 GB | base plus sudo, vulkan-tools, the examples' and utils' Python requirements, and user `carla` with the host UID/GID and the host's docker GID |

`Util/Docker/build.sh` copies the requirements files into `Util/Docker/.tmp` (gitignored) as the build context, then builds base and development, creating the `carla-development-ue5-<distro>` volume on the way. `--force-rebuild` builds without cache and deletes that volume.

## The development container

`Util/Docker/run.sh --dev` runs `carla-development:ue5-<distro>` interactively with `--rm`, `--runtime=nvidia`, `--net=host`, and mounts:

| Host | Container |
|---|---|
| `CARLA_UNREAL_ENGINE_PATH` | `/workspaces/unreal-engine` |
| the checkout | `/workspaces/carla` (working directory) |
| `/tmp/.X11-unix` | same, for the editor window |
| `/var/run/docker.sock` | same: the container drives the host's Docker |
| volume `carla-development-ue5-<distro>` | `/home/carla` (`.bashrc`, pip packages, the DLSS SDK) |

It refuses to start without `CARLA_UNREAL_ENGINE_PATH`, and it passes `-it`, so it needs a terminal. To use it from automation, start it from a real terminal and drive it with `docker exec carla-development-ue5-22.04 …` (the skill's `dev-exec`).

First time inside: build the engine if it is not built (`cd /workspaces/unreal-engine && ./Setup.sh && ./GenerateProjectFiles.sh && make`), then `bash CarlaSetup.sh --skip-prerequisites --interactive`, which fetches the DLSS SDK into the volume, clones the content and runs the first configure and build. Later builds are ordinary `cmake` commands; see `Docs/build_devcontainer.md` and the build-carla-ue58 skill.

## Ports

Each server uses three TCP ports: RPC (`-carla-rpc-port`, default 2000), streaming (RPC + 1) and secondary (RPC + 2). The server tells the client which streaming port to connect to, by its own number. With `--net=host` there is nothing to map. With `-p`, the host side must use the same numbers as the container side.

## Measured on this machine

Docker 29.9.2, NVIDIA Container Toolkit 1.20.1, RTX A6000, ue58-dev ccc52d24d. The release image was built from `Build/Release/Package/Carla-1.0-Linux-Shipping.tar.gz`, whose `VERSION` reports Carla `35cb3d943`.

- Release image build from the extracted package: 4 min 25 s.
- `--net=host`, legacy runtime, `-carla-rpc-port=4000`: RPC up, the image's wheel (`carla-1.0`, `1.0-35cb3d943`) connected, Town10HD_Opt with 155 spawn points, and an 800x600 camera frame came back to the host.
- `-p 4000-4002:2000-2002` (host ports different from the container's): `get_server_version()` answered, but `get_world()` timed out (`time-out of 15000ms while waiting for the simulator`) for over 4 minutes.
- `-p 4000-4002:4000-4002` with the matching `-carla-*-port` flags, through CDI (`--device=nvidia.com/gpu=all`): `get_world()` and a camera frame both worked.
- `docker.sh release-build` (cached), `release-run` on 4000, `wheel`, `probe` and `stop` in sequence: all passed; a second `release-run` on the busy port refused with `port 4000 is already taken`.
- Development images: `build.sh` took 3 min 26 s. Inside `run.sh --dev`, the user was `carla`, CMake 3.28.3, the engine binary was visible under `/workspaces/unreal-engine`, and `nvidia-smi` and `vulkaninfo` both saw the RTX A6000. The shell prints `groups: cannot find name for group ID 110` on start, which is harmless.
- Not run: building the engine or CARLA inside the development container, and the VS Code devcontainer.
