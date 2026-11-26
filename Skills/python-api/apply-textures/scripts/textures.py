#!/usr/bin/env python3
"""Replace the textures of map objects on a running server (texture streaming).

Commands:
    list  [--match TEXT]                                          names the texture calls accept
    apply (--object NAME[,NAME...] | --match TEXT) [maps] [--verify]   push image files onto the objects
    solid (--object NAME[,NAME...] | --match TEXT) --color R,G,B [--verify]  push a flat diffuse colour

maps: --diffuse IMG --normal IMG --orme IMG --emissive IMG (ORME = AO/Roughness/Metallic/Emissive packed).
--max-size N downscales images first (default 1024): the client fills textures one pixel per call.
--verify renders the object with a temporary RGB camera before and after, and reports the pixel change.
Changes last until the world is reloaded (client.reload_world()). Connection from env.sh.
"""
from __future__ import annotations

import argparse
import math
import os
import queue
import time

import carla  # provided by the active interpreter; check_env.sh verifies this


def _client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"), int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60.0")))
    return c


def _targets(world, args):
    names = world.get_names_of_all_objects()
    if args.object:
        wanted = args.object.split(",")
        missing = [n for n in wanted if n not in names]
        if missing:
            raise SystemExit(f"not paintable on this map: {missing} (find names with: list --match ...)")
        return wanted
    hits = sorted(n for n in names if args.match.lower() in n.lower())
    if not hits:
        raise SystemExit(f"no object name contains '{args.match}'")
    if len(hits) > args.max_objects:
        raise SystemExit(f"{len(hits)} objects match '{args.match}'; narrow it or raise --max-objects")
    return hits


def _image(path, max_size):
    from PIL import Image
    img = Image.open(path).convert("RGBA")
    if max(img.size) > max_size:
        img.thumbnail((max_size, max_size))
    return img


def _color_texture(img):
    w, h = img.size
    px, tex = img.load(), carla.TextureColor(w, h)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            tex.set(x, y, carla.Color(r, g, b, a))
    return tex


def _float_texture(img, scale):
    w, h = img.size
    px, tex = img.load(), carla.TextureFloatColor(w, h)
    for y in range(h):
        for x in range(w):
            r, g, b, a = px[x, y]
            tex.set(x, y, carla.FloatColor(r / 255.0 * scale, g / 255.0 * scale, b / 255.0 * scale, a / 255.0))
    return tex


def _object_box(world, name):
    """World-space box of an object, from its mesh components (named <object>_SM_<n>)."""
    boxes = [o.bounding_box for o in world.get_environment_objects(carla.CityObjectLabel.Any)
             if o.name == name or o.name.startswith(name + "_")]
    if not boxes:
        return None
    lo = [min(b.location.x - b.extent.x for b in boxes), min(b.location.y - b.extent.y for b in boxes), min(b.location.z - b.extent.z for b in boxes)]
    hi = [max(b.location.x + b.extent.x for b in boxes), max(b.location.y + b.extent.y for b in boxes), max(b.location.z + b.extent.z for b in boxes)]
    return carla.Location(*[(a + b) / 2 for a, b in zip(lo, hi)]), max(hi[i] - lo[i] for i in range(3))


class Verifier:
    """A temporary camera framed on the first target, to measure the change in pixels."""

    def __init__(self, world, name):
        import numpy as np
        self.np, self.world = np, world
        box = _object_box(world, name)
        if box is None:
            raise SystemExit(f"--verify: no environment object named {name}_SM_* to frame")
        centre, size = box
        dist = max(size, 10.0) * 1.2
        eye = carla.Location(centre.x - dist * 0.7, centre.y - dist * 0.7, centre.z + dist * 0.4)
        d = centre - eye
        rot = carla.Rotation(pitch=math.degrees(math.atan2(d.z, math.hypot(d.x, d.y))), yaw=math.degrees(math.atan2(d.y, d.x)))
        bp = world.get_blueprint_library().find("sensor.camera.rgb")
        bp.set_attribute("image_size_x", "640")
        bp.set_attribute("image_size_y", "480")
        self.q = queue.Queue()
        self.cam = world.spawn_actor(bp, carla.Transform(eye, rot))
        self.cam.listen(self.q.put)

    def grab(self, settle=10):
        # Skip frames so the camera exposure and the streamed texture have both settled.
        img = None
        for _ in range(settle):
            img = self.q.get(timeout=float(os.environ.get("CARLA_TIMEOUT", "60.0")))
        a = self.np.frombuffer(img.raw_data, self.np.uint8).reshape((img.height, img.width, 4))[:, :, :3]
        return a.astype(self.np.int16), img

    def close(self):
        self.cam.stop()
        self.cam.destroy()


