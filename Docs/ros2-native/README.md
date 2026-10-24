# CARLA Native ROS 2 Interface & Autoware Integration Guide

CARLA includes a **native C++ ROS 2 interface** (`libcarla-ros2-native`) built directly into the simulator server. Unlike external bridge nodes (`carla_ros_bridge`), the native interface publishes sensor streams, transforms (`/tf`), simulation clock (`/clock`), and vehicle status reports directly from the simulation loop, and subscribes directly to vehicle control commands with minimal latency.

By default:

- **Native ROS 2 is enabled at build time** (`ENABLE_ROS2=ON`) on both **Linux** and **macOS (Apple Silicon)**.
- **CycloneDDS (`cyclonedds`) is the default RMW middleware**, matching the default DDS middleware used by **Autoware** (`rmw_cyclonedds_cpp`). Fast-DDS (`fastdds`) and Zenoh (`zenoh`) are also compiled in and selectable at runtime via `--rmw=<cyclonedds|fastdds|zenoh>`.

---

## Table of Contents

1. [Architecture Overview](#1-architecture-overview)
2. [Building CARLA with Native ROS 2](#2-building-carla-with-native-ros-2)
3. [Command-Line Options](#3-command-line-options)
4. [Networking Setup: CARLA Host ↔ Autoware Docker Container](#4-networking-setup-carla-host--autoware-docker-container)
5. [End-to-End Example: CARLA ↔ Autoware (`universe-devel-jazzy`)](#5-end-to-end-example-carla--autoware-universe-devel-jazzy)
   - [Step 1: Generate CycloneDDS Configurations](#step-1-generate-cyclonedds-configurations)
   - [Step 2: Launch the CARLA Server with Native ROS 2](#step-2-launch-the-carla-server-with-native-ros-2)
   - [Step 3: Spawn the Ego Vehicle and Sensors in CARLA](#step-3-spawn-the-ego-vehicle-and-sensors-in-carla)
   - [Step 4: Start the Autoware Container & Verify Topic Visibility](#step-4-start-the-autoware-container--verify-topic-visibility)
   - [Step 5: Subscribe to Camera and LiDAR Topics Inside Autoware](#step-5-subscribe-to-camera-and-lidar-topics-inside-autoware)
   - [Step 6: Publish Steering Angle & Acceleration Commands from Autoware](#step-6-publish-steering-angle--acceleration-commands-from-autoware)
6. [Running the Automated Integration Test](#6-running-the-automated-integration-test)
7. [Autoware ROS 2 Topic & QoS Reference](#7-autoware-ros-2-topic--qos-reference)

---

## 1. Architecture Overview

```
┌──────────────────────────────────────────────┐          CycloneDDS (UDP)          ┌────────────────────────────────────────────────────┐
│           CARLA Simulator (Host)             │ ◄────────────────────────────────► │        Autoware Container (Docker / Linux)         │
│   (macOS Apple Silicon or Linux Native)      │                                    │  ghcr.io/autowarefoundation/autoware:              │
│                                              │   /clock                           │          universe-devel-jazzy                      │
│  ┌────────────────────────────────────────┐  │   /tf                              │                                                    │
│  │         libcarla-ros2-native           │  │   /sensing/camera/front/image      │  ┌──────────────────────────────────────────────┐  │
│  │  • CycloneDDS (default) / FastDDS /    │──┼──►/sensing/camera/front/camera_info│──►  Subscriptions: Camera, LiDAR, Velocity,     │  │
│  │    Zenoh                               │  │   /sensing/lidar/top/pointcloud... │  │                 Steering, Clock, TF          │  │
│  │  • Sensor Publishers (Image, LiDAR...) │  │   /vehicle/status/velocity_status  │  └──────────────────────────────────────────────┘  │
│  │  • AutowareVehicleStatusPublisher      │  │   /vehicle/status/steering_status  │  ┌──────────────────────────────────────────────┐  │
│  │  • AutowareControlSubscriber           │◄─┼───/control/command/control_cmd─────┼──│  Publisher: autoware_control_msgs/msg/Control│  │
│  └────────────────────────────────────────┘  │   (steering_tire_angle + accel)    │  │             (steering angle & acceleration)  │  │
└──────────────────────────────────────────────┘                                    └──┴──────────────────────────────────────────────┴──┘
```

---

## 2. Building CARLA with Native ROS 2

`ENABLE_ROS2` is `ON` by default in `CMake/Options.cmake`. A standard CMake configure and build compiles `Ros2Native` (Fast-DDS, CycloneDDS, Zenoh, and `libcarla-ros2-native`) and links it into the CARLA server and Unreal plugin:

```bash
# Configure (macOS or Linux)
cmake -B Build -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DPython3_EXECUTABLE=$(which python3)

# Build the native ROS 2 library, Python API, and CARLA Unreal Editor/Server
cmake --build Build --target carla-ros2-native
cmake --build Build --target carla-python-api-install
cmake --build Build --target carla-unreal-editor
```

---

## 3. Command-Line Options

Pass `-ros2` (or `--ros2`) when starting the CARLA server:

| Option | Default | Description |
|---|---|---|
| `-ros2` / `--ros2` | off | Enables the native ROS 2 interface at server startup. |
| `--rmw=<name>` | `cyclonedds` | Selects the ROS 2 middleware: `cyclonedds`, `fastdds`, or `zenoh`. |
| `--ros-domain-id=<N>` | `ROS_DOMAIN_ID` or `0` | Sets the DDS domain ID (`0`–`232`). Must match the container's `ROS_DOMAIN_ID`. |

When the server starts with `-ros2`, verify that the log reports:

```text
LogCarla: ROS2: Creating ROS2 Instance...
LogCarla: ROS2: enabled with middleware 'cyclonedds'.
```

---

## 4. Networking Setup: CARLA Host ↔ Autoware Docker Container

### Why a Custom `CYCLONEDDS_URI` Is Required in the Autoware Image

The official Autoware image (`ghcr.io/autowarefoundation/autoware:universe-devel-jazzy`) sets `CYCLONEDDS_URI=file:///home/aw/cyclonedds.xml` by default. That stock file:
1. Pins `<NetworkInterface name="lo"/>` (loopback only), which prevents communication across container boundaries.
2. Requests `<SocketReceiveBufferSize min="10MB"/>`, which fails in unprivileged containers without custom sysctls.

Providing a custom `CYCLONEDDS_URI` resolves both issues.

### macOS Host ↔ Docker Desktop Linux Container

On macOS, Docker Desktop runs Linux containers inside a lightweight Linux VM. UDP multicast does not traverse the macOS ↔ VM boundary, and `--net=host` shares the Linux VM's network namespace rather than the macOS Darwin host.

To establish bidirectional CycloneDDS unicast communication between native macOS CARLA and the Linux container:
1. **CARLA (macOS Host)** binds to the host's IP (`${HOST_IP}`) at `ParticipantIndex = 0` (UDP ports `7410` and `7411`) with `<Peer address="${HOST_IP}"/>` and `<Peer address="127.0.0.1"/>`.
2. **Autoware Container (Linux/Docker)** is launched with UDP port forwarding `-p 7412-7440:7412-7440/udp` (covering CycloneDDS participant indices `1..15` on domain 0), binds to `eth0` with `<ExternalNetworkAddress>127.0.0.1</ExternalNetworkAddress>`, and peers with `<Peer address="${HOST_IP}"/>`.
   - Using `<ExternalNetworkAddress>127.0.0.1</ExternalNetworkAddress>` ensures the container advertises `127.0.0.1:7412+` (which the macOS host reaches via Docker's localhost UDP port forward) without rewriting CARLA's `${HOST_IP}:7410` locator.
   - Holding UDP port `7410` inside the container causes `ParticipantIndex=auto` to skip index `0` (`7410/7411`, used by CARLA on the host) and assign indices `1+` (`7412/7413`, `7414/7415`, etc.).

### Linux Host ↔ Docker Container

On Linux, start the container with `--net=host` and mount the host's CycloneDDS configuration directly.

---

## 5. End-to-End Example: CARLA ↔ Autoware (`universe-devel-jazzy`)

### Step 1: Generate CycloneDDS Configurations

On macOS, detect your host IP and create `/tmp/cyclonedds_host.xml` and `/tmp/cyclonedds_container.xml`:

```bash
HOST_IP=$(ipconfig getifaddr $(route -n get default | awk '/interface:/{print $2}'))
echo "Host IP: ${HOST_IP}"

cat <<EOF > /tmp/cyclonedds_host.xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain Id="any">
    <General>
      <Interfaces>
        <NetworkInterface address="${HOST_IP}" multicast="false"/>
      </Interfaces>
      <AllowMulticast>false</AllowMulticast>
      <MaxMessageSize>65500B</MaxMessageSize>
    </General>
    <Discovery>
      <ParticipantIndex>0</ParticipantIndex>
      <MaxAutoParticipantIndex>20</MaxAutoParticipantIndex>
      <Peers>
        <Peer address="${HOST_IP}"/>
        <Peer address="127.0.0.1"/>
      </Peers>
    </Discovery>
  </Domain>
</CycloneDDS>
EOF

cat <<EOF > /tmp/cyclonedds_container.xml
<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain Id="any">
    <General>
      <Interfaces>
        <NetworkInterface name="eth0" multicast="false"/>
      </Interfaces>
      <ExternalNetworkAddress>127.0.0.1</ExternalNetworkAddress>
      <AllowMulticast>false</AllowMulticast>
      <MaxMessageSize>65500B</MaxMessageSize>
    </General>
    <Discovery>
      <ParticipantIndex>auto</ParticipantIndex>
      <MaxAutoParticipantIndex>20</MaxAutoParticipantIndex>
      <Peers>
        <Peer address="${HOST_IP}"/>
      </Peers>
    </Discovery>
  </Domain>
</CycloneDDS>
EOF
```

### Step 2: Launch the CARLA Server with Native ROS 2

**macOS:**

```bash
export CYCLONEDDS_URI="file:///tmp/cyclonedds_host.xml"
/Users/ra94giv/Workspace/UnrealEngine5_carla/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor \
  "$(pwd)/Unreal/CarlaUnreal/CarlaUnreal.uproject" \
  Town10HD_Opt -game -metal -carla-rpc-port=2000 -carla-streaming-port=2001 -RenderOffScreen -ros2
```

**Linux:**

```bash
export CYCLONEDDS_URI="file:///tmp/cyclonedds_host.xml"
./CarlaUnreal.sh Town10HD_Opt -carla-rpc-port=2000 -RenderOffScreen -ros2
```

### Step 3: Spawn the Ego Vehicle and Sensors in CARLA

To enable Autoware control and status reporting on an ego vehicle:
1. Set `role_name="ego"` (or `"hero"`) and `ros2_autoware_control="true"` on the vehicle blueprint.
2. Attach a `sensor.other.vehicle_status` sensor and call `enable_for_ros()` to publish `/vehicle/status/*` reports.
3. Attach camera (`sensor.camera.rgb`) and LiDAR (`sensor.lidar.ray_cast`) sensors with `ros_topic_name` set to Autoware's expected topic paths and call `enable_for_ros()`.

```python
import carla

client = carla.Client("127.0.0.1", 2000)
client.set_timeout(20.0)
world = client.get_world()

settings = world.get_settings()
settings.synchronous_mode = True
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

bp_lib = world.get_blueprint_library()

# 1. Ego vehicle with Autoware control enabled
ego_bp = bp_lib.filter("vehicle.lincoln.mkz*")[0]
ego_bp.set_attribute("role_name", "ego")
ego_bp.set_attribute("ros_name", "ego")
ego_bp.set_attribute("ros2_autoware_control", "true")
ego = world.spawn_actor(ego_bp, world.get_map().get_spawn_points()[0])

# 2. Vehicle status sensor (/vehicle/status/*)
status_bp = bp_lib.find("sensor.other.vehicle_status")
status_bp.set_attribute("ros_name", "vehicle_status")
status_sensor = world.spawn_actor(status_bp, carla.Transform(), attach_to=ego)
status_sensor.enable_for_ros()

# 3. Front RGB camera (/sensing/camera/front/image & /sensing/camera/front/camera_info)
cam_bp = bp_lib.find("sensor.camera.rgb")
cam_bp.set_attribute("image_size_x", "400")
cam_bp.set_attribute("image_size_y", "200")
cam_bp.set_attribute("ros_name", "front_camera")
cam_bp.set_attribute("ros_topic_name", "/sensing/camera/front")
cam = world.spawn_actor(
    cam_bp, carla.Transform(carla.Location(x=1.5, z=1.6)), attach_to=ego
)
cam.enable_for_ros()

# 4. Top Ray-Cast LiDAR (/sensing/lidar/top/pointcloud_raw_ex)
lidar_bp = bp_lib.find("sensor.lidar.ray_cast")
lidar_bp.set_attribute("channels", "32")
lidar_bp.set_attribute("ros_name", "top_lidar")
lidar_bp.set_attribute("ros_topic_name", "/sensing/lidar/top/pointcloud_raw_ex")
lidar = world.spawn_actor(
    lidar_bp, carla.Transform(carla.Location(z=2.2)), attach_to=ego
)
lidar.enable_for_ros()
```

### Step 4: Start the Autoware Container & Verify Topic Visibility

Start a container from `ghcr.io/autowarefoundation/autoware:universe-devel-jazzy`:

```bash
docker run -d --rm --name carla_autoware_ros2_test \
  -e AUTOWARE_RUNTIME=1 \
  -e RMW_IMPLEMENTATION=rmw_cyclonedds_cpp \
  -e CYCLONEDDS_URI=file:///tmp/cyclonedds.xml \
  -v /tmp/cyclonedds_container.xml:/tmp/cyclonedds.xml:ro \
  -p 7412-7440:7412-7440/udp \
  ghcr.io/autowarefoundation/autoware:universe-devel-jazzy \
  bash -c "python3 -c \"import socket, time; s=socket.socket(socket.AF_INET, socket.SOCK_DGRAM); s.bind(('0.0.0.0', 7410)); time.sleep(86400)\" & sleep infinity"
```

List the active ROS 2 topics from inside the Autoware container:

```bash
docker exec carla_autoware_ros2_test bash -c \
  "source /opt/ros/jazzy/setup.bash && source /opt/autoware/setup.bash && ros2 topic list -t --no-daemon"
```

Expected output includes all CARLA and Autoware interface topics:

```text
/clock [rosgraph_msgs/msg/Clock]
/control/command/control_cmd [autoware_control_msgs/msg/Control]
/control/command/emergency_cmd [tier4_vehicle_msgs/msg/VehicleEmergencyStamped]
/control/command/gear_cmd [autoware_vehicle_msgs/msg/GearCommand]
/control/command/hazard_lights_cmd [autoware_vehicle_msgs/msg/HazardLightsCommand]
/control/command/turn_indicators_cmd [autoware_vehicle_msgs/msg/TurnIndicatorsCommand]
/sensing/camera/front/camera_info [sensor_msgs/msg/CameraInfo]
/sensing/camera/front/image [sensor_msgs/msg/Image]
/sensing/lidar/top/pointcloud_raw_ex [sensor_msgs/msg/PointCloud2]
/tf [tf2_msgs/msg/TFMessage]
/vehicle/engage [autoware_vehicle_msgs/msg/Engage]
/vehicle/status/control_mode [autoware_vehicle_msgs/msg/ControlModeReport]
/vehicle/status/gear_status [autoware_vehicle_msgs/msg/GearReport]
/vehicle/status/hazard_lights_status [autoware_vehicle_msgs/msg/HazardLightsReport]
/vehicle/status/steering_status [autoware_vehicle_msgs/msg/SteeringReport]
/vehicle/status/turn_indicators_status [autoware_vehicle_msgs/msg/TurnIndicatorsReport]
/vehicle/status/velocity_status [autoware_vehicle_msgs/msg/VelocityReport]
```

### Step 5: Subscribe to Camera and LiDAR Topics Inside Autoware

Inside the container, you can inspect live camera and LiDAR streams using `ros2 topic echo` or an `rclpy` node:

```bash
# Verify camera image header
docker exec carla_autoware_ros2_test bash -c \
  "source /opt/ros/jazzy/setup.bash && ros2 topic echo /sensing/camera/front/image --no-arr --once --no-daemon"

# Verify LiDAR extended PointCloud2 header
docker exec carla_autoware_ros2_test bash -c \
  "source /opt/ros/jazzy/setup.bash && ros2 topic echo /sensing/lidar/top/pointcloud_raw_ex --no-arr --once --no-daemon"
```

### Step 6: Publish Steering Angle & Acceleration Commands from Autoware

CARLA's `AutowareControlSubscriber` listens on `/control/command/control_cmd` (`autoware_control_msgs/msg/Control`) with QoS `RELIABLE` + `TRANSIENT_LOCAL` (`KEEP_LAST`, depth 1):
- `longitudinal.acceleration` ($\text{m/s}^2$) drives the ego vehicle's longitudinal acceleration controller.
- `lateral.steering_tire_angle` ($\text{rad}$, positive = left) drives the ego vehicle's steering controller.

Example `rclpy` publisher inside the Autoware container:

```python
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy
from autoware_control_msgs.msg import Control

rclpy.init()
node = Node("autoware_control_cmd_publisher")

qos = QoSProfile(
    reliability=ReliabilityPolicy.RELIABLE,
    durability=DurabilityPolicy.TRANSIENT_LOCAL,
    history=HistoryPolicy.KEEP_LAST,
    depth=1,
)
pub = node.create_publisher(Control, "/control/command/control_cmd", qos)

msg = Control()
msg.lateral.steering_tire_angle = 0.35          # rad (left turn)
msg.lateral.steering_tire_rotation_rate = 1.0   # rad/s
msg.lateral.is_defined_steering_tire_rotation_rate = True
msg.longitudinal.acceleration = 3.0             # m/s^2 forward acceleration
pub.publish(msg)
```

---

## 6. Running the Automated Integration Test

A self-contained integration test script is provided at `PythonAPI/examples/ros2/test_carla_autoware_ros2.py`. It automates all four checks against a running CARLA server (`-ros2`) and the local `ghcr.io/autowarefoundation/autoware:universe-devel-jazzy` Docker image:

1. **Check 1**: Verifies ROS 2 CycloneDDS connectivity between the Autoware container and the native CARLA server.
2. **Check 2**: Verifies CARLA's topics (`/clock`, `/tf`, camera, LiDAR, vehicle status, and Autoware control topics) are visible inside the container.
3. **Check 3**: Subscribes to `/sensing/camera/front/image` and `/sensing/lidar/top/pointcloud_raw_ex` inside the Autoware container and verifies non-empty frames and point clouds are received.
4. **Check 4**: Publishes `autoware_control_msgs/msg/Control` messages with steering angle (`0.35 rad`) and acceleration (`3.0 m/s^2`) on `/control/command/control_cmd` from inside the container and verifies that the CARLA ego vehicle accelerates, steers, and reports updated telemetry on `/vehicle/status/velocity_status` and `/vehicle/status/steering_status`.

### Usage

```bash
python3 PythonAPI/examples/ros2/test_carla_autoware_ros2.py
```

### Example Output

```text
==============================================================================
CARLA Native ROS 2 <-> Autoware Container Integration Test
==============================================================================
Host OS           : Darwin (arm64)
Host IP           : 192.168.178.180
CARLA Server      : 127.0.0.1:2000
Autoware Image    : ghcr.io/autowarefoundation/autoware:universe-devel-jazzy
ROS_DOMAIN_ID     : 0
------------------------------------------------------------------------------
[Setup] Spawned ego vehicle (vehicle.lincoln.mkz, id=29) and 3 ROS 2 sensors.
[Setup] Starting Autoware container 'carla_autoware_ros2_test'...

[Check 1, 2 & 3] Verifying ROS 2 discovery, topic visibility, and sensor subscriptions...
  [PASS] Check 1: Autoware container connected to CARLA host over CycloneDDS.
  [PASS] Check 2: Discovered 19 ROS 2 topics from CARLA inside Autoware:
           - /clock (rosgraph_msgs/msg/Clock)
           - /tf (tf2_msgs/msg/TFMessage)
           - /sensing/camera/front/image (sensor_msgs/msg/Image)
           - /sensing/camera/front/camera_info (sensor_msgs/msg/CameraInfo)
           - /sensing/lidar/top/pointcloud_raw_ex (sensor_msgs/msg/PointCloud2)
           - /vehicle/status/velocity_status (autoware_vehicle_msgs/msg/VelocityReport)
           - /vehicle/status/steering_status (autoware_vehicle_msgs/msg/SteeringReport)
           - /control/command/control_cmd (autoware_control_msgs/msg/Control)
  [PASS] Check 3: Subscribed and received sensor messages inside Autoware container:
           - Camera (/sensing/camera/front/image)          : {'width': 400, 'height': 200, 'encoding': 'bgra8', 'bytes': 320000, 'frame_id': 'front_camera'}
           - LiDAR  (/sensing/lidar/top/pointcloud_raw_ex) : {'width': 2595, 'height': 1, 'point_step': 32, 'bytes': 83040, 'frame_id': 'top_lidar'}
           - Clock  (/clock)                               : {'sec': 361, 'nanosec': 376152057}

[Check 4] Publishing steering angle (0.35 rad) & acceleration (3.0 m/s^2) from Autoware container...
  Initial CARLA Ego State : location=(106.42, -12.71, -0.15), throttle=0.00, steer=0.000
  Final CARLA Ego State   : location=(97.34, -19.11, 0.28), displacement=11.12 m, speed=9.93 m/s, throttle=0.00, steer=-0.367
  Autoware Status Feedback: {'published_count': 190, 'target_acceleration': 3.0, 'target_steering_tire_angle': 0.35, 'reported_longitudinal_velocity': 9.59999942779541, 'reported_steering_tire_angle': 0.41530317068099976}
  [PASS] Check 4: Published /control/command/control_cmd from Autoware container and verified CARLA ego vehicle accelerated and steered.

==============================================================================
ALL 4 CHECKS PASSED SUCCESSFULLY!
==============================================================================
```

---

## 7. Autoware ROS 2 Topic & QoS Reference

| Direction (CARLA Perspective) | Topic | Message Type | QoS Profile |
|---|---|---|---|
| Published by CARLA | `/clock` | `rosgraph_msgs/msg/Clock` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/tf` | `tf2_msgs/msg/TFMessage` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `<ros_topic_name>/image` | `sensor_msgs/msg/Image` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `<ros_topic_name>/camera_info` | `sensor_msgs/msg/CameraInfo` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `<ros_topic_name>` (LiDAR) | `sensor_msgs/msg/PointCloud2` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/vehicle/status/velocity_status` | `autoware_vehicle_msgs/msg/VelocityReport` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/vehicle/status/steering_status` | `autoware_vehicle_msgs/msg/SteeringReport` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/vehicle/status/control_mode` | `autoware_vehicle_msgs/msg/ControlModeReport` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/vehicle/status/gear_status` | `autoware_vehicle_msgs/msg/GearReport` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/vehicle/status/turn_indicators_status` | `autoware_vehicle_msgs/msg/TurnIndicatorsReport` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Published by CARLA | `/vehicle/status/hazard_lights_status` | `autoware_vehicle_msgs/msg/HazardLightsReport` | `RELIABLE`, `VOLATILE`, `KEEP_LAST(1)` |
| Subscribed by CARLA | `/control/command/control_cmd` | `autoware_control_msgs/msg/Control` | `RELIABLE`, `TRANSIENT_LOCAL`, `KEEP_LAST(1)` |
| Subscribed by CARLA | `/control/command/gear_cmd` | `autoware_vehicle_msgs/msg/GearCommand` | `RELIABLE`, `TRANSIENT_LOCAL`, `KEEP_LAST(1)` |
| Subscribed by CARLA | `/control/command/turn_indicators_cmd` | `autoware_vehicle_msgs/msg/TurnIndicatorsCommand` | `RELIABLE`, `TRANSIENT_LOCAL`, `KEEP_LAST(1)` |
| Subscribed by CARLA | `/control/command/hazard_lights_cmd` | `autoware_vehicle_msgs/msg/HazardLightsCommand` | `RELIABLE`, `TRANSIENT_LOCAL`, `KEEP_LAST(1)` |
| Subscribed by CARLA | `/control/command/emergency_cmd` | `tier4_vehicle_msgs/msg/VehicleEmergencyStamped` | `RELIABLE`, `TRANSIENT_LOCAL`, `KEEP_LAST(1)` |
| Subscribed by CARLA | `/vehicle/engage` | `autoware_vehicle_msgs/msg/Engage` | `RELIABLE`, `TRANSIENT_LOCAL`, `KEEP_LAST(1)` |

