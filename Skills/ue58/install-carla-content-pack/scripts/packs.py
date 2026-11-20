#!/usr/bin/env python3
"""Installs, lists, checks and removes content packs in a CARLA package, with the checks carla-pack skips.

  packs.py list                       installed packs, their maps, and clashes
  packs.py check   <pack.tar.gz>      fit with this package: base release, platform, engine, clashes, disk
  packs.py install <pack.tar.gz> [--replace] [--allow-clash] [--force]
  packs.py remove  <Pack>             delete <package>/CarlaUnreal/Packs/<Pack> (server stopped)

Reads CARLA_PACKAGE_ROOT and CARLA_PACK_TOOL (source scripts/env.sh).
"""
import argparse
import json
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path

PKG = Path(os.environ.get("CARLA_PACKAGE_ROOT") or "/nonexistent")
TOOL = os.environ.get("CARLA_PACK_TOOL") or ""
PACKS = PKG / "CarlaUnreal" / "Packs"


def die(msg, code=1):
    print("[packs] ERROR " + msg)
    sys.exit(code)


def need_package():
    if not ((PKG / "CarlaUnreal.sh").is_file() or (PKG / "CarlaUnreal.exe").is_file()):
        die("no CARLA package at CARLA_PACKAGE_ROOT={} (the folder holding CarlaUnreal.sh)".format(PKG))
    if not TOOL or not Path(TOOL).is_file():
        die("no carla_pack.py: the package's Tools/ or a checkout's Util/ContentPacks/ (CARLA_PACK_TOOL)")


def tool(*args, capture=True):
    return subprocess.run([sys.executable, TOOL, *args], capture_output=capture, text=True)


def stamp():
    for f in (PKG / "CarlaUnreal" / "BaseRelease", PKG / "BaseRelease"):
        if f.is_file():
            return f.read_text().strip()
    return None


def engine_hash():
    """The engine commit in the package's VERSION file, or None."""
    f = PKG / "VERSION"
    if f.is_file():
        m = re.search(r"UnrealEngine git hash:\s*([0-9a-f]{7,40})", f.read_text())
        if m:
            return m.group(1)
    return None


def installed():
    out = []
    for mf in sorted(PACKS.glob("*/carla-pack.json")):
        try:
            out.append((mf.parent, json.loads(mf.read_text())))
        except ValueError:
            out.append((mf.parent, None))
    return out


def server_running():
    """PIDs of a CARLA server started from this package (Linux /proc only)."""
    pids = []
    root = str(PKG.resolve())
    for p in Path("/proc").glob("[0-9]*"):
        try:
            exe = os.readlink(str(p / "exe"))
        except OSError:
            continue
        if exe.startswith(root) and "CarlaUnreal" in exe:
            pids.append(p.name)
    return pids


def problems_for(manifest, skip_name=None):
    """(errors, warnings) of one manifest against this package and the packs installed in it."""
    errs, warns = [], []
    s = stamp()
    if s is None:
        errs.append("package has no BaseRelease stamp: built without content-pack support")
    elif manifest.get("base_release") != s:
        errs.append("base release: pack {!r}, package {!r}".format(manifest.get("base_release"), s))
    eh, ph = (manifest.get("engine") or {}).get("commit") or "", engine_hash()
    if eh and ph and not (ph.startswith(eh) or eh.startswith(ph)):
        warns.append("engine commit: pack {} vs package {} (cooked data needs the same engine build)".format(eh, ph[:12]))
    # A clash is the same package or the same map name: either way only one copy is loadable.
    mine = manifest.get("maps", [])
    for d, om in installed():
        if om is None or om.get("name") in (manifest.get("name"), skip_name):
            continue
        theirs = om.get("maps", [])
        clash = sorted({m["name"] for m in mine for t in theirs if m["package"] == t["package"] or m["name"] == t["name"]})
        if clash:
            warns.append("pack {} already ships {}: only one copy of a town is loadable, and which one is undefined"
                         .format(om["name"], ", ".join(clash)))
    return errs, warns


def read_tar_manifest(tar):
    r = tool("inspect", str(tar), "--json")
    if r.returncode != 0:
        die("cannot read {}: {}".format(tar, (r.stderr or r.stdout).strip()))
    m = json.loads(r.stdout)
    return m.get("manifest", m)


def report(manifest, errs, warns):
    maps = ", ".join(m["name"] for m in manifest.get("maps", [])) or "-"
    cats = ", ".join(Path(c).name for c in manifest.get("catalogs", [])) or "-"
    print("[packs] {} {}  base {}  maps: {}  catalogs: {}".format(
        manifest["name"], manifest["version"], manifest.get("base_release"), maps, cats))
    for e in errs:
        print("[packs]   FAIL " + e)
    for w in warns:
        print("[packs]   WARN " + w)


