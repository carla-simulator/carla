# CARLA in Docker

Linux users can pull an image based on a CARLA release to run in a Docker container. This is useful for users who:

- Want to run CARLA without needing to install all dependencies
- Run multiple CARLA servers and perform GPU mapping
- Run the CARLA server without a display

## Before you begin

You will need to have installed:

- __Docker:__ Follow the installation instructions [here](https://docs.docker.com/engine/install/).
- __Post installation:__ Follow [these instructions](https://docs.docker.com/engine/install/linux-postinstall/) to avoid the need for `sudo` commands
- __NVIDIA Container Toolkit v2:__ The NVIDIA Container Toolkit exposes NVIDIA GPUs to Linux containers. Install the toolkit by following the [installation guide](https://docs.nvidia.com/datacenter/cloud-native/container-toolkit/latest/install-guide.html). After installation, configure CDI (Container Device Interface) support:

    ```sh
    sudo nvidia-ctk cdi generate --output=/etc/cdi/nvidia.yaml
    sudo systemctl restart docker
    ```

## Pull the CARLA Docker image

Pull the image with the following command:

```sh
docker pull carlasim/carla:0.10.0
```

## Run the image

**Running the Docker image without display (CDI — recommended)**:

```sh
docker run \
    --device=nvidia.com/gpu=all \
    --net=host \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

**Running the Docker image without display (legacy runtime)**:

```sh
docker run \
    --runtime=nvidia \
    --net=host \
    --env=NVIDIA_VISIBLE_DEVICES=all \
    --env=NVIDIA_DRIVER_CAPABILITIES=all \
    carlasim/carla:0.10.0 bash CarlaUnreal.sh -RenderOffScreen -nosound
```

---

**Running the Docker image with a display (CDI — recommended)**:

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

**Running the Docker image with a display (legacy runtime)**:

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
