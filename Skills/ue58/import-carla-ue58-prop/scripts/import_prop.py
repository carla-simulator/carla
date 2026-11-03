#!/usr/bin/env python3
"""Import FBX meshes into CARLA on UE 5.8 as spawnable props.

    import_prop.py plan     <path> [--tag Static] [--scale 1.0]
    import_prop.py import   <path> [--tag Static] [--scale 1.0] [--size Medium]
    import_prop.py verify   [--name NAME]
    import_prop.py list
    import_prop.py revert   --name NAME

`<path>` is an .fbx or a directory of them. A directory may be organised by tag:

    ~/meshes/
    ├── Static/Bench.fbx
    └── Building/Kiosk.fbx

which mirrors the destination, /Game/Carla/Static/<Tag>/<Name>.

Why not Util/Tools/Import.py
----------------------------
That is a map pipeline props were bolted onto: `generate_json_package` only ever
auto-detects maps and hardcodes `'props': []`, so props require a hand-written
package JSON; it ignores commandlet exit codes on POSIX; and it registers props in
a `*.Package.json`, which on ue58 nothing reads (see below). This drives the
editor directly instead.

Registration on ue58 differs from UE4
-------------------------------------
`UCarlaBlueprintRegistry::LoadPropDefinitions`, which scans Content/ for
`*.Package.json`, has **zero callers** on ue58-dev -- it is dead code. Props are
loaded by `APropActorFactory` via
`LoadPropParametersArrayFromFile("PropParameters.json", ...)`, i.e. from
Content/Carla/Config/PropParameters.json with key "Props" and fields
Name / Mesh / Size. Writing a `.Package.json` has no effect; verified by doing it
and watching the prop count stay put.
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
EDITOR_SCRIPT = HERE / "editor" / "import_prop_editor.py"
VALID_SIZES = ("Tiny", "Small", "Medium", "Big")


def die(msg: str) -> None:
    sys.exit(f"ERROR {msg}")


def carla_root() -> Path:
    for c in (os.environ.get("CARLA_UE58_ROOT"), os.getcwd(),
              str(Path.home() / "UE58" / "carla")):
        if c and (Path(c) / "CMakePresets.json").is_file() and (Path(c) / "Unreal" / "CarlaUnreal").is_dir():
            return Path(c).resolve()
    die("no ue58 CARLA checkout — export CARLA_UE58_ROOT")


def engine_path(root: Path) -> Path:
    env = os.environ.get("CARLA_UNREAL_ENGINE_PATH")
    if env and (Path(env) / "Engine" / "Build" / "Build.version").is_file():
        return Path(env)
    cache = root / "Build" / os.environ.get("CARLA_PRESET", "Release") / "CMakeCache.txt"
    if cache.is_file():
        for line in cache.read_text().splitlines():
            if line.startswith("CARLA_UNREAL_ENGINE_PATH:"):
                p = Path(line.split("=", 1)[1])
                if (p / "Engine" / "Build" / "Build.version").is_file():
                    return p
    die("no engine — export CARLA_UNREAL_ENGINE_PATH")


def params_json(root: Path) -> Path:
    return root / "Unreal" / "CarlaUnreal" / "Content" / "Carla" / "Config" / "PropParameters.json"


def collect(path: Path, tag: str) -> list[dict]:
    """FBX files with the tag each should be filed under."""
    if path.is_file():
        if path.suffix.lower() != ".fbx":
            die(f"{path} is not an .fbx")
        return [{"fbx": str(path), "name": path.stem, "tag": tag}]
    if not path.is_dir():
        die(f"{path} does not exist")
    out = []
    for fbx in sorted(path.rglob("*.fbx")):
        rel = fbx.relative_to(path)
        # A directory level below the root is read as the tag, matching the
        # /Game/Carla/Static/<Tag>/<Name> destination shape.
        out.append({"fbx": str(fbx), "name": fbx.stem,
                    "tag": rel.parts[0] if len(rel.parts) > 1 else tag})
    if not out:
        die(f"no .fbx under {path}")
    return out


def dest_for(prop: dict, dest_root: str) -> str:
    return f"{dest_root}/{prop['tag']}/{prop['name']}"


def read_params(root: Path) -> dict:
    p = params_json(root)
    if not p.is_file():
        die(f"{p} missing — this is what ue58 actually reads; is the content cloned?")
    return json.loads(p.read_text())


def blueprint_id(name: str) -> str:
    return f"static.prop.{name.lower()}"


def cmd_plan(args) -> None:
    root = carla_root()
    eng = engine_path(root)
    props = collect(Path(args.path).expanduser(), args.tag)
    data = read_params(root)
    existing = {e.get("Name") for e in data.get("Props", [])}

    print(f"checkout : {root}")
    print(f"engine   : {eng}")
    print(f"registry : {params_json(root)}  ({len(data.get('Props', []))} props)")
    print(f"scale    : {args.scale}   (FBX unit metadata is often absent/wrong)")
    print()
    for p in props:
        dest = dest_for(p, args.dest_root)
        clash = " <-- ALREADY REGISTERED, will be replaced" if p["name"] in existing else ""
        print(f"  {Path(p['fbx']).name}")
        print(f"      -> asset      {dest}/{p['name']}")
        print(f"      -> blueprint  {blueprint_id(p['name'])}{clash}")
    print()
    if not args.dest_root.startswith("/Game/Carla/"):
        print("  WARNING destination is outside /Game/Carla/. The asset imports and works")
        print("  WARNING in the editor, but a packaged server resolves content only under")
        print("  WARNING /Game/Carla/..., so keep props there to ship them.")
    print("  size is derived from the imported bounds unless --size is given:")
    print("      <=0.5 m Tiny | <=2 m Small | <=8 m Medium | else Big")
    print("\n(plan only — nothing changed)")


def run_editor(root: Path, eng: Path, job: dict, verbose: bool) -> dict:
    with tempfile.TemporaryDirectory() as tmp:
        job_path = Path(tmp) / "job.json"
        res_path = Path(tmp) / "result.json"
        job["result"] = str(res_path)
        job_path.write_text(json.dumps(job, indent=2))
        cmd = [str(eng / "Engine" / "Binaries" / "Linux" / "UnrealEditor"),
               str(root / "Unreal" / "CarlaUnreal" / "CarlaUnreal.uproject"),
               "-run=pythonscript", f"-Script={EDITOR_SCRIPT}",
               "-unattended", "-nullrhi", "-stdout"]
        env = dict(os.environ, CARLA_PROP_JOB=str(job_path))
        print(f"[prop] {' '.join(cmd)}")
        print("[prop] the editor commandlet takes ~30-60 s to boot the project")
        # Log to a FILE, not a pipe. With stdout=PIPE this call can hang long
        # after the editor has exited: a grandchild (shader worker, crash
        # reporter) inherits the write end, so the pipe never reaches EOF and
        # subprocess.run waits on a process that is already gone. A file also
        # survives the run, so the log is still there to read afterwards.
        log_path = Path(tempfile.gettempdir()) / "carla_ue58_prop_editor.log"
        print(f"[prop] editor log {log_path}")
        with open(log_path, "w") as log:
            try:
                proc = subprocess.run(cmd, env=env,
                                      stdout=None if verbose else log,
                                      stderr=subprocess.STDOUT, text=True,
                                      timeout=1800.0)
                rc_editor = proc.returncode
            except subprocess.TimeoutExpired:
                rc_editor = None
                print("[prop] editor still running after 1800 s — killed it")
        # The commandlet exits 0 on success and 255 on a Python error; its stdout
        # carries no results, so the result file is the channel. `print()` and
        # `unreal.log()` from the commandlet do not reach the log, but
        # `LogPython: Error` does.
        if not res_path.is_file():
            if not verbose and log_path.is_file():
                for line in log_path.read_text(errors="replace").splitlines():
                    if "LogPython: Error" in line or "Fatal" in line:
                        print("  " + line.strip()[:200])
            die(f"the editor produced no result file (exit {rc_editor}); "
                f"read {log_path}")
        result = json.loads(res_path.read_text())
        if rc_editor not in (0, None):
            print(f"[prop] WARNING commandlet exited {rc_editor}")
        return result


def cmd_import(args) -> None:
    root = carla_root()
    eng = engine_path(root)
    props = collect(Path(args.path).expanduser(), args.tag)
    if args.size and args.size not in VALID_SIZES:
        die(f"--size must be one of {', '.join(VALID_SIZES)}")

    job = {"scale": args.scale, "combine": not args.no_combine,
           "collision": not args.no_collision, "props": []}
    for p in props:
        job["props"].append({"fbx": p["fbx"], "dest": dest_for(p, args.dest_root),
                             "scale": args.scale, "size": args.size})

    result = run_editor(root, eng, job, args.verbose)
    print(f"[prop] engine {result.get('engine')}")

    imported = []
    for entry in result.get("imported", []):
        mesh = entry.get("mesh")
        name = Path(entry["fbx"]).stem
        if not mesh:
            print(f"  FAIL {name}: no StaticMesh produced "
                  f"({len(entry.get('assets', []))} asset(s) created)")
            continue
        d = mesh["dimensions_m"]
        print(f"  PASS {name}: {mesh['path']}")
        print(f"       {d[0]} x {d[1]} x {d[2]} m  -> size {mesh['size']}  "
              f"lods {mesh['num_lods']} materials {mesh['materials']}")
        # A prop bigger than a bus is almost always missing unit conversion.
        if mesh["longest_m"] > 30:
            print(f"       WARNING longest dimension {mesh['longest_m']} m — that is")
            print(f"       WARNING building-scale. The FBX probably carries no usable unit")
            print(f"       WARNING metadata; re-import with --scale 0.01 (or the right factor).")
        imported.append((name, mesh))
    for err in result.get("errors", []):
        print(f"  FAIL {err.get('fbx')}")
        if args.verbose:
            print(err.get("traceback", ""))
    if not imported:
        die("nothing imported")

    if args.no_register:
        print("[prop] --no-register: not touching PropParameters.json")
        return

    path = params_json(root)
    backup = path.with_suffix(".json.bak")
    shutil.copy2(path, backup)
    data = json.loads(path.read_text())
    before = len(data.get("Props", []))
    data.setdefault("Props", [])
    for name, mesh in imported:
        data["Props"] = [e for e in data["Props"] if e.get("Name") != name]
        data["Props"].append({"Name": name, "Mesh": mesh["path"], "Size": mesh["size"]})
    path.write_text(json.dumps(data, indent=2))
    print(f"[prop] registered in {path.name}: {before} -> {len(data['Props'])} props "
          f"(backup {backup.name})")
    print("[prop] RESTART the server/editor — definitions are read once at startup")
    for name, _ in imported:
        print(f"[prop]   then: {blueprint_id(name)}")


def cmd_list(args) -> None:
    root = carla_root()
    data = read_params(root)
    props = data.get("Props", [])
    print(f"{params_json(root)}  ({len(props)} props)")
    non_carla = []
    for e in sorted(props, key=lambda x: x.get("Name", "")):
        mesh = e.get("Mesh", "")
        mark = "" if mesh.startswith("/Game/Carla/") else "  <-- outside /Game/Carla"
        if mark:
            non_carla.append(e.get("Name"))
        print(f"  {e.get('Name',''):28} {e.get('Size',''):7} {mesh}{mark}")
    if non_carla:
        print(f"\n{len(non_carla)} prop(s) outside /Game/Carla will not resolve on a "
              f"packaged server: {', '.join(non_carla)}")


def cmd_verify(args) -> None:
    root = carla_root()
    data = read_params(root)
    names = [args.name] if args.name else [e["Name"] for e in data.get("Props", [])]
    registered = {e.get("Name"): e for e in data.get("Props", [])}
    rc = 0
    content = root / "Unreal" / "CarlaUnreal" / "Content"

    for name in names if args.name else names[:0] or names:
        entry = registered.get(name)
        if entry is None:
            print(f"  FAIL {name} is not in PropParameters.json")
            rc = 1
            continue
        print(f"  PASS {name} registered: size={entry.get('Size')} mesh={entry.get('Mesh')}")
        # /Game/X/Y -> Content/X/Y.uasset
        rel = entry.get("Mesh", "").split(".")[0].replace("/Game/", "")
        if rel and (content / f"{rel}.uasset").is_file():
            print(f"  PASS asset on disk: Content/{rel}.uasset")
        else:
            print(f"  FAIL asset missing on disk: Content/{rel}.uasset")
            rc = 1
        if not entry.get("Mesh", "").startswith("/Game/Carla/"):
            print("  WARN outside /Game/Carla — editor only, will not resolve in a package")

    try:
        import carla  # noqa: PLC0415
    except Exception:
        print("  INFO no carla module — skipped the live check")
        sys.exit(rc)
    try:
        client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                             int(os.environ.get("CARLA_PORT", 2000)))
        client.set_timeout(10.0)
        world = client.get_world()
    except Exception:
        print("  INFO no server — skipped the live check "
              "(run-carla-ue58-server, then re-verify)")
        sys.exit(rc)
    library = world.get_blueprint_library()
    for name in names:
        bid = blueprint_id(name)
        found = [b.id for b in library.filter(bid)]
        if bid in found:
            bp = library.find(bid)
            print(f"  PASS {bid} in the blueprint library "
                  f"(mesh_path={bp.get_attribute('mesh_path').as_str()})")
            if args.spawn:
                spawn = world.get_map().get_spawn_points()[0]
                actor = world.try_spawn_actor(
                    bp, carla.Transform(spawn.location + carla.Location(z=2.0), spawn.rotation))
                if actor:
                    e = actor.bounding_box.extent
                    print(f"  PASS spawned id={actor.id} "
                          f"bbox {2*e.x:.1f} x {2*e.y:.1f} x {2*e.z:.1f} m")
                    if not actor.semantic_tags:
                        print("  WARN semantic_tags empty — imported props carry no")
                        print("  WARN segmentation tag until their materials are tagged;")
                        print("  WARN ue58 has no GenerateTaggedMaterialsRegistry commandlet")
                    actor.destroy()
                else:
                    print(f"  FAIL {bid} did not spawn")
                    rc = 1
        else:
            print(f"  FAIL {bid} not in the blueprint library — restart the server; "
                  "definitions load once at startup")
            rc = 1
    sys.exit(rc)


def cmd_revert(args) -> None:
    root = carla_root()
    path = params_json(root)
    data = json.loads(path.read_text())
    entry = next((e for e in data.get("Props", []) if e.get("Name") == args.name), None)
    if entry is None:
        die(f"{args.name} is not registered")
    content = root / "Unreal" / "CarlaUnreal" / "Content"
    rel = entry.get("Mesh", "").split(".")[0].replace("/Game/", "")
    asset_dir = (content / rel).parent

    print(f"will remove registry entry {args.name} ({entry.get('Mesh')})")
    print(f"will delete asset directory Content/{asset_dir.relative_to(content)}")
    if not args.yes:
        print("\n(dry run — pass --yes to do it)")
        return
    shutil.copy2(path, path.with_suffix(".json.bak"))
    data["Props"] = [e for e in data["Props"] if e.get("Name") != args.name]
    path.write_text(json.dumps(data, indent=2))
    print(f"unregistered ({len(data['Props'])} props left)")
    if asset_dir.is_dir() and content in asset_dir.parents:
        shutil.rmtree(asset_dir)
        print(f"deleted Content/{asset_dir.relative_to(content)}")
    print("RESTART the server/editor for the change to take effect")


def main() -> None:
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--dest-root", default="/Game/Carla/Static",
                        help="package path root (default /Game/Carla/Static; keep it under "
                             "/Game/Carla to stay package-resolvable)")
    common.add_argument("--tag", default="Static", help="subfolder/tag (default Static)")
    common.add_argument("--scale", type=float, default=1.0,
                        help="import_uniform_scale; use 0.01 for centimetre-authored FBX")
    common.add_argument("--verbose", action="store_true")

    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    p = sub.add_parser("plan", parents=[common]); p.add_argument("path"); p.set_defaults(func=cmd_plan)
    p = sub.add_parser("import", parents=[common])
    p.add_argument("path")
    p.add_argument("--size", choices=VALID_SIZES, help="override the derived size")
    p.add_argument("--no-register", action="store_true", help="import without touching the registry")
    p.add_argument("--no-combine", action="store_true", help="do not combine meshes")
    p.add_argument("--no-collision", action="store_true", help="do not auto-generate collision")
    p.set_defaults(func=cmd_import)
    p = sub.add_parser("verify")
    p.add_argument("--name")
    p.add_argument("--spawn", action="store_true", help="also spawn and destroy it")
    p.set_defaults(func=cmd_verify)
    sub.add_parser("list").set_defaults(func=cmd_list)
    p = sub.add_parser("revert"); p.add_argument("--name", required=True)
    p.add_argument("--yes", action="store_true"); p.set_defaults(func=cmd_revert)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
