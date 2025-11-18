# BehaviorAgent Data Collection for Bench2Drive

This directory contains tools for collecting autonomous driving datasets using CARLA's BehaviorAgent in the Bench2Drive format.

## Overview

The data collection system consists of three main components:

1. **SensorManager** (`carla/agents/tools/sensor_manager.py`) - Manages all sensors attached to the vehicle
2. **DataCollector** (`carla/agents/tools/data_collector.py`) - Collects and saves data in Bench2Drive format
3. **Example Script** (`behavior_agent_data_collection.py`) - Demonstrates complete data collection workflow

## Features

- **360° Sensor Coverage**: 6 RGB cameras + 1 top-down camera for full visibility
- **Multi-Modal Data**: RGB, depth, semantic segmentation, instance segmentation, and LiDAR
- **Bench2Drive Compatible**: Data saved in B2D format for easy integration with existing datasets
- **Behavior Profiles**: Collect data with cautious, normal, or aggressive driving behaviors
- **CAN Bus Telemetry**: Vehicle state, control inputs, and navigation data
- **Traffic Simulation**: Configurable number of AI vehicles and pedestrians

## Quick Start

### Prerequisites

- CARLA 0.9.15 running on localhost (or specify `--host`)
- Python 3.7+
- Required Python packages: numpy, PIL

### Basic Usage

```bash
# Start CARLA server first
./CarlaUE5.sh

# In another terminal, run data collection
cd PythonAPI/examples
python behavior_agent_data_collection.py \
    --town Town01 \
    --behavior normal \
    --weather 0 \
    --max-frames 1000 \
    --output-dir ./my_dataset
```

### Advanced Usage

```bash
python behavior_agent_data_collection.py \
    --host 127.0.0.1 \
    --port 2000 \
    --sync \
    --town Town03 \
    --behavior aggressive \
    --weather 5 \
    --route-id 42 \
    --max-frames 5000 \
    --num-vehicles 50 \
    --num-pedestrians 20 \
    --output-dir /path/to/dataset
```

## Command Line Arguments

| Argument | Default | Description |
|----------|---------|-------------|
| `--host` | 127.0.0.1 | IP address of CARLA server |
| `--port` | 2000 | TCP port for CARLA connection |
| `--tm-port` | 8000 | Traffic Manager port |
| `--sync` | False | Enable synchronous mode (recommended) |
| `--behavior` | normal | Driving behavior: cautious, normal, aggressive |
| `--town` | Town01 | Map/town to load |
| `--weather` | 0 | Weather preset ID (0-13) |
| `--route-id` | 1 | Route number for dataset naming |
| `--output-dir` | ./b2d_dataset | Root directory for dataset |
| `--max-frames` | 1000 | Maximum frames to collect |
| `--num-vehicles` | 30 | Number of traffic vehicles |
| `--num-pedestrians` | 10 | Number of pedestrians |

## Dataset Structure

The collected data follows the Bench2Drive format:

```
output_dir/
└── GeneralDriving_Town01_Route001_Weather00/
    ├── anno/
    │   ├── 00000.json.gz
    │   ├── 00001.json.gz
    │   └── ...
    ├── camera/
    │   ├── rgb_front/
    │   │   ├── 00000.jpg
    │   │   └── ...
    │   ├── rgb_front_left/
    │   ├── rgb_front_right/
    │   ├── rgb_back/
    │   ├── rgb_back_left/
    │   ├── rgb_back_right/
    │   ├── rgb_top_down/
    │   ├── depth_front/
    │   │   ├── 00000.png
    │   │   └── ...
    │   ├── depth_front_left/
    │   ├── ... (depth for all 6 cameras)
    │   ├── semantic_front/
    │   │   ├── 00000.png
    │   │   └── ...
    │   ├── ... (semantic for all 6 cameras)
    │   ├── instance_front/
    │   │   ├── 00000.png
    │   │   └── ...
    │   └── ... (instance for all 6 cameras)
    └── lidar/
        ├── 00000.ply
        ├── 00001.ply
        └── ...
```

## Sensor Configuration

### Cameras (Bench2Drive Preset)

| Camera | Position (x, y, z) | Rotation (yaw) | Resolution | FOV |
|--------|-------------------|----------------|------------|-----|
| Front | (0.80, 0.0, 1.60) | 0° | 1600x900 | 70° |
| Front Left | (0.27, -0.55, 1.60) | -55° | 1600x900 | 70° |
| Front Right | (0.27, 0.55, 1.60) | 55° | 1600x900 | 70° |
| Back | (-2.0, 0.0, 1.60) | 180° | 1600x900 | 110° |
| Back Left | (-0.32, -0.55, 1.60) | -110° | 1600x900 | 70° |
| Back Right | (-0.32, 0.55, 1.60) | 110° | 1600x900 | 70° |
| Top Down | (0.0, 0.0, 50.0) | 0° (pitch: -90°) | 1600x900 | 90° |

Each RGB camera has corresponding depth, semantic segmentation, and instance segmentation cameras at the same position.

### LiDAR

- **Position**: (-0.39, 0.0, 1.84)
- **Range**: 85 meters
- **Channels**: 64
- **Rotation Frequency**: 10 Hz
- **Points per Second**: 600,000

