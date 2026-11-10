#!/usr/bin/env python3
"""Spawn and attach sensors to a CARLA actor, and destroy them.

Commands:
    types                                  list sensor blueprints
    spawn --type camera.rgb [--attach-to hero | --parent-id N]
          [--x 1.5 --y 0 --z 2.4 --pitch 0 --yaw 0 --roll 0]
          [--attachment Rigid|SpringArm] [--attr image_size_x=800 --attr fov=90]
                                           spawn one sensor; prints its id
    destroy [--filter sensor.*]            remove sensors

`--type` accepts a short name (camera.rgb, lidar.ray_cast, other.gnss) or the full
`sensor.*` id. The transform is RELATIVE to the parent when attached (a dashcam
default of x=1.5, z=2.4). Attach to the ego with `--attach-to hero`, or any actor
with `--parent-id`. `SpringArm` gives a smooth (spring-damped) mount for chase
cams; `Rigid` is fixed.

Repeat `--attr key=value` for blueprint attributes: cameras take image_size_x,
image_size_y, fov, sensor_tick; lidar takes range, points_per_second, channels,
rotation_frequency; etc.

The spawned sensor persists as an actor (its id is printed) — feed that id to the
read-sensor skill to save or view its data. Connection from env.sh.

ROS 2 (only meaningful on a server started with --ros2, see run-carla-server):
    --ros                enable_for_ros() after spawn. REQUIRED to publish: a
                         sensor with no listening client is not ticked at all,
                         so without this it produces neither RPC data nor topics.
    --ros-name NAME      topic segment for this sensor (default: actor<id>)
    --ros-topic-name T   exact topic, replacing the generated name (UE5 line)
    --ros-frame-id ID    TF frame id (default: the ros name; UE4 line only)
    --no-ros-tf          do not publish this sensor's transform on rt/tf (UE4
                         line only; on UE5 rt/tf is per world, set_publish_tf)
Each is applied only if the blueprint has the attribute; otherwise it is
reported and ignored rather than failing the spawn.
`spawn` prints the topics the sensor is expected to publish.
"""
from __future__ import annotations

import argparse
import os

import carla  # provided by the active interpreter; check_env.sh verifies this

# --- ROS 2 topic derivation --------------------------------------------------
# Mirrors LibCarla/source/carla/ros2: ROS2::GetActorBaseTopicName builds
# "rt/carla/<ros_name>", prefixed by the parent's base name when the actor was
# attached ("rt/carla/<parent>/<sensor>"), and each publisher appends its own
# suffix. Sensors NOT in ROS2::GetOrCreateSensor's switch have no publisher.
_CAMERA_TOPICS = (("/image", "sensor_msgs/Image"),
                  ("/camera_info", "sensor_msgs/CameraInfo"))
_CLOUD_TOPICS = (("/point_cloud", "sensor_msgs/PointCloud2"),)
# Keyed by the part of the blueprint id after "sensor.".
_ROS_UNSUPPORTED = ("other.lane_invasion", "other.obstacle", "other.rss")


def _fresh(world):
    """A world handle that already holds a snapshot.

    In synchronous mode a freshly connected client has not seen a frame yet, so
    `get_actors()` comes back EMPTY and every --id/--filter lookup reports "no
    matching actor" while the scene is full of them. Observed against a live
    world holding 48 vehicles. One frame of waiting is the whole fix, and it
    must be a wait rather than a tick: another client owns that clock.
    """
    if world.get_settings().synchronous_mode:
        world.wait_for_tick()
    return world


def ros_topics_for(type_id: str, base: str):
    """[(topic, msg_type)] the server publishes for this sensor, or []."""
    short = type_id[len("sensor."):] if type_id.startswith("sensor.") else type_id
    if short in _ROS_UNSUPPORTED:
        return []
    if short == "camera.dvs":                      # camera + point cloud
        return [(base + s, m) for s, m in _CAMERA_TOPICS + _CLOUD_TOPICS]
    if short.startswith("camera."):
        return [(base + s, m) for s, m in _CAMERA_TOPICS]
    if short.startswith("lidar.") or short == "other.radar":
        return [(base + s, m) for s, m in _CLOUD_TOPICS]
    if short == "other.imu":
        return [(base, "sensor_msgs/Imu")]
    if short == "other.gnss":
        return [(base, "sensor_msgs/NavSatFix")]
    if short == "other.collision":
        return [(base, "carla_msgs/CarlaCollisionEvent")]
    return []