def cmd_list(a):
    need_package()
    print("[packs] package {}  stamp {}  engine {}".format(PKG, stamp(), (engine_hash() or "?")[:12]))
    items = installed()
    if not items:
        print("[packs] no packs in {}".format(PACKS))
    for d, m in items:
        if m is None:
            print("[packs] {}: carla-pack.json is not valid JSON".format(d.name))
            continue
        errs, warns = problems_for(m)
        report(m, errs, warns)
    pids = server_running()
    if pids:
        print("[packs] a server from this package is running (pid {}): changes apply at its next start".format(", ".join(pids)))


def cmd_check(a):
    need_package()
    m = read_tar_manifest(a.tarball)
    errs, warns = problems_for(m)
    r = tool("inspect", str(a.tarball))
    sizes = re.search(r"Files \((\d+), ([\d.]+) (MB|GB)\)", r.stdout)
    if sizes:
        need = float(sizes.group(2)) * (1e9 if sizes.group(3) == "GB" else 1e6)
        free = shutil.disk_usage(str(PKG)).free
        if free < need * 1.1:
            errs.append("disk: {:.1f} GB free, the pack unpacks to {:.1f} GB".format(free / 1e9, need / 1e9))
    if (PACKS / m["name"]).exists() and not getattr(a, "replace", False):
        warns.append("{} is already installed: install needs --replace (restart the server afterwards)".format(m["name"]))
    report(m, errs, warns)
    return m, errs, warns


def cmd_install(a):
    m, errs, warns = cmd_check(a)
    if errs and not a.force:
        die("not installing (--force overrides the base-release check; only for a pack cooked from this exact package)")
    if any("already ships" in w for w in warns) and not a.allow_clash:
        die("another installed pack ships the same town; remove it first or pass --allow-clash")
    target = PACKS / m["name"]
    if target.exists() and not a.replace:
        die("{} is already installed: pass --replace (with the server stopped)".format(m["name"]))
    if server_running():
        if a.replace and target.exists():
            die("a server from this package is running and {} is installed: stop it before replacing the pack".format(m["name"]))
        print("[packs] WARN a server from this package is running: the pack appears at its next start, or with client.mount_content_pack()")
    # The old copy is moved aside, not deleted, until the new tarball has installed and verified.
    backup = PACKS / ".{}.replaced".format(m["name"])
    if a.replace and target.exists():
        if backup.exists():
            shutil.rmtree(str(backup))
        target.rename(backup)
    args = ["install", str(a.tarball), "--server", str(PKG)] + (["--force"] if a.force else [])
    r = tool(*args, capture=False)
    if r.returncode != 0:
        if backup.exists():
            if target.exists():
                shutil.rmtree(str(target))
            backup.rename(target)
            print("[packs] restored the previously installed {}".format(m["name"]))
        die("carla-pack install failed (exit {})".format(r.returncode))
    if backup.exists():
        shutil.rmtree(str(backup))
        print("[packs] replaced the previously installed {}".format(m["name"]))
    v = tool("verify", str(target))
    print("[packs] installed folder: " + (v.stdout.strip().splitlines() or ["?"])[-1])
    if v.returncode != 0 or not (target / "carla-pack.json").is_file():
        die("{} did not verify after install".format(target))
    print("[packs] OK {} -> {}".format(m["name"], target))
    print("[packs] start or restart the server; check with: manage-content-packs list")


def cmd_remove(a):
    need_package()
    target = PACKS / a.pack
    if not (target / "carla-pack.json").is_file():
        die("no pack {} in {}".format(a.pack, PACKS))
    pids = server_running()
    if pids:
        die("a server from this package is running (pid {}): stop it first; its paks are mounted".format(", ".join(pids)))
    shutil.rmtree(str(target))
    print("[packs] removed {}".format(target))


def main():
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    sub.add_parser("list").set_defaults(func=cmd_list)
    p = sub.add_parser("check")
    p.add_argument("tarball")
    p.set_defaults(func=lambda a: sys.exit(1 if cmd_check(a)[1] else 0))
    p = sub.add_parser("install")
    p.add_argument("tarball")
    p.add_argument("--replace", action="store_true", help="replace an installed pack of the same name (server stopped)")
    p.add_argument("--force", action="store_true", help="ignore a base release mismatch (only for a pack cooked from this exact package)")
    p.add_argument("--allow-clash", action="store_true", help="install although another pack ships the same town")
    p.set_defaults(func=cmd_install)
    p = sub.add_parser("remove")
    p.add_argument("pack")
    p.set_defaults(func=cmd_remove)
    a = ap.parse_args()
    a.func(a)


if __name__ == "__main__":
    main()
