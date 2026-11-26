#!/usr/bin/env python3
"""Read a walker's skeleton, project it into a camera as 2D keypoints, or pose it by hand.

Commands:
    bones     --id W [--relative]                         every bone's name and transform
    keypoints --id W[,W2...] --camera C [--frames N] [--out DIR]   per-frame 2D keypoints, JSON + annotated PNG
    draw      --id W [--seconds S]                        draw the skeleton in the world with debug lines
    pose      --id W --file POSE.json [--blend B]         set bones (relative to their parent) and show the pose
    freeze    --id W                                      copy the current animation frame into the pose and show it
    release   --id W                                      hide the custom pose, back to the animation
    blend     --id W --value B                            mix pose and animation, 0 = animation, 1 = pose

POSE.json maps a bone name to {"location": [x, y, z], "rotation": [pitch, yaw, roll]}: changes added to the bone's
current transform relative to its parent, or the transform itself with --absolute. Omitted keys keep the current value.
In synchronous mode pass --tick to advance the world yourself, so the bones and the image come from the same frame.
Connection from env.sh (CARLA_HOST, CARLA_PORT, CARLA_TIMEOUT).
"""
from __future__ import annotations

import argparse
import json
import os
import queue

import carla  # provided by the active interpreter; check_env.sh verifies this


def _rgb(s: str) -> carla.Color:
    """--color r,g,b (0-255 each)."""
    try:
        r, g, b = (int(v) for v in s.split(","))
    except ValueError:
        raise argparse.ArgumentTypeError(f"expected r,g,b but got {s!r}")
    return carla.Color(r, g, b)

# Parent of each bone drawn, on the shared crl_ pedestrian skeleton (PythonAPI/examples/draw_skeleton.py).
LINKS = [
    ("crl_hips__C", "crl_spine__C"), ("crl_spine__C", "crl_spine01__C"), ("crl_spine01__C", "crl_neck__C"),
    ("crl_neck__C", "crl_Head__C"), ("crl_Head__C", "crl_eye__L"), ("crl_Head__C", "crl_eye__R"),
    ("crl_spine01__C", "crl_shoulder__L"), ("crl_shoulder__L", "crl_arm__L"), ("crl_arm__L", "crl_foreArm__L"), ("crl_foreArm__L", "crl_hand__L"),
    ("crl_spine01__C", "crl_shoulder__R"), ("crl_shoulder__R", "crl_arm__R"), ("crl_arm__R", "crl_foreArm__R"), ("crl_foreArm__R", "crl_hand__R"),
    ("crl_hips__C", "crl_thigh__L"), ("crl_thigh__L", "crl_leg__L"), ("crl_leg__L", "crl_foot__L"), ("crl_foot__L", "crl_toe__L"),
    ("crl_hips__C", "crl_thigh__R"), ("crl_thigh__R", "crl_leg__R"), ("crl_leg__R", "crl_foot__R"), ("crl_foot__R", "crl_toe__R"),
]


def _world(args=None):
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"), int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    world = c.get_world()
    # In synchronous mode a new client sees no actors until its first frame arrives.
    if world.get_settings().synchronous_mode:
        _step(world, getattr(args, "tick", False))
    return world


def _step(world, tick):
    if tick and world.get_settings().synchronous_mode:
        world.tick()
        return world.get_snapshot()
    return world.wait_for_tick()


def _walker(world, wid):
    a = world.get_actors().find(wid)
    if a is None:
        raise SystemExit(f"no actor id {wid}")
    if not isinstance(a, carla.Walker):
        print(f"[pose] note: {a.type_id} is not a walker; bones are read, posing is walker-only")
    return a


def _skeleton(actor):
    names = actor.get_bone_names()
    if not names:
        raise SystemExit(f"actor {actor.id} reports no bones")
    return dict(zip(names, actor.get_bone_world_transforms()))


