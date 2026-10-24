#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
End-to-end integration test for CARLA Native ROS 2 (CycloneDDS) <-> Autoware Docker container.

Verifies:
1. Autoware (Docker/Linux container) connects to CARLA (Native macOS or Linux host) via ROS 2.
2. CARLA's ROS 2 topics are visible from inside the Autoware container.
3. Subscribing to CARLA camera and LiDAR topics from inside the Autoware container receives live data.
4. Publishing vehicle steering angle and acceleration on /control/command/control_cmd from inside
   the Autoware container drives the CARLA ego vehicle and updates its telemetry.
"""

import argparse
import json
import math
import os
import platform
import subprocess
import sys
import threading
import time
from pathlib import Path

import carla


DEFAULT_AUTOWARE_IMAGE = "ghcr.io/autowarefoundation/autoware:universe-devel-jazzy"
DEFAULT_CONTAINER_NAME = "carla_autoware_ros2_test"


def detect_host_ip() -> str:
    """Detect the primary LAN IPv4 address of the host machine."""
    if platform.system() == "Darwin":
        route_out = subprocess.check_output(
            ["route", "-n", "get", "default"], text=True
        )
        iface = None
        for line in route_out.splitlines():
            line = line.strip()
            if line.startswith("interface:"):
                iface = line.split(":", 1)[1].strip()
                break
        if iface:
            ip = subprocess.check_output(
                ["ipconfig", "getifaddr", iface], text=True
            ).strip()
            if ip:
                return ip
    # Fallback via UDP socket route lookup
    import socket
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.connect(("8.8.8.8", 80))
        return s.getsockname()[0]


def write_cyclonedds_configs(host_ip: str, host_xml: Path, container_xml: Path) -> None:
    """Write CycloneDDS unicast peer configs for the host and the Docker container."""
    host_cfg = f"""<?xml version="1.0" encoding="UTF-8" ?>
<CycloneDDS xmlns="https://cdds.io/config">
  <Domain Id="any">
    <General>
      <Interfaces>
        <NetworkInterface address="{host_ip}" multicast="false"/>
      </Interfaces>
      <AllowMulticast>false</AllowMulticast>
      <MaxMessageSize>65500B</MaxMessageSize>
    </General>
    <Discovery>
      <ParticipantIndex>0</ParticipantIndex>
      <MaxAutoParticipantIndex>20</MaxAutoParticipantIndex>
      <Peers>
        <Peer address="{host_ip}"/>
        <Peer address="127.0.0.1"/>
      </Peers>
    </Discovery>
  </Domain>
</CycloneDDS>
"""
    container_cfg = f"""<?xml version="1.0" encoding="UTF-8" ?>
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
        <Peer address="{host_ip}"/>
      </Peers>
    </Discovery>
  </Domain>
