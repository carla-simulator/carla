#!/usr/bin/env python3
"""Install / switch / verify a ScenarioRunner checkout for the CARLA in use.

Subcommands
    detect   what CARLA and what scenario_runner are present (read-only)
    plan     recommend a branch and print the exact commands (read-only)
    install  clone or checkout the branch, then pip install requirements
    verify   import the checkout and report what it can actually see

The branch decision is the whole point: the failure mode for a wrong branch is
not a version error but scenarios that "do not exist" or spawn nothing, because
scenario classes, town names and vehicle blueprints all changed between them.
"""
from __future__ import annotations

import argparse
import importlib.util
import os
import shutil
import subprocess
import sys
from pathlib import Path

REPO = "https://github.com/carla-simulator/scenario_runner.git"

# (predicate on the parsed client version) -> branch, why
#
# Ordered; first match wins. Numeric per-component comparison matters here:
# 0.10.0 > 0.9.16, which is also why scenario_runner's own MIN_CARLA_VERSION
# check ('0.9.14') passes on UE5 clients.
BRANCHES = [
    ("ue5-master", lambda v: v >= (0, 10), "CARLA 0.10.0+ is UE5; master targets UE4 maps and blueprints"),
    ("master", lambda v: v >= (0, 9, 14), "master supports CARLA 0.9.14 through 0.9.16"),
    ("leaderboard-1.0", lambda v: v >= (0, 9, 10) and v < (0, 9, 11), "CARLA 0.9.10.1 is the Leaderboard 1.0 build"),
]

FLAVORS = {
    "master": "UE4, CARLA 0.9.14-0.9.16. All towns; ~21 standalone scenario types.",
    "ue5-master": "UE5, CARLA 0.10.0. Town10HD_Opt only; 'vehicle.lincoln.mkz'; weather control disabled; + Scenic.",
    "leaderboard-2.1": "Leaderboard 2.1 (identical commit to leaderboard-2.0).",
    "leaderboard-2.0": "Leaderboard 2.0 (identical commit to leaderboard-2.1).",
    "leaderboard-1.0": "Leaderboard 1.0, CARLA 0.9.10.1.",
    "leaderboard": "Legacy Leaderboard 1.0 branch; prefer leaderboard-1.0.",
}


def sh(cmd: list[str], cwd: str | None = None, check: bool = True) -> str:
    p = subprocess.run(cmd, cwd=cwd, capture_output=True, text=True)
    if check and p.returncode != 0:
        sys.exit(f"FAILED: {' '.join(cmd)}\n{p.stdout}{p.stderr}")
    return (p.stdout or "").strip()


def have(mod: str) -> bool:
    """Is `mod` importable?

    importlib.util.find_spec() imports the PARENT packages of a dotted name, so
    find_spec("agents.navigation.x") raises ModuleNotFoundError when `agents`
    itself is missing — which is exactly the condition being tested.
    """
    try:
        return importlib.util.find_spec(mod) is not None
    except (ImportError, AttributeError, ValueError):
        return False


def parse_version(s: str) -> tuple[int, ...]:
    out = []
    for part in s.split("."):
        digits = "".join(c for c in part if c.isdigit())
        out.append(int(digits) if digits else 0)
    return tuple(out) or (0,)


def client_version() -> str | None:
    """Version of the importable carla client, or None."""
    if not have("carla"):
        return None
    try:
        from importlib.metadata import version

        return version("carla")
    except Exception:
        # A raw .egg on PYTHONPATH has no dist metadata; fall back to the server.
        return "unknown"


def server_version(host: str, port: int) -> str | None:
    try:
        import carla

        c = carla.Client(host, port)
        c.set_timeout(3.0)
        return c.get_server_version()
    except Exception:
        return None


def sr_root(explicit: str | None) -> Path | None:
    cands = [explicit] if explicit else [
        os.environ.get("SCENARIO_RUNNER_ROOT"),
        os.getcwd(),
        str(Path.home() / "scenario_runner"),
        "/workspace/scenario_runner",
    ]
    for c in cands:
        if c and (Path(c) / "scenario_runner.py").is_file() and (Path(c) / "srunner").is_dir():
            return Path(c).resolve()
    return None


def branch_of(root: Path) -> str:
    return sh(["git", "-C", str(root), "rev-parse", "--abbrev-ref", "HEAD"], check=False) or "unknown"