def cmd_bones(args):
    a = _walker(_world(args), args.id)
    names = a.get_bone_names()
    ts = a.get_bone_relative_transforms() if args.relative else a.get_bone_world_transforms()
    print(f"{len(names)} bones on {a.type_id} id={a.id} ({'relative to the actor' if args.relative else 'world'})")
    for n, t in zip(names, ts):
        l, r = t.location, t.rotation
        print(f"  {n:24s} ({l.x:8.3f},{l.y:8.3f},{l.z:8.3f})  pitch={r.pitch:7.2f} yaw={r.yaw:7.2f} roll={r.roll:7.2f}")


def _project(point, w2c, K):
    import numpy as np
    p = w2c @ np.array([point.x, point.y, point.z, 1.0])
    cam = np.array([p[1], -p[2], p[0]])  # UE (x fwd, y right, z up) to camera (x right, y down, z fwd)
    if cam[2] <= 0:
        return None
    uv = K @ cam
    return float(uv[0] / uv[2]), float(uv[1] / uv[2])


def cmd_keypoints(args):
    import numpy as np
    world = _world(args)
    cam = world.get_actors().find(args.camera)
    if cam is None or not cam.type_id.startswith("sensor.camera"):
        raise SystemExit("--camera needs the id of an existing camera (create-sensor)")
    walkers = [_walker(world, int(i)) for i in args.id.split(",")]
    w, h, fov = int(cam.attributes["image_size_x"]), int(cam.attributes["image_size_y"]), float(cam.attributes["fov"])
    f = w / (2.0 * np.tan(np.radians(fov) / 2.0))
    K = np.array([[f, 0, w / 2.0], [0, f, h / 2.0], [0, 0, 1.0]])
    os.makedirs(args.out, exist_ok=True)
    q = queue.Queue()
    cam.listen(q.put)
    try:
        for _ in range(args.frames):
            snap = _step(world, args.tick)
            skeletons = {a.id: _skeleton(a) for a in walkers}
            img = q.get(timeout=float(os.environ.get("CARLA_TIMEOUT", "60.0")))
            while img.frame < snap.frame:
                img = q.get(timeout=5.0)
            if img.frame != snap.frame:
                print(f"[pose] warning: image frame {img.frame} != bones frame {snap.frame}; pass --tick in synchronous mode")
            w2c = np.array(img.transform.get_inverse_matrix())
            record = {"frame": img.frame, "camera": cam.id, "width": w, "height": h, "walkers": []}
            for wid, bones in skeletons.items():
                kps = []
                for name, t in bones.items():
                    uv = _project(t.location, w2c, K)
                    kps.append({"bone": name, "world": [t.location.x, t.location.y, t.location.z],
                                "uv": list(uv) if uv else None,
                                "in_image": bool(uv and 0 <= uv[0] < w and 0 <= uv[1] < h)})
                record["walkers"].append({"id": wid, "keypoints": kps})
            stem = os.path.join(args.out, f"{img.frame:08d}")
            with open(stem + ".json", "w") as fh:
                json.dump(record, fh, indent=1)
            _annotate(img, record, stem + ".png")
            seen = sum(k["in_image"] for wk in record["walkers"] for k in wk["keypoints"])
            print(f"[pose] frame {img.frame}: {seen} keypoints in the image -> {stem}.json/.png")
    finally:
        cam.stop()


def _annotate(img, record, path):
    import numpy as np
    try:
        from PIL import Image, ImageDraw
    except ImportError:
        img.save_to_disk(path)
        return
    bgra = np.frombuffer(img.raw_data, np.uint8).reshape((img.height, img.width, 4))
    pic = Image.fromarray(bgra[:, :, 2::-1].copy())
    d = ImageDraw.Draw(pic)
    for wk in record["walkers"]:
        uv = {k["bone"]: k["uv"] for k in wk["keypoints"] if k["uv"]}
        for a, b in LINKS:
            if a in uv and b in uv:
                d.line([tuple(uv[a]), tuple(uv[b])], fill=(0, 255, 0), width=2)
        for p in uv.values():
            d.ellipse([p[0] - 2, p[1] - 2, p[0] + 2, p[1] + 2], fill=(255, 0, 0))
    pic.save(path)


