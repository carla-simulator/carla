#!/usr/bin/env python3
"""Import an FBX vehicle into CARLA as a spawnable vehicle.<make>.<model>.

    python3 import_vehicle.py ~/models/SK_MyCar.fbx \
        --make ford --model transit --base-type truck --generation 3 \
        --wheel-radius 40

The FBX must be a SKELETAL mesh whose wheels are bones named
`Wheel_Front_Left/Right` and `Wheel_Rear_Left/Right` — PxVehicleDrive4W finds them by
name. check_input.py runs first and stops the import on a mismatch.

What lands where:

    Content/Carla/Static/Vehicles/<Name>/     SK_<Name>, its skeleton, materials,
                                              <Name>_PhysicsAsset, AnimBP_<Name>
    Content/Carla/Blueprints/Vehicles/<Name>/ BP_<Name> + four wheel blueprints
    Content/Carla/Blueprints/Vehicles/VehicleFactory   the entry that makes it spawnable

after which it spawns like any native CARLA vehicle:

    bp = bp_lib.find('vehicle.ford.transit')
    car = world.spawn_actor(bp, spawn_point)
    car.apply_control(carla.VehicleControl(throttle=0.5))

make, model, base_type and generation are REQUIRED and never inferred; the wheel radius
is required too, because PxVehicle sizes the suspension from it and nothing in the mesh
states it.

Two editor boots, both FULL editor sessions
-------------------------------------------
Boot 1 imports the mesh and assembles the vehicle through CarlaTools'
VehicleAuthoringLibrary. Boot 2 runs CARLA's own add_vehicle_to_vehicle_factory.py,
which appends the entry to VehicleFactory.Vehicles and saves it.

Neither can be the fast `-run=pythonscript` commandlet: CreateVehicleAnimBP retargets
animations and UE's retarget path syncs the Content Browser, which builds a Slate
window that a commandlet cannot create, and registration has to compile a blueprint.
"""
from __future__ import annotations

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
BUILD_SCRIPT = HERE / "editor" / "build_vehicle.py"
INPUT_CHECK = HERE / "check_input.py"

# Registration lives in the CARLA repo, next to the prop and walker equivalents.
REGISTER_SCRIPT_REL = ("Unreal/CarlaUE4/Plugins/CarlaTools/Content/Python/"
                       "add_vehicle_to_vehicle_factory.py")

BASE_TYPES = ("car", "truck", "van", "bus", "motorcycle", "bicycle")
MIN_UASSET_BYTES = 4096
BUILD_TIMEOUT_S = 1200
REGISTER_TIMEOUT_S = 1200


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
    die("cannot locate a carla checkout — export CARLA_UE4_ROOT=/path/to/carla")
    raise AssertionError("unreachable")


def ue4_editor() -> Path:
    ue4 = os.environ.get("UE4_ROOT", "").strip()
    if not ue4:
        die("UE4_ROOT is unset — export it to your built CarlaUnreal UE 4.26 fork")
    editor = Path(ue4) / "Engine" / "Binaries" / "Linux" / "UE4Editor"
    if not editor.is_file():
        die(f"no UE4Editor at {editor} — run the build-carla-ue4 skill first")
    return editor


def game_path_to_disk(root: Path, game_path: str) -> Path:
    relative = game_path[len("/Game/"):] if game_path.startswith("/Game/") else game_path
    return root / "Unreal" / "CarlaUE4" / "Content" / (relative + ".uasset")


def sanitise_name(raw: str) -> str:
    name = re.sub(r"[^A-Za-z0-9_]", "_", raw).strip("_")
    if not name:
        die(f"cannot derive an asset name from {raw!r} — pass --name")
    if name[0].isdigit():
        name = "V" + name
    return name


