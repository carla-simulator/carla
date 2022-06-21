# CARLA in Docker

Users can pull an image based on a CARLA release to run in a Docker container. This is useful for users who:

- Want to run CARLA without needing to install all dependencies
- Run multiple CARLA servers and perform GPU mapping
- Run the CARLA server without a display

This tutorial explains the requirements to run the CARLA image and how to run the image with both OpenGL and Vulkan graphics APIs.

- [__Before you begin__](#before-you-begin)
- [__Running CARLA in a container__](#running-carla-in-a-container)
- [__Off-screen mode__](#off-screen-mode)

---
## Before you begin

You will need to have installed:

- __Docker:__ Follow the installation instructions [here](https://docs.docker.com/engine/install/).
- __NVIDIA Container Toolkit:__ The NVIDIA Container Toolkit is a library and toolset that exposes NVIDIA graphics devices to Linux containers. It is designed specifically for Linux containers running on Linux host systems or within Linux distributions under version 2 of the Windows Subsystem for Linux. Install the `nvidia-docker2` package by following the instructions [here](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/install-guide.html#installation-guide).

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
docker pull carlasim/carla:0.9.12
```

__2. Run the CARLA container.__

Different versions of CARLA support different graphics APIs which can affect the conditions in which the Docker image can run:

- 0.9.12 supports only Vulkan
- 0.9.7+ supports both Vulkan and OpenGL.


__CARLA 0.9.12__

To run CARLA with a display:

```
sudo docker run --privileged --gpus all --net=host -e DISPLAY=$DISPLAY carlasim/carla:0.9.12 /bin/bash ./CarlaUE4.sh
```

To run CARLA in off-screen mode:

```
sudo docker run --privileged --gpus all --net=host -v /tmp/.X11-unix:/tmp/.X11-unix:rw carlasim/carla:0.9.12 /bin/bash ./CarlaUE4.sh -RenderOffScreen
```

__CARLA 0.9.7 to 0.9.11__

To run CARLA using Vulkan:

```sh
sudo docker run --privileged --gpus all --net=host -e DISPLAY=$DISPLAY -e SDL_VIDEODRIVER=x11 -v /tmp/.X11-unix:/tmp/.X11-unix:rw carlasim/carla:0.9.11 /bin/bash ./CarlaUE4.sh -vulkan <-additonal-carla-flags>
```

!!! Note
    This command will allow you to run the CARLA image with Vulkan as long as your machine has a display. See the [rendering documentation](adv_rendering_options.md#off-screen-mode) for information on running with Vulkan in off-screen mode.

To run CARLA using OpenGL:

```sh
docker run -e DISPLAY=$DISPLAY --net=host --gpus all --runtime=nvidia carlasim/carla:<version> /bin/bash CarlaUE4.sh -opengl <-additonal-carla-flags>
```

__3. (Optional) Configure Docker flags.__

The above commands use some Docker flags that can be configured according to your needs:

- __Networking:__ The [`--net=host`](https://docs.docker.com/engine/reference/run/#network-settings) argument will allow the container to share the host's entire network. If you prefer to [map specific ports](https://docs.docker.com/engine/reference/run/#expose-incoming-ports) on the host machine to container ports, use the flag `-p <host-ports>:<container-ports>`.
- __GPUs:__ You can choose to use all GPUs with `--gpus all`, or target specific GPUs with `--gpus '"device=<gpu_01>,<gpu_02>"'`. See [here](https://docs.docker.com/config/containers/resource_constraints/#gpu) for more information.

---

## Off-screen mode

OpenGL requires no configuration if you are running CARLA on a machine without a display, however you will need to perform some extra steps to do the same using Vulkan prior to CARLA 0.9.12. See the [rendering documentation](adv_rendering_options.md#off-screen-mode) for information.

---

Any issues or doubts related with this topic can be posted in the CARLA forum.

<div class="build-buttons">
<p>
<a href="https://github.com/carla-simulator/carla/discussions/" target="_blank" class="btn btn-neutral" title="Go to the CARLA forum">
CARLA forum</a>
</p>
</div>
