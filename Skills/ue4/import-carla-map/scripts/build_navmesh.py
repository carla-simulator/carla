#!/usr/bin/env python3
"""Regenerate the pedestrian navmesh for an ALREADY-imported CARLA map.

Runs the same chain as Import.py's build_binary_for_navigation() --
FBX -> OBJ -> (+ OpenDRIVE crosswalks) -> RecastBuilder -> Nav/<map>.bin -- but
standalone, so a navmesh costs minutes instead of a full editor re-import.

Unlike the stock pipeline it FAILS LOUDLY: build.sh gates every stage behind
`if [ -f ... ]` and Import.py ignores its exit status, so a missing FBX2OBJ or an
empty OBJ yields a map with no navmesh and no error.

STANDARD MAPS ONLY. A large (tiled) map gets no navmesh, here or from
the import, and that is deliberate -- CARLA's own large maps ship none
(Town11/12/13 have no Nav/ at all). See references/maps.md.

Usage:
    python3 build_navmesh.py --package MyTown --fbx MyTown.fbx
"""
import argparse
import glob
import os
import re
import shutil
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))

# RoadRunner names a large map's tiles <map>_Tile_X_Y.fbx.
TILE_RE = re.compile(r"_Tile_\d+_\d+\.fbx$", re.IGNORECASE)


def fail(msg):
    print("[navmesh] ERROR %s" % msg, file=sys.stderr)
    sys.exit(1)


def info(msg):
    print("[navmesh] %s" % msg, flush=True)


UPROJECT_REL = "Unreal/CarlaUE4/CarlaUE4.uproject"


def find_carla_root(explicit):
    """Every candidate is validated, so a stale CARLA_UE4_ROOT fails here naming
    the path, instead of surfacing later as a confusing 'map not imported'."""
    for source, cand in (("--carla-root", explicit),
                         ("$CARLA_UE4_ROOT", os.environ.get("CARLA_UE4_ROOT")),
                         ("the current directory", os.getcwd())):
        if not cand:
            continue
        path = os.path.abspath(cand)
        if os.path.isfile(os.path.join(path, UPROJECT_REL)):
            return path
        if source != "the current directory":
            fail("%s points at %s, which is not a CARLA checkout (no %s)"
                 % (source, path, UPROJECT_REL))
    fail("cannot locate the CARLA checkout; pass --carla-root or export CARLA_UE4_ROOT")


def python_with_carla():
    """An interpreter that can `import carla` -- needed for crosswalk extraction.

    No environment manager is assumed (conventions): whatever is active wins,
    then this script's own interpreter. No environment manager is assumed."""
    for py in (shutil.which("python3"), sys.executable):
        if py and subprocess.run([py, "-c", "import carla"],
                                 capture_output=True).returncode == 0:
            return py
    return None


# --- Detour's tile budget ----------------------------------------------------
#
# Sample_TileMesh::handleSettings() splits 22 bits between tile and polygon ids
# and caps the tile half at 14, so a navmesh can address at most 16,384 tiles.
# The tile size is 256 voxels of 0.2 m = 51.2 m, which puts a hard ceiling of
# 128 * 51.2 m ~= 6.5 km on each side of a map. RecastBuilder does not check
# this: it rasterizes all tw*th tiles, drops every one past the budget at
# addTile() time, prints "Max Polys 256" and still exits 0 -- so an oversized
# map burns hours and yields no .bin, with nothing in the exit status to catch.
#
# Both inputs are settable per-map through a .gset file (InputGeom::loadGeomSet)
# without touching the tool: an 's' line carries the build settings, including
# the navmesh bounds and the tile size. So we crop the bounds to the walkable
# geometry -- the terrain mesh is usually far wider than the roads and is not
# walkable anyway -- and grow the tile size until the lattice fits. Cell size is
# untouched, so surface resolution is identical; the tiles just cover more area.
MAX_ADDRESSABLE_TILES = 1 << 14
DEFAULT_TILE_SIZE = 256

