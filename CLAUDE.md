# BehaviorAgent Data Collection Enhancement

## Overview
This document outlines the roadmap for enhancing the CARLA BehaviorAgent to support comprehensive sensor data collection for dataset creation. The goal is to extend the `PythonAPI/carla/agents/navigation/behavior_agent.py` agent to allow flexible sensor attachment and data logging, similar to the sensor setup in `PythonAPI/examples/manual_control.py`.

## Target CARLA Version
- Branch/Tree: `0.9.15`

## Motivation
The BehaviorAgent provides different driving profiles (cautious, normal, aggressive) which is ideal for collecting diverse driving datasets under various scenarios. By attaching configurable sensors to the ego vehicle and collecting CAN bus-like telemetry data, we can create rich datasets for:
- Autonomous driving model training
- Behavior cloning
- Imitation learning
- Multi-modal perception research
- Driving scenario analysis

## Core Requirements

### 1. Sensor Attachment System
Enable flexible attachment of multiple sensor types to the BehaviorAgent's vehicle:

#### Camera Sensors
- **RGB Cameras**: Multiple viewpoints (front, rear, side, BEV)
- **Depth Cameras**: Corresponding depth maps for each RGB camera
- **Semantic Segmentation Cameras**: Per-pixel class labels
- **Instance Segmentation Cameras**: Per-pixel instance IDs
- **Optical Flow Cameras**: Motion estimation

#### Other Sensors
- **LiDAR**: 3D point cloud data (both regular and semantic)
- **Radar**: Object detection and velocity estimation
- **GNSS/GPS**: Global position data
- **IMU**: Accelerometer, gyroscope, compass
- **Collision Sensor**: Collision detection and intensity
- **Lane Invasion Sensor**: Lane marking violations

### 2. CAN Bus / Telemetry Data Collection
Collect vehicle state information similar to automotive CAN bus data:
- **Vehicle Dynamics**:
  - Speed (km/h, m/s)
  - Acceleration (3-axis from IMU)
  - Angular velocity/gyroscope data
  - Steering angle
  - Throttle position
  - Brake pressure

- **Position & Orientation**:
  - Global position (x, y, z in world coordinates)
  - GPS coordinates (latitude, longitude)
  - Rotation (pitch, yaw, roll)
  - Compass heading

- **Control Inputs**:
  - Steering command
  - Throttle command
  - Brake command
  - Gear state
  - Hand brake state

- **Contextual Information**:
  - Current waypoint
  - Target waypoint
  - Route information
  - Traffic light state
  - Nearby vehicles/pedestrians
  - Current behavior mode (cautious/normal/aggressive)

### 3. Data Collection Interface
Create a clean API for:
- Configuring which sensors to attach
- Specifying sensor parameters (resolution, FOV, position, etc.)
- Starting/stopping data recording
- Saving data to disk in organized format
- Synchronizing data across all sensors

### 4. Multi-Vehicle Support (Future)
- Initially: Ego vehicle with BehaviorAgent, other vehicles with TrafficManager
- Future: Multiple vehicles controlled by their own BehaviorAgents

## Architecture Design

### Phase 1: Sensor Manager for BehaviorAgent

Create a `SensorManager` class that:
- Takes sensor configuration as input (dict/config file)
- Spawns and manages all sensors attached to the vehicle
- Provides callbacks for sensor data
- Handles sensor lifecycle (creation, destruction, cleanup)

**Files to Create**:
- `PythonAPI/carla/agents/tools/sensor_manager.py`

**Files to Modify**:
- `PythonAPI/carla/agents/navigation/behavior_agent.py` - Add optional sensor manager integration

### Phase 2: Data Collection System

Create a `DataCollector` class that:
- Receives data from SensorManager
- Collects vehicle telemetry each frame
- Synchronizes data from multiple sensors
- Writes data to disk in structured format (HDF5, rosbag, or custom format)

**Files to Create**:
- `PythonAPI/carla/agents/tools/data_collector.py`

### Phase 3: Example Script

Create a complete example script demonstrating:
- BehaviorAgent setup with sensors
- Data collection in a town with specific weather
- Switching between behavior profiles
- Saving collected dataset

**Files to Create**:
- `PythonAPI/examples/behavior_agent_data_collection.py`

### Phase 4: Configuration System

Implement flexible configuration via YAML/JSON:
- Sensor configurations (types, positions, parameters)
- Recording settings (output path, format, frequency)
- Scenario settings (town, weather, behavior profile)

**Files to Create**:
- `PythonAPI/examples/configs/sensor_config_example.yaml`
- `PythonAPI/carla/agents/tools/config_loader.py`

## Implementation Roadmap

### Milestone 1: Sensor Manager Foundation
- [ ] Create `SensorManager` class
- [ ] Implement sensor factory for different sensor types
- [ ] Add sensor attachment with configurable transforms
- [ ] Implement sensor data callbacks
- [ ] Add sensor cleanup/destruction
- [ ] Test with basic RGB camera

