# Running CARLA in Docker

Linux users can pull an image based on a CARLA release to run in a Docker container. This is useful for users who:

- Want to run CARLA without needing to install all dependencies
- Run multiple CARLA servers and perform GPU mapping
- Run the CARLA server without a display

- [__Before you begin__](#before-you-begin)
- [__Pull the CARLA Docker image__](#pull-the-carla-docker-image)
- [__Run the image__](#run-the-image)
- [__Get the client library__](#get-the-client-library)
- [__Run several servers on different GPUs__](#run-several-servers-on-different-gpus)

---
## Before you begin

You will need to have installed:

- __Docker:__ Follow the installation instructions [here](https://docs.docker.com/engine/install/).
- __Post installation:__ Follow [these instructions](https://docs.docker.com/engine/install/linux-postinstall/) to avoid the need for `sudo` commands.
- __NVIDIA Container Toolkit__ (1.12 or later for CDI support): exposes NVIDIA GPUs to Linux containers. Install it by following the [installation guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html), then configure CDI (Container Device Interface):

    ```sh
    sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml
    sudo systemctl restart docker
    ```

    The legacy runtime commands below use `--runtime=nvidia` instead, which needs the toolkit registered as a Docker runtime:

    ```sh
    sudo nvidia-ctk runtime configure --runtime=docker
    sudo systemctl restart docker
    ```

---
## Pull the CARLA Docker image

Pull the image, tag `1.0` (see the [CARLA 1.0 release](https://github.com/carla-simulator/carla/releases/tag/1.0)), with the following command:

```sh
docker pull carlasim/carla:1.0
```

Always pull a tagged version. The `latest` tag is an old Unreal Engine 4 image.

---
## Run the image

### Without a display (CDI, recommended)

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    carlasim/carla:1.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

### Without a display (legacy runtime)

```sh
docker run \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    carlasim/carla:1.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
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
    carlasim/carla:1.0 bash CarlaUnreal.sh -nosound
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
    carlasim/carla:1.0 bash CarlaUnreal.sh -nosound
```

!!! Note
    `--net=host` gives the container the host's ports directly. Without it, publish the ports CARLA uses explicitly: `-p 2000-2002:2000-2002`. The native ROS 2 interface is started the same way as outside a container, by appending `-ros2` to the `CarlaUnreal.sh` command.

---
## Get the client library

The server runs in the container, but your scripts run on the host and need the client library of the same version. The image contains the matching wheel. Copy it to the current directory and install it:

```sh
docker run --rm \
    --user=$(id -u):$(id -g) \
    --volume="$PWD:/out" \
    carlasim/carla:1.0 bash -c "cp PythonAPI/carla/dist/carla-*.whl /out/"
pip3 install carla-*.whl
```

---
## Run several servers on different GPUs

Give each container one GPU and its own RPC port. Each server also uses the two ports after it, so leave a gap between them:

```sh
# Terminal 1
docker run --device=nvidia.com/gpu=0 --net=host \
    carlasim/carla:1.0 bash CarlaUnreal.sh -RenderOffScreen -nosound -carla-rpc-port=2000
# Terminal 2
docker run --device=nvidia.com/gpu=1 --net=host \
    carlasim/carla:1.0 bash CarlaUnreal.sh -RenderOffScreen -nosound -carla-rpc-port=3000
```

Connect each client to its server's port, for example `carla.Client('localhost', 3000)`.
