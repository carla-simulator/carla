#!/usr/bin/env python3
"""Install / verify Scenic for the CARLA in use.

Subcommands
    detect   what Scenic, what CARLA client and what interpreter are present (read-only)
    plan     print the exact commands for this machine, and why (read-only)
    install  pip install Scenic into the interpreter that has `carla`
    verify   prove the CLI, the client and the blueprint table agree

Scenic is pure Python — nothing is built. The whole problem is that Scenic's
`scenic` CLI and CARLA's `carla` client must live in ONE interpreter, and that
Scenic's blueprint tables are keyed on the CARLA *client* version. Miss either and
the failure is not an install error: it is a model import that fails mid-run, or
every vehicle category resolving empty.
"""
from __future__ import annotations

import argparse
import importlib.util
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

PKG = "scenic"
REPO = "https://github.com/BerkeleyLearnVerify/Scenic.git"


def sh(cmd: list[str], check: bool = True) -> str:
    p = subprocess.run(cmd, capture_output=True, text=True)
    if check and p.returncode != 0:
        raise SystemExit("command failed: %s\n%s%s" % (" ".join(cmd), p.stdout, p.stderr))
    return (p.stdout + p.stderr).strip()


def py(args) -> str:
    """The interpreter to install into. Defaults to the one running this script,
    which is the one the caller chose — not `python3` off PATH."""
    return getattr(args, "python", None) or sys.executable


def dist_version(interp: str, pkg: str) -> str | None:
    p = subprocess.run(
        [interp, "-c",
         "from importlib.metadata import version;print(version(%r))" % pkg],
        capture_output=True, text=True)
    return p.stdout.strip() or None


def module_file(interp: str, mod: str) -> str | None:
    p = subprocess.run([interp, "-c", "import %s;print(%s.__file__ or '')" % (mod, mod)],
                       capture_output=True, text=True)
    return p.stdout.strip() or None


def cli_for(interp: str) -> str | None:
    """The scenic CLI that belongs to `interp`, not whatever is first on PATH.
    A pyenv shim on PATH commonly points at a different environment."""
    cand = Path(interp).parent / "scenic"
    if cand.is_file() and os.access(cand, os.X_OK):
        return str(cand)
    return shutil.which("scenic")


def blueprint_versions(interp: str) -> list[str] | None:
    """CARLA versions Scenic ships blueprint tables for."""
    p = subprocess.run(
        [interp, "-c",
         "from scenic.simulators.carla import _blueprintData as b;"
         "import json;print(json.dumps(sorted(b._IDS)))"],
        capture_output=True, text=True)
    try:
        return json.loads(p.stdout.strip())
    except Exception:
        return None


def available_versions(interp: str) -> list[str]:
    out = sh([interp, "-m", "pip", "index", "versions", PKG], check=False)
    for line in out.splitlines():
        if "Available versions:" in line:
            return [v.strip() for v in line.split(":", 1)[1].split(",")]
    return []


def report(args) -> dict:
    interp = py(args)
    d = {
        "interpreter": interp,
        "python": sh([interp, "-c", "import sys;print('.'.join(map(str,sys.version_info[:3])))"]),
        "scenic": dist_version(interp, "scenic"),
        "carla": dist_version(interp, "carla"),
        "carla_module": module_file(interp, "carla"),
        "cli": cli_for(interp),
    }
    d["tables"] = blueprint_versions(interp) if d["scenic"] else None
    return d


def cmd_detect(args) -> None:
    d = report(args)
    print("interpreter : %s (Python %s)" % (d["interpreter"], d["python"]))
    print("scenic      : %s" % (d["scenic"] or "NOT INSTALLED"))
    print("scenic CLI  : %s" % (d["cli"] or "not found"))
    print("carla client: %s" % (d["carla"] or "NOT INSTALLED"))
    if d["carla_module"]:
        print("carla module: %s" % d["carla_module"])
    if d["tables"] is not None:
        print("blueprint tables for CARLA: %s" % ", ".join(d["tables"]))
        if d["carla"] and d["carla"] not in d["tables"]:
            print("  ^ no table for client %s — every category resolves empty" % d["carla"])
    # The CLI must belong to this interpreter or a run picks up a different Scenic.
    if d["cli"] and d["scenic"]:
        owner = Path(d["cli"]).parent
        if owner != Path(d["interpreter"]).parent:
            print("WARNING: the CLI at %s does not belong to %s" % (d["cli"], d["interpreter"]))