### Milestone 2: Integrate with BehaviorAgent
- [ ] Add optional `sensor_config` parameter to BehaviorAgent
- [ ] Integrate SensorManager into BehaviorAgent lifecycle
- [ ] Ensure sensors don't interfere with agent's decision making
- [ ] Test BehaviorAgent with multiple sensors

### Milestone 3: Telemetry Collection
- [ ] Create telemetry data structure
- [ ] Implement vehicle state polling
- [ ] Implement IMU/GNSS data collection
- [ ] Implement control input logging
- [ ] Add timestamp synchronization

### Milestone 4: Data Persistence
- [ ] Create `DataCollector` class
- [ ] Implement data buffer/queue system
- [ ] Add file I/O for sensor data (images, point clouds)
- [ ] Add CSV/JSON export for telemetry
- [ ] Implement data synchronization across sensors
- [ ] Add metadata recording (scenario info, weather, etc.)

### Milestone 5: Example & Documentation
- [ ] Create comprehensive example script
- [ ] Write configuration file examples
- [ ] Add inline documentation
- [ ] Create usage guide
- [ ] Add example dataset structure

### Milestone 6: Advanced Features (Future)
- [ ] Multi-vehicle BehaviorAgent support
- [ ] Real-time visualization during collection
- [ ] Dataset validation tools
- [ ] Data format converters (to nuScenes, KITTI, etc.)
- [ ] Cloud storage integration

## Data Format Specification (Bench2Drive Compatible)

The data will be saved in **Bench2Drive (B2D) format** for compatibility with existing autonomous driving datasets.

### Root Directory Structure
```
{root_directory}/
├── GeneralDriving_Town01_Route001_Weather00/
├── GeneralDriving_Town01_Route001_Weather10/
├── GeneralDriving_Town02_Route042_Weather05/
└── ...
```

Each scenario is named as: `{ScenarioType}_{Town}_{Route}_{Weather}`
- **ScenarioType**: `GeneralDriving` (for now, can be extended later)
- **Town**: `Town01`, `Town02`, etc.
- **Route**: Route number (e.g., `Route001`, `Route042`)
- **Weather**: Weather preset ID (e.g., `Weather00`, `Weather10`)

### Scenario Directory Structure
```
GeneralDriving_Town01_Route001_Weather00/
├── anno/
│   ├── 00000.json.gz
│   ├── 00001.json.gz
│   ├── 00002.json.gz
│   └── ...
├── camera/
│   ├── rgb_front/
│   │   ├── 00000.jpg
│   │   ├── 00001.jpg
│   │   └── ...
│   ├── rgb_front_left/
│   ├── rgb_front_right/
│   ├── rgb_back/
│   ├── rgb_back_left/
│   ├── rgb_back_right/
│   ├── rgb_top_down/          # BEV camera for debugging
│   ├── depth_front/
│   │   ├── 00000.png
│   │   └── ...
│   ├── depth_front_left/
│   ├── depth_front_right/
│   ├── depth_back/
│   ├── depth_back_left/
│   ├── depth_back_right/
│   ├── instance_front/
│   │   ├── 00000.png
│   │   └── ...
│   ├── instance_front_left/
│   ├── instance_front_right/
│   ├── instance_back/
│   ├── instance_back_left/
│   ├── instance_back_right/
│   ├── semantic_front/
│   │   ├── 00000.png
│   │   └── ...
│   ├── semantic_front_left/
│   ├── semantic_front_right/
│   ├── semantic_back/
│   ├── semantic_back_left/
│   └── semantic_back_right/
└── lidar/
    ├── 00000.laz
    ├── 00001.laz
    └── ...
```

### Annotation File Format (anno/*.json.gz)

Each compressed JSON file contains the following fields:

**Vehicle State:**
- `x`, `y`: Global position coordinates
- `theta`: Vehicle heading angle
- `speed`: Current speed
- `acceleration`: 3-axis acceleration (from IMU)
- `angular_velocity`: 3-axis angular velocity (from IMU)

**Control Inputs:**
- `throttle`: Throttle value [0.0, 1.0]
- `steer`: Steering value [-1.0, 1.0]
- `brake`: Brake value [0.0, 1.0]
- `reverse`: Boolean, reverse gear engaged
- `only_ap_brake`: Boolean, autopilot-only brake flag

**Navigation/Waypoints:**
- `x_command_far`, `y_command_far`: Far waypoint coordinates
- `command_far`: Command at far waypoint
- `x_command_near`, `y_command_near`: Near waypoint coordinates
- `command_near`: Command at near waypoint
- `x_target`, `y_target`: Target destination coordinates
- `next_command`: Next navigation command
- `should_brake`: Boolean, whether vehicle should brake

**Environment:**
- `weather`: Weather preset ID

