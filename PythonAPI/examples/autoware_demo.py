#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Autoware sensor-kit demo (port of TIER IV's autoware-support example).

Spawns an ego vehicle with an AWSIM-compatible sensor kit (VLP16-style lidar,
traffic-light camera, IMU, GNSS, vehicle-status sensor), wires the sensors to
Autoware's expected ROS 2 topic names via the ``ros_topic_name`` blueprint
attribute, and runs a real-time-paced synchronous simulation loop.
"""

import argparse
import json
import math
import os
import time
import random

import carla


# Autoware post-process profile for the traffic light recognition camera.
# These values were originally hardcoded in ActorBlueprintFunctionLibrary.cpp
# and are now managed as a JSON profile loaded by UPostProcessJsonUtils.
AUTOWARE_POSTPROCESS_SETTINGS = {
    "Settings": {
        "bOverride_MotionBlurAmount": True, "MotionBlurAmount": 0.5,
        "bOverride_MotionBlurMax": True, "MotionBlurMax": 5.0,
        "bOverride_MotionBlurPerObjectSize": True, "MotionBlurPerObjectSize": 0.0,
        "bOverride_LensFlareIntensity": True, "LensFlareIntensity": 0.2,
        "bOverride_BloomIntensity": True, "BloomIntensity": 0.0,
        "bOverride_AutoExposureMethod": True, "AutoExposureMethod": 1,
        "bOverride_AutoExposureBias": True, "AutoExposureBias": 1.5,
        "bOverride_LocalExposureHighlightContrastScale": True, "LocalExposureHighlightContrastScale": 0.7,
        "bOverride_LocalExposureShadowContrastScale": True, "LocalExposureShadowContrastScale": 0.65,
        "bOverride_CameraShutterSpeed": True, "CameraShutterSpeed": 15.0,
        "bOverride_CameraISO": True, "CameraISO": 300000.0,
        "bOverride_DepthOfFieldFstop": True, "DepthOfFieldFstop": 9.8,
        "bOverride_DepthOfFieldMinFstop": True, "DepthOfFieldMinFstop": 1.2,
        "bOverride_DepthOfFieldBladeCount": True, "DepthOfFieldBladeCount": 5,
        "bOverride_AutoExposureMinBrightness": True, "AutoExposureMinBrightness": 0.0,
        "bOverride_AutoExposureMaxBrightness": True, "AutoExposureMaxBrightness": 20.0,
        "bOverride_AutoExposureSpeedUp": True, "AutoExposureSpeedUp": 3.0,
        "bOverride_AutoExposureSpeedDown": True, "AutoExposureSpeedDown": 1.0,
        "bOverride_AutoExposureCalibrationConstant_DEPRECATED": True, "AutoExposureCalibrationConstant_DEPRECATED": 16.0,
        "bOverride_DepthOfFieldSensorWidth": True, "DepthOfFieldSensorWidth": 24.576,
        "bOverride_DepthOfFieldFocalDistance": True, "DepthOfFieldFocalDistance": 250.0,
        "bOverride_DepthOfFieldDepthBlurAmount": True, "DepthOfFieldDepthBlurAmount": 1.0,
        "bOverride_DepthOfFieldDepthBlurRadius": True, "DepthOfFieldDepthBlurRadius": 0.0,
        "bOverride_FilmSlope": True, "FilmSlope": 0.88,
        "bOverride_FilmToe": True, "FilmToe": 0.55,
        "bOverride_FilmShoulder": True, "FilmShoulder": 0.26,
        "bOverride_FilmBlackClip": True, "FilmBlackClip": 0.0,
        "bOverride_FilmWhiteClip": True, "FilmWhiteClip": 0.04,
        "bOverride_WhiteTemp": True, "WhiteTemp": 7700.0,
        "bOverride_WhiteTint": True, "WhiteTint": -0.15,
        "bOverride_SceneFringeIntensity": True, "SceneFringeIntensity": 0.15,
        "bOverride_ChromaticAberrationStartOffset": True, "ChromaticAberrationStartOffset": 0.0,
        "bOverride_ColorSaturation": True, "ColorSaturation": {"X": 0.5, "Y": 0.5, "Z": 0.5, "W": 1.0},
        "bOverride_ColorContrast": True, "ColorContrast": {"X": 1.6, "Y": 1.6, "Z": 1.6, "W": 1.0},
        "bOverride_ColorGamma": True, "ColorGamma": {"X": 1.2, "Y": 1.2, "Z": 1.2, "W": 1.0},
        "bOverride_ColorGammaHighlights": True, "ColorGammaHighlights": {"X": 0.5, "Y": 0.5, "Z": 0.5, "W": 1.0},
        "bOverride_ToneCurveAmount": True, "ToneCurveAmount": 1.0,
        "bOverride_SceneColorTint": True, "SceneColorTint": {"R": 0.785339, "G": 0.879092, "B": 0.93125, "A": 1.0},
        "bOverride_VignetteIntensity": True, "VignetteIntensity": 0.7,
    }
}


def deploy_postprocess_profile(profile_name, settings):
    """Deploy a PostProcess JSON profile to the Content directory.

    The file is placed at Content/Carla/Config/PostProcess/{profile_name}.json,
    which is the path UPostProcessJsonUtils::GetPostProcessConfigPath() resolves to.

    Deploys to both:
    - Source tree: {workspace}/Unreal/CarlaUnreal/Content/Carla/Config/PostProcess/
    - Package builds found under: {workspace}/Build/Package/*/Linux/CarlaUnreal/Content/Carla/Config/PostProcess/
    """
    script_dir = os.path.dirname(os.path.abspath(__file__))
    workspace_path = os.path.abspath(os.path.join(script_dir, "..", ".."))

    # Collect all target directories
    target_dirs = []

    # Source tree
    source_config = os.path.join(
        workspace_path, "Unreal", "CarlaUnreal", "Content",
        "Carla", "Config", "PostProcess"
    )
    target_dirs.append(source_config)

    # Package builds
    package_base = os.path.join(workspace_path, "Build", "Package")
    if os.path.isdir(package_base):
        import glob
        for pkg_content in glob.glob(os.path.join(
            package_base, "*", "Linux", "CarlaUnreal", "Content",
            "Carla", "Config", "PostProcess"
        )):
            target_dirs.append(pkg_content)

    for config_dir in target_dirs:
        try:
            os.makedirs(config_dir, exist_ok=True)
            profile_path = os.path.join(config_dir, f"{profile_name}.json")
            with open(profile_path, 'w') as f:
                json.dump(settings, f, indent=2)
            log_info(f"Deployed PostProcess profile: {profile_path}")
        except OSError as error:
            log_warning(f"Could not deploy PostProcess profile to {config_dir}: {error}")


def log_info(text):
    print("[INFO]: %s" % text)


def log_warning(text):
    print("[WARNING]: %s" % text)


def log_error(text):
    print("[ERROR]: %s" % text)


def _rpy_to_matrix(roll, pitch, yaw):
    """3x3 rotation matrix for the RPY convention Rz(yaw) * Ry(pitch) * Rx(roll)."""
    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)
    return [
        [cy * cp, cy * sp * sr - sy * cr, cy * sp * cr + sy * sr],
        [sy * cp, sy * sp * sr + cy * cr, sy * sp * cr - cy * sr],
        [-sp, cp * sr, cp * cr],
    ]


def _matrix_to_rpy(R):
    """Inverse of _rpy_to_matrix (ZYX Euler extraction)."""
    pitch = math.atan2(-R[2][0], math.hypot(R[0][0], R[1][0]))
    yaw = math.atan2(R[1][0], R[0][0])
    roll = math.atan2(R[2][1], R[2][2])
    return roll, pitch, yaw


def _mat_mul(A, B):
    return [
        [sum(A[i][k] * B[k][j] for k in range(3)) for j in range(3)]
        for i in range(3)
    ]


def _mat_vec(A, v):
    return [sum(A[i][k] * v[k] for k in range(3)) for i in range(3)]


class ROS2:
    """
    ROS2 coordinate system to CARLA coordinate system conversions.
    ROS2 uses a right-handed system and CARLA uses a left-handed system.
    Also converts rotation units:
    - ROS2 uses radians
    - CARLA uses degrees
    https://github.com/carla-simulator/ros-bridge/blob/e9063d97ff5a724f76adbb1b852dc71da1dcfeec/carla_common/src/carla_common/transforms.py#L307-L310
    """

    @staticmethod
    def Location(x: float = 0.0, y: float = 0.0, z: float = 0.0):
        """In meters"""
        return carla.Location(x=x, y=-y, z=z)

    @staticmethod
    def Rotation(roll: float = 0.0, pitch: float = 0.0, yaw: float = 0.0):
        """In radians"""
        return carla.Rotation(roll=math.degrees(roll),
                              pitch=-math.degrees(pitch),
                              yaw=-math.degrees(yaw))

    class Transform:
        """Transform in the ROS2 coordinate system."""

        def __init__(self,
                     x: float = 0.0,
                     y: float = 0.0,
                     z: float = 0.0,
                     roll: float = 0.0,
                     pitch: float = 0.0,
                     yaw: float = 0.0):
            self.x = x
            self.y = y
            self.z = z
            self.roll = roll
            self.pitch = pitch
            self.yaw = yaw

        def to_carla(self):
            return carla.Transform(ROS2.Location(x=self.x,
                                                 y=self.y,
                                                 z=self.z),
                                   ROS2.Rotation(roll=self.roll,
                                                 pitch=self.pitch,
                                                 yaw=self.yaw))

        def compose(self, other):
            """Homogeneous-frame composition self * other (pure-python
            replacement for the PyKDL frames the tier4 original used)."""
            R1 = _rpy_to_matrix(self.roll, self.pitch, self.yaw)
            R2 = _rpy_to_matrix(other.roll, other.pitch, other.yaw)
            R = _mat_mul(R1, R2)
            p = _mat_vec(R1, [other.x, other.y, other.z])
            roll, pitch, yaw = _matrix_to_rpy(R)
            return ROS2.Transform(self.x + p[0],
                                  self.y + p[1],
                                  self.z + p[2],
                                  roll, pitch, yaw)


def chain_transforms(transforms):
    """
    :param transforms: list of ROS2.Transform
    :return: ROS2.Transform
    """
    result = transforms[0]
    for t in transforms[1:]:
        result = result.compose(t)
    return result


def try_set_ros_topic_name(blueprint, topic_name):
    """Sets the ros_topic_name attribute if the server exposes it.

    The attribute is added by the Autoware-port ActorBlueprintFunctionLibrary
    changes; warn instead of crashing when running against a server that does
    not have them yet (the sensor then publishes on its default topic).
    """
    if blueprint.has_attribute("ros_topic_name"):
        blueprint.set_attribute("ros_topic_name", topic_name)
    else:
        log_warning(
            f"Blueprint '{blueprint.id}' has no 'ros_topic_name' attribute; "
            f"'{topic_name}' not applied (server predates the Autoware port?)")


def generate_vlp16_blueprint(blueprint_library):
    """Generates a blueprint for VLP16

    The following assumptions were made based on the configuration in AWSIM:
    - 10 Hz publish frequency
    - 0.2 deg horizontal resolution"""

    blueprint = blueprint_library.find("sensor.lidar.ray_cast")

    # The following values are taken from the VLP16 datasheet
    blueprint.set_attribute("channels", "16")
    blueprint.set_attribute("range", "100.0")
    blueprint.set_attribute("upper_fov", "10.0")
    blueprint.set_attribute("lower_fov", "-20.0")

    # Calculated as: horizontal_fov / horizontal_resolution / sensor_tick * channels
    blueprint.set_attribute("points_per_second", "288000")

    blueprint.set_attribute("sensor_tick", "0.1")

    # ROS settings
    blueprint.set_attribute("ros_name", "velodyne_top")  # frame_id
    try_set_ros_topic_name(blueprint, "/sensing/lidar/top/pointcloud_raw_ex")

    return blueprint


def generate_traffic_light_camera_blueprint(blueprint_library):
    """Generates a traffic light camera

    The following assumptions were made based on the configuration in AWSIM:
    - 10 Hz publish frequency
    - 1920 x 1080 image resolution
    - 90 deg horizontal field of view"""

    blueprint = blueprint_library.find("sensor.camera.rgb")

    blueprint.set_attribute("fov", "90.0")
    blueprint.set_attribute("image_size_x", "1920")
    blueprint.set_attribute("image_size_y", "1080")
    blueprint.set_attribute("sensor_tick", "0.1")
    if blueprint.has_attribute("post_process_profile"):
        blueprint.set_attribute("post_process_profile", "autoware_demo")

    # ROS settings
    blueprint.set_attribute("ros_name", "traffic_light_left_camera/camera_optical_link")  # frame_id
    try_set_ros_topic_name(blueprint, "/sensing/camera/traffic_light")

    return blueprint


def generate_front_camera_blueprint(blueprint_library):
    """Generates a forward-facing camera for visualization (e.g. RViz)"""

    blueprint = blueprint_library.find("sensor.camera.rgb")

    blueprint.set_attribute("fov", "90.0")
    blueprint.set_attribute("image_size_x", "1280")
    blueprint.set_attribute("image_size_y", "720")
    blueprint.set_attribute("sensor_tick", "0.1")
    if blueprint.has_attribute("post_process_profile"):
        blueprint.set_attribute("post_process_profile", "autoware_demo")

    # ROS settings
    blueprint.set_attribute("ros_name", "front_camera/camera_optical_link")  # frame_id
    try_set_ros_topic_name(blueprint, "/sensing/camera/front")

    return blueprint


def generate_imu_blueprint(blueprint_library):
    """Generates a blueprint for IMU"""

    blueprint = blueprint_library.find("sensor.other.imu")

    blueprint.set_attribute("sensor_tick", f"{1.0 / 30.0}")

    # ROS settings
    blueprint.set_attribute("ros_name", "tamagawa/imu_link")  # frame_id
    try_set_ros_topic_name(blueprint, "/sensing/imu/tamagawa/imu_raw")

    return blueprint


def generate_gnss_blueprint(blueprint_library, is_mgrs_enabled):
    """Generates a blueprint for GNSS"""

    sensor_name = "sensor.other.autoware_gnss" if is_mgrs_enabled else "sensor.other.gnss"

    try:
        blueprint = blueprint_library.find(sensor_name)
    except Exception:
        if is_mgrs_enabled:
            log_warning(
                "Tried to spawn Autoware GNSS Sensor, but it is inaccessible. "
                "Try running without MGRS offset: autoware_demo.py --mgrs_off"
            )
        else:
            log_error("Cannot spawn Carla GNSS sensor!")
        return None

    # Capture EVERY sim tick (sensor_tick 0.0). Classical only needs GNSS to
    # seed localization init (1 Hz was fine), but the e2e path derives
    # ground-truth kinematic_state from GNSS pose + twist and
    # carla_state_publisher gates on pose/twist stamp freshness (0.2 s):
    # - a slow tick leaves the pose a full period stale (gate rejects all);
    # - sensor_tick == dt drifts: float accumulation skips a capture every
    #   few hundred ticks and the stamp falls cumulatively behind.
    blueprint.set_attribute("sensor_tick", "0.0")

    # ROS settings
    blueprint.set_attribute("ros_name", "map")  # frame_id
    try_set_ros_topic_name(blueprint, "/sensing/gnss")

    return blueprint


def spawn_sensors(world, base_link, ego, args):
    """Spawns sensors relative to the provided base_link actor

    Positioning of the sensors is taken from the URDF for the
    awsim_sensor_kit_description package at:
    https://github.com/autowarefoundation/autoware_launch/tree/0.45.3/sensor_kit/awsim_sensor_kit_launch/awsim_sensor_kit_description
    """

    blueprint_library = world.get_blueprint_library()

    sensor_kit_blueprint = blueprint_library.find("util.actor.empty")
    sensor_kit_blueprint.set_attribute("ros_name", "sensor_kit_base_link")
    vlp16_blueprint = generate_vlp16_blueprint(blueprint_library)
    traffic_light_camera_blueprint \
        = generate_traffic_light_camera_blueprint(blueprint_library)
    imu_blueprint = generate_imu_blueprint(blueprint_library)
    gnss_receiver_blueprint = generate_gnss_blueprint(blueprint_library, args.mgrs_off)

    base_link_to_sensor_kit_transform = ROS2.Transform(
        x=0.9,
        z=2.0,
        roll=-0.001,
        pitch=0.015,
        yaw=-0.0364)
    sensor_kit = world.spawn_actor(
        sensor_kit_blueprint,
        base_link_to_sensor_kit_transform.to_carla(),
        attach_to=base_link)

    # Spawn top lidar
    sensor_kit_to_lidar_top_transform = ROS2.Transform(yaw=1.575)
    lidar_top = world.spawn_actor(
        vlp16_blueprint,
        sensor_kit_to_lidar_top_transform.to_carla(),
        attach_to=sensor_kit)
    lidar_top.enable_for_ros()

    # Spawn traffic light camera
    sensor_kit_to_traffic_light_left_camera_transform = ROS2.Transform(x=0.05,
                                                                       y=0.0175,
                                                                       z=-0.1)
    traffic_light_left_camera = world.spawn_actor(
        traffic_light_camera_blueprint,
        sensor_kit_to_traffic_light_left_camera_transform.to_carla(),
        attach_to=sensor_kit)
    traffic_light_left_camera.enable_for_ros()

    # Spawn front camera (visualization; not part of the AWSIM sensor kit)
    front_camera_blueprint = generate_front_camera_blueprint(blueprint_library)
    sensor_kit_to_front_camera_transform = ROS2.Transform(x=0.3, z=-0.3)
    front_camera = world.spawn_actor(
        front_camera_blueprint,
        sensor_kit_to_front_camera_transform.to_carla(),
        attach_to=sensor_kit)
    front_camera.enable_for_ros()

    # Spawn IMU
    # NOTE: IMU is mounted to Ego directly, because this is required for angular velocity to work
    pivot_to_base_link_transform = ROS2.Transform(x=-1.39706787)
    base_link_to_sensor_kit_transform = ROS2.Transform(x=0.9,
                                                       z=2.0,
                                                       roll=-0.001,
                                                       pitch=0.015,
                                                       yaw=-0.0364)
    sensor_kit_to_imu_transform = ROS2.Transform(roll=3.14159265359,
                                                 yaw=3.14159265359)

    pivot_to_imu_transform = chain_transforms([pivot_to_base_link_transform,
                                               base_link_to_sensor_kit_transform,
                                               sensor_kit_to_imu_transform])
    imu = world.spawn_actor(
        imu_blueprint,
        pivot_to_imu_transform.to_carla(),
        attach_to=ego)
    imu.enable_for_ros()

    # Spawn GNSS receiver
    if gnss_receiver_blueprint is not None:
        sensor_kit_to_gnss_receiver_transform = ROS2.Transform(x=-0.1, z=-0.2)
        gnss_receiver = world.spawn_actor(
            gnss_receiver_blueprint,
            sensor_kit_to_gnss_receiver_transform.to_carla(),
            attach_to=sensor_kit)
        gnss_receiver.enable_for_ros()

    # Spawn Vehicle Status Sensor
    # NOTE: enable_for_ros is not needed, because this sensor uses a global publisher.
    try:
        vehicle_status_blueprint = blueprint_library.find("sensor.other.vehicle_status")
        world.spawn_actor(
            vehicle_status_blueprint,
            carla.Transform(),
            attach_to=base_link)  # Attach to base_link with no offset, because velocities should come from rear axle
    except Exception:
        log_warning(
            "Blueprint 'sensor.other.vehicle_status' unavailable; "
            "/vehicle/status/* reports will not be published "
            "(server predates the Autoware port?)")


def spawn_ego_with_sensors(world, spawn_point, args):
    """Spawns a controllable vehicle with a basic sensor configuration

    The sensor configuration is compatible with the one for the Lexus RX450h
    in AWSIM. The vehicle itself is replaced by the Lincoln MKZ available in
    CARLA."""

    blueprint_library = world.get_blueprint_library()

    try:
        ego_blueprint = blueprint_library.find("vehicle.lincoln.mkz")
    except Exception:
        ego_blueprint = blueprint_library.filter("vehicle.lincoln.mkz*")[0]
    ego_blueprint.set_attribute("role_name", "ego")
    # Ego uses acceleration control from Autoware /control/command/control_cmd (not ackermann).
    # The opt-in attribute registers the vehicle for the Autoware interface
    # (/vehicle/status/* publishers + /control/command/* subscribers).
    if ego_blueprint.has_attribute("ros2_autoware_control"):
        ego_blueprint.set_attribute("ros2_autoware_control", "true")
    else:
        log_warning(
            "Blueprint has no 'ros2_autoware_control' attribute; the ego will not be "
            "registered for the Autoware vehicle interface (server predates the port?)")
    try_set_ros_topic_name(ego_blueprint, "/carla/input")  # Fallback; Autoware control_cmd has priority

    ego = world.spawn_actor(ego_blueprint, spawn_point)

    base_link_blueprint = blueprint_library.find("util.actor.empty")
    base_link_blueprint.set_attribute("ros_name", "sensor_kit_base_link")

    # Transformation between vehicle pivot and projection of the rear
    # axis on the ground (base link) as measured in Unreal Editor
    pivot_to_base_link_transform = ROS2.Transform(x=-1.39706787)
    base_link = world.spawn_actor(
        base_link_blueprint,
        pivot_to_base_link_transform.to_carla(),
        attach_to=ego)

    spawn_sensors(world, base_link, ego, args)

    return ego


def move_spectator(world, ego_vehicle):
    spectator = world.get_spectator()

    spectator_tf = ego_vehicle.get_transform()
    spectator_offset = ROS2.Transform(x=-6.0, z=1.5).to_carla()

    spectator_with_offset_position = spectator_tf.transform(spectator_offset.location)

    spectator_tf = carla.Transform(spectator_with_offset_position, spectator_tf.rotation)

    spectator.set_transform(spectator_tf)


class TimeStepData:
    def __init__(self, synchronous_mode=True, hz_rate=100, phys_substepping=False):
        self.synchronous_mode = synchronous_mode
        self.hz_rate = hz_rate if hz_rate not in (None, 0) else None
        self.phys_substepping = phys_substepping

    def get_sim_dt(self):
        return 1 / self.hz_rate if self.hz_rate not in (None, 0) else None

    def is_pure_step_execution_enabled(self):
        return self.synchronous_mode and self.hz_rate not in (None, 0) and not self.phys_substepping


def get_current_map_name(client):
    """
    Fetches the active world from the client to avoid stale pointer errors,
    and returns the active world map name.
    :returns: Active world map name (not a path).
    """
    world = client.get_world()
    return world.get_map().name.split('/')[-1]


def apply_world_settings(client, time_step_info, map_name=None, force_map_reload=False):
    """
    Stores all settings related to the simulation world.
    Default: applies synchronous mode + fixed time-step.

    :param client: Connected client to the Carla server instance.
    :param time_step_info: Instance of TimeStepData.
    :param map_name: Map to load and apply settings to.
    :param force_map_reload: Forces the map to be reloaded. The reload action cleans up the scene.

    :returns: world instance (possibly new) after loading a map
    """

    # Get current world reference
    world = client.get_world()
    current_map = 'Not set yet'

    # Reload map
    if force_map_reload:
        print(f"Force reloading map: {current_map}")
        world = client.load_world(get_current_map_name(client))
        current_map = get_current_map_name(client)

    # Load a new map
    elif map_name and map_name != current_map:
        print(f"Loading new map: {map_name}")
        client.load_world(map_name)
        world = client.get_world()
        current_map = map_name
    else:
        print(f"Map '{map_name}' is already loaded — skipping reload.")

    print(f'Loaded map: {current_map if map_name is None else map_name}')

    if world is None:
        raise RuntimeError("World instance is invalid after map load.")

    # Get Settings
    settings = world.get_settings()

    # Set synchronous mode
    settings.synchronous_mode = time_step_info.synchronous_mode
    settings.fixed_delta_seconds = time_step_info.get_sim_dt()

    # Set physics substepping
    if time_step_info.is_pure_step_execution_enabled():
        settings.substepping = False
    elif time_step_info.synchronous_mode:
        settings.max_substep_delta_time = 0.001  # max 1 ms per physics substep, swap to 0.01 if no need of extreme physics realism
        settings.max_substeps = 10
        settings.substepping = settings.fixed_delta_seconds <= settings.max_substep_delta_time * settings.max_substeps  # carla condition
    else:
        settings.substepping = time_step_info.phys_substepping

    # Apply settings
    world.apply_settings(settings)

    # Disable TF publishing in CARLA to avoid conflicts: Autoware publishes TF
    # information based on the URDF files of the vehicle and sensor kit.
    if hasattr(world, "set_publish_tf"):
        world.set_publish_tf(False)
    else:
        log_warning("World.set_publish_tf unavailable; CARLA keeps publishing TF")

    return world


def run_sync_simulation_loop(world,
                             target_time_scale=1.0,
                             acceptable_lag=0.05,
                             should_resync=False,
                             ego=None,
                             follow_ego=False,
                             target_sim_dt=0.01):
    """
    Runs a real-time simulation loop for a synchronous simulation environment.

    The loop advances the simulation world in fixed time steps determined by
    `target_time_scale`, maintaining real-time pacing. It adjusts for timing
    and can optionally resynchronize if the simulation falls behind.

    Notes
    -----
    - This function runs indefinitely until interrupted (e.g., with Ctrl+C).
    - When interrupted, it restores the world to asynchronous mode to prevent crashes.

    :param world: The simulation world instance.
    :param ego: The main vehicle actor - player pawn.
    :param target_time_scale: The simulation speed multiplier.
        Higher values make the simulation run faster, and lower to run slower relative to real time
        (2.0 = twice real-time speed, 0.5 = half the real-time speed).
    :param acceptable_lag: The maximum acceptable delay in seconds between real time and simulation time.
        If the loop falls behind by more than this value, a warning is logged.
    :param should_resync: If True, when lag exceeds the acceptable threshold,
        the loop resets its internal schedule to the current wall time,
        causing the next tick to run immediately to catch up to real time.
    :param follow_ego: If True, moves the spectator camera to follow the ego actor each tick.
    :param target_sim_dt: Fixed simulation step (in seconds).
        Used together with `target_time_scale` to determine real-time pacing between ticks.
    """

    real_dt = target_sim_dt / target_time_scale
    frame_count = 0
    next_tick_time = time.time()

    if world is None:
        log_error("World instance is invalid! Cannot proceed with simulation tick.")
        return

    try:
        while True:
            current_time = time.time()

            # Sleep until it's time for the next tick
            wait_time = next_tick_time - current_time
            if wait_time > 0:
                time.sleep(wait_time)

            # Advance simulation
            world.tick()
            frame_count += 1

            # Optional spectator movement
            if follow_ego and ego is not None:
                move_spectator(world, ego)

            # Schedule next tick
            next_tick_time += real_dt

            # Detect and handle lag
            lag = time.time() - next_tick_time
            if lag > acceptable_lag:
                log_warning(f"Simulation is {lag * 1000:.1f} ms behind schedule")
                if should_resync:
                    next_tick_time = time.time()

    except KeyboardInterrupt:
        log_info("Exiting simulation loop, restoring default Carla mode (asynchronous mode + variable time step)")
        settings = world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        world.apply_settings(settings)


def parse_hz_rate(value):
    if value.lower() in ("none", ''):
        return None
    try:
        return int(value)
    except ValueError:
        raise argparse.ArgumentTypeError("hz_rate must be an integer or 'None'.")


def parse_timeout(value):
    seconds = float(value)
    if seconds <= 0:
        raise argparse.ArgumentTypeError("--timeout must be a positive number of seconds.")
    return seconds


def main():
    argparser = argparse.ArgumentParser(
        description='CARLA Autoware sensor-kit demo')
    argparser.add_argument(
        '-v', '--verbose', action='store_true', dest='debug',
        help='Print debug information')
    argparser.add_argument(
        '--host', metavar='H', default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port', metavar='P', default=2000, type=int,
        help='TCP port of the CARLA RPC server (default: 2000)')
    argparser.add_argument(
        '--time_scale', metavar='T', default=1.0, type=float,
        help='Simulation timescale (default: 1.0)')
    argparser.add_argument(
        '--follow', action='store_true',
        help='Follow Ego vehicle')
    argparser.add_argument(
        '--resync', action='store_true',
        help='Resynchronize to the current time when lag exceeds the acceptable threshold. '
             'Only available for synchronized mode.'
    )
    argparser.add_argument(
        '--load_map',
        nargs='?',
        const='Town10HD_Opt',  # used when flag present but no value
        help="Load the provided map by its name."
    )
    argparser.add_argument(
        '--force_reload', action='store_true',
        help='Force map reload.')
    argparser.add_argument(
        '--substepping', action='store_true',
        help='Enable substepping for physics.')
    argparser.add_argument(
        '--run_async', action='store_true',
        help='Run the server and client in asynchronous mode.')
    argparser.add_argument(
        '--hz_rate',
        type=parse_hz_rate,
        default=100,
        help="Set 'None' or 0 for variable time step, otherwise use an integer for fixed time step rate."
    )
    argparser.add_argument(
        '--mgrs_off', action='store_false',
        help='Disable application of MGRS offset.')
    argparser.add_argument(
        '--list_maps', action='store_true',
        help='Lists only available maps and exit. Omit applying world setting and ego spawn.')
    argparser.add_argument(
        '--spawn_index', type=int, default=None,
        help='Index into the ego spawn point list; random choice if omitted.')
    argparser.add_argument(
        '--timeout', type=parse_timeout, default=60.0,
        help='RPC timeout in seconds (default: 60). A cold simulator start on UE 5.8 can '
             'take longer than a minute to answer get_world(); raise this when the map is '
             'still loading.')
    args = argparser.parse_args()

    # Deploy PostProcess profiles before connecting to the server
    deploy_postprocess_profile("autoware_demo", AUTOWARE_POSTPROCESS_SETTINGS)

    # Get Client info
    client = carla.Client(args.host, args.port)
    client.set_timeout(args.timeout)

    if args.list_maps:
        print("Available maps")
        print(client.get_available_maps())
        return

    # Determine TimeStep Data to be used
    time_step_info = TimeStepData(synchronous_mode=(not args.run_async),
                                  hz_rate=args.hz_rate,
                                  phys_substepping=args.substepping)

    # Apply Settings
    world = apply_world_settings(client=client,
                                 time_step_info=time_step_info,
                                 map_name=args.load_map,
                                 force_map_reload=args.force_reload)
    log_info(
        f"Applied settings:\n"
        f"\tsynchronous mode: {time_step_info.synchronous_mode}\n"
        f"\tfixed time step: {time_step_info.hz_rate} (in Hz)\n"
        f"\tsimulation dt: {time_step_info.get_sim_dt()} (in seconds)\n"
        f"\tphysics substepping: {time_step_info.phys_substepping}\n"
        f"\ttime scale: {args.time_scale:.2f}\n"
        f"\tpure step execution: {time_step_info.is_pure_step_execution_enabled()}"
    )

    # Spawn Ego. Prefer the level's dedicated ego spawn points (Autoware-port
    # RPC); fall back to the map's regular spawn points when the API or the
    # server-side RPC is unavailable.
    spawn_points = []
    try:
        spawn_points = world.get_ego_spawn_points()
    except (AttributeError, RuntimeError) as error:
        log_warning(f"get_ego_spawn_points unavailable ({error}); using map spawn points")
    if not spawn_points:
        spawn_points = world.get_map().get_spawn_points()
    if not spawn_points:
        raise RuntimeError("No spawn points available. Load a map that provides spawn points.")
    if args.spawn_index is not None:
        spawn_point = spawn_points[args.spawn_index % len(spawn_points)]
    else:
        spawn_point = random.choice(spawn_points)
    ego = spawn_ego_with_sensors(world, spawn_point, args)

    world.tick()  # tick to process the changes (settings, ego + sensors spawn)
    move_spectator(world, ego)

    log_info('Ego spawned!')
    log_warning('Kill this script before stopping simulation!')

    # Run synchronous simulation loop
    if time_step_info.synchronous_mode:
        run_sync_simulation_loop(target_sim_dt=time_step_info.get_sim_dt(),
                                 target_time_scale=args.time_scale,
                                 should_resync=args.resync,
                                 follow_ego=args.follow,
                                 world=world,
                                 ego=ego)


if __name__ == '__main__':
    main()
