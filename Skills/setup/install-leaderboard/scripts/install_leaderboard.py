#!/usr/bin/env python3
"""Install / switch / verify a CARLA Leaderboard stack at a chosen version.

Subcommands
    detect   which leaderboard, scenario_runner and CARLA are present (read-only)
    plan     print the exact commands for a target version         (read-only)
    install  clone or switch both repos, then pip install requirements
    verify   import the full stack and report what it can see

Version detection reads the code, not the branch name, so it also works on a
tarball, a docker image or a detached HEAD:
  * `SENSORS_QUALIFIER` in autoagents/autonomous_agent.py  -> 2.x, else 1.0
  * `PENALTY_PERC_DICT` in utils/statistics_manager.py     -> 2.0, else 2.1
"""
from __future__ import annotations

import argparse
import importlib.util
import os
import shutil
import subprocess
import sys
from pathlib import Path

LB_REPO = "https://github.com/carla-simulator/leaderboard.git"
SR_REPO = "https://github.com/carla-simulator/scenario_runner.git"

# version -> (leaderboard branch, scenario_runner branch, CARLA, note)
VERSIONS = {
    "1.0": ("leaderboard-1.0", "leaderboard-1.0", "0.9.10.1",
            "legacy; Town01-06; tracks SENSORS/MAP; 4 cam / 1 lidar / 2 radar"),
    "2.0": ("leaderboard-2.0", "leaderboard-2.0", "leaderboard build (0.9.14 + large maps)",
            "Town12/13; multiplicative infraction penalty"),
    "2.1": ("leaderboard-2.1", "leaderboard-2.1", "leaderboard build (0.9.14 + large maps)",
            "current; same routes as 2.0; ADDITIVE penalty  P = 1/(1+sum)"),
}


def sh(cmd: list[str], check: bool = True) -> str:
    p = subprocess.run(cmd, capture_output=True, text=True)
    if check and p.returncode != 0:
        sys.exit(f"FAILED: {' '.join(cmd)}\n{p.stdout}{p.stderr}")
    return (p.stdout or "").strip()


def have(mod: str) -> bool:
    """Is `mod` importable?

    importlib.util.find_spec() imports the PARENT packages of a dotted name, so a
    dotted probe raises ModuleNotFoundError when the parent is missing — which is
    exactly the condition being tested.
    """
    try:
        return importlib.util.find_spec(mod) is not None
    except (ImportError, AttributeError, ValueError):
        return False


def is_lb(p: Path) -> bool:
    return (p / "leaderboard" / "leaderboard_evaluator.py").is_file()


def is_sr(p: Path) -> bool:
    return (p / "scenario_runner.py").is_file() and (p / "srunner").is_dir()


def find(kind: str, explicit: str | None) -> Path | None:
    test = is_lb if kind == "lb" else is_sr
    name = "leaderboard" if kind == "lb" else "scenario_runner"
    env = os.environ.get("LEADERBOARD_ROOT" if kind == "lb" else "SCENARIO_RUNNER_ROOT")
    for c in [explicit, env, os.getcwd(), str(Path.home() / name), f"/workspace/{name}"]:
        if c and test(Path(c)):
            return Path(c).resolve()
    return None


def lb_version(root: Path) -> str:
    aa = root / "leaderboard" / "autoagents" / "autonomous_agent.py"
    sm = root / "leaderboard" / "utils" / "statistics_manager.py"
    if not (aa.is_file() and sm.is_file()):
        return "unknown"
    if "SENSORS_QUALIFIER" not in aa.read_text():
        return "1.0"
    return "2.0" if "PENALTY_PERC_DICT" in sm.read_text() else "2.1"


def branch_of(root: Path) -> str:
    return sh(["git", "-C", str(root), "rev-parse", "--abbrev-ref", "HEAD"], check=False) or "unknown"


def carla_client() -> str | None:
    if not have("carla"):
        return None
    try:
        from importlib.metadata import version

        return version("carla")
    except Exception:
        return "unknown"


