#!/usr/bin/env python3
"""Checks a built pack tarball beyond its exit code: checksums, containers, sidecars, clashes.

Usage: check_pack.py <Pack-ver-release.tar.gz> --tool carla_pack.py [--packs-root DIR] [--server PKG]
"""
import argparse
import json
import subprocess
import sys
import tarfile
from pathlib import Path


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("tarball")
    ap.add_argument("--tool", required=True, help="Util/ContentPacks/carla_pack.py")
    ap.add_argument("--packs-root", help="Plugins/Packs, to find other packs shipping the same maps")
    ap.add_argument("--server", help="a CARLA package to check compatibility against")
    a = ap.parse_args()
    tar = Path(a.tarball)
    bad = []

    def run(*args):
        return subprocess.run([sys.executable, a.tool, *args], capture_output=True, text=True)

    r = run("verify", str(tar))
    print("[check] verify: " + (r.stdout.strip().splitlines() or ["?"])[-1])
    if r.returncode != 0:
        bad.append("checksum verification failed:\n" + r.stdout + r.stderr)

    r = run("inspect", str(tar), "--json")
    if r.returncode != 0:
        print(r.stdout + r.stderr)
        sys.exit("[check] FAIL cannot read the manifest")
    m = json.loads(r.stdout)
    m = m.get("manifest", m)
    files = m.get("files", {})
    size = sum(info.size for info in tarfile.open(tar).getmembers() if info.isfile())
    print("[check] {} {}  base {}  engine {} {}  platform {}".format(
        m["name"], m["version"], m.get("base_release") or "(none)",
        m.get("engine", {}).get("version"), m.get("engine", {}).get("commit"), m.get("platform")))
    print("[check] tarball {} ({:.1f} MB compressed, {:.1f} MB unpacked)".format(
        tar, tar.stat().st_size / 1e6, size / 1e6))
    if not m.get("base_release"):
        bad.append("manifest has no base_release: the pack was never built")
    for ext in (".pak", ".utoc", ".ucas"):
        if not any(f.endswith(ext) and "/Paks/" in f for f in files):
            bad.append("no {} container: the cook produced nothing".format(ext))

    for mp in m.get("maps", []):
        have = [k for k in ("xodr", "nav", "tm") if mp.get(k)]
        print("[check] map {:<16} {}  [{}]".format(mp["name"], mp["package"], ", ".join(have) or "no sidecars"))
        if not mp.get("xodr"):
            bad.append("map {} has no OpenDRIVE: it renders but has no waypoints or Traffic Manager".format(mp["name"]))
        if not mp.get("nav"):
            print("[check]   WARN {} has no navigation .bin: walkers cannot navigate it".format(mp["name"]))
    for cat in m.get("catalogs", []):
        print("[check] catalog {}".format(cat))
    if not m.get("maps") and not m.get("catalogs"):
        bad.append("the pack declares no maps and no catalogs: nothing would be spawnable or loadable")

    # Two packs shipping one town both mount; which assets win is undefined.
    if a.packs_root:
        mine = m.get("maps", [])
        for other in sorted(Path(a.packs_root).glob("*/carla-pack.json")):
            om = json.loads(other.read_text())
            if om.get("name") == m["name"]:
                continue
            clash = sorted({x["name"] for x in mine for y in om.get("maps", []) if x["package"] == y["package"] or x["name"] == y["name"]})
            if clash:
                print("[check]   WARN pack {} also ships {}: never install both, only one copy is loadable".format(om["name"], ", ".join(clash)))

    if a.server:
        r = run("inspect", str(tar), "--server", a.server)
        print("[check] " + "\n[check] ".join(l for l in r.stdout.splitlines() if l.startswith(("Server", "Compatible", "  -"))))
        if r.returncode != 0:
            bad.append("not compatible with {}".format(a.server))

    if bad:
        print("\n[check] FAIL\n  - " + "\n  - ".join(bad))
        sys.exit(1)
    print("[check] OK")


if __name__ == "__main__":
    main()