</CycloneDDS>
"""
    host_xml.write_text(host_cfg, encoding="utf-8")
    container_xml.write_text(container_cfg, encoding="utf-8")


def start_autoware_container(
    image: str,
    container_name: str,
    container_xml: Path,
    host_xml: Path,
    ros_domain_id: int = 0,
) -> None:
    """Start the Autoware Docker container configured for CycloneDDS communication with the host."""
    subprocess.run(
        ["docker", "rm", "-f", container_name],
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
        check=False,
    )

    is_macos = platform.system() == "Darwin"
    cmd = [
        "docker",
        "run",
        "-d",
        "--rm",
        "--name",
        container_name,
        "-e",
        "AUTOWARE_RUNTIME=1",
        "-e",
        "RMW_IMPLEMENTATION=rmw_cyclonedds_cpp",
        "-e",
        f"ROS_DOMAIN_ID={ros_domain_id}",
    ]

    if is_macos:
        # On macOS (Docker Desktop VM), forward CycloneDDS unicast participant ports (indices 1..15)
        # and reserve UDP 7410 inside the container so ParticipantIndex=auto picks index >= 1 (7412+),
        # while index 0 (7410/7411) is owned by CARLA on the macOS host.
        cmd += [
            "-p",
            "7412-7440:7412-7440/udp",
            "-v",
            f"{container_xml}:/tmp/cyclonedds.xml:ro",
            "-e",
            "CYCLONEDDS_URI=file:///tmp/cyclonedds.xml",
            image,
            "bash",
            "-c",
            (
                "python3 -c \""
                "import socket, time; "
                "s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM); "
                "s.bind(('0.0.0.0', 7410)); "
                "time.sleep(86400)"
                "\" & sleep infinity"
            ),
        ]
    else:
        cmd += [
            "--net=host",
            "-v",
            f"{host_xml}:/tmp/cyclonedds.xml:ro",
            "-e",
            "CYCLONEDDS_URI=file:///tmp/cyclonedds.xml",
            image,
            "sleep",
            "infinity",
        ]

    subprocess.run(cmd, check=True, stdout=subprocess.PIPE, text=True)


def docker_exec_python(container_name: str, python_code: str, timeout: float = 30.0) -> str:
    """Execute a Python snippet inside the Autoware container with the ROS 2 / Autoware environment sourced."""
    wrapped = (
        "source /opt/ros/jazzy/setup.bash && "
        "if [ -f /opt/autoware/setup.bash ]; then source /opt/autoware/setup.bash; fi && "
        "python3 - <<'PYEOF'\n"
        f"{python_code}\n"
        "PYEOF"
    )
    res = subprocess.run(
        ["docker", "exec", container_name, "bash", "-c", wrapped],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        timeout=timeout,
        check=False,
    )
    if res.returncode != 0:
        raise RuntimeError(
            f"Container command failed (exit {res.returncode}):\n"
            f"STDOUT:\n{res.stdout}\nSTDERR:\n{res.stderr}"
        )
    return res.stdout.strip()


def spawn_ego_and_sensors(world: carla.World):
    """Spawn an ego vehicle with Autoware ROS 2 control enabled, plus camera, LiDAR, and vehicle_status sensors."""
    bp_lib = world.get_blueprint_library()

    # 1. Ego vehicle with ros2_autoware_control=true
    vehicle_bps = bp_lib.filter("vehicle.lincoln.mkz*")
    if not vehicle_bps:
        vehicle_bps = [
            bp for bp in bp_lib.filter("vehicle.*")
            if int(bp.get_attribute("number_of_wheels").as_int()) == 4
        ]
    ego_bp = vehicle_bps[0]
    ego_bp.set_attribute("role_name", "ego")
    ego_bp.set_attribute("ros_name", "ego")
    if ego_bp.has_attribute("ros2_autoware_control"):
        ego_bp.set_attribute("ros2_autoware_control", "true")

    spawn_points = world.get_map().get_spawn_points()
    if not spawn_points:
        raise RuntimeError("Map has no spawn points")

    ego = None
    for sp in spawn_points:
        ego = world.try_spawn_actor(ego_bp, sp)
        if ego is not None:
            break
    if ego is None:
        raise RuntimeError("Failed to spawn ego vehicle at any spawn point")

    sensors = []

    # 2. Autoware vehicle status sensor (/vehicle/status/*)
    status_bp = bp_lib.find("sensor.other.vehicle_status")
    status_bp.set_attribute("ros_name", "vehicle_status")
    status_bp.set_attribute("role_name", "vehicle_status")
    status_bp.set_attribute("sensor_tick", "0.05")
    status_sensor = world.spawn_actor(status_bp, carla.Transform(), attach_to=ego)
    status_sensor.enable_for_ros()
    sensors.append(status_sensor)

    # 3. Front RGB camera (/sensing/camera/front/image & /sensing/camera/front/camera_info)
    cam_bp = bp_lib.find("sensor.camera.rgb")
    cam_bp.set_attribute("image_size_x", "400")
    cam_bp.set_attribute("image_size_y", "200")
    cam_bp.set_attribute("fov", "90.0")
    cam_bp.set_attribute("sensor_tick", "0.1")
    cam_bp.set_attribute("ros_name", "front_camera")
    cam_bp.set_attribute("role_name", "front_camera")
    cam_bp.set_attribute("ros_topic_name", "/sensing/camera/front")
    cam_tf = carla.Transform(carla.Location(x=1.5, y=0.0, z=1.6))
    cam_sensor = world.spawn_actor(cam_bp, cam_tf, attach_to=ego)
    cam_sensor.enable_for_ros()
    sensors.append(cam_sensor)

    # 4. Top Ray-Cast LiDAR (/sensing/lidar/top/pointcloud_raw_ex)
    lidar_bp = bp_lib.find("sensor.lidar.ray_cast")
    lidar_bp.set_attribute("channels", "32")
    lidar_bp.set_attribute("range", "50.0")
    lidar_bp.set_attribute("points_per_second", "56000")
    lidar_bp.set_attribute("rotation_frequency", "20.0")
    lidar_bp.set_attribute("sensor_tick", "0.1")
    lidar_bp.set_attribute("ros_name", "top_lidar")
    lidar_bp.set_attribute("role_name", "top_lidar")
    lidar_bp.set_attribute("ros_topic_name", "/sensing/lidar/top/pointcloud_raw_ex")
    lidar_tf = carla.Transform(carla.Location(x=0.0, y=0.0, z=2.2))
    lidar_sensor = world.spawn_actor(lidar_bp, lidar_tf, attach_to=ego)
    lidar_sensor.enable_for_ros()
    sensors.append(lidar_sensor)

    return ego, sensors


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Verify CARLA Native ROS 2 <-> Autoware Docker container communication."
    )
    parser.add_argument("--host", default="127.0.0.1", help="CARLA RPC host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port (default: 2000)")
    parser.add_argument(
        "--image",
        default=DEFAULT_AUTOWARE_IMAGE,
        help=f"Autoware Docker image (default: {DEFAULT_AUTOWARE_IMAGE})",
    )
    parser.add_argument(
        "--container-name",
        default=DEFAULT_CONTAINER_NAME,
        help=f"Docker container name (default: {DEFAULT_CONTAINER_NAME})",
    )
    parser.add_argument(
        "--ros-domain-id",
        type=int,
        default=int(os.environ.get("ROS_DOMAIN_ID", "0")),
        help="ROS_DOMAIN_ID (default: 0)",
    )
    args = parser.parse_args()

    host_ip = detect_host_ip()
    host_xml = Path("/tmp/cyclonedds_host.xml")
    container_xml = Path("/tmp/cyclonedds_container.xml")
    write_cyclonedds_configs(host_ip, host_xml, container_xml)

    print("=" * 78)
    print("CARLA Native ROS 2 <-> Autoware Container Integration Test")
    print("=" * 78)
    print(f"Host OS           : {platform.system()} ({platform.machine()})")
    print(f"Host IP           : {host_ip}")
    print(f"CARLA Server      : {args.host}:{args.port}")
    print(f"Autoware Image    : {args.image}")
    print(f"ROS_DOMAIN_ID     : {args.ros_domain_id}")
    print("-" * 78)

    client = carla.Client(args.host, args.port)
    client.set_timeout(20.0)
    world = client.get_world()
    original_settings = world.get_settings()

    ego = None
    sensors = []
    stop_ticking = threading.Event()
    tick_thread = None

    try:
        # Configure synchronous mode (20 Hz)
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)

        ego, sensors = spawn_ego_and_sensors(world)
        for _ in range(5):
            world.tick()

        print(f"[Setup] Spawned ego vehicle ({ego.type_id}, id={ego.id}) and {len(sensors)} ROS 2 sensors.")

        # Start background world ticker so CARLA continuously publishes and processes ROS 2 messages
        def _ticker_loop():
            while not stop_ticking.is_set():
                try:
                    world.tick()
                except Exception:
                    pass
                time.sleep(0.05)

        tick_thread = threading.Thread(target=_ticker_loop, daemon=True)
        tick_thread.start()

        print(f"[Setup] Starting Autoware container '{args.container_name}'...")
        start_autoware_container(
            args.image,
            args.container_name,
            container_xml,
            host_xml,
            args.ros_domain_id,
        )

        # ------------------------------------------------------------------
        # CHECK 1, 2 & 3: Topic Discovery + Subscribing to Camera and LiDAR
        # ------------------------------------------------------------------
        print("\n[Check 1, 2 & 3] Verifying ROS 2 discovery, topic visibility, and sensor subscriptions...")
        sub_and_discover_code = """
