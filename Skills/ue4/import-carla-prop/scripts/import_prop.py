#!/usr/bin/env python3
"""Import FBX meshes into CARLA as spawnable props.

Two input shapes, one positional argument:

    # a directory: each subdirectory names the semantic tag for what is inside
    python3 import_prop.py ~/meshes/props

    # a single file: state its tag
    python3 import_prop.py ~/meshes/Windmill.fbx --tag Building

The directory form expects the tag as the first level below the path given:

    ~/meshes/props/
    ├── Building/Windmill.fbx
    ├── Static/Bench_Modern.fbx
    └── Dynamic/Crate.fbx

which is the same shape as the destination, `/Game/Carla/Static/<Tag>/<Name>`.
Any directory works — nothing here assumes the checkout's `Import/` folder.

Each prop is imported into the stock CARLA content set, registered in
`Content/Carla/Config/Default.Package.json`, and added to `PropFactory`, after
which it is spawnable exactly like a native prop:

    barrier_bp = bp_lib.find('static.prop.policebarrier')
    world.spawn_actor(barrier_bp, spawn_loc)

`size` is measured from the imported mesh rather than asked for — EPropSize is
defined by physical scale, so the bounding box answers it. `--size` overrides.
`tag` cannot be measured and is never defaulted: a wrong one is labelled `None`
by the segmentation camera with nothing raised anywhere.

Prove the result with `verify_prop.py`, which spawns each prop on a running
server. Assets on disk are not proof of registration.

Why this does not use Util/BuildTools/Import.py
-----------------------------------------------
`make import` is a map-package pipeline props were bolted onto: it imports every
package staged under `Import/`, boots the editor twice (the second pass is a
no-op without maps), discards commandlet exit codes, and derives the asset path
from the FBX filename — wrong for any FBX holding more than one mesh node. This
drives the editor once, directly, and reads back what was created.
"""
from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
EDITOR_SCRIPT = HERE / "editor" / "import_and_register.py"

# EPropSize (Carla/Actor/PropParameters.h). Only used to validate an explicit
# --size; normally the size is measured from the mesh.
SIZES = ("tiny", "small", "medium", "big", "huge")

# The semantic-segmentation folder names ATagger::GetLabelByFolderName actually
# recognises (Carla/Game/Tagger.cpp:29-60). Deliberately NOT the list in
# Docs/tuto_A_add_props.md, which also lists `Vehicles` and `Unlabeled` — neither
# is matched there, and both fall through to CityObjectLabel::None.
TAGS = (
    "Bridge", "Building", "Bicycle", "Bus", "Car", "Dynamic", "Fence", "Ground",
    "GuardRail", "Motorcycle", "Other", "Pedestrian", "Pole", "RailTrack",
    "Rider", "Road", "RoadLine", "SideWalk", "Sky", "Static", "Terrain",
    "TrafficLight", "TrafficSign", "Train", "Truck", "Vegetation", "Wall",
    "Water",
)

# A static mesh .uasset is never this small — even a unit cube is several KB.
# Anything under this means the import wrote a stub rather than geometry.
MIN_UASSET_BYTES = 1024


def die(message: str) -> None:
    sys.exit("ERROR: " + message)