def run_input_check(fbx: Path, skip: bool) -> dict:
    if skip:
        print("[vehicle] rig check SKIPPED (--skip-rig-check)")
        return {"skipped": True}
    completed = subprocess.run([sys.executable, str(INPUT_CHECK), str(fbx), "--json"],
                               capture_output=True, text=True)
    try:
        report = json.loads(completed.stdout or "{}")
    except json.JSONDecodeError:
        report = {}
    if completed.returncode != 0:
        detail = (", ".join(report.get("missing", []))
                  or completed.stderr.strip() or completed.stdout.strip())
        die(f"{fbx.name} does not carry CARLA's 4-wheeled rig.\n"
            f"       missing/failing: {detail}\n"
            "       PxVehicleDrive4W finds wheels by bone name; rename the joints and\n"
            "       re-export. Run check_input.py for the full list.")
    print(f"[vehicle] rig OK — 4 wheel bones present"
          + ("" if report.get("has_vehicle_base") else " (chassis inferred)"))
    return report


def clean_previous(root: Path, name: str, bp_name: str) -> list[Path]:
    """Remove a previous import BEFORE the editor boots, so this is a clean import."""
    content = root / "Unreal" / "CarlaUE4" / "Content" / "Carla"
    targets = [
        content / "Static" / "Vehicles" / name,
        content / "Blueprints" / "Vehicles" / name,
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
        print(f"[vehicle] replacing removed previous {path}")
    return removed


def run_build(root: Path, spec: dict, verbose: bool) -> dict:
    editor = ue4_editor()
    uproject = root / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject"
    workdir = Path(tempfile.mkdtemp(prefix="carla-vehicle-build-"))
    try:
        spec_path = workdir / "spec.json"
        result_path = workdir / "result.json"
        spec["result"] = str(result_path)
        spec_path.write_text(json.dumps(spec, indent=2))
        # A FULL editor session, not the fast commandlet: CreateVehicleAnimBP retargets
        # animations, and UE's retarget path syncs the Content Browser, which builds an
        # SWindow. Without Slate that asserts in FSlateInvalidationRoot and takes the
        # process down mid-assembly. -nullrhi keeps it headless.
        command = [
            str(editor), str(uproject),
            "-nullrhi", "-nosplash", "-unattended", "-nopause",
            "-nosourcecontrol", "-NoLiveCoding", "-nocrashreports",
            f"-ExecutePythonScript={BUILD_SCRIPT}",
        ]
        env = dict(os.environ, CARLA_VEHICLE_SPEC=str(spec_path))
        print(f"[vehicle] boot 1/2  full editor: importing {Path(spec['fbx']).name} "
              "and assembling the vehicle (loads the default map first) ...")
        log_path = workdir / "editor.log"
        with log_path.open("w") as sink:
            process = subprocess.Popen(command, env=env, stdout=sink,
                                       stderr=subprocess.STDOUT, text=True)
            deadline = time.time() + BUILD_TIMEOUT_S
            while time.time() < deadline:
                if result_path.is_file():
                    break
                if process.poll() is not None:
                    break
                time.sleep(3)
            result = json.loads(result_path.read_text()) if result_path.is_file() else None
            if process.poll() is None:
                process.terminate()
                try:
                    process.wait(timeout=60)
                except subprocess.TimeoutExpired:
                    process.kill()
        if result is None:
            kept = Path(tempfile.mkdtemp(prefix="carla-vehicle-buildfail-")) / "editor.log"
            shutil.copy(log_path, kept)
            tail = kept.read_text()[-4000:]
            if verbose:
                sys.stderr.write(tail)
            die("the editor produced no build result.\n"
                f"       Log: {kept}\n"
                "       Read the FIRST error, not the last.")
        return result
    finally:
        shutil.rmtree(workdir, ignore_errors=True)


def run_register(root: Path, spec: dict, verbose: bool) -> dict:
    """Boot 2: CARLA's own add_vehicle_to_vehicle_factory.py in a full editor session."""
    editor = ue4_editor()
    uproject = root / "Unreal" / "CarlaUE4" / "CarlaUE4.uproject"
    script = root / REGISTER_SCRIPT_REL
    if not script.is_file():
        die(f"no registration script at {script}\n"
            "       That is CARLA-side tooling and is missing from this checkout.")
    workdir = Path(tempfile.mkdtemp(prefix="carla-vehicle-register-"))
    result_path = workdir / "result.json"
    # The shipped script takes Package.Object and appends '_C', so hand it the object
    # path rather than the package path.
    bp_object = "{}.{}".format(spec["bp_path"], spec["bp_path"].rsplit("/", 1)[-1])
    argv = [str(script), "-v", bp_object, "-n", spec["model"],
            "--make", spec["make"], "--base-type", spec["base_type"],
            "--generation", str(spec["generation"]),
            "--number-of-wheels", "4",
            "--result", str(result_path)]
    if spec.get("object_type"):
        argv += ["--object-type", spec["object_type"]]
    if spec.get("special_type"):
        argv += ["--special-type", spec["special_type"]]
    if not spec.get("has_lights"):
        argv += ["--no-lights"]
    command = [
        str(editor), str(uproject),
        "-nullrhi", "-nosplash", "-unattended", "-nopause",
        "-nosourcecontrol", "-NoLiveCoding", "-nocrashreports",
        "-ExecutePythonScript=" + " ".join(argv),
    ]
    log_path = workdir / "editor.log"
    print("[vehicle] boot 2/2  full editor: registering in VehicleFactory "
          "(loads the default map first) ...")
    with log_path.open("w") as sink:
        process = subprocess.Popen(command, env=dict(os.environ), stdout=sink,
                                   stderr=subprocess.STDOUT, text=True)
        deadline = time.time() + REGISTER_TIMEOUT_S
        # The shipped script writes no result file, so completion is detected by the
        # factory package changing on disk.
        factory_file = game_path_to_disk(root, spec["factory"])
        before = factory_file.stat().st_mtime if factory_file.is_file() else 0
        changed = False
        while time.time() < deadline:
            if factory_file.is_file() and factory_file.stat().st_mtime > before:
                changed = True
                # give the save a moment to finish flushing
                time.sleep(5)
                break
            if process.poll() is not None:
                break
            time.sleep(3)
        if process.poll() is None:
            process.terminate()
            try:
                process.wait(timeout=60)
            except subprocess.TimeoutExpired:
                process.kill()
    tail = log_path.read_text()[-6000:] if log_path.is_file() else ""
    written = json.loads(result_path.read_text()) if result_path.is_file() else {}
    registered = bool(written.get("ok")) or changed
    if not registered:
        if verbose and tail:
            sys.stderr.write(tail)
        die("registration did not modify VehicleFactory.\n"
            f"       Log: {log_path}\n"
            "       The assets are built; re-run with --register-only to retry.")
    return {"factory_written": changed, "log": str(log_path),
            "script": str(script), "blueprint": bp_object, **written}


def confirm_artifacts(root: Path, build: dict) -> list[str]:
    problems = []
    checks = [("mesh", build.get("mesh_path", "")),
              ("blueprint", build.get("bp_path", "")),
              ("physics asset", build.get("physics_asset", "")),
              ("anim blueprint", build.get("anim_bp", ""))]
    for wheel in build.get("wheels", []):
        checks.append((f"wheel {wheel['bone']}", wheel["path"]))
    for label, game_path in checks:
        if not game_path:
            problems.append(f"{label}: nothing reported")
            continue
        path = game_path_to_disk(root, game_path.split(".")[0])
        if not path.is_file():
            problems.append(f"{label}: no .uasset at {path}")
        elif path.stat().st_size < MIN_UASSET_BYTES:
            problems.append(f"{label}: {path} is only {path.stat().st_size} B")
    return problems


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("fbx", type=Path, help="the FBX to import (skeletal, wheels as bones)")
    ap.add_argument("--make", required=True,
                    help="REQUIRED: manufacturer, lowercased into the blueprint id")
    ap.add_argument("--model", required=True,
                    help="REQUIRED: model, lowercased into the blueprint id")
    ap.add_argument("--base-type", required=True, choices=BASE_TYPES,
                    help="REQUIRED: car / truck / van / bus / motorcycle / bicycle")
    ap.add_argument("--generation", required=True, type=int, choices=(1, 2, 3),
                    help="REQUIRED: ask the user")
    ap.add_argument("--wheel-radius", required=True, type=float,
                    help="REQUIRED: wheel radius in cm; PxVehicle sizes the suspension "
                         "from it and the mesh does not state it")
    ap.add_argument("--wheel-width", type=float, help="wheel width in cm (default 45%% of radius)")
    ap.add_argument("--wheel-mass", type=float, default=20.0, help="per-wheel mass in kg")
    ap.add_argument("--steer-angle", type=float, default=70.0,
                    help="front wheel steering angle in degrees")
    ap.add_argument("--name", help="asset name; defaults to the FBX stem")
    ap.add_argument("--object-type", default="", help="FVehicleParameters.ObjectType")
    ap.add_argument("--special-type", default="", help="e.g. electric, emergency, taxi")
    ap.add_argument("--has-lights", action="store_true", help="set HasLights")
    ap.add_argument("--donor", help="donor vehicle blueprint to duplicate")
    ap.add_argument("--donor-anim-bp", help="anim blueprint template to retarget")
    ap.add_argument("--collision-mesh",
                    help="static mesh for the CustomCollision raycast hull; without it "
                         "the donor's hull is inherited")
    ap.add_argument("--mesh", dest="mesh_hint", help="nominate one mesh from a multi-mesh FBX")
    ap.add_argument("--no-register", action="store_true",
                    help="build the assets, leave VehicleFactory alone")
    ap.add_argument("--register-only", action="store_true",
                    help="register an already-built blueprint, skipping the import")
    ap.add_argument("--skip-input-check", action="store_true",
                    help="import even without the canonical wheel bones (it will not drive)")
    ap.add_argument("--verbose", action="store_true", help="stream the editor log")
    args = ap.parse_args()

    if not args.fbx.is_file() and not args.register_only:
        die(f"no such file: {args.fbx}")

    root = carla_root()
    name = sanitise_name(args.name or args.fbx.stem)
    bp_name = f"BP_{name}"
    blueprint_id = f"vehicle.{args.make.lower()}.{args.model.lower()}"
    factory = os.environ.get("CARLA_VEHICLE_FACTORY",
                             "/Game/Carla/Blueprints/Vehicles/VehicleFactory")
    donor = args.donor or os.environ.get(
        "CARLA_VEHICLE_DONOR_BP", "/Game/Carla/Blueprints/Vehicles/Ambulance/BP_Ambulance")
    donor_anim = args.donor_anim_bp or os.environ.get(
        "CARLA_VEHICLE_DONOR_ANIM_BP",
        "/Game/Carla/Static/Truck/Ambulance/AnimBP_Ambulance")
    donor_wheels = [w for w in os.environ.get("CARLA_VEHICLE_DONOR_WHEELS", "").split(",") if w]
    if not donor_wheels:
        base = donor.rsplit("/", 1)[0]
        stem = donor.rsplit("/", 1)[-1]
        donor_wheels = [f"{base}/{stem}_{s}" for s in ("FLW", "FRW", "RLW", "RRW")]

    print(f"[vehicle] checkout   {root}")
    print(f"[vehicle] name       {name}  ->  {bp_name}  ->  {blueprint_id}")
    print(f"[vehicle] attributes make={args.make} model={args.model} "
          f"base_type={args.base_type} generation={args.generation}")
    print(f"[vehicle] wheels     radius {args.wheel_radius} cm, "
          f"width {args.wheel_width or round(args.wheel_radius * 0.45, 1)} cm, "
          f"mass {args.wheel_mass} kg, steer {args.steer_angle} deg")
    print(f"[vehicle] donor      {donor}")

    build: dict = {}
    if not args.register_only:
        run_input_check(args.fbx, args.skip_input_check)
        clean_previous(root, name, bp_name)
        build = run_build(root, {
            "fbx": str(args.fbx.resolve()),
            "name": name,
            "mesh_name": f"SK_{name}",
            "bp_name": bp_name,
            "mesh_destination": f"/Game/Carla/Static/Vehicles/{name}",
            "bp_destination": f"/Game/Carla/Blueprints/Vehicles/{name}",
            "donor_bp": donor,
            "donor_anim_bp": donor_anim,
            "donor_wheels": donor_wheels,
            "wheel_radius_cm": args.wheel_radius,
            "wheel_width_cm": args.wheel_width,
            "wheel_mass_kg": args.wheel_mass,
            "steer_angle_deg": args.steer_angle,
            "mesh_hint": args.mesh_hint,
            "collision_mesh": args.collision_mesh,
        }, args.verbose)

        if not build.get("ok"):
            print(json.dumps(build, indent=2), file=sys.stderr)
            die(build.get("error", "the build step failed with no reason given"))

        problems = confirm_artifacts(root, build)
        if problems:
            die("the editor reported success but artifacts are missing:\n"
                + "\n".join(f"       {p}" for p in problems))

        print(f"[vehicle] mesh       {build['mesh_path']}")
        print(f"[vehicle] skeleton   {build['skeleton']}")
        print(f"[vehicle] size       {build['length_m']} x {build['width_m']} x "
              f"{build['height_m']} m")
        print(f"[vehicle] physics    {build['physics_asset']}")
        print(f"[vehicle] anim BP    {build['anim_bp']}")
        for wheel in build.get("wheels", []):
            print(f"[vehicle] wheel      {wheel['bone']}: {wheel['path'].rsplit('/', 1)[-1]}"
                  f"  steers={wheel['steers']}")
        hull = build.get("collision_mesh")
        if hull:
            note = "INHERITED from the donor" if build.get("collision_mesh_inherited") else "set"
            print(f"[vehicle] collision  {hull}  ({note})")
            if build.get("collision_mesh_inherited"):
                print("[vehicle]            sensor raycasts use the donor's hull — pass "
                      "--collision-mesh for this vehicle's own")
        print(f"[vehicle] blueprint  {build['bp_path']}")
        print(f"[vehicle] CDO mesh   {build.get('cdo_skeletal_mesh')}")
        slots = build.get("material_slots", [])
        unassigned = [s["slot"] for s in slots if not s["material"]]
        print(f"[vehicle] materials  {len(slots)} slots, {len(unassigned)} unassigned")
        if unassigned:
            print("[vehicle] WARNING    unassigned material slots render untextured: "
                  f"{', '.join(unassigned)}")

    bp_path = build.get("bp_path") or f"/Game/Carla/Blueprints/Vehicles/{name}/{bp_name}"

    if args.no_register:
        print("[vehicle] --no-register: VehicleFactory untouched. The vehicle is on disk "
              "but NOT spawnable.")
        return 0

    registration = run_register(root, {
        "factory": factory, "bp_path": bp_path, "model": args.model,
        "make": args.make, "base_type": args.base_type, "generation": args.generation,
        "object_type": args.object_type, "special_type": args.special_type,
        "has_lights": args.has_lights,
    }, args.verbose)
    actual = registration.get("id", blueprint_id)
    print(f"[vehicle] registered {actual} in VehicleFactory "
          f"({registration.get('action', 'written')}, "
          f"{registration.get('entries_after', '?')} entries)")
    if actual != blueprint_id:
        print(f"[vehicle] NOTE       the factory recorded {actual}, not {blueprint_id}")
    print(f"[vehicle] via         CarlaTools/{Path(REGISTER_SCRIPT_REL).name} "
          "(CARLA repo, runnable standalone)")
    print("[vehicle] NOTE       a running server keeps the old content — restart it "
          "before verifying")
    print()
    print(f"[vehicle] verify it: python3 verify_vehicle.py --id {actual}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
