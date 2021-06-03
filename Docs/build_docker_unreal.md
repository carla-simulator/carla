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

- 64-bit version of Docker is Ubuntu 16.04+
- Minimum 8GB of RAM
- Minimum 600GB available disk space for the initial container build process

##### Software requirements

__Docker:__ 

Install Docker by following the installation instructions [here](https://docs.docker.com/engine/install/).

__Python__: 

You will need to have Python 3.6 or higher installed and properly set in your system Path. For installation instructions and Python documentation, check [here](https://www.python.org/downloads/).

__ue4-docker:__ 

_ue4-docker_ is a Python package that contains Dockerfiles and build infrastructure to facilitate the building of Docker images for Unreal Engine 4. You can find more information and documentation [here](https://docs.adamrehn.com/ue4-docker/read-these-first/introduction-to-ue4-docker).

To install _ue4-docker_, run the following command:

```sh
pip3 install ue4-docker
```

Access to port 9876 is required during the build of the Unreal Engine image. If the host system is running an active firewall that blocks that port, you can run the following command to automatically configure access:

```sh
ue4-docker setup
```

__CARLA:__

The Dockerfiles and tools needed to build Unreal Engine for CARLA and CARLA itself are located in the `Util/Docker` directory of the CARLA source repository. 

If you don't already have it, download the repository using the following command:

```sh
git clone https://github.com/carla-simulator/carla
```

---

## Building the images

The following steps will each take a long time.

__1. Build Unreal Engine.__

Run the following command to create a Docker image containing a compiled version of Unreal Engine 4.24.3. Change the version if needed:

```sh
cd Util/Docker

ue4-docker build 4.24.3 --no-engine --no-minimal
```

__2. Build the CARLA prerequisites image.__

The following command will build an image called `carla-prerequisites` using `Prerequisites.Dockerfile`:

```sh
docker build -t carla-prerequisites -f Prerequisites.Dockerfile .
```

__3. Build the final CARLA image.__

The following command will use the image created in the previous step to build the final CARLA image based on the current master branch (latest release) of the CARLA repository:

```sh
docker build -t carla -f Carla.Dockerfile .
```

If you would like to build a specific branch or tag of the CARLA repository, run the following command:

```sh
docker build -t carla -f Carla.Dockerfile . --build-arg GIT_BRANCH=<branch_or_tag_name>
```

__4. Clean up intermediate build images.__

Run the following command to remove intermediate images left over from the Unreal Engine build process:

```sh
ue4-docker clean
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