def recommend(ver: str | None) -> tuple[str, str]:
    if not ver or ver == "unknown":
        return "master", "no client version could be read — master is the default for UE4 CARLA"
    if ver == "leaderboard":
        return "leaderboard-2.1", "the client reports version 'leaderboard': this is a Leaderboard CARLA build"
    v = parse_version(ver)
    for name, pred, why in BRANCHES:
        if pred(v):
            return name, why
    return "master", f"CARLA {ver} predates 0.9.14 and is unsupported; master is the closest"


def cmd_detect(args) -> None:
    cv = client_version()
    sv = server_version(args.host, args.port)
    print(f"carla client        : {cv or 'NOT IMPORTABLE (run install-python-api)'}")
    print(f"carla server        : {sv or f'unreachable at {args.host}:{args.port}'}")
    if cv and sv and cv != sv and cv != "unknown":
        print(f"  WARNING           : client/server mismatch ({cv} vs {sv})")
    agents = have("agents.navigation.global_route_planner")
    print(f"`agents` package    : {'importable' if agents else 'MISSING -> add $CARLA_ROOT/PythonAPI/carla to PYTHONPATH'}")
    root = sr_root(args.root)
    if root:
        br = branch_of(root)
        print(f"scenario_runner     : {root} (branch {br})")
        print(f"  flavor            : {FLAVORS.get(br, 'unknown branch — compatibility unverified')}")
    else:
        print("scenario_runner     : not found")
    print(f"git available       : {'yes' if shutil.which('git') else 'NO — required to clone'}")


def cmd_plan(args) -> None:
    cv = client_version()
    sv = server_version(args.host, args.port)
    auto, auto_why = recommend(cv or sv)
    branch = args.branch or auto
    why = "requested explicitly with --branch" if args.branch else auto_why
    root = sr_root(args.root)
    print(f"recommended branch : {branch}\n  because          : {why}")
    print(f"  flavor           : {FLAVORS.get(branch, 'unknown')}")
    print()
    if root:
        cur = branch_of(root)
        if cur == branch:
            print(f"# {root} is already on {branch}; nothing to clone")
        else:
            print(f"# switch the existing checkout ({cur} -> {branch})")
            print(f"git -C {root} fetch origin {branch}")
            print(f"git -C {root} checkout {branch}")
        target = root
    else:
        target = Path(args.root or Path.home() / "scenario_runner")
        print(f"git clone -b {branch} --single-branch {REPO} {target}")
    print(f"{sys.executable} -m pip install -r {target}/requirements.txt")
    print(f"export SCENARIO_RUNNER_ROOT={target}")
    print("export CARLA_ROOT=/path/to/CARLA            # needed for the `agents` package")
    print('export PYTHONPATH="${SCENARIO_RUNNER_ROOT}:${CARLA_ROOT}/PythonAPI/carla:${PYTHONPATH}"')


def cmd_install(args) -> None:
    if not shutil.which("git"):
        sys.exit("git is required")
    cv = client_version()
    branch = args.branch or recommend(cv or server_version(args.host, args.port))[0]
    root = sr_root(args.root)
    if root is None:
        target = Path(args.root or Path.home() / "scenario_runner").expanduser()
        if target.exists() and any(target.iterdir()):
            sys.exit(f"{target} exists and is not a scenario_runner checkout — refusing to touch it")
        print(f"[install] cloning {branch} -> {target}")
        sh(["git", "clone", "-b", branch, "--single-branch", REPO, str(target)])
        root = target
    else:
        cur = branch_of(root)
        if cur != branch:
            dirty = sh(["git", "-C", str(root), "status", "--porcelain"], check=False)
            if dirty and not args.force:
                sys.exit(f"{root} has uncommitted changes; commit/stash them or pass --force\n{dirty}")
            print(f"[install] {root}: {cur} -> {branch}")
            sh(["git", "-C", str(root), "fetch", "origin", branch])
            sh(["git", "-C", str(root), "checkout", branch])
        else:
            print(f"[install] {root} already on {branch}")

    req = root / "requirements.txt"
    if args.no_deps:
        print(f"[install] skipping requirements ({req})")
    else:
        print(f"[install] pip install -r {req} (into {sys.executable})")
        subprocess.run([sys.executable, "-m", "pip", "install", "-r", str(req)], check=True)
        # The CARLA bindings are built against the numpy 1.x C API: numpy 2
        # imports fine and then fails on the first array the simulator hands
        # back. Report rather than silently downgrading someone's environment.
        try:
            import numpy

            if parse_version(numpy.__version__) >= (2,):
                print(f"[install] WARNING numpy {numpy.__version__} is too new for the CARLA bindings")
                print(f"[install]   fix with: {sys.executable} -m pip install 'numpy<2'")
        except Exception:
            pass

    print("\n[install] done. Export these, then run `verify`:")
    print(f"  export SCENARIO_RUNNER_ROOT={root}")
    print("  export CARLA_ROOT=/path/to/CARLA")
    print('  export PYTHONPATH="${SCENARIO_RUNNER_ROOT}:${CARLA_ROOT}/PythonAPI/carla:${PYTHONPATH}"')


