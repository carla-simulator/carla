# Build Unreal Engine and CARLA in Docker

This guide explains how Unreal Engine and CARLA can be built from scratch using Docker. The resulting image can then used to create CARLA packages or to prepare assets for use in a CARLA package. This process should not be confused with the pre-built CARLA Docker image used to run CARLA on multiple servers or without a display. The documentation for that can be found [here](build_docker.md).

- [__Before you begin__](#before-you-begin)
    - [__System Requirements__](#system-requirements)
    - [__Software requirements__](#software-requirements)
- [__Building the images__](#building-the-images)
- [__Next Steps: Packages__](#next-steps-packages)

---

## Before you begin

##### System Requirements

You will need to meet the following system requirements:

- Minimum 8GB of RAM
- Minimum 600GB available disk space for the initial container build process

##### Software requirements

__Docker:__ 

Install Docker by following the installation instructions [here](https://docs.docker.com/engine/install/).

__Python__: 

You will need to have Python 3.6 or higher installed and properly set in your system Path. For installation instructions and Python documentation, check [here](https://www.python.org/downloads/).

__Unreal Engine GitHub Access__:

Starting with version 0.9.12, CARLA uses a modified fork of Unreal Engine 4.26. This fork contains patches specific to CARLA. This will be downloaded during the Docker build process. For this download, __you need to have a GitHub account linked to Unreal Engine's account__. If you don't have this set up, please follow [this guide](https://www.unrealengine.com/en-US/ue4-on-github) before going any further. You will need to log in to your account during the build process.

__CARLA:__

The Dockerfiles and tools needed to build Unreal Engine for CARLA and CARLA itself are located in the `Util/Docker` directory of the CARLA source repository. 

If you don't already have it, download the repository using the following command:

```sh
git clone https://github.com/carla-simulator/carla
```

---

## Building the image

!!! note 
    If you are using a version earlier than `0.9.16`, please refer to the documentation for that specific version.

We provide scripts to simplify the image-building process. You can create a monolithic CARLA image using the following command:

!!! Warning
    This process may take a significant amount of time.

```sh
Util/Docker/build.sh --monolith --epic-user=<GITHUB_USERNAME> --epic-token=<GITHUB_ACCESS_TOKEN>
```

To build a specific branch or tag of the CARLA repository, use the following command:

```sh
./build.sh --monolith --branch <BRANCH_OR_TAG_NAME> --epic-user=<GITHUB_USERNAME> --epic-token=<GITHUB_ACCESS_TOKEN>
```

---

## Next Steps: Packages

The CARLA image created in this guide is used to create standalone CARLA packages or to package assets such as maps or meshes so they can be used in a CARLA package. This is achieved through the use of the `docker_tools.py` script found in `Util/Docker`. This script uses [`docker-py`](https://github.com/docker/docker-py) to work with the Docker image.

The `docker_tools.py` script can be used to:

- __Create a CARLA package__: Find the tutorial [here](tuto_A_create_standalone.md#export-a-package-using-docker)
- __Cook assets to be consumed in a CARLA package:__ Find the tutorial [here](tuto_A_add_props.md#ingestion-in-a-carla-package)
- __Prepare a map so it's ready for use in a CARLA package:__ Find the tutorial [here](tuto_M_add_map_package.md)

---

Any issues or doubts related with this topic can be posted in the CARLA forum.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
