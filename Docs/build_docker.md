# CARLA in Docker

Users can pull an image based on a CARLA release to run in a Docker container. This is useful for users who:

- Want to run CARLA without needing to install all dependencies
- Run multiple CARLA servers and perform GPU mapping
- Run the CARLA server without a display

This tutorial explains the requirements to run the CARLA image and how to run it with or without a display.

- [__Before you begin__](#before-you-begin)
- [__Running CARLA in a container__](#running-carla-in-a-container)
- [__Running CARLA in a devcontainer__](#running-carla-in-a-devcontainer)

---
## Before you begin

You will need to have installed:

- __Docker:__ Follow the installation instructions [here](https://docs.docker.com/engine/install/).
- __NVIDIA Container Toolkit:__ The NVIDIA Container Toolkit is a library and toolset that exposes NVIDIA graphics devices to Linux containers. It is designed specifically for Linux containers running on Linux host systems. Install the `nvidia-container-toolkit` package by following the instructions [here](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html#installation-guide).

!!! note
    Docker requires sudo to run. Follow [this guide](https://docs.docker.com/install/linux/linux-postinstall/) to add users to the docker sudo group.

---
## Running CARLA in a container

__1. Pull the CARLA image.__

You can pull either the latest CARLA image or a specific release version. The latest image refers to the most [recent packaged release](https://github.com/carla-simulator/carla/releases). To pull the image, run one of the following commands:

```sh
# Pull the latest image
docker pull carlasim/carla:latest

# Pull a specific version
docker pull carlasim/carla:0.9.16
```

__2. Run the CARLA container.__

!!! note
    The way CARLA is executed inside a Docker container has changed over time. If you are using a version earlier than `0.9.16`, please refer to the documentation for that specific version.

Running CARLA without display:

```
docker run \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    carlasim/carla:0.9.16 bash CarlaUE4.sh -RenderOffScreen -nosound

```

Running CARLA with a display:

!!! note
    To run the Docker image with a display, you will need the `x11` display protocol.

```
docker run \
    --runtime=nvidia \
    --net=host \
    --user=$(id -u):$(id -g) \
    --env=DISPLAY=$DISPLAY \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    carlasim/carla:0.9.16 bash CarlaUE4.sh -nosound
```

---

Any issues or doubts related with this topic can be posted in the CARLA forum.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>

---
