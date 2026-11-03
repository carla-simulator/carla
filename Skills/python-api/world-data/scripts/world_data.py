#!/usr/bin/env python3
"""Query the live CARLA world: actors, level bounding boxes, raycast, snapshot.

This is the RESOLVER skill — use it to turn an ambiguous description ("a prius",
"the nearest walker", "the 3rd one") into a concrete actor id that the action
skills (telemetry, bounding-boxes, control-vehicle, control-spectator) take.

Commands:
    actors [--filter vehicle.*] [--role hero] [--color 255,0,0]
           [--near X,Y,Z | --near-id N] [--nearest] [--full] [--limit N]
                                           find actors by attribute / nearest
    snapshot                               frame/timestamp/actor count
    level-bbox --label Buildings [--limit]  static level bounding boxes by label
    raycast --from X,Y,Z --to X,Y,Z        semantic points a ray crosses
    ground  --at X,Y,Z [--search 1000]     drop a point to the ground below it
    ros-topics [--filter] [--all]          the ROS 2 topic tree the server is
                                           expected to publish, per actor, with
                                           the reason for each silent one

Identify by STABLE attributes (id, type, role, color) or the spatial predicate
--nearest (closest to --near/--near-id). There is NO rank/order among peer actors,
so no "Nth". E.g. the prius nearest the ego:
`actors --filter '*prius*' --near-id <ego> --nearest` prints exactly one id;
"the red prius": `actors --filter '*prius*' --color 255,0,0`.

Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import math
import os

import carla  # provided by the active interpreter; check_env.sh verifies this


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


def _world():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                     int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return _fresh(c.get_world())


def _is_ue5_line(ver: str) -> bool:
    """True for the UE5 CARLA line: 0.10.x before the 1.0 release, 1.x after it.

    The line was renamed, not changed, so anything gated on it must accept both
    labels — a check for "0.10" alone goes silent against a 1.0 server.
    """
    if ver.startswith("0.10"):
        return True
    major = ver.split(".")[0]
    return major.isdigit() and int(major) >= 1


def _server_version() -> str:
    """Server version string, e.g. "0.9.16", "0.10.0" or "1.0.0". "" if unreachable."""
    try:
        c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                         int(os.environ.get("CARLA_PORT", 2000)))
        c.set_timeout(10.0)
        return c.get_server_version()
    except Exception:
        return ""


def _speed(a):  # m/s magnitude
    v = a.get_velocity()
    return math.sqrt(v.x * v.x + v.y * v.y + v.z * v.z)


def cmd_actors(args):
    world = _world()
    actors = world.get_actors()
    rows = list(actors.filter(args.filter)) if args.filter else list(actors)
    if args.role:
        rows = [a for a in rows if a.attributes.get("role_name", "") == args.role]
    if args.color:
        rows = [a for a in rows if a.attributes.get("color", "") == args.color]
    if not args.all:
        rows = [a for a in rows if not a.type_id.startswith(("traffic.", "spectator", "sensor.other.v2x"))]

    # Optional spatial reference — a distance is shown for context and lets
    # --nearest pick the single closest. This is a predicate ("the closest one"),
    # NOT an ordering to index into: there is no meaningful rank among peers.
    ref = None
    if args.near:
        x, y, z = (float(v) for v in args.near.split(","))
        ref = carla.Location(x, y, z)
    elif args.near_id is not None:
        r = actors.find(args.near_id)
        if r is None:
            raise SystemExit(f"no actor id {args.near_id}")
        ref = r.get_transform().location

    def dist(a):
        return a.get_transform().location.distance(ref)

    if args.nearest:
        if ref is None:
            raise SystemExit("--nearest needs --near X,Y,Z or --near-id N")
        rows = [min(rows, key=dist)] if rows else []

    shown = rows[:args.limit] if args.limit else rows
    cap = f" (showing {len(shown)} of {len(rows)})" if len(shown) < len(rows) else ""
    print(f"{len(rows)} actor(s) match{cap}:")
    for a in shown:
        loc = a.get_transform().location
        role = a.attributes.get("role_name", "")
        color = a.attributes.get("color", "")
        d = f" dist={dist(a):.0f}m" if ref else ""
        cstr = f" color={color}" if color else ""
        print(f"  id={a.id:6d}  {a.type_id:34s} role={role or '-':8s}{cstr} "
              f"({loc.x:.0f},{loc.y:.0f},{loc.z:.0f}) speed={_speed(a)*3.6:.0f}km/h{d}")
        if args.full:
            bb = a.bounding_box
            attrs = ", ".join(f"{k}={v}" for k, v in sorted(a.attributes.items()))
            print(f"        size(LxWxH)={bb.extent.x*2:.1f}x{bb.extent.y*2:.1f}x{bb.extent.z*2:.1f}m"
                  f"  yaw={a.get_transform().rotation.yaw:.0f}")
            print(f"        attrs: {attrs}")
    print("  → identify by id (stable) or a distinguishing attr (color/role/type); "
          "positions move, so re-query for a fresh distance.")


# --- ROS 2 topic tree -------------------------------------------------------
# Derived exactly as the server does (LibCarla/source/carla/ros2 +
# ActorDispatcher::RegisterActor): base name "rt/carla/<ros_name>", nested under
# the parent's base name when attached, unnamed actors become "actor<id>". Only
# sensors and the hero VEHICLE are registered; sensors additionally have to be
# enabled for ROS to tick at all.
_CAMERA_TOPICS = (("/image", "sensor_msgs/Image"),
                  ("/camera_info", "sensor_msgs/CameraInfo"))
_CLOUD_TOPICS = (("/point_cloud", "sensor_msgs/PointCloud2"),)
_ROS_UNSUPPORTED = ("other.lane_invasion", "other.obstacle", "other.rss")


def _sensor_topics(type_id, base):
    short = type_id[len("sensor."):] if type_id.startswith("sensor.") else type_id
    if short in _ROS_UNSUPPORTED:
        return []
    if short == "camera.dvs":
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


def _ros_name(a):
    return a.attributes.get("ros_name", "") or f"actor{a.id}"


def cmd_ros_topics(args):
    """Map live actors to the ROS 2 topics they should be publishing.

    Works with no ROS 2 installed — it answers "what SHOULD be on the wire and
    why isn't it" from the RPC side, which is the half a `ros2 topic list` can't
    explain. Compare its output with `ros2 topic list` to locate the gap.
    """
    world = _world()
    actors = world.get_actors()
    rows = list(actors.filter(args.filter)) if args.filter else list(actors)

    print("world topics (exist whenever the server runs with --ros2):")
    print("  rt/clock       [rosgraph_msgs/Clock]  every tick")
    # rt/carla/map comes from CarlaMapPublisher, which exists in 0.9.x only: the
    # publisher was dropped on the UE5 line, so advertising the topic there sends
    # people hunting for a topic that is never created.
    try:
        server = _server_version()
    except Exception:
        server = ""
    if _is_ue5_line(server):
        print(f"  rt/carla/map   NOT PUBLISHED on the UE5 line (server {server}) —")
        print("                 no CarlaMapPublisher in this build;")
        print("                 read the OpenDrive with map.to_opendrive() over RPC instead")
    else:
        print("  rt/carla/map   [std_msgs/String]      OpenDRIVE, LATCHED, re-sent on map load")
    print("  rt/tf          [tf2_msgs/TFMessage]   sensor->parent transforms ONLY;")
    print("                 absent until a sensor publishes (a vehicle alone emits no transform)")

    sensors = [a for a in rows if a.type_id.startswith("sensor.")]
    heroes = [a for a in rows if a.type_id.startswith("vehicle.")
              and a.attributes.get("role_name", "") == "hero"]
    others = [a for a in rows if a.type_id.startswith("vehicle.") and a not in heroes]

    print(f"\nhero vehicle(s): {len(heroes)}")
    for v in heroes:
        base = f"rt/carla/{_ros_name(v)}"
        tf = "" if v.attributes.get("ros_publish_tf", "true").lower() != "false" else "  (tf OFF)"
        print(f"  id={v.id} {v.type_id} ros_name={_ros_name(v)}{tf}")
        print(f"    <- {base}/vehicle_control_cmd     [carla_msgs/CarlaEgoVehicleControl]")
        print(f"    <- {base}/ackermann_control_cmd   [ackermann_msgs/AckermannDriveStamped]")
    if others and args.all:
        print(f"  {len(others)} non-hero vehicle(s): not registered with ROS 2 "
              "(the server registers vehicles only when role_name == 'hero')")

    print(f"\nsensors: {len(sensors)}")
    for s in sensors:
        parent = s.parent
        base = f"rt/carla/{_ros_name(s)}"
        if parent is not None:
            base = f"rt/carla/{_ros_name(parent)}/{_ros_name(s)}"
        try:
            enabled = s.is_enabled_for_ros()
        except AttributeError:
            enabled = None
        topics = _sensor_topics(s.type_id, base)
        flag = {True: "", False: "  [NOT enabled_for_ros -> SILENT]",
                None: "  [enabled_for_ros unknown]"}[enabled]
        print(f"  id={s.id} {s.type_id} ros_name={_ros_name(s)}{flag}")
        if not topics:
            print(f"    (no native publisher for {s.type_id})")
        for t, m in topics:
            print(f"    -> {t}  [{m}]")

    print("\nnote: names are DDS names; a ROS 2 node sees rt/X as /X. Nothing is")
    print("      published at all unless the server was BUILT and STARTED with ROS 2")
    print("      (build-carla-ue4 ROS2=1, run-carla-server ROS2=1), and subscribers")
    print("      must share its ROS_DOMAIN_ID.")


def cmd_snapshot(args):
    world = _world()
    snap = world.get_snapshot()
    ts = snap.timestamp
    print(f"frame={snap.frame} elapsed={ts.elapsed_seconds:.2f}s delta={ts.delta_seconds:.4f}s "
          f"actors={len(world.get_actors())}")


def cmd_level_bbox(args):
    world = _world()
    label = getattr(carla.CityObjectLabel, args.label, None)
    if label is None:
        raise SystemExit(f"unknown label {args.label!r} (see toggle-env-objects `labels`)")
    bbs = world.get_level_bbs(label)
    print(f"{len(bbs)} level bounding box(es) for {args.label}:")
    for bb in bbs[:args.limit or 15]:
        print(f"  center=({bb.location.x:.0f},{bb.location.y:.0f},{bb.location.z:.0f}) "
              f"extent=({bb.extent.x:.1f},{bb.extent.y:.1f},{bb.extent.z:.1f})")
    if len(bbs) > (args.limit or 15):
        print(f"  ... and {len(bbs) - (args.limit or 15)} more")


def cmd_raycast(args):
    world = _world()
    a = carla.Location(*[float(v) for v in getattr(args, "from").split(",")])
    b = carla.Location(*[float(v) for v in args.to.split(",")])
    pts = world.cast_ray(a, b)
    print(f"{len(pts)} labelled point(s) along the ray:")
    for p in pts[:30]:
        print(f"  ({p.location.x:.1f},{p.location.y:.1f},{p.location.z:.1f})  {p.label}")


def cmd_ground(args):
    world = _world()
    x, y, z = (float(v) for v in args.at.split(","))
    p = world.project_point(carla.Location(x, y, z), carla.Vector3D(0, 0, -1), args.search)
    if p is None:
        print(f"no ground within {args.search} m below ({x},{y},{z})")
        return
    print(f"ground under ({x},{y},{z}): ({p.location.x:.2f},{p.location.y:.2f},"
          f"{p.location.z:.2f})  label={p.label}")


def main() -> None:
    p = argparse.ArgumentParser(description="Query the live CARLA world.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pa = sub.add_parser("actors", help="find actors by attribute / nearest (the resolver)")
    pa.add_argument("--filter", help="type_id pattern, e.g. '*prius*'")
    pa.add_argument("--role", help="match attributes['role_name'] (e.g. hero)")
    pa.add_argument("--color", help="match a vehicle's color attribute, e.g. 255,0,0")
    pa.add_argument("--near", help="reference point X,Y,Z (shows distance)")
    pa.add_argument("--near-id", type=int, help="reference actor id (shows distance)")
    pa.add_argument("--nearest", action="store_true", help="return only the single closest to --near/--near-id")
    pa.add_argument("--limit", type=int, help="display cap only (truncates, not a selection)")
    pa.add_argument("--all", action="store_true", help="include traffic/spectator/etc")
    pa.add_argument("--full", action="store_true",
                    help="show every identifying field (all attrs, size, yaw)")
    pa.set_defaults(func=cmd_actors)

    sub.add_parser("snapshot", help="frame/time/actor count").set_defaults(func=cmd_snapshot)

    pb = sub.add_parser("level-bbox", help="static level bounding boxes by label")
    pb.add_argument("--label", required=True); pb.add_argument("--limit", type=int)
    pb.set_defaults(func=cmd_level_bbox)

    pr = sub.add_parser("raycast", help="semantic points a ray crosses")
    pr.add_argument("--from", required=True); pr.add_argument("--to", required=True)
    pr.set_defaults(func=cmd_raycast)

    pg = sub.add_parser("ground", help="project a point to the ground")
    pg.add_argument("--at", required=True); pg.add_argument("--search", type=float, default=1000.0)
    pg.set_defaults(func=cmd_ground)

    prt = sub.add_parser("ros-topics", help="ROS 2 topic tree the server should publish")
    prt.add_argument("--filter", default="", help="restrict to matching actors (e.g. 'sensor.*')")
    prt.add_argument("--all", action="store_true", help="also count non-hero vehicles (never registered)")
    prt.set_defaults(func=cmd_ros_topics)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
