#!/usr/bin/env python3
"""Import an FBX pedestrian into CARLA as a spawnable walker.

    python3 import_walker.py ~/models/SK_AfroBoy01_G3.fbx \
        --gender male --age child --speed 0.0,1.1,2.0

The FBX must be skinned to CARLA's GEN3 pedestrian rig (26 `crl_*` bones);
check_input.py runs first and stops the import if it is not, because a walker on a
foreign skeleton imports cleanly and then never moves.

What lands where:

    Content/Carla/Static/Pedestrian/<Name>/Meshes/    SK_<Name>, its materials,
                                                      <Name>_PhysicsAsset
    Content/Carla/Blueprints/Walkers/BP_Walker_<Name>  duplicated from a GEN3 donor
    Content/Carla/Blueprints/Walkers/WalkerFactory     the entry that makes it spawnable

after which it spawns exactly like a native CARLA pedestrian:

    bp = bp_lib.find('walker.pedestrian.0053')
    world.spawn_actor(bp, world.get_random_location_from_navigation())

age, gender and speed are REQUIRED and never inferred. Height is measurable and
so is reported, but a wrong `age` or `gender` is silent: nothing raises, the
attribute is simply wrong for anyone filtering the blueprint library on it.

Two editor boots
----------------
Boot 1 (fast, -run=pythonscript) imports the mesh and builds the blueprint.
Boot 2 (full editor, -ExecutePythonScript) registers it in WalkerFactory.

They are separate because the factory blueprint has to compile, which the
commandlet cannot do, AND because a failed import must never reach the factory.
Boot 2 runs CARLA's own add_walker_to_walker_factory.py — see references, C1.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
BUILD_SCRIPT = HERE / "editor" / "build_walker.py"
INPUT_CHECK = HERE / "check_input.py"

# Registration lives in the CARLA repo, not here: it is the walker counterpart of
# CarlaTools' add_vehicle_to_vehicle_factory.py (PR #9805) and belongs next to it,
# so it is usable without this skill and upstreamable on its own.
REGISTER_SCRIPT_REL = ("Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/"
                       "add_walker_to_walker_factory.py")

GENDERS = ("male", "female", "other")
AGES = ("child", "teenager", "adult", "elderly")

# Stock speed triples (idle, walk, run) in m/s, from the shipped factory entries.
STOCK_SPEEDS = {
    "child": "0.0,1.1,2.0",
    "teenager": "0.0,1.1,2.0",
    "adult": "0.0,1.7,4.0",
    "elderly": "0.0,1.7,4.0",
}
# Default when --speed is omitted: the adult triple, which is also the default of
# CarlaTools' add_walker_to_walker_factory.py, so both agree.
DEFAULT_SPEED = "0.0,1.7,4.0"

# A skeletal-mesh .uasset holding real geometry is never this small.
MIN_UASSET_BYTES = 4096

# How long to wait for boot 2, which loads the default map before running Python.
REGISTER_TIMEOUT_S = 900
BUILD_TIMEOUT_S = 900

# Ids appear in the factory package in one of two shapes, depending on where the
# array lives:
#
#   local variable in GenerateDefinitions -> one DefaultValue text blob:
#       ((Id="0001",Class=...),(Id="0002",...))
#   promoted MEMBER variable              -> serialised CDO data, each id its own
#       length-prefixed FString: <int32 5>"0052"\0
#
# Both are read, and whichever yields more ids wins — so the skill works before and
# after the promotion described in the references (C1).
FACTORY_ID_RE = re.compile(rb'\(Id="(\d{1,8})"')
FACTORY_ID_FSTRING_RE = re.compile(rb'(?=(.{4})(\d{4}\x00))', re.S)


def die(message: str) -> None:
    sys.exit("ERROR: " + message)


def carla_root() -> Path:
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
    ue4 = os.environ.get("UE4_ROOT", "").strip()
    if not ue4:
        die("UE4_ROOT is unset — export it to your built CarlaUnreal UE 4.26 fork")
    editor = Path(ue4) / "Engine" / "Binaries" / "Linux" / "UE4Editor"
    if not editor.is_file():
        die(f"no UE4Editor at {editor}\n"
            "       CARLA is not built — run the build-carla-ue4 skill first")
    return editor


def game_path_to_disk(root: Path, game_path: str) -> Path:
    """/Game/Carla/X -> <root>/Unreal/CarlaUE4/Content/Carla/X.uasset"""
    relative = game_path[len("/Game/"):] if game_path.startswith("/Game/") else game_path
    return root / "Unreal" / "CarlaUE4" / "Content" / (relative + ".uasset")


def factory_ids_on_disk(root: Path, factory_game_path: str) -> list[str]:
    """Count the factory entries by reading the asset, not by asking the editor.

    This is the ground truth the registration gate compares against. The entries
    are readable here in either encoding, which lets the host report a count and
    validate an explicit --id without booting the editor.
    """
    path = game_path_to_disk(root, factory_game_path)
    if not path.is_file():
        die(f"no WalkerFactory at {path} — is CARLA_UE4_ROOT right?")
    data = path.read_bytes()

    # Shape 1: the DefaultValue text blob of a function-local array.
    literal = [m.decode() for m in FACTORY_ID_RE.findall(data)]

    # Shape 2: serialised FStrings of a promoted member array.
    serialised = []
    for match in FACTORY_ID_FSTRING_RE.finditer(data):
        if struct.unpack("<i", match.group(1))[0] == 5:
            serialised.append(match.group(2)[:-1].decode())

    ids = literal if len(set(literal)) >= len(set(serialised)) else serialised
    # De-duplicate while keeping order: a resaved asset can carry a stale copy.
    seen, unique = set(), []
    for entry in ids:
        if entry not in seen:
            seen.add(entry)
            unique.append(entry)
    return unique


def next_free_id(ids: list[str]) -> str:
    """Ids are 4-digit and dense: the next one is max + 1, zero-padded."""
    numeric = [int(i) for i in ids if i.isdigit()]
    return "%04d" % ((max(numeric) + 1) if numeric else 1)


def sanitise_name(raw: str) -> str:
    """A UE object name: keep it to what the editor and the id will accept."""
    name = re.sub(r"[^A-Za-z0-9_]", "_", raw).strip("_")
    if not name:
        die(f"cannot derive an asset name from {raw!r} — pass --name")
    if name[0].isdigit():
        name = "W" + name
    return name


def parse_speed(text: str) -> list[float]:
    parts = [p for p in re.split(r"[,\s]+", text.strip()) if p]
    try:
        values = [float(p) for p in parts]
    except ValueError:
        die(f"--speed must be numbers, got {text!r} (e.g. --speed 0.0,1.7,4.0)")
    if len(values) != 3:
        die(f"--speed takes three values (idle,walk,run), got {len(values)}: {text!r}")
    if values != sorted(values):
        die(f"--speed must be ascending idle,walk,run — got {text!r}")
    return values


def clean_previous(root: Path, name: str, bp_name: str) -> list[Path]:
    """Remove a previous import of this walker BEFORE the editor boots.

    UE's reimport path is unsafe in a commandlet, and deleting from inside
    the editor is not enough: the previous blueprint still references the mesh, so
    EditorAssetLibrary.delete_asset leaves it in place and the import walks into
    the reimport path regardless. Removing the packages from disk while no editor
    is running is the only reliable way to make a re-run a CLEAN import.

    Only paths derived from the walker's own name are touched, and every removal is
    printed.
    """
    content = root / "Unreal" / "CarlaUE4" / "Content" / "Carla"
    targets = [
        content / "Static" / "Pedestrian" / name,
        content / "Blueprints" / "Walkers" / f"{bp_name}.uasset",
    ]
    removed = []
    for target in targets:
        if target.is_dir():
            shutil.rmtree(target)
            removed.append(target)
        elif target.is_file():
            target.unlink()
            removed.append(target)
    for path in removed:
        print(f"[walker] replacing  removed previous {path}")
    return removed


def run_input_check(fbx: Path, skip: bool) -> dict:
    if skip:
        print("[walker] rig check SKIPPED (--skip-rig-check)")
        return {"skipped": True}
    completed = subprocess.run(
        [sys.executable, str(INPUT_CHECK), str(fbx), "--json"],
        capture_output=True, text=True)
    try:
        report = json.loads(completed.stdout or "{}")
    except json.JSONDecodeError:
        report = {}
    if completed.returncode != 0:
        detail = report.get("missing") or completed.stderr.strip() or completed.stdout.strip()
        die(
            f"{fbx.name} is not skinned to CARLA's GEN3 rig.\n"
            f"       {detail}\n"
            "       Run check_input.py for the full comparison. Importing anyway "
            "gives a walker no CARLA animation can drive;\n"
            "       pass --skip-rig-check only if you intend to retarget by hand."
        )
    print(f"[walker] rig OK — GEN3, {report.get('bones_found')} bones")
    return report


def run_build(root: Path, spec: dict, verbose: bool) -> dict:
    """Boot 1: the fast commandlet. Import the mesh, build the blueprint."""
    editor = ue4_editor()
    uproject = root / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject"
    workdir = Path(tempfile.mkdtemp(prefix="carla-walker-build-"))
    try:
        spec_path = workdir / "spec.json"
        result_path = workdir / "result.json"
        spec["result"] = str(result_path)
        spec_path.write_text(json.dumps(spec, indent=2))

        command = [
            str(editor), str(uproject),
            "-run=pythonscript", f"-Script={BUILD_SCRIPT}",
            "-unattended", "-nopause", "-nosourcecontrol", "-NoLiveCoding",
            # A segfaulting commandlet otherwise uploads a minidump of this
            # project to Epic's crash receiver.
            "-nocrashreports",
        ]
        env = dict(os.environ, CARLA_WALKER_SPEC=str(spec_path))

        print(f"[walker] boot 1/2  commandlet: importing {Path(spec['fbx']).name} ...")
        completed = subprocess.run(
            command, env=env,
            stdout=None if verbose else subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True, timeout=BUILD_TIMEOUT_S,
        )
        if not result_path.is_file():
            if not verbose and completed.stdout:
                sys.stderr.write(completed.stdout[-4000:])
            die(
                f"the editor produced no build result (exit {completed.returncode}).\n"
                "       Read the FIRST error above, not the last. Re-run with "
                "--verbose for the full log."
            )
        return json.loads(result_path.read_text())
    finally:
        shutil.rmtree(workdir, ignore_errors=True)


def run_register(root: Path, spec: dict, verbose: bool) -> dict:
    """Boot 2: run CARLA's OWN registration script in a full editor session.

    The script is CarlaTools' add_walker_to_walker_factory.py — part of the CARLA
    repo, argparse-driven, and runnable by hand exactly like its vehicle sibling.
    This host only supplies the arguments and waits for the JSON summary.

    A full editor session (not the commandlet) because the factory blueprint has to
    compile; the result file is polled because a headless editor does not reliably
    exit on its own.
    """
    editor = ue4_editor()
    uproject = root / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject"
    script = root / REGISTER_SCRIPT_REL
    if not script.is_file():
        die(
            f"no registration script at {script}\n"
            "       This is CARLA-side tooling (the walker counterpart of\n"
            "       CarlaTools/Content/Python/add_vehicle_to_vehicle_factory.py) and it\n"
            "       is missing from this checkout. Add it there, not into this skill."
        )
    workdir = Path(tempfile.mkdtemp(prefix="carla-walker-register-"))
    try:
        result_path = workdir / "result.json"
        # -ExecutePythonScript takes the script plus its argv, so the CARLA script is
        # driven by its own documented flags rather than a private env-var protocol.
        argv = [
            str(script),
            "-w", spec["bp_path"],
            "--gender", spec["gender"],
            "--age", spec["age"],
            "--speed", ",".join(str(s) for s in spec["speed"]),
            "--generation", str(spec.get("generation", 3)),
            "--array", os.environ.get("CARLA_WALKER_FACTORY_ARRAY", "Pedestrians"),
            "--result", str(result_path),
        ]
        if spec.get("id"):
            argv += ["--id", spec["id"]]
        if spec.get("wheelchair"):
            argv += ["--wheelchair"]

        command = [
            str(editor), str(uproject),
            "-nullrhi", "-nosplash", "-unattended", "-nopause",
            "-nosourcecontrol", "-NoLiveCoding", "-nocrashreports",
            "-ExecutePythonScript=" + " ".join(argv),
        ]
        env = dict(os.environ)

        print("[walker] boot 2/2  full editor: registering in WalkerFactory "
              "(loads the default map first, this is the slow one) ...")
        log_path = workdir / "editor.log"
        with log_path.open("w") as sink:
            process = subprocess.Popen(command, env=env, stdout=sink,
                                       stderr=subprocess.STDOUT, text=True)
            deadline = time.time() + REGISTER_TIMEOUT_S
            while time.time() < deadline:
                if result_path.is_file():
                    break
                if process.poll() is not None:
                    break
                time.sleep(2)

            result = None
            if result_path.is_file():
                result = json.loads(result_path.read_text())
            # The script has done its work; the editor may still be shutting down.
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=60)
                except subprocess.TimeoutExpired:
                    process.kill()

        if result is None:
            tail = log_path.read_text()[-4000:] if log_path.is_file() else ""
            if verbose and tail:
                sys.stderr.write(tail)
            die(
                "the editor produced no registration result.\n"
                f"       Log: {log_path} (kept for this failure).\n"
                "       The mesh and blueprint from boot 1 are already on disk; "
                "re-run with --register-only to retry just this step."
            )
        return result
    finally:
        # Keep the workdir when registration failed: its log is the only record.
        pass


def missing_on_disk(root: Path, build: dict) -> dict:
    """Which packages the mesh REFERENCES but that never reached disk (C4).

    The editor can report a material or physics asset happily while its package was
    never saved — the reference dangles and the next boot reads None. Only the host,
    looking at the filesystem, can tell the difference.
    """
    report = {"materials": [], "physics": None}
    for slot in build.get("material_slots", []):
        path = slot.get("material")
        if path and not game_path_to_disk(root, path.split(".")[0]).is_file():
            report["materials"].append(slot["slot"])
    physics = build.get("physics_asset")
    if physics and physics != "None":
        if not game_path_to_disk(root, physics.split(".")[0]).is_file():
            report["physics"] = physics
    return report


def print_manual_registration(root: Path, registration: dict, walker_id: str,
                              existing: int) -> None:
    """Show the user exactly where to click, and leave the entry in a file.

    Only reached when the pedestrian list is not a member variable (C1), so it has
    to be unmissable: a wall of prose gets skipped, and a half-followed instruction
    here is what silently costs someone their stock walkers. The layout below
    mirrors what is actually on screen, in the order it is clicked.
    """
    entry = registration["manual_entry"]
    entry_file = (root / "Unreal" / "CarlaUE4" / "Saved"
                  / f"walker_factory_entry_{walker_id}.txt")
    try:
        entry_file.parent.mkdir(parents=True, exist_ok=True)
        entry_file.write_text(entry + "\n")
        saved_note = f"also saved to: {entry_file}"
    except OSError as exc:
        saved_note = f"(could not write it to a file: {exc})"

    bar = "=" * 74
    print()
    print(bar)
    print(" REGISTRATION REFUSED — this is the SAFE outcome, not a failure")
    print(bar)
    print(f" The editor can see 0 of the {existing} walkers already in WalkerFactory,")
    print(" so writing would have replaced them all with one. Nothing was written.")
    print()
    print(" Your walker IS built. One manual paste finishes it.")
    print()
    print(bar)
    print(" FINISH IN THE UNREAL EDITOR")
    print(bar)
    print()
    print(" 1. Open the editor on this checkout:")
    print(f"      cd {root} && make launch")
    print()
    print(" 2. In the Content Browser, go to")
    print("    Content/Carla/Blueprints/Walkers/  and DOUBLE-CLICK:")
    print()
    print("      +----------------------------+")
    print("      |  WalkerFactory             |  <-- double-click")
    print("      +----------------------------+")
    print()
    print(" 3. In the Blueprint editor toolbar, click Class Defaults:")
    print()
    print("      +--------+---------+------------------+")
    print("      |  Save  | Compile |  Class Defaults  |  <-- click")
    print("      +--------+---------+------------------+")
    print()
    print(" 4. In the Details panel on the right, find Definitions:")
    print()
    print(f"      v Definitions            {existing} Array elements   [+] [bin]")
    print("        > Index [ 0 ]   Id \"0001\"")
    print("            ...")
    print(f"        > Index [{existing - 1:2d} ]   Id \"{'%04d' % (existing)}\"   <-- RIGHT-CLICK this row")
    print()
    print(" 5. Choose Paste from the context menu (or select the row and press")
    print("    Ctrl+V). Paste THIS, exactly:")
    print()
    print("      " + entry)
    print()
    print(f"    {saved_note}")
    print()
    print(f" 6. A new row  Index [{existing}]  with  Id \"{walker_id}\"  appears.")
    print("    Then click Compile, then Save.")
    print()
    print(bar)
    print(" THEN VERIFY")
    print(bar)
    print("   bash ../../run-carla-server/scripts/run_server.sh Town10HD_Opt 2000 &")
    print("   until nc -z 127.0.0.1 2000; do sleep 1; done")
    print(f"   python3 verify_walker.py --id {walker_id}")
    print()
    print(" A PASS on all four checks means the walker is properly registered.")
    print(bar)


def confirm_artifacts(root: Path, build: dict) -> list[str]:
    """The editor exiting 0 is not evidence. Stat what it claims to have written."""
    problems = []
    for label, game_path in (("mesh", build.get("mesh_path", "")),
                             ("blueprint", build.get("bp_path", ""))):
        if not game_path:
            problems.append(f"{label}: nothing reported")
            continue
        # Object paths are Package.Object; the file on disk is the package.
        package = game_path.split(".")[0]
        path = game_path_to_disk(root, package)
        if not path.is_file():
            problems.append(f"{label}: no .uasset at {path}")
        elif path.stat().st_size < MIN_UASSET_BYTES:
            problems.append(f"{label}: {path} is only {path.stat().st_size} B "
                            "— too small to hold anything")
    return problems


def main() -> int:
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("fbx", type=Path, help="the FBX to import (skinned to GEN3)")
    ap.add_argument("--name", help="asset name; defaults to the FBX stem")
    # gender, age and generation are REQUIRED and never inferred: none is measurable
    # from a mesh, and each is silent when wrong — nothing raises, the blueprint
    # attribute is simply wrong for anyone filtering on it. The skill asks the user
    # for all three before running (see SKILL.md).
    ap.add_argument("--gender", required=True, choices=GENDERS,
                    help="REQUIRED: ask the user, never infer")
    ap.add_argument("--age", required=True, choices=AGES,
                    help="REQUIRED: ask the user; height is reported, never used to guess")
    ap.add_argument("--speed", default=DEFAULT_SPEED,
                    help=f"idle,walk,run in m/s (default {DEFAULT_SPEED}, the stock "
                         "adult triple; children/teens use 0.0,1.1,2.0)")
    ap.add_argument("--id", help="factory id (4 digits); default is max+1")
    ap.add_argument("--generation", required=True, type=int, choices=(1, 2, 3),
                    help="REQUIRED: ask the user (3 is the GEN3 rig this skill imports)")
    ap.add_argument("--wheelchair", action="store_true",
                    help="set can_use_wheelchair on the definition")
    ap.add_argument("--groom", help="groom asset for hair; default keeps the donor's")
    ap.add_argument("--donor", help="donor GEN3 blueprint to duplicate")
    ap.add_argument("--mesh", dest="mesh_hint",
                    help="nominate one mesh when the FBX yields several")
    ap.add_argument("--capsule-half-height", type=float,
                    help="override the donor's capsule half-height, in cm")
    ap.add_argument("--capsule-radius", type=float,
                    help="override the donor's capsule radius, in cm")
    ap.add_argument("--mesh-scale", type=float,
                    help="override the donor's mesh scale (GEN3 walkers use 0.65)")
    ap.add_argument("--mesh-z", type=float,
                    help="override the donor's mesh relative z, in cm")
    ap.add_argument("--materials", choices=("none", "donor"), default="none",
                    help="'none' (default) leaves the FBX slots as imported, which "
                         "in a commandlet means UNASSIGNED; 'donor' binds matching "
                         "slot names from the donor mesh so the walker renders")
    ap.add_argument("--no-share-physics", action="store_true",
                    help="do not reuse the donor's physics asset (no ragdoll)")
    ap.add_argument("--no-register", action="store_true",
                    help="build the assets but do not touch WalkerFactory")
    ap.add_argument("--register-only", action="store_true",
                    help="skip the import; register an already-built blueprint")
    ap.add_argument("--skip-input-check", action="store_true",
                    help="import even if the rig is not GEN3 (it will not animate)")
    ap.add_argument("--verbose", action="store_true", help="stream the editor log")
    args = ap.parse_args()

    if not args.fbx.is_file() and not args.register_only:
        die(f"no such file: {args.fbx}")

    root = carla_root()
    name = sanitise_name(args.name or args.fbx.stem)
    bp_name = f"BP_Walker_{name}"

    speed = parse_speed(args.speed)
    # A default speed that contradicts the age is the one silent mismatch left, so it
    # is called out rather than accepted quietly.
    if args.speed == DEFAULT_SPEED and STOCK_SPEEDS[args.age] != DEFAULT_SPEED:
        print(f"[walker] NOTE       --speed left at the adult default {DEFAULT_SPEED}, "
              f"but age={args.age} normally uses {STOCK_SPEEDS[args.age]}")

    skeleton = os.environ.get(
        "CARLA_WALKER_SKELETON",
        "/Game/Carla/Static/Pedestrian/ZBAsiaM/Gen3_test/Skel__GEN3")
    factory = os.environ.get("CARLA_WALKER_FACTORY",
                             "/Game/Carla/Blueprints/Walkers/WalkerFactory")
    donor = args.donor or os.environ.get(
        "CARLA_WALKER_DONOR_BP", "/Game/Carla/Blueprints/Walkers/BP_Walker_AB001_G3")

    # Reading ids out of the package is a cross-check, not the source of truth: the
    # encoding differs between a function-local array and a promoted member variable,
    # and a Kismet compile+save rewrites it. When it comes up empty the editor
    # allocates instead (register_walker.next_free_id).
    ids = factory_ids_on_disk(root, factory)
    if args.id:
        if not re.fullmatch(r"\d{4}", args.id):
            die(f"--id must be exactly 4 digits, got {args.id!r}")
        if args.id in ids:
            die(f"id {args.id} is already registered — omit --id to take "
                f"{next_free_id(ids)}")
        walker_id = args.id
    elif ids:
        walker_id = next_free_id(ids)
    else:
        walker_id = ""    # the editor decides

    print(f"[walker] checkout   {root}")
    print(f"[walker] name       {name}  ->  {bp_name}")
    if ids:
        print(f"[walker] factory    {len(ids)} entries on disk, next free id "
              f"{walker_id or '(editor allocates)'}")
    else:
        print("[walker] factory    ids not readable from the package (member-variable "
              "array) — the editor allocates")
    print(f"[walker] attributes gender={args.gender} age={args.age} "
          f"speed=({', '.join(str(s) for s in speed)}) generation={args.generation}"
          + ("" if args.speed != DEFAULT_SPEED else "  [speed default]"))

    build: dict = {}
    if not args.register_only:
        run_input_check(args.fbx, args.skip_input_check)
        clean_previous(root, name, bp_name)

        build = run_build(root, {
            "fbx": str(args.fbx.resolve()),
            "name": name,
            "bp_name": bp_name,
            "skeleton": skeleton,
            "donor_bp": donor,
            "mesh_destination": f"/Game/Carla/Static/Pedestrian/{name}/Meshes",
            "bp_destination": "/Game/Carla/Blueprints/Walkers",
            "physics_asset": True,
            "groom": args.groom,
            "mesh_hint": args.mesh_hint,
            "capsule_half_height": args.capsule_half_height,
            "capsule_radius": args.capsule_radius,
            "mesh_scale": args.mesh_scale,
            "mesh_z": args.mesh_z,
            "materials": args.materials,
            "share_physics": not args.no_share_physics,
            "wheelchair": args.wheelchair,
        }, args.verbose)

        if not build.get("ok"):
            print(json.dumps(build, indent=2), file=sys.stderr)
            die(build.get("error", "the build step failed with no reason given"))

        problems = confirm_artifacts(root, build)
        if problems:
            die("the editor reported success but the artifacts are not on disk:\n"
                + "\n".join(f"       {p}" for p in problems))

        blueprint = build["blueprint"]
        inherited = blueprint.get("inherited", {})
        print(f"[walker] mesh       {build['mesh_path']}")
        print(f"[walker] geometry   {build.get('verts_lod0', '?')} verts, "
              f"half-extents {build['half_extent_cm']} cm")
        print(f"[walker] height     {blueprint.get('visible_height_m')} m in game "
              f"({build['unscaled_height_m']} m unscaled x "
              f"{inherited.get('mesh_scale', ['?'])[0]} mesh scale)")
        print(f"[walker] collision  capsule half-height "
              f"{blueprint.get('capsule_half_height', inherited.get('capsule_half_height'))} cm, "
              f"radius {blueprint.get('capsule_radius', inherited.get('capsule_radius'))} cm, "
              f"mesh z {blueprint.get('mesh_relative_z', inherited.get('mesh_relative_z'))} cm "
              "(the GEN3 convention, inherited from the donor)")
        if blueprint.get("bounds_warning"):
            print(f"[walker] WARNING    {blueprint['bounds_warning']}")

        dangling = missing_on_disk(root, build)
        print(f"[walker] persisted  {len(build.get('persisted_assets', []))} asset(s) "
              "saved in the mesh folder")
        print(f"[walker] physics    {build.get('physics_asset_source')}")
        if dangling["physics"]:
            print(f"[walker] WARNING    the physics asset {dangling['physics']} is "
                  "referenced but NOT on disk — no ragdoll.")
        if build.get("physics_asset_warning"):
            print(f"[walker] WARNING    {build['physics_asset_warning']}")

        slots = build.get("material_slots", [])
        unassigned = build.get("material_slots_unassigned", [])
        raw = build.get("material_slots_raw", [])
        filled = build.get("materials_filled_from_donor", [])
        print(f"[walker] materials  {len(slots)} slots: {len(unassigned)} unassigned, "
              f"{len(raw)} blank from the FBX, {len(filled)} taken from the donor")
        for slot in slots:
            if not slot["material"]:
                state = "UNASSIGNED"
            elif slot.get("raw_import"):
                state = "blank FBX material - " + slot["material"].rsplit("/", 1)[-1]
            else:
                state = slot["material"]
            print(f"[walker]            - {slot['slot']}: {state}")
        if dangling["materials"]:
            print(f"[walker] WARNING    {len(dangling['materials'])} material(s) are "
                  "referenced but NOT on disk — these slots will read as None next "
                  "boot and render untextured:")
            print(f"[walker]            {', '.join(dangling['materials'])}")
            print("[walker]            Re-run with --materials donor to bind the "
                  "donor's saved materials instead.")
        if build.get("materials_still_unassigned"):
            print(f"[walker] WARNING    no donor material for: "
                  f"{', '.join(build['materials_still_unassigned'])}")
        elif (unassigned or raw) and not filled:
            print(f"[walker] WARNING    {len(unassigned) + len(raw)} slot(s) have no "
                  "usable material — the walker renders FLAT WHITE.")
            print("[walker]            Your FBX declares material names but carries "
                  "no textures, so the import")
            print("[walker]            wrote one blank material per slot.")
            print("[walker]            Fix: re-run with --materials donor (binds the "
                  "donor's textured MI_* by slot name),")
            print("[walker]            or import the textures and build the material "
                  "instances by hand.")

        groom = blueprint.get("groom") or blueprint.get("groom_inherited")
        if groom:
            note = "set" if blueprint.get("groom") else "INHERITED from the donor"
            print(f"[walker] hair       {groom}  ({note})")
        print(f"[walker] blueprint  {build['bp_path']}")

        # Height is measured, so it can contradict a hand-passed --age. Compare the
        # VISIBLE height (unscaled x mesh scale) — the unscaled figure is ~1.84 m
        # for every GEN3 mesh, child or not, so checking that would prove nothing.
        height = build["blueprint"].get("visible_height_m") or 0.0
        bands = {"child": (0.9, 1.5), "teenager": (1.3, 1.8),
                 "adult": (1.5, 2.1), "elderly": (1.4, 2.0)}
        expected = bands[args.age]
        if not expected[0] <= height <= expected[1]:
            fits = [a for a, (lo, hi) in bands.items() if lo <= height <= hi]
            print(f"[walker] NOTE       height {height} m is outside the usual range "
                  f"{expected} for age={args.age}"
                  + (f" — it fits {' or '.join(fits)}" if fits else ""))

    bp_path = build.get("bp_path") or f"/Game/Carla/Blueprints/Walkers/{bp_name}"
    bp_class = build.get("bp_class") or f"{bp_path}.{bp_name}_C"

    if args.no_register:
        print("[walker] --no-register: WalkerFactory untouched. The walker is on disk "
              "but NOT spawnable.")
        print(f"[walker] register it later with: --register-only --name {name} "
              f"--gender {args.gender} --age {args.age} --speed {args.speed}")
        return 0

    array_name = os.environ.get("CARLA_WALKER_FACTORY_ARRAY", "Pedestrians")
    if not factory_has_array(root, factory, array_name):
        entry = manual_entry_text(bp_class, walker_id or "0053", args.gender, args.age,
                                  speed, args.generation, args.wheelchair)
        print()
        print("=" * 74)
        print(f" REGISTRATION SKIPPED — no '{array_name}' member variable in WalkerFactory")
        print("=" * 74)
        print(" This checkout still keeps the pedestrian list in a variable LOCAL to")
        print(" GenerateDefinitions. Function locals are not class properties, so")
        print(" neither CARLA's add_walker_to_walker_factory.py nor anything else can")
        print(" reach them by reflection (references, C1). Nothing was written.")
        print()
        print(" To get automatic registration, promote the list ONCE in the editor:")
        print("   WalkerFactory -> My Blueprint -> Local Variables (GenerateDefinitions)")
        print(f"   Move `Walkers` to a MEMBER variable named `{array_name}`")
        print("   (Array of PedestrianParameters), repoint the Get node in the graph,")
        print("   then Compile and Save. VehicleFactory already does this with `Vehicles`.")
        print()
        print(" Or finish this one by hand — paste into the array in Class Defaults:")
        print()
        print("    " + entry)
        print()
        print(f" then: python3 verify_walker.py --id {walker_id or '0053'}")
        print("=" * 74)
        return 3

    registration = run_register(root, {
        "factory": factory,
        "bp_path": bp_path,
        "bp_class": bp_class,
        "id": walker_id,
        "gender": args.gender,
        "age": args.age,
        "speed": speed,
        "generation": args.generation,
        "wheelchair": args.wheelchair,
        "expected_count": len(ids),
    }, args.verbose)

    if registration.get("needs_manual"):
        # Kept for a checkout whose factory array is still a function local: the
        # pre-flight skips boot 2 and the user finishes by hand (C1).
        print_manual_registration(root, registration, walker_id, len(ids))
        return 3

    if not registration.get("ok"):
        print(json.dumps(registration, indent=2), file=sys.stderr)
        die(registration.get("error", "registration failed with no reason given"))

    print(f"[walker] registered {registration['blueprint_id']} "
          f"({registration['action']} in {registration['array']}: "
          f"{registration['entries_before']} -> {registration['entries_after']} "
          f"entries)")
    print(f"[walker] via         CarlaTools/{Path(REGISTER_SCRIPT_REL).name} "
          "(CARLA repo, runnable standalone)")
    print("[walker] NOTE       a running server keeps the old content — restart it "
          "before verifying")
    print()
    print(f"[walker] verify it: python3 verify_walker.py --id {registration['id']}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
