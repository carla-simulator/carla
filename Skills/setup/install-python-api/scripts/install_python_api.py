#!/usr/bin/env python3
"""Install the CARLA Python client (`carla`) into a chosen interpreter.

Commands:
    detect                     what is installed, what sources exist, what matches
    install [--source auto|local|pypi|egg] [--version X.Y.Z] [--dry-run]
                               install the client and verify `import carla`
    verify                     import carla and compare client vs server version

Sources, in the order `auto` tries them — the order matters because the client and
the simulator must be the SAME version, and only a locally bundled artifact is
guaranteed to match the simulator you actually have:

  local  a wheel shipped with your release/checkout
         (<root>/PythonAPI/carla/dist/carla-*-<cp tag>-*.whl, or
         <root>/Build/<preset>/PythonAPI/dist/ on a UE5 checkout)  <- preferred
  pypi   `pip install carla==<version>` from PyPI. Available for 0.9.16 on
         cp310/311/312 and 0.9.15 on cp37..cp310, linux + windows only.
  egg    an .egg shipped with older releases: nothing to install, it goes on
         PYTHONPATH. Printed as an export line (and written to the venv's
         site-packages as a .pth when installing into a venv).

Not attempted here: building from source (that is the build-carla-ue4 skill's
step 04, `make PythonAPI`), and conda/docker distributions.

Connection/interpreter/roots come from env.sh.
"""
from __future__ import annotations

import argparse
import glob
import os
import re
import subprocess
import sys
from pathlib import Path

def _native(path: str) -> str:
    """Git Bash hands a native Windows Python `/d/x` paths it cannot open.

    Rewrites the MSYS drive form to `D:/x` on win32 and leaves every other
    path, and every other platform, untouched.
    """
    if sys.platform == "win32":
        m = re.match(r"^/([A-Za-z])(/.*)?$", path)
        if m:
            return f"{m.group(1).upper()}:{m.group(2) or '/'}"
    return path


PY = _native(os.environ.get("PYTHON_BIN") or os.environ.get("PYTHON") or sys.executable)
PY_XY = os.environ.get("CARLA_PY_XY") or f"{sys.version_info[0]}.{sys.version_info[1]}"
PY_TAG = os.environ.get("CARLA_PY_TAG") or f"cp{PY_XY.replace('.', '')}"
HOST = os.environ.get("CARLA_HOST", "127.0.0.1")
PORT = int(os.environ.get("CARLA_PORT", "2000"))

# Every variable the rest of the skill library uses to name a CARLA, most
# specific first: a standalone package, then the generic target, then checkouts.
ROOT_VARS = ("CARLA_PACKAGE_ROOT", "CARLA_TARGET", "CARLA_ROOT",
             "CARLA_UE58_ROOT", "CARLA_UE5_ROOT", "CARLA_UE4_ROOT")
# Searched recursively as well (an extracted release may nest its PythonAPI one
# level down). A UE5 checkout is not: it holds millions of content files.
DEEP_VARS = ("CARLA_PACKAGE_ROOT", "CARLA_TARGET", "CARLA_UE4_ROOT")
UE5_TREE = "Unreal/CarlaUnreal"

# Where a wheel or egg lives: a release or 0.9.x checkout keeps it under
# PythonAPI/carla/dist, a UE5 CMake checkout under Build/<preset>/PythonAPI/dist.
ARTIFACT_DIRS = ("PythonAPI/carla/dist", "Build/*/PythonAPI/dist")


def roots() -> list[Path]:
    """Places that may ship a client artifact, most specific first."""
    return [p for p, _ in _roots()]


def _roots() -> list[tuple[Path, bool]]:
    out: list[tuple[Path, bool]] = []
    for var in ROOT_VARS:
        v = os.environ.get(var)
        if v:
            p = Path(_native(v)).expanduser()
            if p not in [q for q, _ in out]:
                out.append((p, var in DEEP_VARS and not (p / UE5_TREE).is_dir()))
    return out


def _find(name: str) -> list[Path]:
    """Artifacts called `name` (a glob) under every root, newest mtime first."""
    hits: list[Path] = []
    for root, deep in _roots():
        dirs = ARTIFACT_DIRS + (("**/PythonAPI/carla/dist",) if deep else ())
        for d in dirs:
            hits += [Path(p) for p in glob.glob(str(root / d / name), recursive=True)]
    return sorted(set(hits), key=lambda p: p.stat().st_mtime, reverse=True)


def _glob_wheels(tag: str) -> list[Path]:
    return _find(f"carla-*-{tag}-*.whl")


def find_wheels() -> list[Path]:
    """Bundled wheels matching this interpreter's tag, newest mtime first."""
    return _glob_wheels(PY_TAG)


