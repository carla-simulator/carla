#!/usr/bin/env python3
"""Actor/level bounding boxes: list 3D, draw in the world, or project into a camera.

Commands:
    list   [--filter vehicle.*] [--limit N]        3D boxes of matching actors
    draw   [--filter vehicle.*] [--seconds 30]     draw 3D boxes (tracks moving actors)
    project --camera <id|type> [--filter vehicle.*] [--out boxes.png]
            [--max-dist 100]                        2D boxes projected into one frame

`draw` always tracks: it re-stamps the boxes each frame for --seconds (default 30)
so they follow moving actors — debug shapes don't move on their own.

`project` captures one frame from the camera, computes each matching actor's 3D
bounding box, projects its 8 world vertices into image space using the camera's
intrinsics (from fov + resolution) and pose, draws the 2D box, and writes an
annotated PNG plus a JSON list of boxes — the dataset-generation use case
(client_bounding_boxes.py). Only actors in front of and within --max-dist of the
camera are drawn.

`draw` overlays 3D boxes via world.debug (needs a rendered server). `list` just
reports box geometry. Connection from env.sh: CARLA_HOST/PORT/TIMEOUT.
"""
from __future__ import annotations

import argparse
import json
import os
import threading

import carla  # provided by the active interpreter; check_env.sh verifies this
import numpy as np


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


def _targets(world, filt):
    return list(world.get_actors().filter(filt or "vehicle.*"))


def cmd_list(args):
    world = _world()
    actors = _targets(world, args.filter)
    if args.limit:
        actors = actors[:args.limit]
    print(f"{len(actors)} actor bounding box(es):")
    for a in actors:
        bb = a.bounding_box
        print(f"  id={a.id:6d} {a.type_id:28s} extent=({bb.extent.x:.2f},{bb.extent.y:.2f},"
              f"{bb.extent.z:.2f})  (LxWxH = {bb.extent.x*2:.1f}x{bb.extent.y*2:.1f}x{bb.extent.z*2:.1f} m)")


def _draw_boxes(world, actors, life):
    dbg = world.debug
    for a in actors:
        bb = a.bounding_box
        # draw_box takes a WORLD-space box: place it at the actor's current pose.
        world_bb = carla.BoundingBox(a.get_transform().transform(bb.location), bb.extent)
        world_bb.rotation = a.get_transform().rotation
        dbg.draw_box(world_bb, world_bb.rotation, 0.1, carla.Color(0, 255, 0), life)


def cmd_draw(args):
    # Boxes always TRACK: debug shapes are stamped at a fixed pose and don't move
    # on their own, so we re-stamp every ~0.1 s with a short life. The boxes ride
    # moving actors and hold for --seconds (a static actor just gets a steady box).
    # Re-query each round so newly spawned / destroyed actors are handled.
    world = _world()
    print(f"drawing {args.filter or 'vehicle.*'} boxes for {args.seconds}s "
          "(tracking motion; view on a rendered server)...")
    n = 0
    for snapshot in _for_seconds(world, args.seconds):
        # Once per frame, with a lifetime just over one frame: the boxes then
        # track the actors instead of smearing or flickering, whatever rate the
        # server is running at.
        actors = _targets(world, args.filter)
        _draw_boxes(world, actors, snapshot.timestamp.delta_seconds * 2.0)
        n = len(actors)
    print(f"done — tracked {n} boxes for {args.seconds}s")


# --- waiting on the simulation, not on the wall clock ----------------------
# These commands are observers: they never own the world's clock, so they
# advance by waiting for whoever does (the server itself in async, the holding
# client in sync). A fixed time.sleep() is wrong for both -- it measures wall
# time while the thing being waited for is measured in frames, so it overruns
# a fast server and cuts a slow one short, and in a synchronous world with a
# stalled ticker it "succeeds" having observed nothing at all.

def _for_seconds(world, seconds):
    """Yield one snapshot per frame until `seconds` of SIMULATED time pass.

    The budget is read off the world clock (`elapsed_seconds`), not summed from
    per-frame deltas. Summing deltas counts *observed* frames, so a consumer
    slower than the server -- a client encoding PNGs at 4 Hz against a 30 Hz
    server, say -- accumulates 0.03 s per poll and takes minutes to "reach"
    20 seconds. Reading the clock is correct whether or not frames are missed.
    """
    start = None
    while True:
        snapshot = world.wait_for_tick()
        now = snapshot.timestamp.elapsed_seconds
        if start is None:
            start = now
        elif now - start >= seconds:
            return
        yield snapshot


