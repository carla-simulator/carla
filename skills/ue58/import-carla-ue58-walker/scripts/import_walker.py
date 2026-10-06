#!/usr/bin/env python3
"""Import a pedestrian into CARLA on UE 5.8 as a spawnable walker.

    import_walker.py donors                                  what you can clone from
    import_walker.py export --walker 0015 --out ~/w.fbx      get a rig-conforming FBX
    import_walker.py plan   ~/w.fbx --name MyPed
    import_walker.py import ~/w.fbx --name MyPed [--gen 2] [--id 0099]
    import_walker.py list
    import_walker.py verify --id 0099 [--spawn]
    import_walker.py revert --name MyPed [--yes]

A walker is three things, not one: a SkeletalMesh bound to CARLA's *shared*
pedestrian skeleton, a Blueprint duplicated from a donor walker and repointed at
that mesh, and an entry in WalkerParameters.json. Miss any one and the walker
either never appears or appears and never animates.

`export` exists because the pipeline needs an FBX skinned to CARLA's rig, and
exporting a shipped walker is the only way to obtain one without external art. It
is also the round-trip that validates the whole flow.
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
EDITOR_SCRIPT = HERE / "editor" / "walker_editor.py"

# The shared skeletons. Binding to one of these is what gives a walker CARLA's
# pedestrian animation set; an importer-created skeleton silently gives it none.
SKELETONS = {
    2: "/Game/Carla/Static/Pedestrian/00_GenericComponents/Definitions/"
       "Skel_Pedestrian_G2.Skel_Pedestrian_G2",
    3: "/Game/Carla/Static/Pedestrian/00_GenericComponents/Definitions/"
       "Skel_Pedestrian_G3.Skel_Pedestrian_G3",
}
PED_ROOT = "/Game/Carla/Static/Pedestrian"
BP_ROOT = "/Game/Carla/Blueprints/Walkers"


def die(msg: str) -> None:
    sys.exit(f"ERROR {msg}")


def carla_root() -> Path:
    for c in (os.environ.get("CARLA_UE58_ROOT"), os.getcwd(),
              str(Path.home() / "UE58" / "carla")):
        if c and (Path(c) / "CMakePresets.json").is_file() \
                and (Path(c) / "Unreal" / "CarlaUnreal").is_dir():
            return Path(c).resolve()
    die("no ue58 CARLA checkout — export CARLA_UE58_ROOT")


def engine(root: Path) -> Path:
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


def params_path(root: Path) -> Path:
    return (root / "Unreal" / "CarlaUnreal" / "Content" / "Carla" / "Config"
            / "WalkerParameters.json")


def read_params(root: Path) -> dict:
    p = params_path(root)
    if not p.is_file():
        die(f"{p} missing — is the content cloned?")
    return json.loads(p.read_text())


def content(root: Path) -> Path:
    return root / "Unreal" / "CarlaUnreal" / "Content"


def pkg_to_file(root: Path, pkg: str) -> Path:
    """/Game/X/Y.Y -> <Content>/X/Y.uasset"""
    return content(root) / (pkg.split(".")[0].replace("/Game/", "") + ".uasset")


def run_editor(root: Path, eng: Path, job: dict, verbose: bool) -> dict:
    """Drive a FULL editor, not `-run=pythonscript`.

    Skeletal-mesh export asserts in a commandlet (SkinnedMeshComponent.cpp:4987)
    and takes the process down with SIGSEGV, under -nullrhi and -RenderOffScreen
    alike. `-ExecutePythonScript=` is also the reliable spelling: `-ExecCmds="py
    \"path\""` mangles under nested quotes and silently runs nothing.
    """
    with tempfile.TemporaryDirectory() as tmp:
        job_path = Path(tmp) / "job.json"
        res_path = Path(tmp) / "result.json"
        job["result"] = str(res_path)
        job_path.write_text(json.dumps(job, indent=2))
        cmd = [str(eng / "Engine" / "Binaries" / "Linux" / "UnrealEditor"),
               str(root / "Unreal" / "CarlaUnreal" / "CarlaUnreal.uproject"),
               "-vulkan", f"-ExecutePythonScript={EDITOR_SCRIPT}"]
        env = dict(os.environ, CARLA_WALKER_JOB=str(job_path))
        env.setdefault("DISPLAY", ":1")
        print(f"[walker] {' '.join(cmd)}")
        print(f"[walker] full editor (DISPLAY={env['DISPLAY']}); ~60-90 s to boot")
        # The editor log goes to a FILE, never a pipe. With stdout=PIPE this call
        # can hang long after the editor has exited: a grandchild (shader worker,
        # crash reporter) inherits the write end, so the pipe never reaches EOF
        # and subprocess.run waits on a process that is already gone.
        log_path = Path(tempfile.gettempdir()) / "carla_ue58_walker_editor.log"
        print(f"[walker] editor log {log_path}")
        with open(log_path, "w") as log:
            try:
                proc = subprocess.run(cmd, env=env,
                                      stdout=None if verbose else log,
                                      stderr=subprocess.STDOUT, text=True,
                                      timeout=1800.0)
                rc_editor = proc.returncode
            except subprocess.TimeoutExpired:
                rc_editor = None
                print("[walker] editor still running after 1800 s — killed it")
        if not res_path.is_file():
            die(f"the editor wrote no result (exit {rc_editor}) — read {log_path}")
        result = json.loads(res_path.read_text())
        if result.get("error"):
            last = result["steps"][-2]["step"] if len(result["steps"]) > 1 else "?"
            print(f"[walker] editor failed after step {last!r}")
            print(result["error"] if verbose else result["error"].splitlines()[-1])
            sys.exit(1)
        if rc_editor not in (0, None):
            print(f"[walker] WARNING editor exited {rc_editor}")
        return result


def cmd_donors(args) -> None:
    root = carla_root()
    data = read_params(root)
    walkers = data.get("Walkers", [])
    print(f"{params_path(root)}  ({len(walkers)} walkers)")
    print(f"\n{'ID':6} {'GEN':4} {'GENDER':8} {'AGE':8} BLUEPRINT")
    for w in sorted(walkers, key=lambda x: x.get("Id", "")):
        print(f"{w.get('Id',''):6} {str(w.get('Generation','')):4} "
              f"{w.get('Gender',''):8} {w.get('Age',''):8} {w.get('Class','')}")
    print("\nshared skeletons (bind to one of these or the walker will not animate):")
    for gen, path in SKELETONS.items():
        ok = pkg_to_file(root, path).is_file()
        print(f"  gen {gen}: {'OK  ' if ok else 'MISSING'} {path}")
    used = {w.get("Id") for w in walkers}
    free = [f"{n:04d}" for n in range(90, 100) if f"{n:04d}" not in used]
    print(f"\nfree ids in 0090-0099: {', '.join(free) or '(none)'}")


def cmd_export(args) -> None:
    root, eng = carla_root(), engine(root := carla_root())
    data = read_params(root)
    donor = next((w for w in data.get("Walkers", [])
                  if w.get("Id") == args.walker), None)
    if donor is None:
        die(f"no walker with Id {args.walker!r} — see `donors`")
    # The registry names a blueprint; the mesh has to be found beside it. Walker
    # meshes live under Static/Pedestrian/<Family>/SK_*.uasset.
    bp_name = donor["Class"].rsplit("/", 1)[1].split(".")[0]      # BP_AfroF01_A_G2
    stem = bp_name[3:] if bp_name.startswith("BP_") else bp_name  # AfroF01_A_G2
    family = stem.rsplit("_", 2)[0] + "_" + stem.rsplit("_", 1)[1]  # AfroF01_G2
    guess = content(root) / "Carla" / "Static" / "Pedestrian" / family / f"SK_{stem}.uasset"
    if not guess.is_file():
        matches = sorted((content(root) / "Carla" / "Static" / "Pedestrian").rglob(
            f"SK_{stem}.uasset"))
        if not matches:
            die(f"cannot find the skeletal mesh for {bp_name} "
                f"(looked for SK_{stem}.uasset under Static/Pedestrian)")
        guess = matches[0]
    rel = guess.relative_to(content(root)).with_suffix("")
    mesh_pkg = f"/Game/{rel.as_posix()}.{rel.name}"
    out = Path(args.out).expanduser().resolve()
    out.parent.mkdir(parents=True, exist_ok=True)
    print(f"[walker] donor  {args.walker}  {donor['Class']}")
    print(f"[walker] mesh   {mesh_pkg}")
    print(f"[walker] fbx    {out}")
    if out.stem != out.stem.replace(" ", ""):
        die("the output filename must not contain spaces")
    result = run_editor(root, eng, {"action": "export", "mesh": mesh_pkg,
                                    "fbx": str(out)}, args.verbose)
    exp = result.get("export", {})
    print(f"[walker] skeleton   {result.get('skeleton')}")
    d = result.get("dimensions_m") or []
    if d:
        print(f"[walker] dimensions {d[0]} x {d[1]} x {d[2]} m")
    print(f"[walker] exported   {exp.get('bytes', 0)} bytes -> {exp.get('path')}")
    print("[walker] NOTE the imported asset takes the FBX FILENAME, so name the file")
    print("[walker]      as you want the walker asset to be called.")


def cmd_plan(args) -> None:
    root = carla_root()
    fbx = Path(args.fbx).expanduser()
    data = read_params(root)
    used = {w.get("Id") for w in data.get("Walkers", [])}
    donor = args.donor or next(iter(sorted(
        (w for w in data["Walkers"] if int(w.get("Generation", 0)) == args.gen),
        key=lambda x: x["Id"])), {}).get("Class", "")
    new_id = args.id or next((f"{n:04d}" for n in range(90, 1000)
                              if f"{n:04d}" not in used), None)
    print(f"checkout : {root}")
    print(f"fbx      : {fbx}  ({'exists' if fbx.is_file() else 'MISSING'})")
    print(f"name     : {args.name}   (asset name comes from --name, not the FBX,")
    print(f"           because the FBX is copied to <name>.fbx before import)")
    print(f"generation: {args.gen}")
    print(f"skeleton : {SKELETONS[args.gen]}")
    print(f"donor BP : {donor}")
    print()
    print(f"  mesh       -> {PED_ROOT}/{args.name}/{args.name}")
    print(f"  blueprint  -> {BP_ROOT}/BP_{args.name}")
    print(f"  registry   -> WalkerParameters.json Id {new_id}")
    print(f"  blueprint  -> walker.pedestrian.{new_id}")
    if args.id and args.id in used:
        print(f"\n  WARNING Id {args.id} is already registered and will be replaced")
    print("\n(plan only — nothing changed)")


def cmd_import(args) -> None:
    root = carla_root()
    eng = engine(root)
    fbx = Path(args.fbx).expanduser().resolve()
    if not fbx.is_file():
        die(f"{fbx} does not exist")
    data = read_params(root)
    walkers = data.get("Walkers", [])
    used = {w.get("Id") for w in walkers}

    donor = None
    if args.donor:
        donor = next((w for w in walkers if w.get("Class") == args.donor
                      or w.get("Id") == args.donor), None)
        if donor is None:
            die(f"donor {args.donor!r} not found — see `donors`")
    else:
        gen_matches = [w for w in walkers if int(w.get("Generation", 0)) == args.gen]
        if not gen_matches:
            die(f"no generation-{args.gen} walker to clone from")
        donor = sorted(gen_matches, key=lambda x: x["Id"])[0]

    new_id = args.id or next((f"{n:04d}" for n in range(90, 1000)
                              if f"{n:04d}" not in used), None)
    if new_id is None:
        die("no free id")

    # The imported asset takes the FBX's basename, so stage a correctly named copy
    # rather than inheriting whatever the file happens to be called.
    with tempfile.TemporaryDirectory() as tmp:
        staged = Path(tmp) / f"{args.name}.fbx"
        shutil.copy2(fbx, staged)
        job = {
            "action": "import",
            "fbx": str(staged),
            "name": args.name,
            "dest": f"{PED_ROOT}/{args.name}",
            "skeleton": SKELETONS[args.gen],
            "donor_blueprint": donor["Class"].split(".")[0],
            "blueprint": f"{BP_ROOT}/BP_{args.name}",
            "scale": args.scale,
            "materials": not args.no_materials,
        }
        print(f"[walker] donor      {donor['Id']}  {donor['Class']}")
        print(f"[walker] skeleton   {SKELETONS[args.gen]}")
        result = run_editor(root, eng, job, args.verbose)

    d = result.get("dimensions_m") or []
    print(f"[walker] mesh       {result.get('mesh_path')}")
    print(f"[walker] skeleton   {result.get('bound_skeleton')}  "
          f"{'SHARED (animations will work)' if result.get('skeleton_shared') else 'PRIVATE'}")
    if d:
        print(f"[walker] dimensions {d[0]} x {d[1]} x {d[2]} m")
        if not 1.0 <= d[2] <= 2.4:
            print(f"[walker] WARNING height {d[2]} m is not human-scale; check --scale")
    print(f"[walker] blueprint  {result.get('blueprint_class')} "
          f"(CDO {result.get('cdo_class')})")

    if args.no_register:
        print("[walker] --no-register: not touching WalkerParameters.json")
        return

    path = params_path(root)
    shutil.copy2(path, path.with_suffix(".json.bak"))
    data = json.loads(path.read_text())
    data["Walkers"] = [w for w in data["Walkers"] if w.get("Id") != new_id]
    entry = {"Id": new_id, "Class": result["blueprint_class"],
             "Gender": args.gender or donor.get("Gender", "Female"),
             "Age": args.age or donor.get("Age", "Adult"),
             "Speeds": donor.get("Speeds", [{"Speed": 0}, {"Speed": 1.7}, {"Speed": 4}]),
             "Generation": args.gen}
    data["Walkers"].append(entry)
    path.write_text(json.dumps(data, indent=2))
    print(f"[walker] registered Id {new_id} ({len(data['Walkers'])} walkers, "
          f"backup {path.with_suffix('.json.bak').name})")
    print("[walker] RESTART the server — definitions load once at startup")
    print(f"[walker]   then: walker.pedestrian.{new_id}")


def cmd_list(args) -> None:
    root = carla_root()
    data = read_params(root)
    walkers = data.get("Walkers", [])
    print(f"{params_path(root)}  ({len(walkers)} walkers)")
    for w in sorted(walkers, key=lambda x: x.get("Id", "")):
        bp_file = pkg_to_file(root, w.get("Class", ""))
        mark = "" if bp_file.is_file() else "   <-- BLUEPRINT MISSING ON DISK"
        print(f"  walker.pedestrian.{w.get('Id',''):6} gen{w.get('Generation','')} "
              f"{w.get('Gender',''):7} {w.get('Age',''):7} {w.get('Class','')}{mark}")


def cmd_verify(args) -> None:
    root = carla_root()
    data = read_params(root)
    entry = next((w for w in data.get("Walkers", []) if w.get("Id") == args.id), None)
    rc = 0
    if entry is None:
        print(f"  FAIL Id {args.id} is not in WalkerParameters.json")
        sys.exit(1)
    print(f"  PASS registered: gen{entry.get('Generation')} {entry.get('Gender')} "
          f"{entry.get('Age')} -> {entry.get('Class')}")
    bp_file = pkg_to_file(root, entry.get("Class", ""))
    if bp_file.is_file():
        print(f"  PASS blueprint on disk: {bp_file.relative_to(content(root))}")
    else:
        print(f"  FAIL blueprint missing: {bp_file}")
        rc = 1

    try:
        import carla  # noqa: PLC0415
    except Exception:
        print("  INFO no carla module — skipped the live check")
        sys.exit(rc)
    try:
        client = carla.Client(os.environ.get("CARLA_HOST", "127.0.0.1"),
                              int(os.environ.get("CARLA_PORT", 2000)))
        client.set_timeout(15.0)
        world = client.get_world()
    except Exception:
        print("  INFO no server — start one (run-carla-ue58-server) and re-verify")
        sys.exit(rc)

    bid = f"walker.pedestrian.{args.id}"
    library = world.get_blueprint_library()
    if bid not in [b.id for b in library.filter(bid)]:
        print(f"  FAIL {bid} not in the blueprint library — restart the server")
        sys.exit(1)
    print(f"  PASS {bid} in the blueprint library")
    if not args.spawn:
        sys.exit(rc)

    bp = library.find(bid)
    # get_random_location_from_navigation() returns points where NO walker
    # spawns -- stock ones included -- so fall back to map spawn points with a
    # small z offset rather than blaming the asset.
    actor = None
    for sp in world.get_map().get_spawn_points()[:12]:
        for dz in (0.5, 1.5, 3.0):
            actor = world.try_spawn_actor(
                bp, carla.Transform(sp.location + carla.Location(z=dz), sp.rotation))
            if actor:
                break
        if actor:
            break
    if actor is None:
        print(f"  FAIL {bid} did not spawn at any of 12 spawn points")
        sys.exit(1)
    e = actor.bounding_box.extent
    print(f"  PASS spawned id={actor.id} bbox {2*e.x:.2f} x {2*e.y:.2f} x {2*e.z:.2f} m")
    # Let the transform settle BEFORE sampling. A just-spawned actor reads back
    # (0,0,0) until the first tick lands, so a start captured immediately makes
    # "moved" the distance from the world origin -- a number in the hundreds of
    # metres that looks like a pass no matter what the walker does.
    for _ in range(20):
        world.wait_for_tick()
    start = actor.get_location()
    actor.apply_control(carla.WalkerControl(
        direction=carla.Vector3D(1, 0, 0), speed=1.4))
    peak = 0.0
    for _ in range(80):
        world.wait_for_tick()
        v = actor.get_velocity()
        peak = max(peak, (v.x * v.x + v.y * v.y + v.z * v.z) ** 0.5)
    moved = start.distance(actor.get_location())
    # Movement is the only proof the mesh is driven by the SHARED skeleton's
    # animation set; a private skeleton yields a walker that registers and stands
    # still. Peak speed is reported alongside because it cannot be faked by a
    # sampling artefact: a frozen walker reads exactly 0.00 m/s.
    # Assert on SPEED, not displacement. Measured on a stock walker: peak
    # 1.40 m/s (exactly the commanded speed) with only 0.08 m of net travel,
    # because a map spawn point is a vehicle bay that can be boxed in by
    # geometry -- the walker treadmills against a wall. Speed proves the
    # mesh/skeleton/anim chain is live; displacement proves only that the spot
    # was open.
    if peak > 0.1:
        print(f"  PASS peak {peak:.2f} m/s under WalkerControl — the shared "
              f"skeleton's animations are driving it (net travel {moved:.2f} m)")
    else:
        print(f"  FAIL peak {peak:.2f} m/s, net travel {moved:.2f} m — the mesh is "
              "not being driven; likely bound to a private skeleton")
        rc = 1
    # In async mode the server ticks on its own; in sync mode nothing advances
    # unless a client ticks it, and this skill deliberately does not take over
    # world settings. Say so rather than reporting a false negative.
    if peak == 0.0 and world.get_settings().synchronous_mode:
        print("  INFO the world is in SYNCHRONOUS mode and this check does not tick it "
              "— run it against an async server, or tick from your own client")
    try:
        print(f"  PASS {len(actor.get_bones().bone_transforms)} bones readable")
    except Exception as exc:
        print(f"  WARN get_bones failed: {str(exc)[:60]}")
    actor.destroy()
    sys.exit(rc)


def cmd_revert(args) -> None:
    root = carla_root()
    path = params_path(root)
    data = json.loads(path.read_text())
    bp_pkg = f"{BP_ROOT}/BP_{args.name}"
    entry = next((w for w in data.get("Walkers", [])
                  if w.get("Class", "").startswith(bp_pkg + ".")), None)
    targets = [content(root) / "Carla" / "Static" / "Pedestrian" / args.name,
               pkg_to_file(root, bp_pkg + f".BP_{args.name}")]
    print(f"registry entry : {entry.get('Id') if entry else '(none)'}")
    for t in targets:
        print(f"will delete    : {t.relative_to(content(root))}"
              f"{'' if t.exists() else '  (absent)'}")
    if not args.yes:
        print("\n(dry run — pass --yes)")
        return
    if entry:
        shutil.copy2(path, path.with_suffix(".json.bak"))
        data["Walkers"] = [w for w in data["Walkers"] if w is not entry]
        path.write_text(json.dumps(data, indent=2))
        print(f"unregistered Id {entry.get('Id')} ({len(data['Walkers'])} left)")
    for t in targets:
        if t.is_dir():
            shutil.rmtree(t)
            print(f"deleted {t.relative_to(content(root))}")
        elif t.is_file():
            t.unlink()
            print(f"deleted {t.relative_to(content(root))}")
    print("RESTART the server for the change to take effect")


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)

    sub.add_parser("donors").set_defaults(func=cmd_donors)
    sub.add_parser("list").set_defaults(func=cmd_list)

    p = sub.add_parser("export")
    p.add_argument("--walker", required=True, help="Id of the walker to export, e.g. 0015")
    p.add_argument("--out", required=True, help="output .fbx (its NAME becomes the asset name)")
    p.add_argument("--verbose", action="store_true")
    p.set_defaults(func=cmd_export)

    for name, fn in (("plan", cmd_plan), ("import", cmd_import)):
        p = sub.add_parser(name)
        p.add_argument("fbx")
        p.add_argument("--name", required=True, help="asset/blueprint name, e.g. MyPed")
        p.add_argument("--gen", type=int, default=2, choices=(2, 3),
                       help="pedestrian generation; picks the shared skeleton")
        p.add_argument("--id", help="registry id (default: first free from 0090)")
        p.add_argument("--donor", help="donor Id or blueprint path to clone")
        p.add_argument("--verbose", action="store_true")
        if name == "import":
            p.add_argument("--scale", type=float, default=1.0)
            p.add_argument("--gender", choices=("Male", "Female"))
            p.add_argument("--age", choices=("Child", "Teenager", "Adult", "Elderly"))
            p.add_argument("--no-materials", action="store_true")
            p.add_argument("--no-register", action="store_true")
        p.set_defaults(func=fn)

    p = sub.add_parser("verify")
    p.add_argument("--id", required=True)
    p.add_argument("--spawn", action="store_true")
    p.set_defaults(func=cmd_verify)

    p = sub.add_parser("revert")
    p.add_argument("--name", required=True)
    p.add_argument("--yes", action="store_true")
    p.set_defaults(func=cmd_revert)

    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
