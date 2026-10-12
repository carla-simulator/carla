#!/usr/bin/env python3
"""e2e_state_publishers.launch.py -- ground-truth localization glue for VAD.

Launched by run_carla_autoware.sh in --mode e2e (ros2 launch <this file>).

VAD (autoware_universe e2e/autoware_tensorrt_vad) does not run the normal
Autoware localization stack; it needs /localization/kinematic_state and
/localization/acceleration produced from CARLA ground truth by three plain
ROS nodes (TIER IV's autoware_carla_interface wiring):

  carla_state_publisher              /sensing/gnss/pose_with_covariance (+ twist)
                                       -> /localization/kinematic_state
  autoware_vehicle_velocity_converter  /vehicle/status/velocity_status
                                       -> twist_with_covariance
  autoware_twist2accel               kinematic_state + twist
                                       -> /localization/acceleration

plus image_transport 'republish' nodes turning the six raw camera streams
into the compressed transport VAD consumes
(/sensing/camera/CAM_*/image_raw -> .../image_raw/compressed).

NOTE: the in/out remap names below follow the autoware_carla_interface /
autoware.universe conventions of the TIER IV-validated setup; if your
workspace pins a different Autoware release, check them against
`ros2 node info` for each node.
"""

from launch import LaunchDescription
from launch_ros.actions import Node

# Same load-bearing order as spawn_vad_rig.py:
# image0=FRONT, 1=BACK, 2=FRONT_LEFT, 3=BACK_LEFT, 4=FRONT_RIGHT, 5=BACK_RIGHT
CAMERAS = [
    "CAM_FRONT",
    "CAM_BACK",
    "CAM_FRONT_LEFT",
    "CAM_BACK_LEFT",
    "CAM_FRONT_RIGHT",
    "CAM_BACK_RIGHT",
]

TWIST_TOPIC = "/sensing/vehicle_velocity_converter/twist_with_covariance"


def generate_launch_description():
    nodes = []

    # 1. Ground-truth pose -> nav_msgs/Odometry kinematic state.
    nodes.append(
        Node(
            package="autoware_carla_interface",
            executable="carla_state_publisher",
            name="carla_state_publisher",
            output="screen",
            # The node's ports are PRIVATE names (~/input/..., ~/output/odometry);
            # remap keys must carry the ~/ prefix or they silently miss.
            remappings=[
                ("~/input/pose_with_covariance", "/sensing/gnss/pose_with_covariance"),
                ("~/input/twist_with_covariance", TWIST_TOPIC),
                ("~/output/odometry", "/localization/kinematic_state"),
            ],
        )
    )

    # 2. VelocityReport -> TwistWithCovarianceStamped.
    nodes.append(
        Node(
            package="autoware_vehicle_velocity_converter",
            executable="autoware_vehicle_velocity_converter_node",
            name="autoware_vehicle_velocity_converter",
            output="screen",
            parameters=[
                {
                    "speed_scale_factor": 1.0,
                    "frame_id": "base_link",
                    "velocity_stddev_xx": 0.2,
                    "angular_velocity_stddev_zz": 0.1,
                }
            ],
            remappings=[
                ("velocity_status", "/vehicle/status/velocity_status"),
                ("twist_with_covariance", TWIST_TOPIC),
            ],
        )
    )

    # 3. Differentiate twist -> /localization/acceleration.
    nodes.append(
        Node(
            package="autoware_twist2accel",
            # universe >= 0.52 renamed the executable (was 'twist2accel')
            executable="autoware_twist2accel_node",
            name="autoware_twist2accel",
            output="screen",
            parameters=[
                {
                    "use_odom": True,
                    "accel_lowpass_gain": 0.9,
                }
            ],
            remappings=[
                ("input/odom", "/localization/kinematic_state"),
                ("input/twist", TWIST_TOPIC),
                ("output/accel", "/localization/acceleration"),
            ],
        )
    )

    # 4. raw -> compressed republishers for the six VAD cameras.
    # The simulator's per-sensor 'ros_topic_name' override acts as the
    # sensor's topic NAMESPACE: the camera publisher appends '/image' (and
    # '/camera_info') under it. So the actual raw stream for a rig camera
    # configured with ros_topic_name=<base>/image_raw is <base>/image_raw/image;
    # republish it as <base>/image_raw/compressed, which is what VAD's
    # compressed image_transport subscribers read.
    for cam in CAMERAS:
        base = "/sensing/camera/%s/image_raw" % cam
        nodes.append(
            Node(
                package="image_transport",
                executable="republish",
                name="republish_%s" % cam.lower(),
                output="screen",
                arguments=["raw", "compressed"],
                remappings=[
                    ("in", base + "/image"),
                    ("out/compressed", base + "/compressed"),
                ],
            )
        )

    # 5. camera_info relays. VAD's launch subscribes calibration at
    # /sensing/camera/CAM_*/camera_info, but the simulator publishes it under
    # the sensor's topic namespace (/sensing/camera/CAM_*/image_raw/camera_info).
    # Without these relays the VAD frame is never complete: fill_dropped_data()
    # backfills missing *images* only, so a missing camera_info silently drops
    # every frame (no log line -- trigger_inference returns nullopt).
    for cam in CAMERAS:
        nodes.append(
            Node(
                package="topic_tools",
                executable="relay",
                name="relay_caminfo_%s" % cam.lower(),
                output="screen",
                arguments=[
                    "/sensing/camera/%s/image_raw/camera_info" % cam,
                    "/sensing/camera/%s/camera_info" % cam,
                ],
            )
        )

    # 6. camera frame aliases. The simulator stamps image/camera_info headers
    # with the sensor's ros name (frame_id "CAM_FRONT", ...), but the sensor-kit
    # URDF only defines CAM_*/camera_link -> CAM_*/camera_optical_link. VAD looks
    # up base_link -> <camera_info frame_id>; without an alias the lookup throws
    # (and, until universe fixes the throttle-clock in lookup_base2cam's catch
    # block, the vad_node segfaults). Identity TF: optical link == CARLA frame.
    for cam in CAMERAS:
        nodes.append(
            Node(
                package="tf2_ros",
                executable="static_transform_publisher",
                name="tf_alias_%s" % cam.lower(),
                output="screen",
                arguments=[
                    "--frame-id", "%s/camera_optical_link" % cam,
                    "--child-frame-id", cam,
                ],
            )
        )

    return LaunchDescription(nodes)