import json
import time
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy
from rosgraph_msgs.msg import Clock
from sensor_msgs.msg import Image, CameraInfo, PointCloud2
from autoware_vehicle_msgs.msg import VelocityReport, SteeringReport

rclpy.init()
node = Node("carla_autoware_verifier")

received = {
    "clock": None,
    "camera_image": None,
    "camera_info": None,
    "lidar_points": None,
    "velocity_status": None,
    "steering_status": None,
}

qos_sensor = QoSProfile(
    reliability=ReliabilityPolicy.RELIABLE,
    durability=DurabilityPolicy.VOLATILE,
    history=HistoryPolicy.KEEP_LAST,
    depth=10,
)

def on_clock(msg):
    if received["clock"] is None:
        received["clock"] = {"sec": msg.clock.sec, "nanosec": msg.clock.nanosec}

def on_image(msg):
    if received["camera_image"] is None:
        received["camera_image"] = {
            "width": msg.width,
            "height": msg.height,
            "encoding": msg.encoding,
            "bytes": len(msg.data),
            "frame_id": msg.header.frame_id,
        }

def on_cam_info(msg):
    if received["camera_info"] is None:
        received["camera_info"] = {
            "width": msg.width,
            "height": msg.height,
            "frame_id": msg.header.frame_id,
        }

