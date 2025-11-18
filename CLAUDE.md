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

## Data Format Specification

### Directory Structure
```
dataset/
├── metadata.json                 # Scenario metadata
├── telemetry/
│   ├── vehicle_state.csv        # Vehicle dynamics and position
│   ├── control_inputs.csv       # Steering, throttle, brake
│   └── sensor_sync.csv          # Timestamp synchronization
├── sensors/
│   ├── camera_rgb_front/
│   │   ├── 0000000000.png
│   │   ├── 0000000001.png
│   │   └── ...
│   ├── camera_depth_front/
│   ├── camera_semseg_front/
│   ├── camera_bev/
│   ├── lidar/
│   │   ├── 0000000000.ply
│   │   └── ...
│   ├── imu.csv
│   └── gnss.csv
└── annotations/
    ├── bounding_boxes_3d.json   # 3D bboxes of nearby objects
    └── traffic_lights.json       # Traffic light states
```

### Metadata Fields
- Timestamp
- Frame number
- Town/map name
- Weather parameters
- Behavior profile
- Spawn point
- Destination
- Number of other vehicles
- Number of pedestrians

## Example Usage

```python
from agents.navigation.behavior_agent import BehaviorAgent
from agents.tools.sensor_manager import SensorManager
from agents.tools.data_collector import DataCollector

# Create vehicle
vehicle = world.spawn_actor(blueprint, spawn_point)

# Configure sensors
sensor_config = {
    'cameras': [
        {'type': 'rgb', 'position': 'front', 'width': 1920, 'height': 1080},
        {'type': 'depth', 'position': 'front'},
        {'type': 'semantic_segmentation', 'position': 'front'},
        {'type': 'rgb', 'position': 'bev', 'height': 50, 'pitch': -90},
    ],
    'lidar': {'range': 100, 'rotation_frequency': 10, 'channels': 64},
    'imu': True,
    'gnss': True,
    'collision': True,
}

# Create BehaviorAgent with sensors
agent = BehaviorAgent(vehicle, behavior='normal')
sensor_manager = SensorManager(vehicle, sensor_config)
data_collector = DataCollector(sensor_manager, output_dir='./dataset/town01_sunny_normal')

# Set destination and run
agent.set_destination(destination)
data_collector.start_recording()

while True:
    control = agent.run_step()
    vehicle.apply_control(control)
    data_collector.record_frame()
    world.tick()

    if agent.done():
        break

data_collector.stop_recording()
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
