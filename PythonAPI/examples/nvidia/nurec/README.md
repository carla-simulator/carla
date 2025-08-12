# CARLA - NUREC Integration

This module provides integration between CARLA and NVIDIA's NUREC (NVIDIA Neural Reconstruction Engine). It allows you to replay real-world recordings in CARLA by reconstructing traffic scenarios from NUREC data.

## Overview

NUREC (NVIDIA Neural Reconstruction Engine) is a framework for reconstructing real-world traffic scenarios from sensor data. This integration allows these reconstructions to be replayed in the CARLA simulator, providing a bridge between real-world data and simulation.

Key features:
- Load and replay NUREC scenarios in CARLA
- Visualize ego vehicle and other traffic participants
- Support for camera views including spectator camera
- Transformation between coordinate systems

## Requirements

To use this integration, you'll need:

- CARLA 0.9.16 or newer
- Python 3.10
- The following Python packages:
  - pygame
  - numpy
  - scipy
  - grpc
  - carla
  - nvidia-nvimgcodec-cu12

## Installation

For detailed installation instructions, please refer to the [NUREC Installation Guide](../../../Docs/nvidia_nurec.md).

## Running CARLA with NUREC

### Step 1: Start CARLA

Start carla with the standard process.

### Step 2: Replay a NUREC Scenario

Make sure to specify the nurec image to use by setting the NUREC_IMAGE env variable. See [the docs](../../../Docs/nvidia_nurec.md) for details.

Once CARLA is running, you can replay a NUREC scenario using the `example_replay_recording.py` script:

```bash
python example_replay_recording.py --usdz-filename $(pwd)/maps/clipgt-9e849eeb-073f-424c-838c-493b56c806fb.usdz --move-spectator
```

## Command Line Parameters for example_replay_recording.py

The following table explains the available command-line parameters for the `example_replay_recording.py` script:

| Parameter | Long Form | Default | Description |
|-----------|-----------|---------|-------------|
| -h | --host | 127.0.0.1 | IP address of the CARLA host server |
| -p | --port | 2000 | TCP port for the CARLA server |
| -np | --nurec-port | 46435 | Port for the NUREC server |
| -u | --usdz-filename | (required) | Path to the USDZ file containing the NUREC scenario |
| --move-spectator | | False | Move the spectator camera to follow the ego vehicle |

## Module Structure

- `nurec_integration.py`: Main integration class that handles NUREC service management and scenario replay
- `scenario.py`: Core classes for loading and managing NUREC scenarios
- `track.py`: Track representation and interpolation functions for vehicle trajectories
- `constants.py`: Constants used throughout the module
- `projection_functions.py`: Coordinate system transformation functions
- `pygame_display.py`: Visualization using Pygame for camera feeds
- `example_*.py`: Example scripts demonstrating different use cases:
  - `example_replay_recording.py`: Basic scenario replay with multiple cameras
  - `example_save_images.py`: Save camera images to disk
  - `example_custom_camera.py`: Configure custom camera parameters
- `grpc_proto/`: Protocol buffer definitions and generated code for NUREC service communication
- `tools/`: Additional utility tools for blueprint size extraction and other tasks

## Additional Information

- The blueprint size JSON files are used to match NUREC object dimensions with CARLA blueprints
- Use the `extract_blueprint_sizes.py` tool to generate size information for custom CARLA blueprints

## Starting the NUREC Service

The NUREC service is automatically started when you create a `NurecScenario` instance. The service provides the following main functionality:
- Render RGB images from scenes
- Get version information
- List available scenes, cameras, and trajectories

The service runs on port 46435 by default, which can be configured using the `--nurec-port` parameter. The service will automatically shut down when the `NurecScenario` instance is destroyed.

You can control the service container behavior using the `reuse_container` parameter when creating a `NurecScenario` instance:
- When `reuse_container=True` (default): The service container will be reused if it already exists, preventing multiple instances from running simultaneously
- When `reuse_container=False`: A new service container will be created each time, which is useful for testing or when you need to ensure a clean state

Example usage:
```python
# Reuse existing container (default)
scenario = NurecScenario(client, usdz_filename)

# Force new container
scenario = NurecScenario(client, usdz_filename, reuse_container=False)
```

## Tools

### Extract Blueprint Sizes Tool
See [tools.md](tools.md) for documentation on the blueprint extraction tool.