def carla_root() -> Path:
    """Resolve the target checkout: $CARLA_UE4_ROOT, then $PWD, then a path guess."""
    env = os.environ.get("CARLA_UE4_ROOT", "").strip()
    if env:
        return Path(env).resolve()
    cwd = Path.cwd()
    if (cwd / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject").is_file():
        return cwd
    guess = Path(__file__).resolve().parents[4]
    if (guess / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject").is_file():
        return guess
    die(
        "cannot locate a carla checkout.\n"
        "       export CARLA_UE4_ROOT=/path/to/carla, or run from inside one.\n"
        f"       looked in $PWD ({cwd}) and {guess}"
    )
    raise AssertionError("unreachable")


def ue4_editor() -> Path:
    """The editor binary. Building it is the build-carla-ue4 skill's job, not ours."""
    ue4 = os.environ.get("UE4_ROOT", "").strip()
    if not ue4:
        die("UE4_ROOT is unset — export it to your built CarlaUnreal UE 4.26 fork")
    editor = Path(ue4) / "Engine" / "Binaries" / "Linux" / "UE4Editor"
    if not editor.is_file():
        die(f"no UE4Editor at {editor}\n       CARLA is not built — run the build-carla-ue4 skill first")
    return editor


def collect_props(path: Path, tag: str | None, name: str | None) -> list[tuple[Path, str, str]]:
    """Resolve the input into [(fbx, name, tag)].

    A file needs its tag stated. A directory takes each prop's tag from the first
    directory level below it, which is also where the asset will land.
    """
    if path.is_file():
        if path.suffix.lower() != ".fbx":
            die(f"{path.name} is not an .fbx")
        if not tag:
            die(
                f"--tag is required for a single file.\n"
                f"       It sets the semantic segmentation label and cannot be guessed from\n"
                f"       the mesh. One of: {', '.join(TAGS)}"
            )
        return [(path, name or path.stem, tag)]

    if not path.is_dir():
        die(f"no such file or directory: {path}")
    if tag:
        die("--tag applies to a single file; in a directory the subdirectory names are the tags")
    if name:
        die("--name applies to a single file; in a directory the file stems are the names")

    props: list[tuple[Path, str, str]] = []
    loose: list[Path] = []
    bad_tags: dict[str, list[str]] = {}

    for fbx in sorted(path.rglob("*.fbx")):
        relative = fbx.relative_to(path)
        if len(relative.parts) < 2:
            # Directly in the root, so no directory named its tag.
            loose.append(fbx)
            continue
        candidate = relative.parts[0]
        if candidate not in TAGS:
            bad_tags.setdefault(candidate, []).append(fbx.name)
            continue
        props.append((fbx, fbx.stem, candidate))

    if loose:
        die(
            "these FBX files sit directly in {} with no tag directory:\n  {}\n"
            "       Move each into a subdirectory named for its semantic tag, e.g.\n"
            "       {}/Static/{}".format(
                path, "\n  ".join(f.name for f in loose), path, loose[0].name
            )
        )
    if bad_tags:
        listed = "\n  ".join(f"{d}/  ({', '.join(files)})" for d, files in sorted(bad_tags.items()))
        die(
            "these directories are not semantic tags:\n  {}\n"
            "       A directory name that is not in the list below is labelled None by the\n"
            "       segmentation camera with no error raised. Valid: {}".format(
                listed, ", ".join(TAGS)
            )
        )
    if not props:
        die(f"no .fbx files found under {path}")

    # Two props with the same name would shadow each other in the registry —
    # the entry applied last wins (references/props.md P5). Catch it here rather
    # than after an editor boot.
    seen: dict[str, Path] = {}
    for fbx, prop_name, _ in props:
        if prop_name in seen:
            die(
                f"two meshes would both become the prop '{prop_name}':\n"
                f"  {seen[prop_name]}\n  {fbx}\n"
                "       Rename one — same-named props silently override each other."
            )
        seen[prop_name] = fbx

    return props


def warn_if_shared_content(content_dir: Path, root: Path) -> None:
    """Say so out loud when Content/Carla is shared with other checkouts.

    A CARLA content tree is commonly a symlink to one clone shared by every
    worktree. Writing the mesh, the registry and PropFactory there is correct,
    but it is visible from every other checkout at once — worth knowing before,
    not after.
    """
    carla_content = content_dir / "Carla"
    if not carla_content.is_symlink():
        return
    target = carla_content.resolve()
    try:
        target.relative_to(root)
        return  # resolves back inside this checkout: not shared
    except ValueError:
        pass
    print(f"[import] NOTE  Content/Carla -> {target}")
    print("[import]       shared with any other checkout linking it; these props will")
    print("[import]       appear in all of them. --package NAME keeps an import self-contained.")


def run_editor(root: Path, spec: dict, verbose: bool) -> dict:
    """One editor boot: import, read back, measure and register every prop."""
    editor = ue4_editor()
    uproject = root / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject"

    workdir = Path(tempfile.mkdtemp(prefix="carla-prop-"))
    try:
        spec_path = workdir / "spec.json"
        result_path = workdir / "result.json"
        spec["result"] = str(result_path)
        spec_path.write_text(json.dumps(spec, indent=2))

        command = [
            str(editor), str(uproject),
            "-run=pythonscript", f"-Script={EDITOR_SCRIPT}",
            "-unattended", "-nopause", "-nosourcecontrol", "-NoLiveCoding",
        ]
        env = dict(os.environ, CARLA_PROP_SPEC=str(spec_path))

        print(f"[import] booting the editor once for {len(spec['props'])} prop(s)...")
        # Unlike Import.py the exit code is kept, but it is still not sufficient:
        # the editor can exit 0 having logged a fatal import error, so result.json
        # and then the on-disk artifacts are the real gates.
        completed = subprocess.run(
            command, env=env,
            stdout=None if verbose else subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        if not result_path.is_file():
            if not verbose and completed.stdout:
                sys.stderr.write(completed.stdout[-4000:])
            die(
                f"the editor produced no result (exit {completed.returncode}).\n"
                "       It died before the import finished — read the FIRST error above,\n"
                "       not the last. Re-run with --verbose for the full editor log."
            )
        result = json.loads(result_path.read_text())
        if not result.get("ok") and not verbose and completed.stdout:
            sys.stderr.write(completed.stdout[-4000:])
        return result
    finally:
        shutil.rmtree(workdir, ignore_errors=True)


def confirm_on_disk(prop: dict, content_dir: Path) -> str | None:
    """Check the editor's claim against the filesystem. Returns a problem, or None.

    The editor reporting success is evidence; the .uasset being there at the
    registered path, with real content in it, is proof.
    """
    mesh_path = prop.get("mesh_path", "")
    if not mesh_path.startswith("/Game/"):
        return f"registered path {mesh_path!r} is not under /Game/"
    # Object paths are Package.Object; the package is the file on disk.
    package = mesh_path.rsplit(".", 1)[0]
    uasset = content_dir / (package[len("/Game/"):] + ".uasset")
    if not uasset.is_file():
        return f"no asset on disk at {uasset}"
    size = uasset.stat().st_size
    if size < MIN_UASSET_BYTES:
        return f"{uasset} is only {size} B — the import wrote a stub, not geometry"
    prop["uasset"] = str(uasset)
    prop["uasset_kb"] = size // 1024
    return None


def main() -> int:
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("path", help="an .fbx file, or a directory whose subdirectories name the tags")
    parser.add_argument(
        "--tag", choices=TAGS, metavar="TAG",
        help="semantic segmentation tag for a single file, also its content subfolder. "
             "Choices: " + ", ".join(TAGS),
    )
    parser.add_argument("--name", help="prop name for a single file; defaults to the FBX stem")
    parser.add_argument(
        "--size", choices=SIZES,
        help="override the size measured from the mesh: tiny(<mailbox) small(mailbox) "
             "medium(human) big(bus stop) huge(house+)",
    )
    parser.add_argument("--package", help="import into its own package instead of the stock CARLA content set")
    parser.add_argument(
        "--no-factory", action="store_true",
        help="skip PropFactory; the registry entry alone still makes the prop spawnable",
    )
    parser.add_argument("--mesh", help="object path to nominate when an FBX yields several static meshes")
    parser.add_argument("--no-combine", action="store_true", help="import each FBX mesh node as its own asset")
    parser.add_argument("--no-materials", action="store_true", help="skip importing materials and textures")
    parser.add_argument("--verbose", action="store_true", help="stream the editor log instead of capturing it")
    args = parser.parse_args()

    path = Path(args.path).expanduser()
    collected = collect_props(path, args.tag, args.name)

    root = carla_root()
    content_dir = root / "Unreal" / "CarlaUE4" / "Content"

    if args.package:
        content_root = args.package
        registry_json = content_dir / args.package / "Config" / f"{args.package}.Package.json"
        # PropFactory lives in the stock content set; a standalone package is
        # registered through its own .Package.json alone.
        use_factory = False
    else:
        content_root = "Carla"
        registry_json = content_dir / "Carla" / "Config" / "Default.Package.json"
        use_factory = not args.no_factory
        if not (content_dir / "Carla").exists():
            die(
                "Content/Carla is missing — the stock content set is not installed.\n"
                "       Run the build-carla-ue4 skill (it fetches content), or use --package NAME."
            )
        warn_if_shared_content(content_dir, root)

    if args.mesh and len(collected) > 1:
        die("--mesh nominates a mesh within one FBX; import that file on its own")

    spec = {"props": [
        {
            "fbx": str(fbx.resolve()),
            "name": prop_name,
            "tag": tag,
            "size": args.size,          # None -> measured from the mesh
            "destination": f"/Game/{content_root}/Static/{tag}/{prop_name}",
            "registry_json": str(registry_json),
            "factory": use_factory,
            "combine_meshes": not args.no_combine,
            "import_materials": not args.no_materials,
            "import_textures": not args.no_materials,
            "mesh_hint": args.mesh or "",
        }
        for fbx, prop_name, tag in collected
    ]}

    for prop in spec["props"]:
        print(f"[import] {prop['tag']:<12} {prop['name']:<24} {prop['fbx']}")

    result = run_editor(root, spec, args.verbose)

    if result.get("error"):
        print()
        print("[import] FAILED before any prop was imported: " + result["error"])
        return 1

    print()
    failures = 0
    for prop in result.get("props", []):
        if not prop.get("ok"):
            failures += 1
            print(f"[import] FAIL  {prop['name']}: {prop.get('error', 'unknown error')}")
            continue
        problem = confirm_on_disk(prop, content_dir)
        if problem:
            failures += 1
            print(f"[import] FAIL  {prop['name']}: the editor reported success but {problem}")
            continue
        dims = prop.get("dimensions_m") or []
        shape = " x ".join(f"{d:g}" for d in dims) + " m" if dims else "?"
        print(
            f"[import] OK    {prop['blueprint_id']:<32} {shape:<20} "
            f"size={prop['size']} ({prop['size_source']})  {prop['uasset_kb']} KB"
        )

    registry = Path(spec["props"][0]["registry_json"])
    print()
    print(f"[import] registry  {registry}")
    if use_factory:
        print("[import] factory   PropFactory DefinitionsMap updated")

    if failures:
        print()
        print(f"[import] {failures} of {len(result.get('props', []))} prop(s) FAILED")
        return 1

    names = " ".join(f"--name {p['name']}" for p in result["props"])
    print()
    print("[import] Prove they spawn on a running server:")
    print(f"[import]   python3 {HERE / 'verify_prop.py'} {names}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