def cmd_plan(args) -> None:
    d = report(args)
    interp = d["interpreter"]
    print("# Scenic install plan for %s\n" % interp)
    if not d["carla"]:
        print("# This interpreter has no `carla` client. Install that FIRST — Scenic's")
        print("# blueprint tables are keyed on the client version, and the CLI must share")
        print("# the interpreter. See the install-python-api skill.")
        print("#   the CARLA client wheel is built per Python version; only the matching one exists")
        print()
    if d["scenic"]:
        print("# scenic %s already installed" % d["scenic"])
        if d["carla"] and d["tables"] and d["carla"] not in d["tables"]:
            print("# but it has no blueprint table for client %s — upgrade:" % d["carla"])
            print("%s -m pip install --upgrade %s" % (interp, PKG))
            av = available_versions(interp)
            if av:
                print("#   available: %s" % ", ".join(av[:8]))
        else:
            print("# nothing to do; run `verify`")
    else:
        print("%s -m pip install %s" % (interp, PKG))
    print()
    print("# The wheel ships world models only — no example scenarios. Two sources:")
    print("#   scenario_runner's port:  $SCENARIO_RUNNER_ROOT/srunner/scenic/*.scenic")
    print("#   Scenic upstream:         git clone %s   (examples/ + assets/maps/)" % REPO)
    print("# Set SCENIC_ROOT to a checkout to get examples; the scenic skill group")
    print("# needs SCENIC_ROOT set either way. An export lasts until the shell")
    print("# exits — record it with set_config to keep it.")


def cmd_install(args) -> None:
    interp = py(args)
    if not dist_version(interp, "carla") and not args.force:
        raise SystemExit(
            "refusing to install: %s has no `carla` client.\n"
            "Scenic's blueprint tables are keyed on the client version and the CLI must\n"
            "share the interpreter, so install the client first (install-python-api skill).\n"
            "Pass --force to install Scenic anyway." % interp)
    spec = PKG if not args.version else "%s==%s" % (PKG, args.version)
    print("installing %s into %s" % (spec, interp))
    print(sh([interp, "-m", "pip", "install", "--upgrade", spec]).splitlines()[-1])
    if args.clone:
        dest = Path(args.clone)
        if dest.exists():
            print("checkout already at %s — leaving it alone" % dest)
        else:
            print("cloning examples to %s" % dest)
            sh(["git", "clone", "--depth", "1", REPO, str(dest)])
        print("export SCENIC_ROOT=%s" % dest)
    cmd_verify(args)


def cmd_verify(args) -> None:
    interp = py(args)
    rc = 0
    d = report(args)

    def ok(m): print("  PASS %s" % m)
    def bad(m):
        nonlocal rc
        print("  FAIL %s" % m); rc = 1

    ok("scenic %s" % d["scenic"]) if d["scenic"] else bad("scenic not importable from %s" % interp)
    ok("CLI at %s" % d["cli"]) if d["cli"] else bad("no scenic CLI for this interpreter")
    if d["cli"] and Path(d["cli"]).parent != Path(interp).parent:
        bad("CLI %s belongs to a different environment than %s" % (d["cli"], interp))
    ok("carla client %s" % d["carla"]) if d["carla"] else bad("no carla client in this interpreter")
    if d["carla_module"] and d["carla_module"].endswith("__init__.py"):
        # A directory named `carla` on sys.path imports as a namespace package.
        bad("`carla` resolved to a package directory, not the client extension")
    if d["scenic"] and d["carla"]:
        t = d["tables"] or []
        if d["carla"] in t:
            ok("blueprint table present for client %s" % d["carla"])
        else:
            bad("no blueprint table for client %s (has: %s) — upgrade scenic"
                % (d["carla"], ", ".join(t) or "none"))
    # The world model is model.scenic — a Scenic file, not an importable module.
    # What actually breaks when the two halves live in different environments is
    # the simulator interface, which does `import carla` at module load. Test that.
    p = subprocess.run([interp, "-c", "import scenic.simulators.carla.simulator"],
                       capture_output=True, text=True)
    if p.returncode == 0:
        ok("scenic.simulators.carla.simulator imports (binds Scenic to this client)")
    else:
        bad("simulator import failed: %s" % (p.stderr.strip().splitlines()[-1:] or "?"))
    print("  verify %s" % ("OK" if rc == 0 else "FAILED"))
    raise SystemExit(rc)


def main() -> None:
    common = argparse.ArgumentParser(add_help=False)
    # SUPPRESS, or the subparser re-parses --python with a None default and
    # overwrites a value given before the subcommand.
    common.add_argument("--python", default=argparse.SUPPRESS,
                        help="interpreter to act on (default: the one running this)")
    ap = argparse.ArgumentParser(description=__doc__, parents=[common],
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("detect", parents=[common]).set_defaults(func=cmd_detect)
    sub.add_parser("plan", parents=[common]).set_defaults(func=cmd_plan)
    p = sub.add_parser("install", parents=[common])
    p.add_argument("--version", help="pin a Scenic version")
    p.add_argument("--clone", help="also git clone Scenic here, for examples/ and assets/")
    p.add_argument("--force", action="store_true", help="install even with no carla client")
    p.set_defaults(func=cmd_install)
    sub.add_parser("verify", parents=[common]).set_defaults(func=cmd_verify)
    a = ap.parse_args()
    a.func(a)


if __name__ == "__main__":
    main()