def on_lidar(msg):
    if received["lidar_points"] is None:
        received["lidar_points"] = {
            "width": msg.width,
            "height": msg.height,
            "point_step": msg.point_step,
            "bytes": len(msg.data),
            "frame_id": msg.header.frame_id,
        }

def on_velocity(msg):
    if received["velocity_status"] is None:
        received["velocity_status"] = {
            "longitudinal_velocity": float(msg.longitudinal_velocity),
            "frame_id": msg.header.frame_id,
        }

def on_steering(msg):
    if received["steering_status"] is None:
        received["steering_status"] = {
            "steering_tire_angle": float(msg.steering_tire_angle),
        }

subs = [
    node.create_subscription(Clock, "/clock", on_clock, qos_sensor),
    node.create_subscription(Image, "/sensing/camera/front/image", on_image, qos_sensor),
    node.create_subscription(CameraInfo, "/sensing/camera/front/camera_info", on_cam_info, qos_sensor),
    node.create_subscription(PointCloud2, "/sensing/lidar/top/pointcloud_raw_ex", on_lidar, qos_sensor),
    node.create_subscription(VelocityReport, "/vehicle/status/velocity_status", on_velocity, qos_sensor),
    node.create_subscription(SteeringReport, "/vehicle/status/steering_status", on_steering, qos_sensor),
]

expected = (
    "/clock",
    "/tf",
    "/sensing/camera/front/image",
    "/sensing/camera/front/camera_info",
    "/sensing/lidar/top/pointcloud_raw_ex",
    "/vehicle/status/velocity_status",
    "/vehicle/status/steering_status",
    "/control/command/control_cmd",
)
deadline = time.time() + 15.0
topics_map = {}
while time.time() < deadline:
    rclpy.spin_once(node, timeout_sec=0.1)
    topics_map = {name: types for name, types in node.get_topic_names_and_types()}
    if (
        all(t in topics_map for t in expected)
        and all(received[k] is not None for k in ("clock", "camera_image", "lidar_points", "velocity_status"))
    ):
        break

