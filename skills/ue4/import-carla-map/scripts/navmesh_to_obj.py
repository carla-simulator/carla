#!/usr/bin/env python3
"""Decode a CARLA pedestrian navmesh (Nav/<map>.bin) into inspectable geometry.

Prints a report by default -- tile grid, polygons per area, bounds -- which is
enough to tell an empty or truncated navmesh from a real one. With --obj, writes
the walkable polygons as a mesh grouped by Recast area, for overlaying the map.

    python3 navmesh_to_obj.py --package MyTown          # resolved under the checkout
    python3 navmesh_to_obj.py path/to/Nav/MyTown.bin    # or any .bin, in place

Binary layout (packed), per carla/nav/Navigation.cpp:
  NavMeshSetHeader : int magic('MSET'), int version(1), int num_tiles,
                     dtNavMeshParams{ float orig[3]; float tileW, tileH;
                                      int maxTiles, maxPolys; }        = 40 bytes
  per tile         : dtTileRef(uint32), int data_size, then data_size bytes
                     beginning with dtMeshHeader.
"""
import argparse
import os
import struct
import sys

UPROJECT_REL = "Unreal/CarlaUE4/CarlaUE4.uproject"

NAVMESHSET_MAGIC = 0x4D534554  # 'MSET'
NAVMESHSET_VERSION = 1         # NavMeshSetHeader.version written by CARLA
DT_NAVMESH_MAGIC = 0x444E4156  # 'DNAV'

# Recast area ids -- CARLA_AREA_* in LibCarla/source/carla/nav/Navigation.h
AREA_NAMES = {0: "block", 1: "sidewalk", 2: "crosswalk", 3: "road", 4: "grass"}

# dtMeshHeader: 15 ints, then walkableHeight/Radius/Climb, bmin[3], bmax[3],
# bvQuantFactor -- 100 bytes.
MESH_HEADER = struct.Struct("<15i3f3f3ff")


def resolve_carla_root():
    """CARLA_UE4_ROOT env > $PWD if a checkout > path-derived guess."""
    env = os.environ.get("CARLA_UE4_ROOT")
    if env and os.path.isfile(os.path.join(env, UPROJECT_REL)):
        return env
    cwd = os.getcwd()
    if os.path.isfile(os.path.join(cwd, UPROJECT_REL)):
        return cwd
    # skills/<name>/scripts -> skills/<name> -> skills -> repo -> the checkout
    guess = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)),
                                         *([os.pardir] * 4)))
    if os.path.isfile(os.path.join(guess, UPROJECT_REL)):
        return guess
    return None


def add_navmesh_args(ap):
    """The navmesh can be named by path, or resolved from the package it was
    imported into -- so the skill's commands work from any directory."""
    ap.add_argument("navmesh", nargs="?",
                    help="path to Nav/<map>.bin; omit and pass --package instead")
    ap.add_argument("--package", help="content package the map was imported into; "
                                     "resolves the .bin under $CARLA_UE4_ROOT")
    ap.add_argument("--map", help="map name within --package (default: the package name)")


def resolve_navmesh(args):
    """An explicit path wins; otherwise build it from --package under the root."""
    if args.navmesh:
        if not os.path.isfile(args.navmesh):
            sys.exit("navmesh: no such file: %s" % args.navmesh)
        return args.navmesh
    if not args.package:
        sys.exit("navmesh: pass a path to Nav/<map>.bin, or --package <name>")
    root = resolve_carla_root()
    if root is None:
        sys.exit("navmesh: --package needs the checkout — export CARLA_UE4_ROOT, "
                 "or run from inside one, or pass the .bin path directly")
    name = args.map or args.package
    path = os.path.join(root, "Unreal/CarlaUE4/Content", args.package,
                        "Maps", name, "Nav", "%s.bin" % name)
    if not os.path.isfile(path):
        sys.exit("navmesh: no navmesh at %s\n"
                 "  A standard map needs FBX2OBJ installed and build_navmesh.py run;\n"
                 "  a large (tiled) map has none by design (references/maps.md)." % path)
    return path