**Perception (Complex Objects):**
- `bounding_boxes`: Dictionary containing 3D bounding boxes of nearby objects
- `sensors`: Dictionary containing sensor intrinsics and extrinsics

### Sensor Configuration (B2D Compatible)

**RGB Cameras (6 cameras + 1 top-down):**
- `CAM_FRONT`: x=0.80, y=0.0, z=1.60, yaw=0.0, 1600x900, FOV=70
- `CAM_FRONT_LEFT`: x=0.27, y=-0.55, z=1.60, yaw=-55.0, 1600x900, FOV=70
- `CAM_FRONT_RIGHT`: x=0.27, y=0.55, z=1.60, yaw=55.0, 1600x900, FOV=70
- `CAM_BACK`: x=-2.0, y=0.0, z=1.60, yaw=180.0, 1600x900, FOV=110
- `CAM_BACK_LEFT`: x=-0.32, y=-0.55, z=1.60, yaw=-110.0, 1600x900, FOV=70
- `CAM_BACK_RIGHT`: x=-0.32, y=0.55, z=1.60, yaw=110.0, 1600x900, FOV=70
- `CAM_TOP_DOWN`: For debugging (BEV), high altitude with downward pitch

**Corresponding Depth, Instance, and Semantic Segmentation:**
Each RGB camera has corresponding depth, instance segmentation, and semantic segmentation cameras at the same position.

**LiDAR:**
- `LIDAR_TOP`: x=-0.39, y=0.0, z=1.84, range=85m, 10Hz, 64 channels, 600k points/sec

**Other Sensors:**
- `GPS`: For global position
- `IMU`: x=-1.4, y=0.0, z=0.0, 20Hz sampling
- `Speedometer`: 20Hz reading frequency

### File Naming Convention
- Frame numbers use 5 digits with zero-padding: `00000`, `00001`, ..., `99999`
- RGB images: `.jpg` format
- Depth/Semantic/Instance: `.png` format
- LiDAR: `.laz` format (compressed LAS point cloud)
- Annotations: `.json.gz` format (compressed JSON)

## Example Usage

```python
from agents.navigation.behavior_agent import BehaviorAgent
from agents.tools.sensor_manager import SensorManager
from agents.tools.data_collector import DataCollector

# Create vehicle
vehicle = world.spawn_actor(blueprint, spawn_point)

# Create BehaviorAgent with sensors
agent = BehaviorAgent(vehicle, behavior='normal')

# Create sensor manager with B2D configuration
sensor_manager = SensorManager(vehicle, config_preset='bench2drive')

# Create data collector with B2D format
# Scenario name will be: GeneralDriving_Town01_Route001_Weather00
data_collector = DataCollector(
    sensor_manager=sensor_manager,
    vehicle=vehicle,
    root_dir='./b2d_dataset',
    scenario_type='GeneralDriving',
    town='Town01',
    route_id=1,
    weather_id=0,
    format='bench2drive'
)

# Set destination and run
agent.set_destination(destination)
data_collector.start_recording()

while True:
    control = agent.run_step()
    vehicle.apply_control(control)

    # Record frame with telemetry data
    data_collector.record_frame(
        control=control,
        waypoint_info=agent.get_waypoint_info()  # Navigation commands
    )

    world.tick()

    if agent.done():
        break

data_collector.stop_recording()
sensor_manager.destroy()
```

## Testing Plan

1. **Unit Tests**:
   - Sensor creation and destruction
   - Data collector buffer management
   - Configuration parsing

2. **Integration Tests**:
   - BehaviorAgent with sensors in Town01-10
   - Multi-behavior profile collection
   - Different weather conditions
   - Long duration runs (memory leaks, performance)

3. **Dataset Validation**:
   - Data completeness
   - Timestamp synchronization
   - File integrity
   - Format correctness

## Future Enhancements

1. **Multi-Agent Scenarios**:
   - Support multiple BehaviorAgents collecting data simultaneously
   - Agent interaction scenarios
   - Diverse traffic patterns

2. **Scenario Generation**:
   - Automated scenario generation for diverse data
   - Adversarial scenario creation
   - Edge case collection

3. **Real-time Processing**:
   - Online data augmentation
   - Real-time annotation
   - Streaming to training pipeline

4. **Analysis Tools**:
   - Dataset statistics and visualization
   - Driving behavior analysis
   - Data quality metrics

## References

- Existing CARLA sensor documentation
- `PythonAPI/examples/manual_control.py` - Sensor setup reference
- `PythonAPI/carla/agents/navigation/behavior_agent.py` - Agent implementation
- Industry CAN bus data standards (J1939, OBD-II)

## Timeline Estimate

- **Milestone 1-2**: 1 week
- **Milestone 3-4**: 1 week
- **Milestone 5**: 3-4 days
- **Testing & Refinement**: 3-4 days

**Total**: ~3 weeks for core functionality