def _client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return c


def _full_type(t: str) -> str:
    return t if t.startswith("sensor.") else f"sensor.{t}"


def cmd_types(_):
    bl = _fresh(_client().get_world()).get_blueprint_library().filter("sensor.*")
    print("sensor blueprints:")
    for b in bl:
        print(f"  {b.id}")


def cmd_spawn(args):
    world = _fresh(_client().get_world())
    bp = world.get_blueprint_library().find(_full_type(args.type))
    for kv in args.attr or []:
        k, _, v = kv.partition("=")
        if not bp.has_attribute(k):
            raise SystemExit(f"{bp.id} has no attribute {k!r}")
        bp.set_attribute(k, v)

    # ROS 2 naming. These attributes are read once, at registration time, so
    # they must be set BEFORE spawning — there is no way to rename a sensor's
    # topic afterwards. Which ones exist depends on the server line
    # (ActorBlueprintFunctionLibrary's FillIdAndTags): ros_name everywhere;
    # ros_frame_id and ros_publish_tf only on UE4; ros_topic_name only on UE5.
    # Ask the blueprint rather than assume, and say which flag had no effect.
    args.applied = set()
    for flag, attr, value in (("--ros-name", "ros_name", args.ros_name),
                              ("--ros-topic-name", "ros_topic_name", args.ros_topic_name),
                              ("--ros-frame-id", "ros_frame_id", args.ros_frame_id),
                              ("--no-ros-tf", "ros_publish_tf",
                               "false" if args.no_ros_tf else None)):
        if not value:
            continue
        if bp.has_attribute(attr):
            bp.set_attribute(attr, value)
            args.applied.add(attr)
        else:
            hint = (" — on this server the frame id is the ros name"
                    if attr == "ros_frame_id" else
                    " — rt/tf is switched per world here: world.set_publish_tf(False)"
                    if attr == "ros_publish_tf" else "")
            print(f"warn: {bp.id} has no {attr!r} attribute on this server; "
                  f"{flag} ignored{hint}")

    parent = None
    if args.parent_id is not None:
        parent = world.get_actors().find(args.parent_id)
        if parent is None:
            raise SystemExit(f"no actor id {args.parent_id}")
    elif args.attach_to:
        matches = [a for a in world.get_actors().filter("vehicle.*")
                   if a.attributes.get("role_name", "") == args.attach_to]
        if not matches:
            raise SystemExit(f"no vehicle with role_name={args.attach_to!r} "
                             "(spawn an ego with spawn-vehicles, or use --parent-id)")
        parent = matches[0]

    tf = carla.Transform(carla.Location(args.x, args.y, args.z),
                         carla.Rotation(pitch=args.pitch, yaw=args.yaw, roll=args.roll))
    attach = getattr(carla.AttachmentType, args.attachment)
    if parent is not None:
        sensor = world.spawn_actor(bp, tf, attach_to=parent, attachment_type=attach)
        where = f"attached to id={parent.id} ({parent.type_id}) at rel ({args.x},{args.y},{args.z})"
    else:
        sensor = world.spawn_actor(bp, tf)   # world-fixed sensor
        where = f"world-fixed at ({args.x},{args.y},{args.z})"
    print(f"spawned {sensor.type_id} id={sensor.id} {where}")
    print(f"  view/save it with: read-sensor --id {sensor.id}")

    if args.ros:
        # Without this the stream has no listener, ASensor::Tick returns early
        # and NOTHING is produced — not even for ROS. It is also what makes a
        # sensor publish with no Python client attached.
        sensor.enable_for_ros()
    _report_ros(sensor, parent, args)


