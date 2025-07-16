
# Use NVIDIA Neural Reconstruction with CARLA

NVIDIA Neural Reconstruction (NuRec) refers to the reconstruction and rendering models and services from NVIDIA that support the seamless ingestion of real-world
data converted to a simulated environment suitable for training and testing Physical AI Agents, including robotics and autononomous driving systems.

With NuRec, developers can convert recorded camera and lidar data into 3D scenes. NuRec uses multiple AI networks to create
interactive 3D test environments where developers can modify the scene and see how the world reacts. Developers can change scenarios,
add synthetic objects, and apply randomizations — such as a child following a bouncing ball into the road — making the initial scenarios
even more challenging. With the NuRec gRPC API, developers can bring rendering services directly to their simulation platform of choice, for example, CARLA.

The NuRec gRPC API serves as a conduit of data and rendering between the CARLA replay and the NuRec container, where the scenes are reconstructed and rendered. You can load pre-trained scenes from the NVIDIA Physical AI Dataset for Autonomous Vehicles and define your scenes using the NuRec gRPC API in a python script (`replay_recording.py`). The diagram below further illustrates the relationship between NuRec and CARLA. 

![NVIDIA NuRec and CARLA](img/carla-nurec-api.svg)

When you run the replay script, CARLA loads the map and actors from the CARLA Server through the CARLA API. Rendering requests in the script return frames from the NuRec container through the NuRec gRPC API. Both APIs serve as a convenient interface to the CARLA and NuRec servers to deliver seamless updates to your simulation.

To use neural rendering in your CARLA simulations, use the NVIDIA Neural Reconstruction API and data from the NVIDIA Physical AI Dataset. Follow the instructions in this guide.

## Before you Begin

Before you get started, make sure you have satisifed the following prerequisites:

- [CARLA 0.9.16 or newer installed](https://carla.readthedocs.io/en/latest/start_quickstart/#carla-installation)
- Python 3.10+

## Setup

To get started with the sample dataset from NVIDIA, use the installer script. If you'd rather customize your dataset, follow the instructions to get the assets from HuggingFace and set up your environment manually.

### Use the Installer Script

To get started quickly and easily with the curated sample set from the [NVIDIA PhysicalAI-Autonomous-Vehicles-NuRec dataset](https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec), navigate to the CARLA directory on your machine and run the following launch script:

```bash
./PythonAPI/nvidia/nurec/install_nurec.sh
```

The script helps you set your HuggingFace access token (if there isn't one already), sets the required environment variables for the NuRec container, pulls the curated sample dataset from HuggingFace, and installs the required Python packages.

### Use a Custom Dataset

If you'd rather customize the datasets you use, follow the instructions below to get the assets, launch the NuRec container, and install the required Python packages. 

1. **Get the pre-trained assets** from the [NVIDIA PhysicalAI-Autonomous-Vehicles-NuRec dataset on HuggingFace](https://huggingface.co/datasets/nvidia/PhysicalAI-Autonomous-Vehicles-NuRec).  

2. **Set up your environment.** Download and install the required Python packages by running the following command from the CARLA directory on your machine:

    ```bash
    pip install -r requirements.txt
    ```
3. **Set your environment variables.** The replay script takes two environment variables — `NUREC_IMAGE` and `CUDA_VISIBLE_DEVICES`.

    * `NUREC_IMAGE` is required and must be set to the full path of the NuRec image in the CARLA repository. Run the following command to set it:
        ```bash
        export NUREC_IMAGE="docker.io/carlasimulator/nvidia-nurec-grpc:0.1.0"
        ```
    * [`CUDA_VISIBLE_DEVICES`](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html#env-vars) is optional and you can use it to designate the GPU that runs the replays. If you don't set it to a specific GPU, the script defaults to "0" and runs on GPU 0. If you've already set this environment variable, the script inherits whatever has previously been set.


## Run the CARLA NuRec Replays

1. **Start the CARLA Server.**  From the directory where your CARLA package exists, run the following command:

    ```bash
    ./CarlaUE4.sh 

    ```

2. **Replay a NuRec Scenario.** Once CARLA is running, open a new terminal window and navigate to the directory where your
   CARLA package exists, then replay a NuRec scenario with one of the following scripts:

   * **Multi-camera replay:** The script provides a complete, multi-view visualization system, ideal for understanding
     how to integrate various camera types and create comprehensive monitoring setups. When you run it, it replays simulations
     with multiple NuRec cameras (front, left cross, right cross) in different camera positions in a Pygame display grid. It
     also supports additional perspectives pulled from standard CARLA cameras attached to the ego vehicle and multiple
     camera feeds with different framerates and resolutions. 

    ```bash
        source vecarla/bin/activate

        cd PythonAPI/nvidia/nurec/

        python example_replay_recording.py --usdz-filename /path/to/scenario.usdz
    ```

   * **Custom camera parameters:** If you need to replicate specific camera hardware or match real-world camera calibrations,
     use this script to configure NuRec cameras with custom intrinsic parameters. The advanced camera configurations available
     include custom F-Theta configuration, precise intrinsic parameter specification (principal point, distortion polynomials),
     custom positioning through camera transform matrices, rolling shutter simulation, and real-time visualization using Pygame.

    ```bash
        source vecarla/bin/activate

        cd PythonAPI/nvidia/nurec/

        python example_custom_camera.py --usdz-filename /path/to/scenario.usdz
    ```

   * **Image capture:** If you need to export and save images from the scenario replays, use this script. It replays
     the NuRec scenario and captures images from both NuRec and CARLA cameras, then saves them to the specified output directory.
     You can customize the framerate and resolution on the NuRec cameras, attach standard CARLA cameras to the ego vehicle,
     display real-time camera feeds using Pygame, and save the captured images as JPEG (.jpg) files in folders organized by
     camera type.

    ```bash
        source vecarla/bin/activate

        cd PythonAPI/nvidia/nurec/

        python example_save_images.py --usdz-filename /path/to/scenario.usdz --output-dir ./captured_images
    ```

Run the replay script with as many sample scenarios as needed.

### Command Line Parameters

The following table explains the available command-line parameters for the scripts:

| Parameter | Long Form | Default | Description |
|-----------|-----------|---------|-------------|
| -h | --host | 127.0.0.1 | IP address of the CARLA host server |
| -p | --port | 2000 | TCP port for the CARLA server |
| -u | --usdz-filename | (required) | Path to the USDZ file containing the NuRec scenario |
| -c | --camera | camera_front_wide_120fov | Name of the camera to use for visualization |
| --move-spectator | | False | Move the spectator camera to follow the ego vehicle |