def other_tag_wheels() -> list[Path]:
    """Bundled wheels built for a DIFFERENT interpreter: present, but unusable here."""
    return [w for w in _glob_wheels("*") if f"-{PY_TAG}-" not in w.name]


def same_release(a: str, b: str) -> bool:
    """Client and server name one release, ignoring a build suffix.

    A source-built server reports `1.0-fe0178f89` (version + commit) while the
    wheel's metadata says `1.0`; comparing whole strings flagged that as a
    mismatch when both came from the same tree.
    """
    return a.split("-")[0] == b.split("-")[0]


def find_eggs() -> list[Path]:
    """Bundled eggs for this interpreter (older releases ship these instead)."""
    return _find(f"carla-*-py{PY_XY}-*.egg")


def installed_version() -> str | None:
    """The `carla` version importable by the TARGET interpreter, or None."""
    code = (
        "import importlib.metadata as m\n"
        "try:\n"
        "    import carla\n"
        "except Exception:\n"
        "    print('')\n"
        "else:\n"
        "    try: print(m.version('carla'))\n"
        "    except Exception: print('unknown')\n"
    )
    r = subprocess.run([PY, "-c", code], capture_output=True, text=True)
    v = r.stdout.strip()
    return v or None


def server_version() -> str | None:
    """Ask a running simulator for its version, if one is reachable."""
    code = (
        "import carla, sys\n"
        f"c = carla.Client({HOST!r}, {PORT})\n"
        "c.set_timeout(5.0)\n"
        "print(c.get_server_version())\n"
    )
    r = subprocess.run([PY, "-c", code], capture_output=True, text=True)
    return r.stdout.strip() or None


def package_version() -> str | None:
    """Version implied by a local artifact name, e.g. carla-0.9.16-cp310-...whl
    or carla-1.0-cp312-...whl."""
    for p in find_wheels() + find_eggs():
        m = re.search(r"carla-(\d+(?:\.\d+)+)", p.name)
        if m:
            return m.group(1)
    return None


def pypi_tags(version: str) -> list[str]:
    """Python tags PyPI actually has wheels for, so we fail before pip does."""
    import json
    import urllib.request

    try:
        with urllib.request.urlopen("https://pypi.org/pypi/carla/json", timeout=15) as fh:
            data = json.load(fh)
    except Exception as exc:  # offline is a normal state for this skill
        print(f"  (could not reach PyPI: {exc})")
        return []
    files = data.get("releases", {}).get(version, [])
    return sorted({f["filename"].split("-")[2] for f in files
                   if f["packagetype"] == "bdist_wheel"})


def cmd_detect(_args) -> int:
    print(f"target interpreter : {PY} ({PY_XY}, wheel tag {PY_TAG})")
    cur = installed_version()
    print(f"carla installed    : {cur or 'NO'}")

    wheels, eggs = find_wheels(), find_eggs()
    print(f"bundled wheels     : {len(wheels)}")
    for w in wheels[:3]:
        print(f"  {w}")
    print(f"bundled eggs       : {len(eggs)}")
    for e in eggs[:3]:
        print(f"  {e}")
    if not roots():
        print(f"  (none of {', '.join(ROOT_VARS)} is set — cannot look for bundled artifacts)")
    elif not wheels:
        _report_tag_mismatch()

    want = package_version()
    if want:
        tags = pypi_tags(want)
        ok = PY_TAG in tags
        print(f"pypi carla=={want}   : tags {tags or '?'} -> {'MATCHES' if ok else 'no wheel for ' + PY_TAG}")

    if cur:
        sv = server_version()
        if sv:
            print(f"server version     : {sv} ({'match' if same_release(sv, cur) else 'MISMATCH with client ' + cur})")
        else:
            print(f"server version     : unreachable at {HOST}:{PORT} (fine — not needed to install)")
    print()
    print("next: install_python_api.py install         (auto-picks the best source)")
    return 0


def _report_tag_mismatch() -> None:
    """Say so when the tree HAS wheels, just none for this interpreter.

    Otherwise "0 bundled wheels" sends the user hunting for a wheel that is
    sitting right there, built for another Python.
    """
    others = other_tag_wheels()
    if others:
        tags = sorted({w.name.split("-")[2] for w in others if w.name.count("-") >= 4})
        print(f"  tag mismatch: found {len(others)} wheel(s) for {', '.join(tags) or '?'}, "
              f"none for {PY_TAG} ({PY}) — pick that interpreter with PYTHON, or rebuild "
              f"the wheel with this one")
        for w in others[:3]:
            print(f"    {w}")


def needs_numpy1(version: str) -> bool:
    """Releases before 1.0 are built against the numpy 1.x C API; 1.0+ against 2.x."""
    m = re.match(r"(\d+)", version)
    return bool(m) and int(m.group(1)) < 1


