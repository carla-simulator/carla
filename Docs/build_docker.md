# Running CARLA in Docker

Linux users can pull an image of a CARLA release and run the server in a container. That is useful to:

- run CARLA without installing its dependencies
- run several CARLA servers and map them to different GPUs
- run the server with no display at all

- [__Before you begin__](#before-you-begin)
- [__Pull the CARLA Docker image__](#pull-the-carla-docker-image)
- [__Run the image__](#run-the-image)

!!! Note
    The published images are of the released versions, currently `0.10.0` on Unreal Engine 5.5. There is no image of the Unreal Engine 5.8 line, so a client wheel built from this tree does not match the server in the image; use the wheel that ships inside it.

---
## Before you begin

You will need to have installed:

- __Docker:__ Follow the installation instructions [here](https://docs.docker.com/engine/install/).
- __Post installation:__ Follow [these instructions](https://docs.docker.com/engine/install/linux-postinstall/) to avoid the need for `sudo` commands
- __NVIDIA Container Toolkit__ (1.14 or later for CDI support): exposes NVIDIA GPUs to Linux containers. Install it by following the [installation guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html), then configure CDI (Container Device Interface):

    ```sh
    sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml
    sudo systemctl restart docker
    ```

---
## Pull the CARLA Docker image

Pull the image with the following command:

```sh
docker pull carlasim/carla:0.10.0
```

---
## Run the image

### Without a display (CDI, recommended)

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

### Without a display (legacy runtime)

```sh
docker run \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

### With a display (CDI, recommended)

To run the Docker image with a display, you will need the `x11` display protocol:

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    --user=$(id -u):$(id -g) \
    --env=DISPLAY=$DISPLAY \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -nosound
```

### With a display (legacy runtime)

```sh
docker run \
    --runtime=nvidia \
    --net=host \
    --user=$(id -u):$(id -g) \
    --env=DISPLAY=$DISPLAY \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    --volume="/tmp/.X11-unix:/tmp/.X11-unix:rw" \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -nosound
```

!!! Note
    `--net=host` gives the container the host's ports directly. Without it, publish the ports CARLA uses explicitly: `-p 2000-2002:2000-2002`. The native ROS 2 interface is started the same way as outside a container, by appending `-ros2` to the `CarlaUnreal.sh` command.
