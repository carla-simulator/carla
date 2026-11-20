#!/usr/bin/env python3
"""Lists, mounts, unmounts and diagnoses content packs on a running CARLA server.

  content_packs.py list
  content_packs.py mount   <pack folder on the server's file system>
  content_packs.py unmount <Pack> [--switch-to Town10HD_Opt]
  content_packs.py why     <pack folder>      why a pack is not mounted (Shipping logs nothing)

Reads CARLA_HOST, CARLA_PORT, CARLA_TIMEOUT (source scripts/env.sh).
"""
import argparse
import os
import sys
import time

import carla


def client():
    c = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"), int(os.environ.get("CARLA_PORT", "2000")))
    c.set_timeout(float(os.environ.get("CARLA_TIMEOUT", "60")))
    return c


def blueprint_ids(c):
    return {b.id for b in c.get_world().get_blueprint_library()}


def current_map(c):
    return c.get_world().get_map().name


def pack_of_map(packs, map_path):
    """The mounted pack whose map is the current world (map names are matched on the last path element)."""
    leaf = map_path.rstrip("/").split("/")[-1]
    for p in packs:
        if p.mounted and leaf in list(p.maps):
            return p
    return None


def cmd_list(c, a):
    packs = c.get_content_packs()
    if not packs:
        print("[packs] no content packs known to this server")
    for p in packs:
        print("[packs] {:<16} {:<8} {:<9} base {}  maps: {}".format(
            p.name, p.version, "mounted" if p.mounted else "UNMOUNTED", p.base_release, ", ".join(p.maps) or "-"))
        print("[packs]   {}".format(p.path))
    cur = current_map(c)
    owner = pack_of_map(packs, cur)
    print("[packs] current map {}{}".format(cur, " (from pack {})".format(owner.name) if owner else ""))
    print("[packs] available maps: {}".format(", ".join(c.get_available_maps())))


def cmd_mount(c, a):
    if a.path.endswith((".tar.gz", ".tgz", ".tar")):
        sys.exit("[packs] ERROR {} is a tarball: extract it (or install it with install-carla-content-pack) and mount the folder".format(a.path))
    before = blueprint_ids(c)
    t0 = time.time()
    try:
        info = c.mount_content_pack(a.path)
    except RuntimeError as e:
        sys.exit("[packs] ERROR {}".format(e))
    added = sorted(blueprint_ids(c) - before)
    print("[packs] mounted {} {} from {} in {:.1f} s".format(info.name, info.version, info.path, time.time() - t0))
    print("[packs]   maps (load with client.load_world): {}".format(", ".join(info.maps) or "-"))
    print("[packs]   new blueprints: {}".format(", ".join(added) or "-"))


def cmd_unmount(c, a):
    packs = {p.name: p for p in c.get_content_packs()}
    p = packs.get(a.pack)
    if p is None:
        sys.exit("[packs] ERROR no pack {} on this server (known: {})".format(a.pack, ", ".join(packs) or "none"))
    cur = current_map(c)
    if pack_of_map([p], cur):
        if not a.switch_to:
            sys.exit("[packs] ERROR the current map {} comes from {}: pass --switch-to <map> to load another one first".format(cur, a.pack))
        print("[packs] loading {} first".format(a.switch_to))
        c.load_world(a.switch_to)
    try:
        c.unmount_content_pack(a.pack)
    except RuntimeError as e:
        msg = str(e)
        if "still loaded" in msg:
            msg += "\n[packs] destroy the pack's actors, or client.reload_world(), then retry"
        sys.exit("[packs] ERROR {}".format(msg))
    print("[packs] unmounted {}".format(a.pack))
    # The server keeps the episode's blueprint library until the episode changes.
    print("[packs] note: get_blueprint_library() lists its blueprints until the next load_world/reload_world")


def cmd_why(c, a):
    path = a.path.rstrip("/")
    name = os.path.basename(path[:-len("/carla-pack.json")] if path.endswith("carla-pack.json") else path)
    known = {p.name: p for p in c.get_content_packs()}
    p = known.get(name)
    if p and p.mounted:
        print("[packs] {} is mounted, from {}".format(name, p.path))
        if os.path.realpath(p.path) != os.path.realpath(path):
            print("[packs] that is another copy than {}: a name mounts once per server".format(path))
        return
    try:
        info = c.mount_content_pack(path)
    except RuntimeError as e:
        print("[packs] {} does not mount: {}".format(name, e))
        sys.exit(1)
    if p is not None:
        print("[packs] {} was known but unmounted (a client unmounted it); mounted it again ({})".format(name, ", ".join(info.maps) or "no maps"))
        return
    print("[packs] {} mounts fine now ({}), so the server never found it at start.".format(info.name, ", ".join(info.maps) or "no maps"))
    print("[packs] Start-up scans <package>/CarlaUnreal/Packs/, -carla-packs=<dir> and $CARLA_PACKS; each is the PARENT of pack folders.")


def main():
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("list").set_defaults(func=cmd_list)
    p = sub.add_parser("mount")
    p.add_argument("path", help="pack folder (or its carla-pack.json) as the SERVER sees it")
    p.set_defaults(func=cmd_mount)
    p = sub.add_parser("unmount")
    p.add_argument("pack")
    p.add_argument("--switch-to", help="map to load first when the current map belongs to the pack")
    p.set_defaults(func=cmd_unmount)
    p = sub.add_parser("why")
    p.add_argument("path", help="pack folder as the server sees it")
    p.set_defaults(func=cmd_why)
    a = ap.parse_args()
    a.func(client(), a)


if __name__ == "__main__":
    main()