def cmd_draw(args):
    world = _world(args)
    a = _walker(world, args.id)
    bones = _skeleton(a)
    n = 0
    for p, c in LINKS:
        if p in bones and c in bones:
            world.debug.draw_line(bones[p].location, bones[c].location, thickness=0.02,
                                  color=args.color, life_time=args.seconds)
            n += 1
    print(f"[pose] drew {n} skeleton links on id={a.id} for {args.seconds}s")


def cmd_pose(args):
    world = _world(args)
    a = _walker(world, args.id)
    with open(args.file) as fh:
        spec = json.load(fh)
    current = {b.name: b.relative for b in a.get_bones().bone_transforms}
    unknown = [n for n in spec if n not in current]
    if unknown:
        raise SystemExit(f"bones not on this skeleton: {unknown} (list them with: bones --id {a.id})")
    pairs = []
    for name, v in spec.items():
        # Start from the bone's current parent-relative transform: a zero location collapses it onto its parent.
        cur = current[name]
        if args.absolute:
            loc = carla.Location(*v.get("location", [cur.location.x, cur.location.y, cur.location.z]))
            rot = carla.Rotation(*v.get("rotation", [cur.rotation.pitch, cur.rotation.yaw, cur.rotation.roll]))
        else:
            dl, dr = v.get("location", [0, 0, 0]), v.get("rotation", [0, 0, 0])
            loc = carla.Location(cur.location.x + dl[0], cur.location.y + dl[1], cur.location.z + dl[2])
            rot = carla.Rotation(cur.rotation.pitch + dr[0], cur.rotation.yaw + dr[1], cur.rotation.roll + dr[2])
        pairs.append((name, carla.Transform(loc, rot)))
    before = dict(zip(a.get_bone_names(), a.get_bone_world_transforms()))
    a.set_bones(carla.WalkerBoneControlIn(pairs))
    a.blend_pose(args.blend)
    for _ in range(3):
        _step(world, args.tick)
    after = dict(zip(a.get_bone_names(), a.get_bone_world_transforms()))
    moved = [n for n in after if n in before and after[n].location.distance(before[n].location) > 0.01]
    print(f"[pose] set {len(pairs)} bones on id={a.id} at blend {args.blend}; {len(moved)} bones moved more than 1 cm")
    if not moved and args.blend > 0:
        print("[pose] nothing moved: a controller.ai.walker overwrites the pose every tick; use a walker without one")


def cmd_simple(args):
    world = _world(args)
    a = _walker(world, args.id)
    if args.cmd == "freeze":
        a.get_pose_from_animation()
        a.show_pose()
    elif args.cmd == "release":
        a.hide_pose()
    else:
        a.blend_pose(args.value)
    print(f"[pose] {args.cmd} applied to id={a.id}")


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("bones"); s.add_argument("--id", type=int, required=True); s.add_argument("--relative", action="store_true"); s.set_defaults(func=cmd_bones)
    s = sub.add_parser("keypoints"); s.add_argument("--id", required=True, help="walker id, or ids comma-separated")
    s.add_argument("--camera", type=int, required=True); s.add_argument("--frames", type=int, default=1)
    s.add_argument("--out", default="keypoints"); s.add_argument("--tick", action="store_true"); s.set_defaults(func=cmd_keypoints)
    s = sub.add_parser("draw"); s.add_argument("--id", type=int, required=True); s.add_argument("--seconds", type=float, default=120.0)
    s.add_argument("--color", type=_rgb, default=carla.Color(0, 255, 0), help="r,g,b (default 0,255,0)"); s.set_defaults(func=cmd_draw)
    s = sub.add_parser("pose"); s.add_argument("--id", type=int, required=True); s.add_argument("--file", required=True)
    s.add_argument("--blend", type=float, default=1.0); s.add_argument("--tick", action="store_true")
    s.add_argument("--absolute", action="store_true", help="POSE.json holds transforms, not changes"); s.set_defaults(func=cmd_pose)
    for name in ("freeze", "release"):
        s = sub.add_parser(name); s.add_argument("--id", type=int, required=True); s.set_defaults(func=cmd_simple)
    s = sub.add_parser("blend"); s.add_argument("--id", type=int, required=True); s.add_argument("--value", type=float, required=True); s.set_defaults(func=cmd_simple)
    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