def pip_install(target: str, version: str, dry: bool) -> int:
    """pip into the TARGET interpreter, never into this script's own."""
    cmd = [PY, "-m", "pip", "install", target]
    # A user-site install is the least surprising outside a venv; inside one, pip
    # rejects --user, so only add it when the target is not a venv.
    in_venv = subprocess.run(
        [PY, "-c", "import sys; print(sys.prefix != sys.base_prefix)"],
        capture_output=True, text=True).stdout.strip() == "True"
    if not in_venv:
        cmd.insert(4, "--user")
    # Pre-1.0 bindings fail on import under numpy 2: pin in the same transaction.
    if needs_numpy1(version):
        cmd.append("numpy<2")
    print("  " + " ".join(cmd))
    if dry:
        print("  (dry run — nothing executed)")
        return 0
    return subprocess.run(cmd).returncode


def cmd_install(args) -> int:
    src = args.source
    wheels, eggs = find_wheels(), find_eggs()
    cur = installed_version()
    if cur and not args.force:
        print(f"carla {cur} is already importable by {PY} — nothing to do (--force to reinstall)")
        return 0

    if src in ("auto", "local") and not wheels:
        _report_tag_mismatch()
    if src in ("auto", "local") and wheels:
        print(f"source: local wheel ({wheels[0].name}) — matches your simulator")
        rc = pip_install(str(wheels[0]), wheels[0].name.split("-")[1], args.dry_run)
    elif src in ("auto", "pypi"):
        version = args.version or package_version()
        if not version:
            print("no version known: pass --version X.Y.Z, or set CARLA_PACKAGE_ROOT (or "
                  "CARLA_TARGET) so the bundled artifact can be read", file=sys.stderr)
            return 2
        tags = pypi_tags(version)
        if tags and PY_TAG not in tags:
            print(f"PyPI has no {PY_TAG} wheel for carla=={version} (has {tags}).", file=sys.stderr)
            print("Options: use a different interpreter (PYTHON), a local wheel, "
                  "or build it (build-carla-ue4 step 04).", file=sys.stderr)
            return 2
        print(f"source: PyPI carla=={version}")
        rc = pip_install(f"carla=={version}", version, args.dry_run)
    elif src in ("auto", "egg") and eggs:
        egg = eggs[0]
        print(f"source: bundled egg ({egg.name}) — nothing to install, it goes on PYTHONPATH")
        # os.pathsep: a native Windows Python splits PYTHONPATH on ';'.
        print(f'  export PYTHONPATH="{egg}{os.pathsep}$PYTHONPATH"')
        # In a venv we can make it permanent without touching the user's shell.
        site = subprocess.run([PY, "-c", "import site;print(site.getsitepackages()[0])"],
                              capture_output=True, text=True).stdout.strip()
        if site and not args.dry_run:
            pth = Path(site) / "carla-egg.pth"
            try:
                pth.write_text(f"{egg}\n")
                print(f"  wrote {pth}")
            except OSError as exc:
                print(f"  (could not write {pth}: {exc}; use the export line above)")
        return 0
    else:
        print(f"no usable source for --source {src}", file=sys.stderr)
        return 2

    if rc != 0 or args.dry_run:
        return rc
    return cmd_verify(args)


def cmd_verify(_args) -> int:
    cur = installed_version()
    if not cur:
        print("FAIL: carla is still not importable by " + PY, file=sys.stderr)
        return 1
    print(f"PASS: carla {cur} importable by {PY}")
    sv = server_version()
    if sv is None:
        print(f"      no server at {HOST}:{PORT} to check against (start one to confirm the match)")
        return 0
    if same_release(sv, cur):
        print(f"PASS: server version {sv} matches the client")
        return 0
    # Not fatal: CARLA tolerates some skew but warns, and subtle API gaps follow.
    print(f"WARN: client {cur} vs server {sv} — mismatched versions; expect API warnings")
    return 0


def main() -> None:
    p = argparse.ArgumentParser(description="Install the CARLA Python client.")
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("detect", help="report interpreter, sources and versions").set_defaults(func=cmd_detect)
    pi = sub.add_parser("install", help="install the client and verify it")
    pi.add_argument("--source", choices=("auto", "local", "pypi", "egg"), default="auto")
    pi.add_argument("--version", help="explicit CARLA version for --source pypi")
    pi.add_argument("--dry-run", action="store_true", help="print the command, change nothing")
    pi.add_argument("--force", action="store_true", help="reinstall even if carla already imports")
    pi.set_defaults(func=cmd_install)
    sub.add_parser("verify", help="import carla and compare with the server").set_defaults(func=cmd_verify)
    args = p.parse_args()
    raise SystemExit(args.func(args))


if __name__ == "__main__":
    main()