def _push(world, names, args):
    if args.cmd == "solid":
        r, g, b = (int(v) for v in args.color.split(","))
        tex = carla.TextureColor(8, 8)
        for y in range(8):
            for x in range(8):
                tex.set(x, y, carla.Color(r, g, b, 255))
        world.apply_color_texture_to_objects(names, carla.MaterialParameter.Diffuse, tex)
        return ["Diffuse"]
    done = []
    t0 = time.time()
    if args.diffuse:
        world.apply_color_texture_to_objects(names, carla.MaterialParameter.Diffuse, _color_texture(_image(args.diffuse, args.max_size)))
        done.append("Diffuse")
    for flag, param in (("normal", carla.MaterialParameter.Normal), ("orme", carla.MaterialParameter.AO_Roughness_Metallic_Emissive),
                        ("emissive", carla.MaterialParameter.Emissive)):
        path = getattr(args, flag)
        if path:
            scale = args.emissive_scale if flag == "emissive" else 1.0
            world.apply_float_color_texture_to_objects(names, param, _float_texture(_image(path, args.max_size), scale))
            done.append(str(param))
    if not done:
        raise SystemExit("apply needs at least one of --diffuse --normal --orme --emissive")
    print(f"[textures] built and sent in {time.time() - t0:.1f}s")
    return done


def cmd_list(args):
    names = sorted(_client().get_world().get_names_of_all_objects())
    hits = [n for n in names if not args.match or args.match.lower() in n.lower()]
    for n in hits:
        print(n)
    print(f"[textures] {len(hits)} of {len(names)} paintable objects")


def cmd_apply(args):
    world = _client().get_world()
    names = _targets(world, args)
    ver = Verifier(world, names[0]) if args.verify else None
    try:
        before = ver.grab()[0] if ver else None
        done = _push(world, names, args)
        print(f"[textures] {', '.join(done)} -> {len(names)} object(s): {', '.join(names[:5])}{' ...' if len(names) > 5 else ''}")
        if ver:
            after, img = ver.grab()
            changed = float((abs(after - before).max(axis=2) > 12).mean() * 100)
            out = args.verify_out or "texture_after.png"
            img.save_to_disk(out)
            print(f"[textures] verify: {changed:.1f}% of the camera's pixels changed (frame saved to {out})")
            if changed < 0.5:
                print("[textures] verify: almost nothing changed; the material may not expose this parameter (see references)")
    finally:
        if ver:
            ver.close()


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    s = sub.add_parser("list")
    s.add_argument("--match")
    s.set_defaults(func=cmd_list)
    for name in ("apply", "solid"):
        s = sub.add_parser(name)
        g = s.add_mutually_exclusive_group(required=True)
        g.add_argument("--object", help="exact object name(s), comma-separated")
        g.add_argument("--match", help="every object whose name contains this text")
        s.add_argument("--max-objects", type=int, default=20)
        s.add_argument("--verify", action="store_true")
        s.add_argument("--verify-out")
        if name == "apply":
            for m in ("diffuse", "normal", "orme", "emissive"):
                s.add_argument(f"--{m}")
            s.add_argument("--max-size", type=int, default=1024)
            s.add_argument("--emissive-scale", type=float, default=5.0, help="multiplier on emissive texels (HDR)")
        else:
            s.add_argument("--color", required=True, help="R,G,B 0-255")
        s.set_defaults(func=cmd_apply)
    args = p.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