def parse(path):
    blob = open(path, "rb").read()
    if len(blob) < 40:
        sys.exit("navmesh: file too small (%d bytes): %s" % (len(blob), path))

    magic, version, num_tiles = struct.unpack_from("<iii", blob, 0)
    if magic != NAVMESHSET_MAGIC:
        sys.exit("navmesh: bad magic 0x%08X (expected 'MSET') in %s" % (magic, path))
    if version != NAVMESHSET_VERSION:
        sys.exit("navmesh: NAVMESHSET version %d, this decoder reads version %d — "
                 "CARLA's writer changed (carla/nav/Navigation.cpp)"
                 % (version, NAVMESHSET_VERSION))
    orig = struct.unpack_from("<3f", blob, 12)
    tile_w, tile_h = struct.unpack_from("<2f", blob, 24)
    max_tiles, max_polys = struct.unpack_from("<2i", blob, 32)
    pos = 40

    tiles = []
    for _ in range(num_tiles):
        if pos + 8 > len(blob):
            break
        tile_ref, data_size = struct.unpack_from("<Ii", blob, pos)
        pos += 8
        if not tile_ref or not data_size:
            continue
        data = blob[pos:pos + data_size]
        pos += data_size
        if len(data) < MESH_HEADER.size:
            continue
        h = MESH_HEADER.unpack_from(data, 0)
        (hmagic, hversion, tx, ty, layer, user_id, poly_count, vert_count,
         max_link, dmesh_count, dvert_count, dtri_count, bv_count,
         omc_count, omb) = h[:15]
        if hmagic != DT_NAVMESH_MAGIC:
            continue
        off = MESH_HEADER.size
        verts = struct.unpack_from("<%df" % (vert_count * 3), data, off)
        off += vert_count * 12          # 3 floats per vertex

        # dtPoly is 32 bytes (DetourNavMesh.h): uint firstLink; ushort verts[6];
        # ushort neis[6]; ushort flags; uchar vertCount; uchar areaAndtype.
        # areaAndtype packs the area in the low 6 bits and the poly type in the
        # top 2, hence the mask.
        POLY_STRIDE = 32
        POLY_VERTS_OFF = 4              # after firstLink
        POLY_VERTCOUNT_OFF = 30
        POLY_AREA_OFF = 31
        polys = []
        for p in range(poly_count):
            base = off + p * POLY_STRIDE
            pv = struct.unpack_from("<6H", data, base + POLY_VERTS_OFF)
            nvert = data[base + POLY_VERTCOUNT_OFF]
            area_and_type = data[base + POLY_AREA_OFF]
            polys.append((pv[:nvert], area_and_type & 0x3F))
        tiles.append({"x": tx, "y": ty, "verts": verts, "polys": polys})

    return {
        "num_tiles": num_tiles, "orig": orig, "tile_w": tile_w, "tile_h": tile_h,
        "max_tiles": max_tiles, "max_polys": max_polys, "tiles": tiles,
    }


def _poly_area(pts):
    """Shoelace on the XZ projection; the navmesh is a ground surface."""
    s = 0.0
    for k in range(len(pts)):
        x1, z1 = pts[k]
        x2, z2 = pts[(k + 1) % len(pts)]
        s += x1 * z2 - x2 * z1
    return abs(s) / 2