node.destroy_node()
rclpy.shutdown()

print("JSON_RESULT:" + json.dumps({"topics": topics_map, "received": received}))
"""
        raw_out = docker_exec_python(args.container_name, sub_and_discover_code, timeout=30.0)
        json_line = [line for line in raw_out.splitlines() if line.startswith("JSON_RESULT:")][-1]
        result_123 = json.loads(json_line[len("JSON_RESULT:"):])

        discovered_topics = result_123["topics"]
        received_msgs = result_123["received"]

        expected_topics = [
            "/clock",
            "/tf",
            "/sensing/camera/front/image",
            "/sensing/camera/front/camera_info",
            "/sensing/lidar/top/pointcloud_raw_ex",
            "/vehicle/status/velocity_status",
            "/vehicle/status/steering_status",
            "/control/command/control_cmd",
        ]
        missing_topics = [t for t in expected_topics if t not in discovered_topics]
        if missing_topics:
            raise AssertionError(
                f"Missing expected CARLA topics inside Autoware container: {missing_topics}\n"
                f"Discovered: {sorted(discovered_topics.keys())}"
            )

        print("  [PASS] Check 1: Autoware container connected to CARLA host over CycloneDDS.")
        print(f"  [PASS] Check 2: Discovered {len(discovered_topics)} ROS 2 topics from CARLA inside Autoware:")
        for t in expected_topics:
            print(f"           - {t} ({', '.join(discovered_topics[t])})")

        assert received_msgs["camera_image"] is not None and received_msgs["camera_image"]["bytes"] > 0, (
            f"Did not receive valid camera image: {received_msgs['camera_image']}"
        )
        assert received_msgs["lidar_points"] is not None and received_msgs["lidar_points"]["bytes"] > 0, (
            f"Did not receive valid LiDAR pointcloud: {received_msgs['lidar_points']}"
        )
        print("  [PASS] Check 3: Subscribed and received sensor messages inside Autoware container:")
        print(f"           - Camera (/sensing/camera/front/image)          : {received_msgs['camera_image']}")
        print(f"           - LiDAR  (/sensing/lidar/top/pointcloud_raw_ex) : {received_msgs['lidar_points']}")
        print(f"           - Clock  (/clock)                               : {received_msgs['clock']}")

        # ------------------------------------------------------------------
        # CHECK 4: Publish Steering Angle & Acceleration from Autoware
        # ------------------------------------------------------------------
        print("\n[Check 4] Publishing steering angle (0.35 rad) & acceleration (3.0 m/s^2) from Autoware container...")
        initial_loc = ego.get_location()
        initial_ctrl = ego.get_control()
        print(
            f"  Initial CARLA Ego State : location=({initial_loc.x:.2f}, {initial_loc.y:.2f}, {initial_loc.z:.2f}), "
            f"throttle={initial_ctrl.throttle:.2f}, steer={initial_ctrl.steer:.3f}"
        )

        pub_control_code = """
import json
import time
import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy, HistoryPolicy
from autoware_control_msgs.msg import Control
from autoware_vehicle_msgs.msg import VelocityReport, SteeringReport

rclpy.init()
node = Node("autoware_control_publisher")

qos_control = QoSProfile(
    reliability=ReliabilityPolicy.RELIABLE,
    durability=DurabilityPolicy.TRANSIENT_LOCAL,
    history=HistoryPolicy.KEEP_LAST,
    depth=1,
)
qos_status = QoSProfile(
    reliability=ReliabilityPolicy.RELIABLE,
    durability=DurabilityPolicy.VOLATILE,
    history=HistoryPolicy.KEEP_LAST,
    depth=10,
)

pub = node.create_publisher(Control, "/control/command/control_cmd", qos_control)

latest_velocity = 0.0
latest_steering = 0.0