def _report_ros(sensor, parent, args):
    """Print the topics this sensor is expected to publish, and the caveats."""
    applied = getattr(args, "applied", set())
    ros_name = (args.ros_name if "ros_name" in applied else None) or f"actor{sensor.id}"
    base = f"rt/carla/{ros_name}"
    if parent is not None:
        # On attach, the server registers the parent chain, so the sensor's base
        # topic is nested under its immediate parent's ros name.
        parent_ros = parent.attributes.get("ros_name", "") or f"actor{parent.id}"
        base = f"rt/carla/{parent_ros}/{ros_name}"
    topics = ros_topics_for(sensor.type_id, base)
    if "ros_topic_name" in applied:
        # The override replaces the whole generated base; lidar and radar then
        # publish on it with no /point_cloud suffix, cameras keep theirs.
        t = args.ros_topic_name
        base = "rt" + ("" if t.startswith("/") else "/") + t
        topics = ros_topics_for(sensor.type_id, base)
        if sensor.type_id.startswith("sensor.lidar.") or sensor.type_id == "sensor.other.radar":
            topics = [(base, m) for _, m in topics]
    frame_id = args.ros_frame_id if "ros_frame_id" in applied else ros_name
    tf_off = "ros_publish_tf" in applied
    print(f"  ros: name={ros_name} frame_id={frame_id} "
          f"tf={'off' if tf_off else 'on (rt/tf, parented to ' + ('map' if parent is None else 'the parent frame') + ')'} "
          f"enabled_for_ros={'yes' if args.ros else 'NO'}")
    if not topics:
        print(f"  ros: {sensor.type_id} has NO native publisher — no topic is created")
    for t, m in topics:
        print(f"  ros: {t}  [{m}]")
    if not args.ros:
        print("  ros: NOT enabled for ROS — pass --ros, or the sensor never ticks "
              "and publishes nothing")


def cmd_ros(args):
    """Turn native ROS publishing on/off for sensors that already exist.

    enable_for_ros() marks the sensor's stream as listened-to server-side, which
    is what lets ASensor::Tick run without a Python client — the difference
    between a sensor that publishes and one that is silent. Idempotent.
    """
    world = _fresh(_client().get_world())
    if args.id is not None:
        actor = world.get_actors().find(args.id)
        if actor is None:
            raise SystemExit(f"no actor id {args.id}")
        targets = [actor]
    else:
        targets = list(world.get_actors().filter(args.filter))
    if not targets:
        raise SystemExit(f"no sensors match {args.filter!r}")
    for s in targets:
        if args.disable:
            s.disable_for_ros()
        else:
            s.enable_for_ros()
        state = "yes" if s.is_enabled_for_ros() else "no"
        print(f"id={s.id} ({s.type_id}) enabled_for_ros={state}")


def _stalled(world):
    """True if the world is synchronous with nothing advancing it.

    In that state `get_actors()` hands back a stale snapshot, so a destroy can
    report "0 actors" -- or a partial count -- while the world is still full.
    Observed: 141 actors survived three `destroy` calls that all claimed
    success, because a holder had been interrupted and left the clock claimed.
    """
    if not world.get_settings().synchronous_mode:
        return False
    before = world.get_snapshot().frame
    try:
        world.wait_for_tick(2.0)
    except RuntimeError:
        return True
    return world.get_snapshot().frame == before


