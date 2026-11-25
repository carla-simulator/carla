---
name: build-carla-ue58-docker
description: Builds and runs CARLA UE 5.8 in Docker — the release image from a Linux package (folder or Carla-*-Linux-Shipping.tar.gz) run headless with the NVIDIA runtime or CDI, its matching Python wheel copied out to the host, several servers on separate ports or GPUs; and the development image from Util/Docker (build.sh, run.sh --dev) that builds CARLA inside an Ubuntu 22.04 container with the engine and checkout bind-mounted. Use when the user asks to "run CARLA in Docker", "build the CARLA docker image", "containerise the server", "run CARLA headless on a server/cluster", "use the dev container" or "build CARLA in Docker".
license: MIT
compatibility: Linux with Docker and the NVIDIA Container Toolkit (nvidia runtime or a CDI spec). The release image needs a Linux package (from package-carla-ue58 or a download) and ~18 GB of image space; the development image needs a ue58-dev checkout and, to build anything in it, the CarlaUnreal UE 5.8 engine. Verified with Docker 29.9.2 and toolkit 1.20.1 on ue58-dev ccc52d24d.
metadata:
  group: ue58
  prerequisites: scripts/check_env.sh
  reference: references/docker.md
---

# Run and build CARLA UE 5.8 in Docker

> **Paths.** `scripts/…` and `references/…` below are relative to the directory holding this SKILL.md. Your working directory is the user's project, not that directory, so prefix them with its absolute path or the command is not found.

Two unrelated images share the word "Docker":

| Image | From | For |
|---|---|---|
| release | the `Dockerfile` at the root of a Linux package | running a server: headless, on a cluster, several per machine |
| development (`carla-base`, `carla-development`) | `Util/Docker/build.sh` | building CARLA on a host whose OS cannot, with the source and engine bind-mounted |

The release image only copies a package; it builds nothing. To build a package first, use package-carla-ue58. The development image does not contain the engine or CARLA either: they stay on the host.

## Instructions

```
Progress:
- [ ] Step 1: Check prerequisites (bash scripts/check_env.sh)
- [ ] Step 2a: Release image: build, run, copy the wheel, probe
- [ ] Step 2b: Development image: build, run, build CARLA inside
- [ ] Step 3: Stop only the containers you started
```

### Step 1: Check prerequisites

```bash
bash scripts/check_env.sh
```

It reports docker access, whether the GPU reaches containers through the nvidia runtime, a CDI spec, or both, the free space under Docker's root, and any package under `Build/*/Package`.

### Step 2a: Release image

```bash
bash scripts/docker.sh release-build Build/Release/Package/Carla-1.0-Linux-Shipping.tar.gz carla-local:1.0
bash scripts/docker.sh release-run carla-local:1.0 2000            # container carla-2000, waits for the RPC port
bash scripts/docker.sh wheel carla-local:1.0 ./wheels && pip install ./wheels/carla-*.whl
bash scripts/docker.sh probe 2000                                  # versions, map, and one camera frame
```

`release-build` takes a package folder or tarball. A tarball is extracted next to itself, or into `WORK_DIR`; that needs ~10 GB more. It fails when the image is under 4 GB, which means the package was not copied in. `release-run` starts the server offscreen with `-nosound`, with `--net=host`, and sets the RPC, streaming and secondary ports to `PORT`, `PORT+1` and `PORT+2`. It uses CDI (`--device=nvidia.com/gpu=…`) when a spec exists, otherwise the nvidia runtime. Set `GPU=1` to pin one GPU, and put extra server flags after `--`, e.g. `-- -ros2`. Install the wheel from the image rather than the host's: client and server must come from the same commit, and `probe` prints `MISMATCH` when they do not.

For several servers, run one container per port with a gap of at least 3 between ports, e.g. `release-run TAG 2000` and `release-run TAG 3000`, each with its own `GPU`.

### Step 2b: Development image

```bash
bash scripts/docker.sh dev-build                # carla-base:ue5-22.04 and carla-development:ue5-22.04
bash scripts/docker.sh dev-run                  # interactive; needs a terminal and CARLA_UNREAL_ENGINE_PATH
bash scripts/docker.sh dev-exec cmake --build Build --target carla-unreal-editor    # from another terminal
```

Inside, the checkout is `/workspaces/carla` and the engine is `/workspaces/unreal-engine`. First time only: `bash CarlaSetup.sh --skip-prerequisites --interactive`. Building the engine inside the container (`./Setup.sh && ./GenerateProjectFiles.sh && make` in `/workspaces/unreal-engine`) takes hours and ~225 GB on the host. Do not start it without the user asking. `dev-build` passes extra arguments to `Util/Docker/build.sh` (`--force-rebuild`, `--user UID:GID`, `--docker-gid GID`), and `UBUNTU_DISTRO` picks the distro for every dev command.

### Step 3: Stop

```bash
bash scripts/docker.sh stop 2000                # docker stop carla-2000, nothing else
```

The dev container is `--rm` and goes away when its shell exits.

## Gotchas

- **Host ports must equal container ports.** The server tells the client its streaming port by its own number. `-p 4000-4002:2000-2002` let `get_server_version()` answer, but `get_world()` timed out for minutes, because the client looked for streaming on host port 2001. Use `--net=host`, as `release-run` does, or publish each port on the same number and pass the `-carla-*-port` flags to match.
- **`--force-rebuild` deletes the `carla-development-ue5-<distro>` volume**, and with it the DLSS SDK and pip packages in `/home/carla`. It skips the deletion without a word while a container uses the volume.
- **Binaries built in the dev container do not run on the host.** They reference `/workspaces/...` paths. Build a package inside, then run the package anywhere.
- **Do not share a build directory between host and container.** The checkout is the same files, but CMake records absolute paths, and the container's are `/workspaces/carla`. Give the container its own `Build/<name>`. This was not exercised here, so treat it as a caution.
- **`docker pull carlasim/carla:latest` is an old UE4 image.** Always pull or build a tagged version.
- **Never stop other servers by process name.** run-carla-ue58-server's `stop` kills every UnrealEditor process on the machine, whatever its port. Stop containers by name, as `stop PORT` does.

## Outputs

A tagged release image, a running `carla-<PORT>` container serving CARLA on `PORT..PORT+2`, the image's `carla-*.whl` in the directory you named, and the `carla-base` / `carla-development` images with the `carla-development-ue5-<distro>` volume. Nothing is written into the checkout except `Util/Docker/.tmp`, which is gitignored.

Detail (image contents, the dev mounts, the port model, what was measured) in [references/docker.md](references/docker.md).