def _until(world, predicate, seconds):
    """Advance frames until predicate() is true; returns whether it became true.

    The budget is simulated seconds, so a stalled clock ends this by timing out
    in wait_for_tick() rather than by silently burning the budget.
    """
    if predicate():
        return True
    for _ in _for_seconds(world, seconds):
        if predicate():
            return True
    return False


def _build_K(w, h, fov):
    f = w / (2.0 * np.tan(fov * np.pi / 360.0))
    return np.array([[f, 0, w / 2.0], [0, f, h / 2.0], [0, 0, 1.0]])


def cmd_project(args):
    world = _world()
    sensors = world.get_actors().filter("sensor.camera.rgb")
    cam = None
    if args.camera.isdigit():
        cam = world.get_actors().find(int(args.camera))
    else:
        m = [s for s in sensors if s.type_id == (args.camera if args.camera.startswith("sensor.")
                                                 else f"sensor.{args.camera}")]
        cam = m[0] if m else None
    if cam is None or not cam.type_id.startswith("sensor.camera"):
        raise SystemExit("need an existing RGB camera (create-sensor); pass --camera <id|type>")

    grabbed = {"img": None}
    ev = threading.Event()

    def cb(image):
        grabbed["img"] = image
        ev.set()

    cam.listen(cb)
    ok = ev.wait(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    cam.stop()
    if not ok:
        raise SystemExit("no frame from the camera (is the world ticking?)")
    image = grabbed["img"]

    w, h, fov = image.width, image.height, float(cam.attributes["fov"])
    K = _build_K(w, h, fov)
    world_2_cam = np.array(cam.get_transform().get_inverse_matrix())
    cam_loc = cam.get_transform().location

    bgr = np.frombuffer(image.raw_data, np.uint8).reshape((h, w, 4))[:, :, :3].copy()
    parent_id = cam.parent.id if cam.parent is not None else None
    boxes = []
    for a in _targets(world, args.filter):
        # Skip the camera itself and the vehicle it is mounted on (its box would
        # wrap the whole frame), and anything beyond --max-dist.
        if a.id == cam.id or a.id == parent_id or a.get_transform().location.distance(cam_loc) > args.max_dist:
            continue
        verts = a.bounding_box.get_world_vertices(a.get_transform())
        pts = []
        for v in verts:
            p = world_2_cam @ np.array([v.x, v.y, v.z, 1.0])
            # UE (x fwd, y right, z up) -> camera (x right, y down, z fwd)
            cam_pt = np.array([p[1], -p[2], p[0]])
            if cam_pt[2] <= 0:      # behind the camera
                continue
            img_pt = K @ cam_pt
            pts.append(img_pt[:2] / img_pt[2])
        if len(pts) < 4:
            continue
        pts = np.array(pts)
        x0, y0 = pts[:, 0].min(), pts[:, 1].min()
        x1, y1 = pts[:, 0].max(), pts[:, 1].max()
        if x1 < 0 or y1 < 0 or x0 > w or y0 > h:   # fully off-frame
            continue
        boxes.append({"id": a.id, "type": a.type_id,
                      "bbox": [int(x0), int(y0), int(x1), int(y1)]})

    out_png = args.out or "boxes.png"
    try:
        import cv2
        for b in boxes:
            x0, y0, x1, y1 = b["bbox"]
            cv2.rectangle(bgr, (x0, y0), (x1, y1), (0, 255, 0), 2)
        cv2.imwrite(out_png, bgr)
    except Exception:
        image.save_to_disk(out_png)   # fall back to the raw frame if cv2 fails
    with open(os.path.splitext(out_png)[0] + ".json", "w") as f:
        json.dump(boxes, f, indent=2)
    print(f"projected {len(boxes)} boxes from camera id={cam.id} -> {out_png} "
          f"(+ {os.path.splitext(out_png)[0]}.json)")


def main() -> None:
    p = argparse.ArgumentParser(description="Actor/level bounding boxes.")
    sub = p.add_subparsers(dest="cmd", required=True)

    pl = sub.add_parser("list", help="3D box geometry of actors")
    pl.add_argument("--filter"); pl.add_argument("--limit", type=int)
    pl.set_defaults(func=cmd_list)

    pdw = sub.add_parser("draw", help="draw 3D boxes in the world (tracks moving actors)")
    pdw.add_argument("--filter")
    pdw.add_argument("--seconds", type=float, default=30.0, help="how long boxes stay drawn (default 30)")
    pdw.set_defaults(func=cmd_draw)

    pp = sub.add_parser("project", help="project boxes into a camera frame")
    pp.add_argument("--camera", required=True, help="camera actor id or type")
    pp.add_argument("--filter"); pp.add_argument("--out")
    pp.add_argument("--max-dist", type=float, default=100.0)
    pp.set_defaults(func=cmd_project)

    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