### Other Sensors

- **IMU**: 20 Hz sampling rate
- **GNSS**: Global positioning
- **Speedometer**: 20 Hz reading frequency

## Annotation Data Format

Each `anno/*.json.gz` file contains:

```json
{
  "x": 123.45,                    // Global X position
  "y": 67.89,                     // Global Y position
  "theta": 1.57,                  // Heading angle (radians)
  "speed": 8.5,                   // Speed (m/s)
  "acceleration": [0.1, 0.0, -9.8], // 3-axis acceleration
  "angular_velocity": [0.0, 0.0, 0.05], // 3-axis angular velocity
  "throttle": 0.5,                // Throttle [0.0, 1.0]
  "steer": -0.1,                  // Steering [-1.0, 1.0]
  "brake": 0.0,                   // Brake [0.0, 1.0]
  "reverse": false,               // Reverse gear
  "weather": 0,                   // Weather ID
  "x_command_far": 150.0,         // Far waypoint X
  "y_command_far": 80.0,          // Far waypoint Y
  "command_far": 2,               // Far command
  "x_command_near": 130.0,        // Near waypoint X
  "y_command_near": 75.0,         // Near waypoint Y
  "command_near": 2,              // Near command
  "x_target": 200.0,              // Target X
  "y_target": 100.0,              // Target Y
  "next_command": 3,              // Next navigation command
  "should_brake": false,          // Whether to brake
  "only_ap_brake": false,         // Autopilot brake only
  "bounding_boxes": {},           // 3D bounding boxes (optional)
  "sensors": {}                   // Sensor calibration (optional)
}
```

## Weather Presets

| ID | Weather |
|----|---------|
| 0 | Clear Noon |
| 1 | Cloudy Noon |
| 2 | Wet Noon |
| 3 | Wet Cloudy Noon |
| 4 | Mid Rainy Noon |
| 5 | Hard Rain Noon |
| 6 | Soft Rain Noon |
| 7 | Clear Sunset |
| 8 | Cloudy Sunset |
| 9 | Wet Sunset |
| 10 | Wet Cloudy Sunset |
| 11 | Mid Rain Sunset |
| 12 | Hard Rain Sunset |
| 13 | Soft Rain Sunset |

## Programmatic Usage

```python
from agents.navigation.behavior_agent import BehaviorAgent
from agents.tools.sensor_manager import SensorManager
from agents.tools.data_collector import DataCollector

# Create vehicle and agent
vehicle = world.spawn_actor(blueprint, spawn_point)
agent = BehaviorAgent(vehicle, behavior='normal')

# Setup sensors
sensor_manager = SensorManager(vehicle, config_preset='bench2drive')

# Setup data collector
data_collector = DataCollector(
    sensor_manager=sensor_manager,
    vehicle=vehicle,
    root_dir='./dataset',
    scenario_type='GeneralDriving',
    town='Town01',
    route_id=1,
    weather_id=0
)

# Set destination
agent.set_destination(destination)

# Start recording
data_collector.start_recording()

# Main loop
while not agent.done():
    control = agent.run_step()
    vehicle.apply_control(control)
    data_collector.record_frame(control=control)
    world.tick()

# Stop and cleanup
data_collector.stop_recording()
sensor_manager.destroy()
```

## Tips for Large-Scale Data Collection

1. **Use Synchronous Mode**: Add `--sync` flag for deterministic behavior
2. **Adjust Traffic**: Vary `--num-vehicles` and `--num-pedestrians` for diversity
3. **Multiple Runs**: Collect multiple routes with different weather conditions
4. **Disk Space**: Ensure sufficient storage (~1-2 GB per 1000 frames)
5. **Performance**: For better FPS, reduce number of traffic vehicles or use headless mode

## Known Limitations

1. LiDAR data is saved as `.ply` files (can be converted to `.laz` offline for compression)
2. Bounding boxes and sensor calibration data are placeholders (to be implemented)
3. Waypoint information requires integration with navigation system

## Troubleshooting

**Issue**: Low FPS during collection
- Solution: Reduce `--num-vehicles`, enable `--sync`, or use headless CARLA

**Issue**: Sensors not initializing
- Solution: Wait a few ticks after spawning vehicle, check CARLA server logs

**Issue**: Missing sensor data in some frames
- Solution: This warning is normal for first few frames, sensors initialize asynchronously

**Issue**: Out of disk space
- Solution: Monitor disk usage, each 1000-frame collection uses ~1-2 GB

## Future Enhancements

- [ ] Add bounding box extraction for nearby vehicles/pedestrians
- [ ] Include sensor intrinsic/extrinsic calibration matrices
- [ ] Support for multiple ego vehicles (multi-agent scenarios)
- [ ] Real-time visualization of collected data
- [ ] Automatic `.ply` to `.laz` conversion for LiDAR
- [ ] Resume collection from checkpoint
- [ ] Dataset statistics and validation tools

## References

- [CARLA Documentation](https://carla.readthedocs.io/)
- [Bench2Drive Dataset](https://github.com/Thinklab-SJTU/Bench2Drive)
- [BehaviorAgent API](https://carla.readthedocs.io/en/latest/adv_agents/)

## License

This work is licensed under the terms of the MIT license.
