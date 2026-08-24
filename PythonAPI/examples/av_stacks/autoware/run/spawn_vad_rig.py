#!/usr/bin/env python3
"""spawn_vad_rig.py -- attach the six VAD cameras to the ego vehicle.

Attaches the nuScenes-style six-camera rig required by Autoware's VAD
end-to-end planner (autoware_universe e2e/autoware_tensorrt_vad) to the ego
vehicle (role_name 'hero' or 'ego', normally spawned by
PythonAPI/examples/autoware_demo.py) and enables each camera for CARLA's
native ROS 2 publisher.

Camera table (order is LOAD-BEARING for VAD: image0=FRONT, 1=BACK,
2=FRONT_LEFT, 3=BACK_LEFT, 4=FRONT_RIGHT, 5=BACK_RIGHT). All cameras are
1600x900; FOV 70 deg except CAM_BACK which is 110 deg. Topics:
/sensing/camera/CAM_*/image_raw (+ /camera_info, published natively).

This client is strictly READ-ONLY with respect to simulation time: it NEVER
ticks and never touches world settings -- autoware_demo.py must remain the
single ticking client (sync mode).

Guard: the per-sensor 'ros_topic_name' blueprint attribute is provided by the
concurrent Autoware-layer migration. If it is absent, this script exits with
a clear message instead of publishing on wrong topics.

Runs until SIGTERM/SIGINT, then destroys its sensors and exits cleanly.
"""

import argparse
import signal
import sys
import time

try:
    import carla
except ImportError:
    sys.exit(
        "spawn_vad_rig.py: cannot 'import carla'. Install the CARLA Python "
        "wheel (PythonAPI/carla/dist) into this environment first."
    )

# name, fov_deg, (x, y, z, yaw_deg) relative to the ego vehicle.
# CARLA left-handed frame: +x forward, +y right, positive yaw turns right.
# Order matters (VAD image index) -- do not reorder.
CAMERAS = [
    ("CAM_FRONT",       70.0, ( 0.70,  0.00, 1.60,    0.0)),
    ("CAM_BACK",       110.0, (-1.60,  0.00, 1.55,  180.0)),
    ("CAM_FRONT_LEFT",  70.0, ( 0.50, -0.50, 1.60,  -55.0)),
    ("CAM_BACK_LEFT",   70.0, (-0.70, -0.50, 1.55, -110.0)),
    ("CAM_FRONT_RIGHT", 70.0, ( 0.50,  0.50, 1.60,   55.0)),
    ("CAM_BACK_RIGHT",  70.0, (-0.70,  0.50, 1.55,  110.0)),
]

# 800x450 keeps the nuScenes 16:9 aspect (so the stack's resize to the
# model's 640x384 applies the same stretch as training) at 4x less DDS
# bandwidth than 1600x900 -- six raw surround streams at full res saturate
# UDP transport and starve the whole pipeline.
IMAGE_WIDTH = 800
IMAGE_HEIGHT = 450
TOPIC_TEMPLATE = "/sensing/camera/{name}/image_raw"
EGO_ROLE_NAMES = ("hero", "ego")


def find_ego(world, wait_seconds):
    """Wait for a vehicle with role_name hero/ego to appear."""
    deadline = time.monotonic() + wait_seconds
    while True:
        for vehicle in world.get_actors().filter("vehicle.*"):
            if vehicle.attributes.get("role_name") in EGO_ROLE_NAMES:
                return vehicle
        if time.monotonic() >= deadline:
            return None
        time.sleep(1.0)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--host", default="127.0.0.1", help="CARLA host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port (default: 2000)")
    parser.add_argument("--wait", type=float, default=120.0,
                        help="seconds to wait for the ego vehicle (default: 120)")
    args = parser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(30.0)
    world = client.get_world()

    blueprint_library = world.get_blueprint_library()
    camera_bp = blueprint_library.find("sensor.camera.rgb")

    # --- migration guard -------------------------------------------------
    if not camera_bp.has_attribute("ros_topic_name"):
        sys.exit(
            "spawn_vad_rig.py: blueprint 'sensor.camera.rgb' has no "
            "'ros_topic_name' attribute. The per-sensor ROS topic override "
            "lands with the concurrent Autoware-layer migration -- this "
            "server build predates it (migration branch not built yet). "
            "Cannot publish on /sensing/camera/CAM_*/image_raw; aborting."
        )

    ego = find_ego(world, args.wait)
    if ego is None:
        sys.exit(
            "spawn_vad_rig.py: no vehicle with role_name in %r appeared "
            "within %.0f s. Start PythonAPI/examples/autoware_demo.py (it "
            "spawns the ego and is the single ticking client) first."
            % (list(EGO_ROLE_NAMES), args.wait)
        )
    print("spawn_vad_rig.py: found ego id=%d role_name=%s type=%s"
          % (ego.id, ego.attributes.get("role_name"), ego.type_id))

    sensors = []

    def destroy_all():
        while sensors:
            sensor = sensors.pop()
            try:
                sensor.stop()
            except RuntimeError:
                pass
            try:
                sensor.destroy()
            except RuntimeError:
                pass

    try:
        for name, fov, (x, y, z, yaw) in CAMERAS:
            bp = blueprint_library.find("sensor.camera.rgb")
            bp.set_attribute("image_size_x", str(IMAGE_WIDTH))
            bp.set_attribute("image_size_y", str(IMAGE_HEIGHT))
            bp.set_attribute("fov", str(fov))
            bp.set_attribute("role_name", name)
            bp.set_attribute("ros_name", name)
            bp.set_attribute("ros_topic_name", TOPIC_TEMPLATE.format(name=name))
            transform = carla.Transform(
                carla.Location(x=x, y=y, z=z),
                carla.Rotation(pitch=0.0, yaw=yaw, roll=0.0),
            )
            sensor = world.spawn_actor(bp, transform, attach_to=ego)
            sensor.enable_for_ros()
            sensors.append(sensor)
            print("spawn_vad_rig.py: attached %-16s fov=%5.1f -> %s"
                  % (name, fov, TOPIC_TEMPLATE.format(name=name)))
    except Exception:
        destroy_all()
        raise

    print("spawn_vad_rig.py: rig up (6 cameras, ROS-enabled). "
          "Not ticking; waiting for SIGTERM/SIGINT.")

    stop = {"flag": False}

    def handle_signal(signum, _frame):
        print("spawn_vad_rig.py: received signal %d, shutting down" % signum)
        stop["flag"] = True

    signal.signal(signal.SIGTERM, handle_signal)
    signal.signal(signal.SIGINT, handle_signal)

    try:
        while not stop["flag"]:
            time.sleep(0.5)   # passive wait only -- never tick
    finally:
        destroy_all()
        print("spawn_vad_rig.py: sensors destroyed, bye")


if __name__ == "__main__":
    main()
