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
         (<root>/PythonAPI/carla/dist/carla-*-<cp tag>-*.whl)  <- preferred
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

PY = os.environ.get("PYTHON_BIN") or os.environ.get("PYTHON") or sys.executable
PY_XY = os.environ.get("CARLA_PY_XY") or f"{sys.version_info[0]}.{sys.version_info[1]}"
PY_TAG = os.environ.get("CARLA_PY_TAG") or f"cp{PY_XY.replace('.', '')}"
HOST = os.environ.get("CARLA_HOST", "127.0.0.1")
PORT = int(os.environ.get("CARLA_PORT", "2000"))


def roots() -> list[Path]:
    """Places that may ship a client artifact, most specific first."""
    out = []
    for var in ("CARLA_PACKAGE_ROOT", "CARLA_UE4_ROOT"):
        v = os.environ.get(var)
        if v:
            out.append(Path(v).expanduser())
    return out


def find_wheels() -> list[Path]:
    """Bundled wheels matching this interpreter's tag, newest mtime first."""
    hits: list[Path] = []
    for root in roots():
        for pat in (f"PythonAPI/carla/dist/carla-*-{PY_TAG}-*.whl",
                    f"**/PythonAPI/carla/dist/carla-*-{PY_TAG}-*.whl"):
            hits += [Path(p) for p in glob.glob(str(root / pat), recursive=True)]
    return sorted(set(hits), key=lambda p: p.stat().st_mtime, reverse=True)


def find_eggs() -> list[Path]:
    """Bundled eggs for this interpreter (older releases ship these instead)."""
    hits: list[Path] = []
    for root in roots():
        for pat in (f"PythonAPI/carla/dist/carla-*-py{PY_XY}-*.egg",
                    f"**/PythonAPI/carla/dist/carla-*-py{PY_XY}-*.egg"):
            hits += [Path(p) for p in glob.glob(str(root / pat), recursive=True)]
    return sorted(set(hits), key=lambda p: p.stat().st_mtime, reverse=True)


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
    """Version implied by a local artifact name, e.g. carla-0.9.16-cp310-...whl."""
    for p in find_wheels() + find_eggs():
        m = re.search(r"carla-(\d+\.\d+\.\d+)", p.name)
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
        print("  (no CARLA_PACKAGE_ROOT / CARLA_UE4_ROOT set — cannot look for bundled artifacts)")

    want = package_version()
    if want:
        tags = pypi_tags(want)
        ok = PY_TAG in tags
        print(f"pypi carla=={want}   : tags {tags or '?'} -> {'MATCHES' if ok else 'no wheel for ' + PY_TAG}")

    if cur:
        sv = server_version()
        if sv:
            print(f"server version     : {sv} ({'match' if sv == cur else 'MISMATCH with client ' + cur})")
        else:
            print(f"server version     : unreachable at {HOST}:{PORT} (fine — not needed to install)")
    print()
    print("next: install_python_api.py install         (auto-picks the best source)")
    return 0


def pip_install(target: str, dry: bool) -> int:
    """pip into the TARGET interpreter, never into this script's own."""
    cmd = [PY, "-m", "pip", "install", target]
    # A user-site install is the least surprising outside a venv; inside one, pip
    # rejects --user, so only add it when the target is not a venv.
    in_venv = subprocess.run(
        [PY, "-c", "import sys; print(sys.prefix != sys.base_prefix)"],
        capture_output=True, text=True).stdout.strip() == "True"
    if not in_venv:
        cmd.insert(4, "--user")
    # CARLA's bindings are compiled against the numpy 1.x C API and crash on
    # import under 2.x, so pin it in the same transaction.
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

    if src in ("auto", "local") and wheels:
        print(f"source: local wheel ({wheels[0].name}) — matches your simulator")
        rc = pip_install(str(wheels[0]), args.dry_run)
    elif src in ("auto", "pypi"):
        version = args.version or package_version()
        if not version:
            print("no version known: pass --version X.Y.Z, or set CARLA_PACKAGE_ROOT so the "
                  "bundled artifact can be read", file=sys.stderr)
            return 2
        tags = pypi_tags(version)
        if tags and PY_TAG not in tags:
            print(f"PyPI has no {PY_TAG} wheel for carla=={version} (has {tags}).", file=sys.stderr)
            print("Options: use a different interpreter (PYTHON), a local wheel, "
                  "or build it (build-carla-ue4 step 04).", file=sys.stderr)
            return 2
        print(f"source: PyPI carla=={version}")
        rc = pip_install(f"carla=={version}", args.dry_run)
    elif src in ("auto", "egg") and eggs:
        egg = eggs[0]
        print(f"source: bundled egg ({egg.name}) — nothing to install, it goes on PYTHONPATH")
        print(f'  export PYTHONPATH="{egg}:$PYTHONPATH"')
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
    if sv == cur:
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