# Sample::resetCommonSettings(), which is what the stock tool builds with. A
# .gset overrides ALL of these, so they have to match or the navmesh silently
# changes character.
RECAST_DEFAULTS = dict(
    cell_size=0.2, cell_height=0.01, agent_height=1.8, agent_radius=0.2,
    agent_max_climb=0.3, agent_max_slope=45.0, region_min_size=8.0,
    region_merge_size=20.0, edge_max_len=12.0, edge_max_error=1.3,
    verts_per_poly=6.0, detail_sample_dist=3.0, detail_sample_max_error=0.5,
    partition_type=0,  # SAMPLE_PARTITION_WATERSHED
)

# FBX2OBJ's material names; everything else (notably the terrain) becomes
# 'block', which Recast never makes walkable.
WALKABLE_MATERIALS = ("road", "sidewalk", "crosswalk", "grass")

# Bounds are cropped to the road network only. 'grass' is deliberately absent:
# under CARLA_TERRAIN_AREA=grass the terrain mesh becomes walkable-but-costly,
# and including it here would stretch the bounds back over the whole terrain --
# the exact extent the crop exists to shed. Grass inside the cropped area is
# still classified normally; only grass beyond the road network is dropped.
CROP_MATERIALS = ("road", "sidewalk", "crosswalk")


def obj_bounds(path):
    """(road_network_bbox, full_bbox) as (min[3], max[3]) in Recast space (Y up).

    Bounds come from the vertices each face actually references, per material,
    so a stray unreferenced vertex cannot inflate the lattice."""
    verts = []
    mat = None
    wmin = [None] * 3
    wmax = [None] * 3
    fmin = [None] * 3
    fmax = [None] * 3

    def widen(lo, hi, v):
        for i in range(3):
            if lo[i] is None or v[i] < lo[i]:
                lo[i] = v[i]
            if hi[i] is None or v[i] > hi[i]:
                hi[i] = v[i]

    with open(path, "r", errors="replace") as fh:
        for line in fh:
            if line.startswith("v "):
                p = line.split()
                verts.append((float(p[1]), float(p[2]), float(p[3])))
            elif line.startswith("usemtl "):
                mat = line.split(None, 1)[1].strip().lower()
            elif line.startswith("f "):
                on_network = mat in CROP_MATERIALS
                for tok in line.split()[1:]:
                    idx = int(tok.split("/")[0])
                    v = verts[idx - 1] if idx > 0 else verts[idx]
                    widen(fmin, fmax, v)
                    if on_network:
                        widen(wmin, wmax, v)
    if None in fmin:
        fail("OBJ %s has no faces" % os.path.basename(path))
    if None in wmin:
        # No recognised road-network material: fall back to the whole mesh
        # rather than cropping to nothing.
        return (fmin, fmax), (fmin, fmax)
    return (wmin, wmax), (fmin, fmax)


def tile_lattice(bmin, bmax, cell_size, tile_size):
    """tw, th, tiles, maxTiles, maxPolys exactly as Sample_TileMesh computes."""
    # rcCalcGridSize
    gw = int((bmax[0] - bmin[0]) / cell_size + 0.5)
    gh = int((bmax[2] - bmin[2]) / cell_size + 0.5)
    tw = (gw + tile_size - 1) // tile_size
    th = (gh + tile_size - 1) // tile_size
    tiles = tw * th
    pow2 = 1
    while pow2 < max(tiles, 1):
        pow2 <<= 1
    tile_bits = min(pow2.bit_length() - 1, 14)
    return tw, th, tiles, 1 << tile_bits, 1 << (22 - tile_bits)


def fit_tile_size(bmin, bmax, cell_size, start=DEFAULT_TILE_SIZE):
    """Smallest power-of-two tile size whose lattice fits the tile budget."""
    ts = start
    while ts <= 8192:
        _, _, tiles, max_tiles, _ = tile_lattice(bmin, bmax, cell_size, ts)
        if tiles <= min(max_tiles, MAX_ADDRESSABLE_TILES):
            return ts
        ts *= 2
    return None


