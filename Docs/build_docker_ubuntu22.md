# CARLA Docker Dev Environment (Ubuntu 22.04)

This repository provides **two** distinct Docker-based approaches for building and running **CARLA (version UE4)** on **Ubuntu Jammy (22.04)**:

1. **Monolithic**  
   - Bundles Unreal Engine (UE4) and CARLA into a single Docker image.  
   - Requires a lengthy build process and produces a large final image (often 100+ GB).  
   - Creates a fully self-contained environment with everything compiled inside Docker.

2. **Lightweight Devcontainer**  
   - Installs only the dependencies (plus NVIDIA support) required to compile and run CARLA.  
   - Requires mounting an existing **UE4** build from the host into the container (see [Build UE4 for Devcontainer Option](#build-ue4-prerequisite-for-lightweight--devcontainer)).  
   - Much faster to build but relies on a locally compiled Unreal Engine folder.

Your choice between **monolithic** and **lightweight** depends primarily on disk space, build times, and whether you prefer a fully self-contained environment (monolithic) or a setup that reuses a locally compiled Unreal Engine (lightweight).

---

## Overview

Two main scripts in the `Scripts/` directory manage building and launching the Docker containers:

1. **`run_container.sh`**  
   - Detects **monolithic** or **lightweight** mode via the `--monolith` (or `-m`) argument.  
   - If `--monolith` is present, uses `carla-ue4.dockerfile` (monolithic). Otherwise, defaults to `carla.dockerfile` (lightweight).  

2. **`build_image.sh`**  
   - Generally called indirectly by `run_container.sh`.  
   - Checks whether `--monolith` was provided and chooses the corresponding Dockerfile.  
   - Handles Epic credentials (from a `.env` file) needed for cloning and building Unreal Engine in the monolithic mode.

---

## Dockerfiles in `Util/Docker/`

### `carla.dockerfile` (Lightweight / Devcontainer)

- Installs only CARLA dependencies, plus NVIDIA GPU support.  
- **Excludes** Unreal Engine; instead, a precompiled UE4 folder (matching Carla’s requirements) must be mounted from the host via `UE4_ROOT`.  
- Ideal for developers who already have a compatible UE4 build and want faster builds with smaller images.

### `carla-ue4.dockerfile` (Monolithic)

- Clones Unreal Engine from Epic’s private GitHub (requires valid Epic credentials). **If you don't have this set up, please follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github) before going any further**.
- Compiles UE4 and then CARLA within a single Docker image, which may exceed 100 GB and take multiple hours to build.  
- The image **retains** the `Dist/` directory, enabling you to run or extract the packaged CARLA binaries without additional scripts like `docker_tools.py` in comparison to `CarlaLegacy.Dockefile`. Although this makes the final image large, modern hardware generally has sufficient resources (e.g., larger SSDs, more RAM) to handle this scale. It also eliminates the need to repeatedly rebuild or rely on external tooling.

You can extract the CARLA package from a monolithic image using a simple Docker command sequence (replacing `0.9.15.2` with your CARLA version):

```bash
docker create --name temp_container carla-0.9.15.2-ue4-jammy-dev
docker cp temp_container:/workspaces/carla-0.9.15.2/Dist/CARLA_$(docker run --rm carla-0.9.15.2-ue4-jammy-dev ls /workspaces/carla-0.9.15.2/Dist | grep CARLA_).tar.gz .
docker rm temp_container
```

If disk usage is a concern and you do not need the final packaged artifacts, you can comment out `make build.utils` and `make package` in `carla-ue4.dockerfile`, or switch to the lightweight approach.

---

## Build UE4 (Prerequisite for Lightweight / Devcontainer)

> Note: Ensure your GitHub account is linked to Epic’s UnrealEngine repository before attempting to clone.

If you plan to use the **lightweight** approach, you need a **compiled** Unreal Engine folder on your host that matches CARLA’s requirements (usually UE4.26). You have the 2 following options

### Option 1: Build Natively

Follow CARLA official documentation: [Unreal Engine - Linux Build](https://carla.readthedocs.io/en/latest/build_linux/#:~:text=wheel%20auditwheel%3D%3D4.0.0-,Unreal%20Engine,-Starting%20with%20version)

### Option 2: Build UE4 in Build Container 


The following steps outline a general workflow for building UE4 inside a Docker container. For detailed instructions, refer to `Engine/Documentation/Docker/run_with_docker.md` in the cloned repository:

```bash
# Example of cloning a custom CarlaUnrealEngine repository
git clone git@github.com:wambitz/CarlaUnrealEngine.git CarlaUE4

cd CarlaUE4
Scripts/run_container.sh

# Inside the container follow official UE4.26 build steps:
./Setup.sh
./GenerateProjectFiles.sh
make

# Exit container
exit
```

These commands generate the required Unreal Engine binaries and store them persistently on your host. You can later reuse them for the CARLA devcontainer. To verify a successful compilation, run the following command inside (or optionally outside) the container:

```bash
Engine/Binaries/Linux/UE4Editor
```
---

## Scripts Usage

### 1. Lightweight Mode

This mode relies on a **locally compiled** Unreal Engine folder. You mount that folder into the container for building CARLA.

```bash
# 1) Set UE4_ROOT to your existing UE4 folder on the host:
export UE4_ROOT=/absolute/path/to/UnrealEngine_4.26

# 2) Run the container WITHOUT the --monolith flag:
./Scripts/run_container.sh

# 3) Once inside the container, build CARLA as usual:
./Update.sh
make PythonAPI
make CarlaUE4Editor

# Optionally build for distribution:
make build.utils
make package
```

- **Result**:  
  - Uses **`carla.dockerfile`** (lightweight).  
  - Mounts your host’s `$UE4_ROOT` folder into `/opt/UE4.26` inside the container.  
  - Expects a fully compiled UE4 on the host. See [Build UE4 for Devcontainer Option](#build-ue4-prerequisite-for-lightweight--devcontainer) for more details.

### 2. Monolithic Mode

In this mode, Docker clones and builds Unreal Engine, then compiles CARLA within the same image.

```bash
# 1) Create a .env file with your Epic credentials at the repo root:
#    EPIC_USER=YourGitHubUsername
#    EPIC_PASS=YourGitHubToken

# 2) Run the container WITH the --monolith flag:
./Scripts/run_container.sh --monolith
```

- **Result**:  
  - Uses **`carla-ue4-monolith.dockerfile`**.  
  - Builds UE4 and CARLA in one Docker image (commonly exceeding 200 GB).  
  - Takes significant time (hours) on most hardware.

---



## Using a Devcontainer for CARLA Server/Client Development

You can use a **Visual Studio Code devcontainer** with the lightweight approach. This setup mounts your host’s directories (including UE4) into a Docker environment. Note that the monolithic image is less suited to devcontainers since it stores everything inside the image.

Create `.devcontainer/devcontainer.json` in your CARLA repository:

```jsonc
{
    "name": "CARLA UE4 Dev (jammy)",
    "image": "carla-ue4-jammy-dev",

    "initializeCommand": "./Scripts/build_image.sh",

    // We do NOT need to set "remoteUser" if the Dockerfile's default user is already correct
    // but you can if you want to be explicit. Also "updateRemoteUserUID" can be false, since
    // our Dockerfile already set the user to our exact UID/GID.
    "updateRemoteUserUID": false,

    "customizations": {
      "vscode": {
        "settings": {
          "terminal.integrated.shell.linux": "bash"
        },
        "extensions": [
          "ms-vscode.cpptools"
        ]
      }
    },

    "postStartCommand": "bash",

    // NOTE1: DO NOT pass --user here (we want the Dockerfile default user, not an override)
    // NOTE2: Make sure /usr/share/vulkan/icd.d/nvidia_icd.json exist in the host
    // NOTE3: Ensure UE4_ROOT environment variable is defined in host
    "runArgs": [
      "--rm",
      "--name", "carla-ue4-jammy-devcontainer",
      "--hostname", "carla-devcontainer",
      "--env", "DISPLAY=${localEnv:DISPLAY}",
      "--volume", "/tmp/.X11-unix:/tmp/.X11-unix",
      "--volume", "/usr/share/vulkan/icd.d/nvidia_icd.json:/usr/share/vulkan/icd.d/nvidia_icd.json",
      "--volume", "${localEnv:UE4_ROOT}:/opt/UE4.26",
      "--gpus", "all"
    ]
}
```

### Important Devcontainer Notes

1. **GPU Access**: Install and configure the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html).  
2. **X11 Forwarding**: Ensure `/tmp/.X11-unix` is accessible and `DISPLAY` is set properly on your host.  
3. **UE4_ROOT Environment Variable**: Must be defined on your host so the devcontainer can mount the UE4 directory.
4. **Vulkan Support**: `/usr/share/vulkan/icd.d/nvidia_icd.json` must be exist in your host so the devcontainer can mount the vulkan configuration.

---

## Tips & Caveats

- **NVIDIA GPU**  
  For hardware acceleration, install the [NVIDIA Container Toolkit](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html).

- **Disk Space**  
  The monolithic image can exceed **200 GB** (e.g., ~221 GB in some builds). Ensure you have enough space before choosing this route.

- **Build Times**  
  - Monolithic mode: ~2 hours on a high-end workstation (e.g., Intel Core i9-14900KF, 64 GB RAM, RTX 4090).  
  - Lightweight mode: Much faster since it reuses a local UE4 build.

- **Volumes**  
  For lightweight mode, `$UE4_ROOT` must point to a fully compiled UE4. If incorrect or missing, builds will fail.

- **Epic Credentials**  
  In monolithic mode, store `EPIC_USER` and `EPIC_PASS` in a `.env` file in the project root.

- **Running Binaries on the Host**  
  **Do not** run `make launch` or `make launch-only` on your host after building in the container, because internal container paths (e.g., `/opt/UE4.26`) do not match your host environment.  
  If you need host access to CARLA binaries, first build a shipping package (`make build.utils && make package`), then run them from the resulting files in `Dist/` on the host.

- **Mount Directory**  
  Avoid modifying `/workspaces` if you’re using VS Code devcontainers, as it might force a rebuild. The container’s internal paths often differ from the host, making direct host launches problematic.


- **`Dist` Folder Handling**  
   Monolithic mode leaves `Dist` in place to facilitate direct usage. If you need a smaller image, remove or adjust `make build.utils` and `make package`.

---

## Known Issues

1. **Truncated Output in `./Update.sh`**  
   Sometimes, `./Update.sh` in the CARLA repository may truncate logs. A workaround is redirecting the output to a file and tailing it.

2. **Audio Support**  
   ALSA or PulseAudio are not pre-configured, so audio features might not work by default.

3. **UE4Editor Rebuilds**  
   In some cases, once `UE4Editor` (built in UE4.26) is opened or used to build CARLA, it may refuse to open again without a rebuild. Errors about missing plugins can occur, a scenario also observed in local (non-Docker) setups. The workaround is to rebuild from the repository using the container or a devcontainer, or carefully adjust configuration files.

---