def _verified_destroy(client, actors, what):
    """Destroy, then re-read the world and report what is actually gone."""
    world = client.get_world()
    if _stalled(world):
        print("WARNING the world is synchronous and nothing is ticking it, so "
              "the actor list cannot be trusted. Restore asynchronous mode "
              "(set-world-settings) or start the client that owns the clock, "
              "then destroy again.")
    wanted = {a.id for a in actors}
    if not wanted:
        print(f"no {what} to destroy")
        return 0
    for a in actors:
        if a.type_id.startswith("controller."):
            try:
                a.stop()
            except RuntimeError:
                pass
    try:
        client.apply_batch_sync(
            [carla.command.DestroyActor(i) for i in wanted], False)
    except RuntimeError as error:
        print(f"  batch destroy failed ({error}); falling back one at a time")
    try:
        world.wait_for_tick(5.0)
    except RuntimeError:
        pass
    # Only retry what the batch actually left behind. Destroying every actor
    # again unconditionally makes the client print "ERROR: failed to destroy
    # actor N" for each one the batch already removed, which reads like a
    # failure when nothing is wrong.
    left = {a.id for a in world.get_actors()} & wanted
    if left:
        for a in actors:
            if a.id in left:
                try:
                    a.destroy()
                except RuntimeError:
                    pass
        try:
            world.wait_for_tick(5.0)
        except RuntimeError:
            pass
        left = {a.id for a in world.get_actors()} & wanted
    gone = len(wanted) - len(left)
    print(f"destroyed {gone} {what}"
          + (f" -- {len(left)} STILL PRESENT (ids {sorted(left)[:8]})" if left else ""))
    return gone


def cmd_destroy(args):
    client = _client()
    world = _fresh(client.get_world())
    sensors = list(world.get_actors().filter(args.filter))
    for sensor in sensors:
        try:
            # is_listening is a property on some client builds and a method on
            # others; a bound method is always truthy, so call it if it is one.
            v = sensor.is_listening
            if (v() if callable(v) else v):
                sensor.stop()
        except RuntimeError:
            pass
    _verified_destroy(client, sensors, f"sensors matching {args.filter!r}")


def main() -> None:
    p = argparse.ArgumentParser(description="Spawn/attach/destroy CARLA sensors.")
    sub = p.add_subparsers(dest="cmd", required=True)

    sub.add_parser("types", help="list sensor blueprints").set_defaults(func=cmd_types)

    ps = sub.add_parser("spawn", help="spawn a sensor")
    ps.add_argument("--type", required=True, help="e.g. camera.rgb, lidar.ray_cast, other.imu")
    ps.add_argument("--attach-to", help="parent vehicle role_name (e.g. hero)")
    ps.add_argument("--parent-id", type=int, help="parent actor id (alternative to --attach-to)")
    ps.add_argument("--x", type=float, default=1.5); ps.add_argument("--y", type=float, default=0.0)
    ps.add_argument("--z", type=float, default=2.4)
    ps.add_argument("--pitch", type=float, default=0.0); ps.add_argument("--yaw", type=float, default=0.0)
    ps.add_argument("--roll", type=float, default=0.0)
    ps.add_argument("--attachment", choices=("Rigid", "SpringArm", "SpringArmGhost"), default="Rigid")
    ps.add_argument("--attr", action="append", help="blueprint attribute key=value (repeatable)")
    ps.add_argument("--hold", action="store_true",
                    help="stay resident and destroy the sensor on exit, so the "
                         "world is left as it was found")
    ps.add_argument("--ros", action="store_true",
                    help="enable_for_ros() after spawn — required for the sensor to publish")
    ps.add_argument("--ros-name", help="ROS topic segment (default actor<id>)")
    ps.add_argument("--ros-topic-name",
                    help="exact topic, replacing the generated rt/carla/... name "
                         "(ros_topic_name attribute; UE5 line)")
    ps.add_argument("--ros-frame-id",
                    help="TF frame id (default: the ros name). UE4 servers only; "
                         "warned and ignored where the attribute does not exist")
    ps.add_argument("--no-ros-tf", action="store_true",
                    help="do not publish this sensor's transform on rt/tf. UE4 "
                         "servers only; on UE5 use world.set_publish_tf(False)")
    ps.set_defaults(func=cmd_spawn)

    pr = sub.add_parser("ros", help="enable/disable ROS publishing on existing sensors")
    pr.add_argument("--id", type=int, help="one sensor (default: every sensor matching --filter)")
    pr.add_argument("--filter", default="sensor.*")
    pr.add_argument("--disable", action="store_true", help="disable instead of enable")
    pr.set_defaults(func=cmd_ros)

    pd = sub.add_parser("destroy", help="destroy sensors")
    pd.add_argument("--filter", default="sensor.*")
    pd.set_defaults(func=cmd_destroy)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