def cmd_detect(args) -> None:
    lb = find("lb", args.leaderboard_root)
    sr = find("sr", args.scenario_runner_root)
    cv = carla_client()
    print(f"carla client       : {cv or 'NOT IMPORTABLE (run install-python-api)'}")
    if cv == "leaderboard":
        print("                     ^ this is the leaderboard CARLA build (LB 2.x)")
    if lb:
        v = lb_version(lb)
        print(f"leaderboard        : {lb} (branch {branch_of(lb)})")
        print(f"  detected version : {v}   {VERSIONS.get(v, ('', '', '', 'unrecognised'))[3]}")
        if branch_of(lb) == "master":
            print("  NOTE             : 'master' scores like 2.0, not 2.1")
    else:
        print("leaderboard        : not found")
    if sr:
        print(f"scenario_runner    : {sr} (branch {branch_of(sr)})")
        if lb:
            want = VERSIONS.get(lb_version(lb), (None, None, None, None))[1]
            cur = branch_of(sr)
            if want and cur != want:
                same2x = {cur, want} == {"leaderboard-2.0", "leaderboard-2.1"}
                print(f"  pairing          : {'OK (identical commits)' if same2x else f'MISMATCH — needs {want}'}")
            elif want:
                print("  pairing          : OK")
    else:
        print("scenario_runner    : not found")
    print(f"git available      : {'yes' if shutil.which('git') else 'NO'}")


def cmd_plan(args) -> None:
    v = args.version
    lbb, srb, carla, note = VERSIONS[v]
    base = Path(args.dir or Path.home()).expanduser()
    print(f"target             : Leaderboard {v}  ({note})")
    print(f"CARLA required     : {carla}")
    if v != "1.0":
        print("  get it from      : https://leaderboard.carla.org/get_started_v2_1/")
        print("  (a stock release + AdditionalMaps runs the routes but is NOT the eval environment)")
    print()
    lb, sr = find("lb", args.leaderboard_root), find("sr", args.scenario_runner_root)
    for root, want, repo, name in ((lb, lbb, LB_REPO, "leaderboard"), (sr, srb, SR_REPO, "scenario_runner")):
        if root is None:
            print(f"git clone -b {want} --single-branch {repo} {base / name}")
        elif branch_of(root) != want:
            print(f"git -C {root} fetch origin {want} && git -C {root} checkout {want}")
        else:
            print(f"# {root} already on {want}")
    lbp = lb or base / "leaderboard"
    srp = sr or base / "scenario_runner"
    print(f"{sys.executable} -m pip install -r {lbp}/requirements.txt -r {srp}/requirements.txt")
    print()
    print("export CARLA_ROOT=/path/to/CARLA")
    print(f"export SCENARIO_RUNNER_ROOT={srp}")
    print(f"export LEADERBOARD_ROOT={lbp}")
    print('export PYTHONPATH="${CARLA_ROOT}/PythonAPI/carla":"${SCENARIO_RUNNER_ROOT}":"${LEADERBOARD_ROOT}":${PYTHONPATH}')
    print('#   plus the egg:  ls ${CARLA_ROOT}/PythonAPI/carla/dist/')


def _clone_or_switch(root: Path | None, want: str, repo: str, dest: Path, force: bool) -> Path:
    if root is None:
        if dest.exists() and any(dest.iterdir()):
            sys.exit(f"{dest} exists and is not a checkout of {repo} — refusing to touch it")
        print(f"[install] cloning {want} -> {dest}")
        sh(["git", "clone", "-b", want, "--single-branch", repo, str(dest)])
        return dest
    cur = branch_of(root)
    if cur == want:
        print(f"[install] {root} already on {want}")
        return root
    dirty = sh(["git", "-C", str(root), "status", "--porcelain"], check=False)
    if dirty and not force:
        sys.exit(f"{root} has uncommitted changes; stash them or pass --force\n{dirty}")
    print(f"[install] {root}: {cur} -> {want}")
    sh(["git", "-C", str(root), "fetch", "origin", want])
    sh(["git", "-C", str(root), "checkout", want])
    return root


def cmd_install(args) -> None:
    if not shutil.which("git"):
        sys.exit("git is required")
    lbb, srb, carla, note = VERSIONS[args.version]
    base = Path(args.dir or Path.home()).expanduser()
    base.mkdir(parents=True, exist_ok=True)
    lb = _clone_or_switch(find("lb", args.leaderboard_root), lbb, LB_REPO, base / "leaderboard", args.force)
    sr = _clone_or_switch(find("sr", args.scenario_runner_root), srb, SR_REPO, base / "scenario_runner", args.force)

    if args.no_deps:
        print("[install] skipping requirements")
    else:
        for req in (sr / "requirements.txt", lb / "requirements.txt"):
            print(f"[install] pip install -r {req}")
            p = subprocess.run([sys.executable, "-m", "pip", "install", "-r", str(req)])
            if p.returncode != 0:
                # opencv-python==4.2.0.32 has no wheels for py3.9+ and its source
                # build fails; nothing in the leaderboard needs that exact version.
                print(f"[install] WARNING {req} failed. If it was opencv-python==4.2.0.32,"
                      f" run: {sys.executable} -m pip install opencv-python")

    print(f"\n[install] Leaderboard {args.version} ready ({note})")
    print(f"[install] CARLA required: {carla}")
    print("\nexport CARLA_ROOT=/path/to/CARLA")
    print(f"export SCENARIO_RUNNER_ROOT={sr}")
    print(f"export LEADERBOARD_ROOT={lb}")
    print('export PYTHONPATH="${CARLA_ROOT}/PythonAPI/carla":"${SCENARIO_RUNNER_ROOT}":"${LEADERBOARD_ROOT}":${PYTHONPATH}')


