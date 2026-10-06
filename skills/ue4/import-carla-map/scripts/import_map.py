#!/usr/bin/env python3
"""Import an FBX+XODR map into a CARLA source build, and verify the cooked
artifacts.

The map's source directory is an argument: pass the path the user named. It can
be anywhere on disk.

Handles both variants of the same pipeline:
  * standard map  — one <name>.fbx + <name>.xodr
  * large map     — many <name>_Tile_X_Y.fbx + one <name>.xodr

Usage:
    # a directory holding the map's files (or a single .xodr / .fbx in it)
    python3 import_map.py <map-dir> --package MyTown
    python3 import_map.py <map-dir>/MyTown.xodr --package MyTown

    # large map: bound editor memory by importing tiles in size-limited batches
    python3 import_map.py <map-dir> --package BigCity --tile-size 1000 --batch-size 200

The map's files are never modified in place. Import.py copies the .fbx/.xodr into
Util/DockerUtils/dist/ for the navmesh stage and removes the copies afterwards;
the originals are only read. Import.py finds packages by walking
`<CARLA_ROOT>/Import` for `*.json`, so that is the one thing written there: a
`<package>.json` naming the .fbx/.xodr by absolute path, removed again once the
import finishes (`--keep-json` to leave it). Loose .fbx/.xodr in that tree are
not discovered, only jsons are.

We generate the json ourselves (rather than relying on Import.py's
auto-generation) so tile_size and use_carla_materials are explicit and the
package name is fixed by the json filename.

Import.py is invoked DIRECTLY, not through `make import`. Two reasons, both
verified against this checkout:
  * `make import` is not build-free — `import: CarlaUE4Editor PythonAPI`
    (Util/BuildTools/Linux.mk) relinks LibCarla, the UE4 plugins, the editor and
    the PythonAPI wheel, and fetches plugins over the network, before importing
    anything. This skill checks for a built editor instead of rebuilding one.
  * batch size cannot reach Import.py through make: Import.sh's getopt knows
    `--batch`, not `--batch-size`, so `ARGS=--batch-size=N` is rejected and every
    argument is dropped; and `--batch` is then discarded by Import.py's own
    parse_known_args, which only defines `--batch-size`. Called directly, the
    flag works.

No env manager is assumed: CARLA_UE4_ROOT resolves from the env var, else $PWD
if it is a checkout, else a path-derived guess. The interpreter that runs
Import.py must `import carla` (it cooks the Traffic Manager graph in-process);
this script resolves and verifies one before starting. UE4_ROOT must be exported
— Import.py reads it to find the editor.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

UPROJECT_REL = Path("Unreal/CarlaUE4/CarlaUE4.uproject")
TILE_RE = re.compile(r"^(?P<name>.+)_Tile_\d+_\d+$")

# Stamped into every package json we write, so we can tell ours from a
# hand-written one and never delete someone else's file (see write_package).
MARKER_KEY = "_written_by"
MARKER = "carla-agentic-tools/import-carla-map"

# CARLA's hard ceiling for a large map's tile edge; Import.py's own fallback is
# this value (Util/BuildTools/Import.py, tile_size = 2000).
MAX_TILE_SIZE_M = 2000


def resolve_carla_root() -> Path:
    """CARLA_UE4_ROOT env > $PWD if a checkout > path-derived guess."""
    env = os.environ.get("CARLA_UE4_ROOT")
    if env and (Path(env) / UPROJECT_REL).is_file():
        return Path(env)
    if (Path.cwd() / UPROJECT_REL).is_file():
        return Path.cwd()
    guess = Path(__file__).resolve().parents[4]  # skills/<name>/scripts -> repo -> ...
    if (guess / UPROJECT_REL).is_file():
        return guess
    sys.exit(
        "error: could not resolve CARLA_UE4_ROOT — export it to your carla "
        "checkout, or run from inside one."
    )


def collect_source(src: Path, prefer_tiled: bool = False):
    """From a directory or a single .fbx/.xodr, return (source_dir, xodr_path,
    fbx_paths, map_name, is_tiled). Fails loudly on the naming rules Import.py
    enforces: the .xodr and .fbx share a name root, tiles are
    <name>_Tile_X_Y.fbx.

    A RoadRunner "Export to Tiles" + "Export Individual Tiles" run emits BOTH a
    combined <name>.fbx and the per-tile files, so both present is a normal
    export, not an error. The combined mesh wins by default (simpler path, and
    the import builds a navmesh for it); prefer_tiled picks the tiles."""
    src = src.expanduser().resolve()
    if src.is_file():
        source_dir = src.parent
    elif src.is_dir():
        source_dir = src
    else:
        sys.exit(f"error: no such file or directory: {src}")

    xodrs = sorted(source_dir.glob("*.xodr"))
    if len(xodrs) == 0:
        sys.exit(f"error: no .xodr in {source_dir} — a CARLA map needs an OpenDRIVE file")
    if len(xodrs) > 1:
        sys.exit(
            f"error: {len(xodrs)} .xodr files in {source_dir} "
            f"({', '.join(p.name for p in xodrs)}) — import one map at a time"
        )
    xodr = xodrs[0]
    map_name = xodr.stem

    single = source_dir / f"{map_name}.fbx"
    tiles = sorted(
        p for p in source_dir.glob("*.fbx")
        if (m := TILE_RE.match(p.stem)) and m.group("name") == map_name
    )

    if single.is_file() and tiles:
        if prefer_tiled:
            print(f"[import] {single.name} and {len(tiles)} tiles both present "
                  f"— using the tiles (--tiled)")
            return source_dir, xodr, tiles, map_name, True
        print(f"[import] {single.name} and {len(tiles)} tiles both present "
              f"— using the combined mesh; pass --tiled for the large-map path")
        return source_dir, xodr, [single], map_name, False
    if single.is_file():
        if prefer_tiled:
            sys.exit(
                f"error: --tiled given but no '{map_name}_Tile_X_Y.fbx' files in "
                f"{source_dir} — only the combined {single.name} is there"
            )
        return source_dir, xodr, [single], map_name, False
    if tiles:
        return source_dir, xodr, tiles, map_name, True

    # Neither matched: name the mismatch precisely — this is the #1 import failure.
    stray = sorted(p.name for p in source_dir.glob("*.fbx"))
    sys.exit(
        f"error: no .fbx matching the .xodr '{map_name}'.\n"
        f"  A standard map needs '{map_name}.fbx'.\n"
        f"  A large map needs '{map_name}_Tile_X_Y.fbx' tiles.\n"
        f"  Found instead: {stray or 'no .fbx files'}\n"
        f"  The .fbx and .xodr MUST share the same name root."
    )


def is_ours(json_path: Path) -> bool:
    """Did this script write that json? Only then may we overwrite or delete it."""
    try:
        return json.loads(json_path.read_text()).get(MARKER_KEY) == MARKER
    except (OSError, ValueError):
        return False


def write_package(import_dir: Path, package: str, map_name: str, xodr: Path,
                  fbx_paths, is_tiled: bool, tile_size: int, carla_materials: bool,
                  force: bool = False):
    """Write <package>.json into the folder Import.py walks. The json filename
    fixes the package name; Import.py reads it verbatim.

    Every path in it is absolute: Import.py resolves them with
    os.path.join(<json's dir>, <value>), which returns an absolute value
    unchanged, so the map is read from wherever it lives.

    A user's own package json of the same name is NOT overwritten: this script
    also deletes the json when it finishes, so clobbering one would destroy it."""
    import_dir.mkdir(parents=True, exist_ok=True)
    json_path = import_dir / f"{package}.json"
    if json_path.is_file() and not is_ours(json_path) and not force:
        sys.exit(
            f"error: {json_path} already exists and was not written by this skill.\n"
            f"  Importing would overwrite it, and this script removes the json when\n"
            f"  it finishes — so that file would be lost.\n"
            f"  Choose another --package name, move that json aside, or pass --force."
        )

    map_entry = {
        "name": map_name,
        "xodr": str(xodr),
        "use_carla_materials": carla_materials,
    }
    if is_tiled:
        map_entry["tile_size"] = tile_size
        map_entry["tiles"] = [str(p) for p in fbx_paths]
    else:
        # A standard map uses "source", NOT "tiles" — Import.py switches on the
        # presence of "tiles" to take the batched large-map path.
        map_entry["source"] = str(fbx_paths[0])

    payload = {"maps": [map_entry], "props": [], MARKER_KEY: MARKER}
    json_path.write_text(json.dumps(payload, indent=3))
    return json_path


def resolve_client_python() -> str:
    """Return an interpreter that can `import carla`, or exit saying how to get
    one. Import.py does `import carla` at module scope and calls
    carla.Map().cook_in_memory_map() for the Traffic Manager graph, so without
    the wheel it dies on its first line (M4) — a second's check here saves an
    editor boot. This interpreter is the one we then run Import.py with, so what
    is verified is what is used.

    No environment manager is assumed: whatever is active wins, then this
    script's own interpreter."""
    candidates = [shutil.which("python3"), sys.executable]
    for py in candidates:
        if py and subprocess.run([py, "-c", "import carla"],
                                 capture_output=True).returncode == 0:
            print(f"[import] client python: {py} (imports carla)")
            return py
    sys.exit(
        "error: no interpreter here can `import carla`, and "
        "Util/BuildTools/Import.py imports it on its first line (M4).\n"
        f"  Tried: {', '.join(p for p in candidates if p)}\n"
        "  Activate the environment holding the CARLA client wheel (built by\n"
        "  build-carla-ue4 step 04), or set CARLA_ENV_ACTIVATE to its activate\n"
        "  script, then re-run."
    )


def run_import(carla_root: Path, py: str, batch_size, is_tiled: bool):
    """Run Import.py directly — NOT `make import`. See the module docstring: the
    make target rebuilds half the project first, and drops the batch-size flag on
    the floor. Import.py locates the checkout from its own path and needs only
    UE4_ROOT from the environment."""
    if not os.environ.get("UE4_ROOT"):
        sys.exit(
            "error: UE4_ROOT is unset — Import.py reads it to find the editor "
            "(os.environ['UE4_ROOT']).\n"
            "  Export it to your built CarlaUnreal UE 4.26 fork and re-run."
        )
    script = carla_root / "Util/BuildTools/Import.py"
    if not script.is_file():
        sys.exit(f"error: {script} not found — is CARLA_UE4_ROOT really a checkout?")

    cmd = [py, str(script)]
    if is_tiled and batch_size:
        cmd += ["--batch-size", str(batch_size)]
    print(f"[import] running: {' '.join(cmd)}  (cwd={carla_root})", flush=True)
    proc = subprocess.run(cmd, cwd=str(carla_root))
    if proc.returncode != 0:
        sys.exit(
            f"error: Import.py exited {proc.returncode}. Read the FIRST "
            f"error in the output, not the last — the editor log scrolls past it."
        )


def verify_artifacts(carla_root: Path, package: str, map_name: str, need_nav: bool,
                     n_tiles: int = 0):
    """Import.py can exit 0 having produced nothing — on posix it launches every
    editor commandlet with subprocess.call and discards the status. Confirm the
    real cooked outputs by path, size the .umap so an empty import is caught, and
    for a large map require one level per tile, not just the streaming shell."""
    base = carla_root / "Unreal/CarlaUE4/Content" / package / "Maps" / map_name
    ok = True

    umaps = list(base.glob("*.umap")) if base.is_dir() else []
    if umaps:
        biggest = max(umaps, key=lambda p: p.stat().st_size)
        kb = biggest.stat().st_size / 1024
        # Floor for "the level asset holds something": an empty ue4 level
        # serialises to ~2 KB of header, a real imported map to hundreds.
        if kb < 4:
            print(f"FAIL  {biggest} is only {kb:.1f} KB — the import produced an empty level")
            ok = False
        else:
            print(f"PASS  {len(umaps)} level asset(s) under {base.relative_to(carla_root)} "
                  f"(largest {kb:.0f} KB)")
    else:
        print(f"FAIL  no .umap under {base} — the map was not imported")
        ok = False

    if n_tiles:
        # A tiled import that produced only the base level is a silent half
        # failure: the map loads and is empty. Import.py writes one
        # <name>_Tile_X_Y.umap per tile, plus TilesInfo.txt.
        tiles = [p for p in umaps if TILE_RE.match(p.stem)]
        if len(tiles) == n_tiles:
            print(f"PASS  {len(tiles)} tile level(s), one per imported tile")
        else:
            print(f"FAIL  {len(tiles)} tile level(s) under {base.relative_to(carla_root)}, "
                  f"expected {n_tiles} — the tiled import did not finish")
            ok = False
        info = base / "TilesInfo.txt"
        if info.is_file():
            print(f"PASS  TilesInfo.txt: {info.relative_to(carla_root)}")
        else:
            print("FAIL  no TilesInfo.txt — the streaming setup was not written")
            ok = False

    nav_path = base / "Nav" / f"{map_name}.bin"
    checks = [
        (base / "OpenDrive" / f"{map_name}.xodr", "OpenDRIVE", True),
        (base / "TM" / f"{map_name}.bin", "Traffic Manager route graph", True),
        (nav_path, "pedestrian navmesh", False),
    ]
    for path, label, required in checks:
        if path.is_file() and path.stat().st_size > 0:
            print(f"PASS  {label}: {path.relative_to(carla_root)}")
        elif path.is_file():
            # A zero-byte artifact is worse than a missing one: build.sh leaves an
            # empty .obj/.bin behind when a stage silently no-ops, and everything
            # downstream then treats the map as "has a navmesh".
            print(f"FAIL  {label} is EMPTY (0 bytes): {path.relative_to(carla_root)}")
            ok = False
        elif required:
            print(f"FAIL  {label} missing: {path}")
            ok = False
        elif label.startswith("pedestrian"):
            # Never fatal. Three different expected states end up here — no
            # FBX2OBJ, a tiled map (which never gets one), and a map past
            # Detour's ~6.5 km/side ceiling (M7) — and the map drives fine in all
            # of them. Step 4 is where a navmesh is actually demanded, and
            # navmesh_to_obj.py exits non-zero on an empty one.
            if need_nav:
                print(f"WARN  {label} absent: {nav_path.relative_to(carla_root)} — "
                      f"FBX2OBJ is installed, so either build it with "
                      f"`build_navmesh.py --package {package}` or the map exceeds "
                      f"Detour's ~6.5 km/side ceiling (M7). Vehicles are unaffected.")
            else:
                print(f"WARN  {label} absent: {nav_path.relative_to(carla_root)} — "
                      f"expected here (no FBX2OBJ, or a large map). Walkers cannot "
                      f"navigate; vehicles are unaffected.")
        else:
            print(f"WARN  {label} absent: {path.relative_to(carla_root)}")

    cfg = carla_root / "Unreal/CarlaUE4/Content" / package / "Config" / f"{package}.Package.json"
    if cfg.is_file():
        print(f"PASS  package registered: {cfg.relative_to(carla_root)}")
    else:
        print(f"FAIL  package config missing: {cfg} — the map won't list on the server")
        ok = False

    return ok, base


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("source", help="the directory holding the map's .fbx/.xodr, or one of those "
                                   "files; any path on disk")
    ap.add_argument("--package",
                    help="content package to import into — the folder under Content/ "
                         "holding the map and its Package.json (default: the map name). "
                         "Two packages cannot share a name; never leave it as upstream's "
                         "map_package")
    ap.add_argument("--tiled", action="store_true",
                    help="when the export holds both a combined .fbx and _Tile_X_Y files, "
                         "take the large-map (tiled) path; default is the combined mesh")
    ap.add_argument("--tile-size", type=int, default=1000,
                    help="large maps only: tile edge in metres (default 1000; CARLA max 2000)")
    ap.add_argument("--batch-size", type=int, default=200,
                    help="large maps only: import tiles in batches up to this many MB (bounds editor RAM)")
    ap.add_argument("--no-carla-materials", action="store_true",
                    help="use RoadRunner materials instead of CARLA's road textures")
    ap.add_argument("--json-only", action="store_true",
                    help="write the package json and stop, to inspect it before importing")
    ap.add_argument("--keep-json", action="store_true",
                    help="leave the package json in place afterwards (default: remove it)")
    ap.add_argument("--force", action="store_true",
                    help="overwrite a package json this skill did not write, and skip "
                         "the are-you-sure on re-importing an existing package")
    args = ap.parse_args()

    if not 0 < args.tile_size <= MAX_TILE_SIZE_M:
        sys.exit(f"error: --tile-size {args.tile_size} out of range — CARLA's maximum "
                 f"is {MAX_TILE_SIZE_M} m (Import.py's own fallback); ~1000 is the "
                 f"usual choice.")

    carla_root = resolve_carla_root()
    src = Path(args.source)
    source_dir, xodr, fbx_paths, map_name, is_tiled = collect_source(src, args.tiled)
    package = args.package or map_name

    # Re-importing overwrites the whole package tree. Say so BEFORE the editor
    # boots — and say where it really lands: Content is very commonly a symlink
    # to one content clone shared by every worktree, so this is not necessarily a
    # write inside this checkout.
    content = carla_root / "Unreal/CarlaUE4/Content"
    dest = content / package
    if dest.exists():
        real = dest.resolve()
        where = f"{dest}" if real == dest else f"{dest}\n           -> {real}"
        print(f"[import] WARNING: package '{package}' already exists — this import "
              f"overwrites it,\n           including any Nav/ navmesh built for it: {where}")
        if not args.force:
            print("[import]          (pass --force to silence this, or --package NAME "
                  "to import alongside it)")
    elif content.is_symlink() or content.resolve() != content:
        print(f"[import] note: Content is a symlink — the map lands in "
              f"{content.resolve()},\n         which every checkout sharing that clone "
              f"will see.")

    kind = f"large map, {len(fbx_paths)} tiles" if is_tiled else "standard map"
    print(f"[import] {kind}: '{map_name}' -> package '{package}'")
    print(f"[import] xodr: {xodr.name}")
    for p in fbx_paths:
        print(f"[import] fbx:  {p.name}")

    # Import.py discovers content packages by walking this folder for *.json.
    import_dir = carla_root / "Import"
    json_path = write_package(import_dir, package, map_name, xodr, fbx_paths,
                              is_tiled, args.tile_size, not args.no_carla_materials,
                              args.force)
    print(f"[import] reading from {source_dir}  (package json: {json_path})")

    # Every package json found in that walk is imported by the same run, and a
    # large and a standard map cannot be mixed in one.
    others = [p for p in import_dir.rglob("*.json")
              if p != json_path and p.name != "roadpainter_decals.json"]
    if others:
        print(f"[import] WARNING: {len(others)} other package json under {import_dir} "
              f"(e.g. {others[0].relative_to(import_dir)}) — those packages import too. "
              f"Move them out to import this map alone.")

    if args.json_only:
        print(f"[import] --json-only: run `python3 {carla_root}/Util/BuildTools/Import.py` "
              f"when ready.")
        return

    py = resolve_client_python()
    try:
        run_import(carla_root, py, args.batch_size, is_tiled)
    finally:
        # Only ever remove the json we wrote — is_ours() re-checks, so a file
        # swapped in underneath us survives.
        if not args.keep_json and is_ours(json_path):
            json_path.unlink(missing_ok=True)

    # Navmesh is only produced for standard (single-source) maps: the tiled Recast
    # path is disabled in Import.py, so don't expect a large map to have one.
    need_nav = (not is_tiled) and (carla_root / "Util/DockerUtils/dist/FBX2OBJ").exists()
    ok, base = verify_artifacts(carla_root, package, map_name, need_nav,
                                len(fbx_paths) if is_tiled else 0)

    print()
    if ok:
        print(f"[import] DONE — load it with:  /Game/{package}/Maps/{map_name}/{map_name}")
    else:
        sys.exit("[import] FAILED verification — resolve the FAIL lines above.")


if __name__ == "__main__":
    main()