def on_vel(msg):
    global latest_velocity
    latest_velocity = float(msg.longitudinal_velocity)

def on_steer(msg):
    global latest_steering
    latest_steering = float(msg.steering_tire_angle)

sub_vel = node.create_subscription(VelocityReport, "/vehicle/status/velocity_status", on_vel, qos_status)
sub_steer = node.create_subscription(SteeringReport, "/vehicle/status/steering_status", on_steer, qos_status)

target_accel = 3.0
target_steer = 0.35

start = time.time()
published_count = 0
while time.time() - start < 3.5:
    msg = Control()
    msg.lateral.steering_tire_angle = target_steer
    msg.lateral.steering_tire_rotation_rate = 1.0
    msg.lateral.is_defined_steering_tire_rotation_rate = True
    msg.longitudinal.acceleration = target_accel
    msg.longitudinal.velocity = 10.0
    pub.publish(msg)
    published_count += 1
    rclpy.spin_once(node, timeout_sec=0.05)

node.destroy_node()
rclpy.shutdown()

print("JSON_CONTROL_RESULT:" + json.dumps({
    "published_count": published_count,
    "target_acceleration": target_accel,
    "target_steering_tire_angle": target_steer,
    "reported_longitudinal_velocity": latest_velocity,
    "reported_steering_tire_angle": latest_steering,
}))
"""
        raw_ctrl_out = docker_exec_python(args.container_name, pub_control_code, timeout=30.0)
        ctrl_json_line = [
            line for line in raw_ctrl_out.splitlines() if line.startswith("JSON_CONTROL_RESULT:")
        ][-1]
        ctrl_result = json.loads(ctrl_json_line[len("JSON_CONTROL_RESULT:"):])

        final_loc = ego.get_location()
        final_vel = ego.get_velocity()
        final_ctrl = ego.get_control()
        speed_mps = math.sqrt(final_vel.x ** 2 + final_vel.y ** 2 + final_vel.z ** 2)
        displacement = final_loc.distance(initial_loc)

        print(
            f"  Final CARLA Ego State   : location=({final_loc.x:.2f}, {final_loc.y:.2f}, {final_loc.z:.2f}), "
            f"displacement={displacement:.2f} m, speed={speed_mps:.2f} m/s, "
            f"throttle={final_ctrl.throttle:.2f}, steer={final_ctrl.steer:.3f}"
        )
        print(f"  Autoware Status Feedback: {ctrl_result}")

        assert abs(final_ctrl.steer) > 0.05 or abs(ctrl_result["reported_steering_tire_angle"]) > 0.05, (
            f"Vehicle steering angle did not respond to /control/command/control_cmd: "
            f"carla_steer={final_ctrl.steer}, reported={ctrl_result['reported_steering_tire_angle']}"
        )
        assert displacement > 0.5 or speed_mps > 0.5 or ctrl_result["reported_longitudinal_velocity"] > 0.5, (
            f"Vehicle did not accelerate in response to /control/command/control_cmd: "
            f"displacement={displacement}, speed={speed_mps}, reported_vel={ctrl_result['reported_longitudinal_velocity']}"
        )

        print(
            "  [PASS] Check 4: Published /control/command/control_cmd from Autoware container "
            "and verified CARLA ego vehicle accelerated and steered."
        )
        print("\n" + "=" * 78)
        print("ALL 4 CHECKS PASSED SUCCESSFULLY!")
        print("=" * 78)
        return 0

    finally:
        stop_ticking.set()
        if tick_thread is not None:
            tick_thread.join(timeout=2.0)

        for s in sensors:
            try:
                s.destroy()
            except Exception:
                pass
        if ego is not None:
            try:
                ego.destroy()
            except Exception:
                pass
        try:
            world.apply_settings(original_settings)
        except Exception:
            pass
        subprocess.run(
            ["docker", "rm", "-f", args.container_name],
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
            check=False,
        )


if __name__ == "__main__":
    sys.exit(main())