def write_gset(path, obj_name, bmin, bmax, tile_size, agent_radius):
    d = dict(RECAST_DEFAULTS)
    d["agent_radius"] = agent_radius
    with open(path, "w") as fh:
        fh.write("f %s\n" % obj_name)
        fh.write("s %f %f %f %f %f %f %f %f %f %f %f %f %f %d "
                 "%f %f %f %f %f %f %f\n"
                 % (d["cell_size"], d["cell_height"], d["agent_height"],
                    d["agent_radius"], d["agent_max_climb"], d["agent_max_slope"],
                    d["region_min_size"], d["region_merge_size"],
                    d["edge_max_len"], d["edge_max_error"], d["verts_per_poly"],
                    d["detail_sample_dist"], d["detail_sample_max_error"],
                    d["partition_type"],
                    bmin[0], bmin[1], bmin[2], bmax[0], bmax[1], bmax[2],
                    float(tile_size)))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--package", required=True,
                    help="content package the map was imported into (the folder "
                         "under Content/), e.g. MyTown")
    ap.add_argument("--map", help="map name (defaults to --package)")
    ap.add_argument("--carla-root", help="CARLA checkout (default: $CARLA_UE4_ROOT or cwd)")
    ap.add_argument("--fbx", required=True, metavar="FBX",
                    help="the map's source .fbx — the same file that was imported. "
                         "Standard maps only; a tiled map's per-tile FBXs are rejected")
    ap.add_argument("--timeout", type=int, default=3600,
                    help="seconds allowed for RecastBuilder (default 3600); big maps are slow")
    ap.add_argument("--keep-temp", action="store_true", help="keep the working directory")
    ap.add_argument("--tile-size", type=int, metavar="VOXELS",
                    help="force the Recast tile size in voxels (default %d = %.1f m "
                         "at 0.2 m cells). Raise it for a map over ~6.5 km/side: it "
                         "buys extent at the cost of polygons per tile, and leaves "
                         "cell resolution untouched"
                         % (DEFAULT_TILE_SIZE, DEFAULT_TILE_SIZE * 0.2))
    ap.add_argument("--no-crop", action="store_true",
                    help="do not crop the navmesh bounds to the walkable geometry; "
                         "use the whole mesh, terrain included, as the stock tool does")
    args = ap.parse_args()

    root = find_carla_root(args.carla_root)
    package = args.package
    map_name = args.map or package

    dist = os.path.join(root, "Util/DockerUtils/dist")
    recast = os.path.join(dist, "RecastBuilder")
    fbx2obj = os.path.join(dist, "FBX2OBJ")
    map_dir = os.path.join(root, "Unreal/CarlaUE4/Content", package, "Maps", map_name)
    xodr = os.path.join(map_dir, "OpenDrive", "%s.xodr" % map_name)

    # --fbx may be a literal path or a glob; the shell will not have expanded a
    # quoted one. A glob matching many files means a tiled map, which we refuse.
    pattern = os.path.expanduser(args.fbx)
    matched = sorted(glob.glob(pattern)) or [pattern]
    if len(matched) > 1 or TILE_RE.search(os.path.basename(matched[0])):
        fail("that is a large (tiled) map's geometry — %d file(s) matching %s.\n"
             "         Large maps get no pedestrian navmesh, by design: CARLA's own\n"
             "         large maps ship none (Town11/12/13 have no Nav/ directory), and\n"
             "         Detour cannot address one mesh over that extent. Walkers are\n"
             "         unsupported there; vehicles and Traffic Manager are unaffected.\n"
             "         For a standard map, pass its single combined .fbx."
             % (len(matched), args.fbx))
    fbx = matched[0]

    if not os.path.isdir(map_dir):
        fail("map not imported: %s" % map_dir)

    # The authoritative test is the imported map, not the FBX name: the import
    # writes TilesInfo.txt (and <map>_Tile_X_Y.umap) only for a large map.
    if (os.path.isfile(os.path.join(map_dir, "TilesInfo.txt"))
            or glob.glob(os.path.join(map_dir, "*_Tile_*.umap"))):
        fail("%s is a large (tiled) map — it gets no pedestrian navmesh.\n"
             "         This is deliberate and matches upstream: CARLA's own large maps\n"
             "         ship none (Town11 22x24 km, Town12 12x12 km, Town13 16x12 km all\n"
             "         have no Nav/ directory), and Detour's 22-bit tile/poly budget\n"
             "         cannot address one mesh past ~6.5 km per side.\n"
             "         Walkers are unsupported on large maps; vehicles, Traffic Manager\n"
             "         and everything else are unaffected. See references/maps.md."
             % map_name)

    if not os.path.isfile(fbx):
        fail("source FBX not found: %s" % fbx)
    if not os.path.isfile(recast):
        fail("RecastBuilder not found: %s" % recast)
    if not os.access(recast, os.X_OK):
        # Deliberately NOT chmod'ing the checkout's copy: the binary is copied
        # into the work dir below and made executable there, so changing the
        # user's tree would buy nothing.
        info("note: %s is not executable; running an executable copy instead" % recast)
    if not os.path.isfile(fbx2obj):
        fail("FBX2OBJ not found: %s\n"
             "         'make build.utils' cannot install it (the Autodesk FBX SDK URL "
             "returns HTTP 403).\n"
             "         Install the Blender-backed replacement:\n"
             "           bash %s/install_fbx2obj.sh" % (fbx2obj, HERE))

    work = tempfile.mkdtemp(prefix="carla_navmesh_")
    info("work dir: %s" % work)
    try:
        # RecastBuilder is run from the work dir so it writes <stem>.bin there
        # rather than into dist/ (where the stock build.sh leaves it, and where a
        # stale .bin then feeds the next run). Copying the binary in keeps the
        # whole stage self-contained and lets us guarantee the exec bit without
        # touching the checkout.
        local_recast = os.path.join(work, "RecastBuilder")
        shutil.copy2(recast, local_recast)
        os.chmod(local_recast, 0o755)

        # Crosswalks come from the map's OpenDRIVE and are folded into the OBJ.
        crosswalks = None
        if os.path.isfile(xodr):
            py = python_with_carla()
            if py is None:
                info("crosswalks: SKIP, no interpreter can 'import carla'")
            else:
                r = subprocess.run(
                    [py, os.path.join(dist, "get_xodr_crosswalks.py"), "-f", xodr],
                    cwd=work, capture_output=True, text=True)
                if r.returncode != 0:
                    fail("crosswalk extraction failed:\n%s" % r.stderr)
                cw = os.path.join(work, "crosswalks.obj")
                if os.path.isfile(cw) and os.path.getsize(cw) > 0:
                    crosswalks = cw
                    info("crosswalks: extracted from %s" % os.path.basename(xodr))
                else:
                    info("crosswalks: none in this OpenDRIVE")
        else:
            info("crosswalks: SKIP, no OpenDRIVE at %s" % xodr)

        stem = os.path.splitext(os.path.basename(fbx))[0]
        obj = os.path.join(work, "%s.obj" % stem)
        info("source: %s" % stem)
        if stem != map_name:
            # The .bin is installed as <map_name>.bin whatever the FBX is called,
            # so passing another map's geometry silently produces a navmesh over
            # surfaces that are not there. M1 makes the names match for anything
            # imported normally.
            info("WARNING: FBX '%s' does not match map '%s' — make sure this is "
                 "the geometry that was imported as '%s'" % (stem, map_name, map_name))

        r = subprocess.run([fbx2obj, fbx, obj])
        if r.returncode != 0:
            fail("FBX2OBJ failed on %s (exit %d)" % (fbx, r.returncode))
        if not os.path.isfile(obj) or os.path.getsize(obj) == 0:
            fail("FBX2OBJ produced an empty OBJ for %s" % fbx)
        info("      OBJ %.1f MB" % (os.path.getsize(obj) / 1e6))

        if crosswalks:
            r = subprocess.run(
                [sys.executable, os.path.join(dist, "addOBJ.py"), obj, crosswalks],
                cwd=work, capture_output=True, text=True)
            if r.returncode != 0:
                fail("addOBJ.py failed on %s:\n%s" % (stem, r.stderr))

        # --- Tile budget preflight -------------------------------------------
        # Decided here, before anything expensive: RecastBuilder itself never
        # compares the tile count it needs against the one it can address.
        cs = RECAST_DEFAULTS["cell_size"]
        (wmin, wmax), (fmin, fmax) = obj_bounds(obj)
        info("extent: mesh %.0f x %.0f m, road network %.0f x %.0f m"
             % (fmax[0] - fmin[0], fmax[2] - fmin[2],
                wmax[0] - wmin[0], wmax[2] - wmin[2]))

        crop = not args.no_crop
        bmin, bmax = (wmin, wmax) if crop else (fmin, fmax)
        # Y always spans the full mesh: cropping is about plan extent, and
        # clipping height would drop walkable surfaces above or below.
        bmin = [bmin[0], fmin[1], bmin[2]]
        bmax = [bmax[0], fmax[1], bmax[2]]
        # A margin keeps geometry off the outermost tile edge, where Recast
        # trims the walkable surface by the agent radius and border size.
        for i in (0, 2):
            bmin[i] -= 4.0
            bmax[i] += 4.0

        if args.tile_size:
            tile_size = args.tile_size
        else:
            tile_size = fit_tile_size(bmin, bmax, cs)
            if tile_size is None:
                fail("map is too large for a navmesh even at the maximum tile size: "
                     "%.1f x %.1f km of walkable geometry. Reduce the extent."
                     % ((bmax[0] - bmin[0]) / 1000.0, (bmax[2] - bmin[2]) / 1000.0))

        tw, th, tiles, max_tiles, max_polys = tile_lattice(bmin, bmax, cs, tile_size)
        info("tiles: %d x %d = %d (max %d), tile %.1f m, %d polys/tile"
             % (tw, th, tiles, max_tiles, tile_size * cs, max_polys))
        if tiles > max_tiles:
            fail("%d tiles needed but only %d addressable at tile size %d. "
                 "Raise --tile-size (a power of two), or reduce the map extent."
                 % (tiles, max_tiles, tile_size))

        # The stock tool is driven with a .gset only when the defaults would not
        # do, so every map that builds today keeps building exactly as before.
        recast_input = obj
        if tile_size != DEFAULT_TILE_SIZE or crop:
            gset = os.path.join(work, "%s.gset" % stem)
            write_gset(gset, os.path.basename(obj), bmin, bmax, tile_size,
                       RECAST_DEFAULTS["agent_radius"])
            recast_input = gset
            info("build settings: %s (tile %d voxels, bounds %s)"
                 % (os.path.basename(gset), tile_size,
                    "cropped to the road network" if crop else "full mesh"))

        try:
            r = subprocess.run([local_recast, recast_input], cwd=work,
                               timeout=args.timeout)
        except subprocess.TimeoutExpired:
            fail("RecastBuilder exceeded --timeout %ds on %s. Raise it — a wide flat "
                 "terrain mesh dominates the voxel volume." % (args.timeout, stem))
        if r.returncode != 0:
            fail("RecastBuilder failed on %s (exit %d)" % (stem, r.returncode))

        produced = os.path.join(work, "%s.bin" % stem)
        if not os.path.isfile(produced) or os.path.getsize(produced) == 0:
            fail("RecastBuilder wrote no navmesh for %s. The OBJ has geometry but "
                 "nothing walkable came out -- check the FBX has "
                 "Roads_Sidewalk/Roads_Crosswalk nodes, and that the map is "
                 "under Detour's ~6.5 km/side ceiling." % stem)

        # --- Install ---------------------------------------------------------
        nav_dir = os.path.join(map_dir, "Nav")
        os.makedirs(nav_dir, exist_ok=True)
        target = os.path.join(nav_dir, "%s.bin" % map_name)
        if os.path.isfile(target):
            # Replacing a navmesh is the job, but say so and show both sizes: a
            # much smaller replacement is the visible symptom of a build over
            # the wrong or partial geometry.
            info("replacing existing navmesh: %.1f KB -> %.1f KB"
                 % (os.path.getsize(target) / 1e3, os.path.getsize(produced) / 1e3))
        shutil.copy2(produced, target)
        info("OK  %s (%.1f KB)" % (target, os.path.getsize(target) / 1e3))
        info("inspect it: python3 %s/navmesh_to_obj.py --package %s%s --coverage"
             % (HERE, package, "" if map_name == package else " --map %s" % map_name))
    finally:
        if args.keep_temp:
            info("kept work dir: %s" % work)
        else:
            shutil.rmtree(work, ignore_errors=True)


if __name__ == "__main__":
    main()
