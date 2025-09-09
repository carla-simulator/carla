# CARLA Docker Dev Environment

CARLA can be built and developed entirely inside Docker containers. This approach eliminates concerns about dependencies, Python versions, or system configuration—everything is already set up in our Dockerfiles. For example, our Dockerfiles provide support for multiple Python versions, with all required libraries pre-installed.

We offer two Docker-based approaches for building CARLA: **monolith mode** and **lightweight mode**.

Your choice between **monolith** and **lightweight** depends on your intended use, as well as factors such as disk space, build time, and whether you prefer a fully self-contained environment (monolith) or one that reuses a locally compiled Unreal Engine (lightweight).

## Monolith

This Docker image is intended for use with the `docker_tools.py` scripts. Refer to [Build Unreal Engine and CARLA in Docker](build_docker_unreal.md) for more information.

* Bundles Unreal Engine (UE4) and CARLA into a single Docker image.  
* Requires a long build process and produces a large final image (often 100+ GB).  
* Provides a fully self-contained environment with everything compiled inside Docker.


## Lightweight (Recommended)

This Docker image is intended for development purposes and is the recommended way to build packages and work with CARLA. Compared to monolith mode, it offers greater flexibility and faster iteration.

* Installs only the dependencies (plus NVIDIA support) required to build and run CARLA.  
* Requires mounting an existing **UE4** build from the host into the container.
* Much faster to build but relies on a locally compiled Unreal Engine folder.

---

## Build the docker image

### Monolith

As previously said, the monolith build bundles both Unreal Engine (UE4) and CARLA into a single Docker image. The process can take several hours and the resulting image may exceed 200 GB in size.

Because this build clones Unreal Engine from Epic Games’ private GitHub repository, you must have valid Epic credentials linked to your GitHub account. **If you don't have this set up, please follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github) before proceeding**.

To build the monolith image, run the following command:

```sh
Util/Docker/build.sh --monolith --epic-user <GITHUB_USERNAME> --epic-token <GITHUB_ACCESS_TOKEN>

# Or build the monolith image for a specific CARLA branch
Util/Docker/build.sh --monolith --epic-user <GITHUB_USERNAME> --epic-token <GITHUB_ACCESS_TOKEN> --branch <BRANCH_NAME_OR_TAG>
```

This will create a Docker image named `carla-monolith:<branch>`.

### Lightweight (development)

The lightweight build installs only CARLA’s dependencies, so no additional setup is required at this stage:

```sh
Util/Docker/build.sh --dev
```
---

## Run the docker images

### Monolith

This image is not intended to be run directly. Instead, it is used through the docker_tools.py script. Refer to [Build Unreal Engine and CARLA in Docker](build_docker_unreal.md) for more information.

```sh
# Build a CARLA package
# Note: The package will be created using the branch that was used to build the image
./docker_tools.py

# Import some assets
./docker_tools.py --input /assets/to/import/path --output /output/path
```

### Lightweight

The lightweight/development workflow requires a compiled Unreal Engine folder on your host machine that matches CARLA’s requirements. This folder will remain on your host and be mounted into the container. You can either build it natively following the official CARLA documentation or use the lightweight container to compile it inside Docker:

!!! note
    Ensure your GitHub account is linked to Epic’s UnrealEngine repository before attempting to clone.

```sh
# Clone a Carla-specific Unreal Engine repository
git clone git@github.com:wambitz/CarlaUnrealEngine.git CarlaUE4

# Export UE4_ROOT so it can be mounted inside the dev container
export UE4_ROOT=$(pwd)/CarlaUE4

# From the CARLA root, start the lightweight container
Util/Docker/run.sh --dev

# Inside the container, build UE4.26 following the official steps:
cd /workspace/unreal-engine
./Setup.sh && ./GenerateProjectFiles.sh && make

# Exit the container
exit
```
After building Unreal Engine, you can run the lightweight container again:

!!! note
    Make sure the `UE4_ROOT` environment variable points to the root directory of your compiled UE4 build.

```sh
Util/Docker/run.sh --dev
```

Once inside the container, you can continue with the normal CARLA build steps:

```bash
./Update.sh
make PythonAPI
make CarlaUE4Editor
make package ARGS="--python-version=3.10,3.11,3.12"
```

---

## Using a Devcontainer for CARLA Server/Client Development

You can use a **Visual Studio Code devcontainer** with the lightweight approach. This setup mounts your host’s directories (including UE4) into a Docker environment. Note that the monolith image is not well suited to devcontainers since it stores everything inside the image.

Create a `.devcontainer/devcontainer.json` in your CARLA repository with the following content:

```json
{
    "name": "CARLA UE4 Dev",
    "image": "carla-development:ue4-20.04",

    "initializeCommand": "./Util/Docker/build.sh --dev --ubuntu-distro 20.04",

    // We do NOT need to set "remoteUser" if the Dockerfile's default user is already correct
    // but you can if you want to be explicit. Also "updateRemoteUserUID" can be false, since
    // our Dockerfile already set the user to our exact UID/GID.
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

    // NOTE1: DO NOT pass --user here (we want the Dockerfile default user, not an override)
    // NOTE3: Ensure UE4_ROOT environment variable is defined in host
    "runArgs": [
      "--rm",
      "--runtime", "nvidia",
      "--name", "carla-ue4-development-20.04",
      "--env", "NVIDIA_VISIBLE_DEVICES=all",
      "--env", "NVIDIA_DRIVER_CAPABILITIES=all",
      "--env", "UE4_ROOT=/workspaces/unreal-engine",
      "--env", "CARLA_UE4_ROOT=/workspaces/carla",
      "--env", "DISPLAY=${localEnv:DISPLAY}",
      "--volume", "/tmp/.X11-unix:/tmp/.X11-unix",
      "--volume", "/var/run/docker.sock:/var/run/docker.sock",
      "--volume", "${localEnv:UE4_ROOT}:/workspaces/unreal-engine",
      "--mount", "source=carla-development-ue4-20.04,target=/home/carla",
      "--gpus", "all"
    ]
}
```
---

## Tips & Known Issues

1. **Running Binaries on the Host**  
  **Do not** run `make launch` or `make launch-only` on your host after building in the container, because internal container paths (e.g., `/workspaces/unreal-engine`) do not match your host environment.  
  If you need host access to CARLA binaries, first build a shipping package (`make package`), then run them from the resulting files in `Dist/` on the host.

2. **Truncated Output in `./Update.sh`**  
   Sometimes, `./Update.sh` in the CARLA repository may truncate logs. A workaround is redirecting the output to a file and tailing it.

3. **Audio Support**  
   ALSA and PulseAudio are not pre-configured, so you might see audio warnings when executing some scripts inside the container.

---