def coverage(nav, cell=50.0):
    """How much ground the navmesh actually covers, and how it is classified.

    Nothing in the stock pipeline reports this: a navmesh can load, spawn a
    walker and still cover a fraction of the map, or classify most of itself as
    RC_WALKABLE_AREA (63) instead of a CARLA_AREA_*. Both are silent.
    """
    per_area, polys = {}, []
    xs, zs = [], []
    for t in nav["tiles"]:
        v = t["verts"]
        for idx, a in t["polys"]:
            pts = [(v[i * 3], v[i * 3 + 2]) for i in idx]
            ar = _poly_area(pts)
            per_area[a] = per_area.get(a, 0.0) + ar
            cx = sum(p[0] for p in pts) / len(pts)
            cz = sum(p[1] for p in pts) / len(pts)
            polys.append((a, cx, cz, ar))
            xs.extend(p[0] for p in pts)
            zs.extend(p[1] for p in pts)
    if not polys:
        print("  coverage: EMPTY navmesh")
        return

    total = sum(per_area.values())
    w, h = max(xs) - min(xs), max(zs) - min(zs)
    print("  --- coverage ---")
    print("  walkable         : %.0f m2 over a %.0f x %.0f m extent (%.1f%%)"
          % (total, w, h, 100 * total / (w * h) if w * h else 0))
    for a in sorted(per_area, key=lambda k: -per_area[k]):
        name = AREA_NAMES.get(a, "area%d (unclassified)" % a)
        print("    %-24s %9.0f m2  %5.1f%% of navmesh"
              % (name, per_area[a], 100 * per_area[a] / total))

    # Road-corridor test: bucket road/sidewalk/crosswalk into a coarse grid, then
    # ask how much of everything else lies outside those cells and their
    # neighbours. A navmesh that only hugs the roads answers ~0%.
    ROAD_AREAS = {1, 2, 3}
    near = set()
    for a, cx, cz, _ in polys:
        if a in ROAD_AREAS:
            gx, gz = int(cx // cell), int(cz // cell)
            for dx in (-1, 0, 1):
                for dz in (-1, 0, 1):
                    near.add((gx + dx, gz + dz))
    other = away = 0.0
    for a, cx, cz, ar in polys:
        if a in ROAD_AREAS:
            continue
        other += ar
        if (int(cx // cell), int(cz // cell)) not in near:
            away += ar
    if other:
        print("  off-road walkable: %.0f m2, of which %.0f m2 (%.0f%%) is beyond "
              "~%.0f m of any road/sidewalk" % (other, away, 100 * away / other, cell))


def main():
    ap = argparse.ArgumentParser()
    add_navmesh_args(ap)
    ap.add_argument("--obj", help="write the walkable polygons to this .obj")
    ap.add_argument("--ue4", action="store_true",
                    help="emit UE4 world space (cm, Z-up) instead of Recast space (m, Y-up)")
    ap.add_argument("--coverage", action="store_true",
                    help="report walkable area, how it is classified, and whether it "
                         "only hugs the road network")
    args = ap.parse_args()

    navmesh = resolve_navmesh(args)
    nav = parse(navmesh)
    tiles = nav["tiles"]
    total_polys = sum(len(t["polys"]) for t in tiles)
    total_verts = sum(len(t["verts"]) // 3 for t in tiles)

    per_area = {}
    mn = [1e30] * 3
    mx = [-1e30] * 3
    for t in tiles:
        v = t["verts"]
        for i in range(0, len(v), 3):
            for k in range(3):
                mn[k] = min(mn[k], v[i + k])
                mx[k] = max(mx[k], v[i + k])
        for _, area in t["polys"]:
            per_area[area] = per_area.get(area, 0) + 1

    print("navmesh: %s" % navmesh)
    print("  tiles stored     : %d (loaded %d)" % (nav["num_tiles"], len(tiles)))
    print("  max tiles / polys: %d / %d" % (nav["max_tiles"], nav["max_polys"]))
    print("  tile size        : %.1f x %.1f m" % (nav["tile_w"], nav["tile_h"]))
    print("  polygons         : %d" % total_polys)
    print("  vertices         : %d" % total_verts)
    if total_polys:
        print("  per area         : %s" % ", ".join(
            "%s=%d" % (AREA_NAMES.get(a, "area%d" % a), n)
            for a, n in sorted(per_area.items())))
        print("  bounds (Recast m): x %.1f..%.1f  y %.1f..%.1f  z %.1f..%.1f"
              % (mn[0], mx[0], mn[1], mx[1], mn[2], mx[2]))
    else:
        print("  EMPTY -- no walkable polygons. Walkers cannot navigate this map.")

    if args.coverage and total_polys:
        coverage(nav)

    if not args.obj:
        return 0 if total_polys else 1

    groups = {}
    out_v = []
    for t in tiles:
        v = t["verts"]
        base = len(out_v)
        for i in range(0, len(v), 3):
            x, y, z = v[i], v[i + 1], v[i + 2]
            if args.ue4:
                # Recast (x, up, z) metres  ->  UE4 (x, y, z) centimetres, Z up.
                out_v.append((x * 100.0, z * 100.0, y * 100.0))
            else:
                out_v.append((x, y, z))
        for idx, area in t["polys"]:
            if len(idx) < 3:
                continue
            name = AREA_NAMES.get(area, "area%d" % area)
            fan = [(base + idx[0] + 1, base + idx[i] + 1, base + idx[i + 1] + 1)
                   for i in range(1, len(idx) - 1)]
            groups.setdefault(name, []).extend(fan)

    with open(args.obj, "w") as fh:
        fh.write("# CARLA navmesh decoded from %s\n" % navmesh)
        fh.write("# space: %s\n" % ("UE4 cm, Z-up" if args.ue4 else "Recast m, Y-up"))
        for x, y, z in out_v:
            fh.write("v %f %f %f\n" % (x, y, z))
        for name, faces in groups.items():
            fh.write("g %s\no %s\nusemtl %s\n" % (name, name, name))
            for a, b, c in faces:
                fh.write("f %d %d %d\n" % (a, b, c))
    print("  wrote %s (%d verts, %d faces, groups: %s)"
          % (args.obj, len(out_v), sum(len(f) for f in groups.values()),
             ", ".join(sorted(groups))))
    return 0 if total_polys else 1


if __name__ == "__main__":
    sys.exit(main())