def cmd_verify(args) -> None:
    lb, sr = find("lb", args.leaderboard_root), find("sr", args.scenario_runner_root)
    if lb is None:
        sys.exit("no leaderboard checkout found — run `install` first")
    v = lb_version(lb)
    print(f"leaderboard     : {lb} (branch {branch_of(lb)}, version {v})")
    if sr:
        print(f"scenario_runner : {sr} (branch {branch_of(sr)})")
        sys.path.insert(0, str(sr))
    sys.path.insert(0, str(lb))

    bad = []
    for mod, hint in [
        ("carla", "install-python-api"),
        ("agents.navigation.global_route_planner", "add $CARLA_ROOT/PythonAPI/carla to PYTHONPATH"),
        ("srunner.scenariomanager.carla_data_provider", "add SCENARIO_RUNNER_ROOT to PYTHONPATH"),
        ("leaderboard.utils.statistics_manager", "add LEADERBOARD_ROOT to PYTHONPATH"),
        ("leaderboard.autoagents.autonomous_agent", "add LEADERBOARD_ROOT to PYTHONPATH"),
    ]:
        try:
            __import__(mod)
            print(f"import          : {mod} OK")
        except Exception as e:
            print(f"import          : {mod} FAILED ({e}) -> {hint}")
            bad.append(mod)

    if "leaderboard.autoagents.autonomous_agent" not in bad:
        from leaderboard.autoagents.autonomous_agent import Track

        print(f"tracks          : {', '.join(t.value for t in Track)}")
    if "leaderboard.utils.statistics_manager" not in bad:
        from leaderboard.utils import statistics_manager as sm

        scheme = "multiplicative (2.0)" if hasattr(sm, "PENALTY_PERC_DICT") else "additive 1/(1+sum) (2.1)"
        print(f"penalty scheme  : {scheme}")

    import xml.etree.ElementTree as ET

    for f in sorted((lb / "data").glob("*.xml")):
        root = ET.parse(f).getroot()
        routes = root.findall("route")
        towns = sorted({r.attrib.get("town", "?") for r in routes})
        n_sc = len(root.findall(".//scenario"))
        print(f"routes          : {f.name}: {len(routes)} routes, {n_sc} scenarios, towns {','.join(towns)}")

    try:
        import carla

        c = carla.Client(args.host, args.port)
        c.set_timeout(4.0)
        maps = [m.split("/")[-1] for m in c.get_available_maps()]
        print(f"server          : {c.get_server_version()}")
        need = ["Town01"] if v == "1.0" else ["Town12", "Town13"]
        for m in need:
            print(f"  map {m:8}: {'available' if m in maps else 'MISSING (routes will fail to load)'}")
    except Exception as e:
        print(f"server          : unreachable at {args.host}:{args.port} ({e})")

    sys.exit(1 if bad else 0)


def main() -> None:
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--host", default=os.environ.get("CARLA_HOST", "127.0.0.1"))
    common.add_argument("--port", type=int, default=int(os.environ.get("CARLA_PORT", 2000)))
    common.add_argument("--leaderboard-root", help="existing leaderboard checkout")
    common.add_argument("--scenario-runner-root", help="existing scenario_runner checkout")
    common.add_argument("--dir", help="parent directory for new clones (default: $HOME)")
    ap = argparse.ArgumentParser(description=__doc__, parents=[common],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("detect", parents=[common]).set_defaults(func=cmd_detect)
    for name, fn in (("plan", cmd_plan), ("install", cmd_install)):
        p = sub.add_parser(name, parents=[common])
        p.add_argument("--version", choices=sorted(VERSIONS), required=True)
        if name == "install":
            p.add_argument("--no-deps", action="store_true")
            p.add_argument("--force", action="store_true", help="switch branch even with local changes")
        p.set_defaults(func=fn)
    sub.add_parser("verify", parents=[common]).set_defaults(func=cmd_verify)
    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