def cmd_verify(args) -> None:
    root = sr_root(args.root)
    if root is None:
        sys.exit("no scenario_runner checkout found — run `install` first")
    print(f"checkout : {root} (branch {branch_of(root)})")

    sys.path.insert(0, str(root))
    problems = []
    for mod, hint in [
        ("carla", "run the install-python-api skill"),
        ("agents.navigation.global_route_planner", "add $CARLA_ROOT/PythonAPI/carla to PYTHONPATH"),
        ("py_trees", "pip install -r requirements.txt"),
        ("srunner.scenariomanager.carla_data_provider", "add SCENARIO_RUNNER_ROOT to PYTHONPATH"),
    ]:
        try:
            __import__(mod)
            print(f"import   : {mod} OK")
        except Exception as e:
            print(f"import   : {mod} FAILED ({e}) -> {hint}")
            problems.append(mod)

    try:
        import py_trees

        v = py_trees.__version__
        print(f"py_trees : {v}{'' if v.startswith('0.8') else '  <-- only 0.8.x works'}")
    except Exception:
        pass

    # Count configs without importing carla-dependent code: the same XML files
    # `scenario_runner.py --list` reads.
    import xml.etree.ElementTree as ET

    types, configs = set(), 0
    for f in sorted((root / "srunner" / "examples").glob("*.xml")):
        try:
            for s in ET.parse(f).getroot().iter("scenario"):
                types.add(s.attrib.get("type"))
                configs += 1
        except ET.ParseError as e:
            print(f"warning  : {f.name} is not valid XML ({e})")
    print(f"scenarios: {configs} configs across {len(types)} types in srunner/examples/")
    for d in ("osc_examples", "examples"):
        n = len(list((root / "srunner" / d).glob("*.xosc"))) if (root / "srunner" / d).is_dir() else 0
        if n:
            print(f"xosc     : {n} OpenSCENARIO files in srunner/{d}/")
    if (root / "srunner" / "scenic").is_dir():
        print(f"scenic   : {len(list((root / 'srunner' / 'scenic').glob('*.scenic')))} .scenic files (ue5-master)")

    sv = server_version(args.host, args.port)
    print(f"server   : {sv or f'unreachable at {args.host}:{args.port} (start one to run anything)'}")
    sys.exit(1 if problems else 0)


def main() -> None:
    # The shared flags live on a parent parser, not on the top-level one: with
    # argparse, a top-level optional must precede the subcommand, so
    # `detect --root X` would be an "unrecognized arguments" error. Inheriting
    # them makes both orders work.
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--host", default=os.environ.get("CARLA_HOST", "127.0.0.1"))
    common.add_argument("--port", type=int, default=int(os.environ.get("CARLA_PORT", 2000)))
    common.add_argument("--root", help="scenario_runner checkout path "
                                      "(default: $SCENARIO_RUNNER_ROOT, $PWD, ~/scenario_runner)")
    ap = argparse.ArgumentParser(description=__doc__, parents=[common],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("detect", parents=[common]).set_defaults(func=cmd_detect)
    p = sub.add_parser("plan", parents=[common]); p.add_argument("--branch"); p.set_defaults(func=cmd_plan)
    p = sub.add_parser("install", parents=[common])
    p.add_argument("--branch", help="override the recommendation")
    p.add_argument("--no-deps", action="store_true", help="skip pip install")
    p.add_argument("--force", action="store_true", help="switch branch even with local changes")
    p.set_defaults(func=cmd_install)
    sub.add_parser("verify", parents=[common]).set_defaults(func=cmd_verify)
    args = ap.parse_args()
    args.func(args)


if __name__ == "__main__":
    main()
