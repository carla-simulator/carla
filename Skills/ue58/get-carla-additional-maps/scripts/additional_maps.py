#!/usr/bin/env python3
"""Gets the AdditionalMaps content pack that matches a CARLA UE 5.8 package, and installs it.

  additional_maps.py urls     [--channel release|weekly] [--version 1.0] [--platform Linux|Windows]
  additional_maps.py download [--channel ...] [--dest DIR] [--with-package]
  additional_maps.py build    [--maps-file F | --maps A,B] [--fresh]     (on the ue58-dev checkout)
  additional_maps.py pair     <AdditionalMaps tarball> [--pack-base META] [--package-base META]
  additional_maps.py install  <AdditionalMaps tarball>

Reads CARLA_PACKAGE_ROOT, CARLA_UE58_ROOT, CARLA_PRESET (source scripts/env.sh).
"""
import argparse
import email.utils
import json
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
import urllib.request
from pathlib import Path

MIRROR = "https://downloads.carlasim.com"
HERE = Path(__file__).resolve().parent
ROOT = Path(os.environ.get("CARLA_UE58_ROOT") or "/nonexistent")
PKG = Path(os.environ.get("CARLA_PACKAGE_ROOT") or "/nonexistent")
PRESET = os.environ.get("CARLA_PRESET", "Release")
# A pair is uploaded by one workflow run: the pack lands minutes after the package.
PAIR_WINDOW_S = 6 * 3600


def die(msg):
    print("[maps] ERROR " + msg)
    sys.exit(1)


def names(channel, version, platform):
    ext = ".zip" if platform == "Windows" else ".tar.gz"
    if channel == "weekly":
        d = "{}/Dev".format(platform)
        return {"package": "{}/CARLA_UE5_Weekly{}".format(d, ext),
                "pack": "{}/CARLA_UE5_Weekly_AdditionalMaps.tar.gz".format(d),
                "metadata": "{}/CARLA_UE5_Weekly_release-metadata.tar.gz".format(d)}
    return {"package": "{}/CARLA_{}{}".format(platform, version, ext),
            "pack": "{}/AdditionalMaps_{}.tar.gz".format(platform, version),
            "metadata": "{}/CARLA_{}_release-metadata.tar.gz".format(platform, version)}


def head(url):
    req = urllib.request.Request(url, method="HEAD", headers={"User-Agent": "carla-agentic-tools"})
    try:
        with urllib.request.urlopen(req, timeout=30) as r:
            lm = r.headers.get("Last-Modified")
            return int(r.headers.get("Content-Length") or 0), (email.utils.parsedate_to_datetime(lm) if lm else None)
    except Exception as e:  # 404 and network errors alike: the caller reports "missing"
        return None, str(e)


def cmd_urls(a):
    rows = {}
    for k, path in names(a.channel, a.version, a.platform).items():
        url = "{}/{}".format(MIRROR, path)
        size, when = head(url)
        rows[k] = (url, size, when)
        print("[maps] {:<9} {}  {}".format(k, url, "MISSING ({})".format(when) if size is None else
                                           "{}  {}".format("{:.2f} GB".format(size / 1e9) if size >= 1e8 else "{:.1f} MB".format(size / 1e6), when.strftime("%Y-%m-%d %H:%M UTC"))))
    pkg, pack = rows["package"], rows["pack"]
    if pkg[1] is None or pack[1] is None:
        die("this channel has no complete package + AdditionalMaps pair for {}".format(a.platform))
    gap = (pack[2] - pkg[2]).total_seconds()
    if not 0 <= gap <= PAIR_WINDOW_S:
        print("[maps] WARN pack and package were uploaded {:.1f} h apart: probably not one build; wait for the next upload".format(gap / 3600))
    else:
        print("[maps] pair OK: pack uploaded {:.0f} min after the package (same workflow run)".format(gap / 60))
    return rows


def curl(url, dest, size):
    out = dest / url.rsplit("/", 1)[1]
    if out.is_file() and out.stat().st_size == size:
        print("[maps] have {} ({:.2f} GB)".format(out, size / 1e9))
        return out
    print("[maps] downloading {} ({:.2f} GB, resumable)".format(url, size / 1e9))
    r = subprocess.run(["curl", "-fsSL", "-C", "-", "--retry", "5", "-o", str(out), url])
    if r.returncode != 0 or out.stat().st_size != size:
        die("download of {} incomplete ({} of {} bytes); rerun to resume".format(url, out.stat().st_size if out.exists() else 0, size))
    return out


def cmd_download(a):
    rows = cmd_urls(a)
    dest = Path(a.dest).expanduser().resolve()
    dest.mkdir(parents=True, exist_ok=True)
    want = ["metadata", "pack"] + (["package"] if a.with_package else [])
    done = lambda k: (dest / rows[k][0].rsplit("/", 1)[1]).is_file() and (dest / rows[k][0].rsplit("/", 1)[1]).stat().st_size == rows[k][1]
    need = sum(rows[k][1] for k in want if not done(k)) + (rows["package"][1] if a.with_package else 0)
    if shutil.disk_usage(str(dest)).free < need:
        die("{:.1f} GB free in {}, need ~{:.1f} GB".format(shutil.disk_usage(str(dest)).free / 1e9, dest, need / 1e9))
    got = {k: curl(rows[k][0], dest, rows[k][1]) for k in want}
    if a.with_package:
        print("[maps] extracting {}".format(got["package"]))
        name = got["package"].name
        target = dest / name[:-len(".tar.gz")] if name.endswith(".tar.gz") else dest / got["package"].stem
        target.mkdir(exist_ok=True)
        cmd = ["unzip", "-q", "-o", str(got["package"]), "-d", str(target)] if got["package"].suffix == ".zip" \
            else ["tar", "xzf", str(got["package"]), "-C", str(target)]
        if subprocess.run(cmd).returncode != 0:
            die("extracting {} failed".format(got["package"]))
        launchers = sorted(target.glob("CarlaUnreal.*")) + sorted(target.glob("*/CarlaUnreal.*"))
        launchers = [l for l in launchers if l.suffix in (".sh", ".exe")]
        if not launchers:
            die("no CarlaUnreal.sh / CarlaUnreal.exe under {}: extraction incomplete?".format(target))
        root = launchers[0].parent
        print("[maps] package at {} -> export CARLA_PACKAGE_ROOT={}".format(root, root))
    for k, p in got.items():
        print("[maps] {:<9} {}".format(k, p))


def cmd_build(a):
    tool = ROOT / "Util" / "ContentPacks" / "carla_pack.py"
    if not tool.is_file():
        die("no ue58-dev checkout at CARLA_UE58_ROOT={}".format(ROOT))
    build = ROOT / "Build" / PRESET
    pack_src = ROOT / "Unreal" / "CarlaUnreal" / "Plugins" / "Packs" / a.name
    if a.fresh and pack_src.exists():
        shutil.rmtree(str(pack_src))
        print("[maps] removed the old pack sources {} (create only adds maps)".format(pack_src))
    elif pack_src.is_dir() and (a.maps or a.maps_file):
        old = [m["name"] for m in json.loads((pack_src / "carla-pack.json").read_text()).get("maps", [])]
        if old:
            print("[maps] WARN {} already registers {}: they stay in the pack; --fresh starts from your list only".format(pack_src, ", ".join(old)))
    if not (a.maps or a.maps_file):
        cmd = ["cmake", "--build", str(build), "--target", "package-additional-maps"]
    else:
        cache = (build / "CMakeCache.txt").read_text()
        get = lambda k: next((l.split("=", 1)[1] for l in cache.splitlines() if l.startswith(k + ":")), "")
        version = get("CARLA_VERSION") or "1.0"
        release = get("CARLA_BASE_RELEASE") or "carla-{}-Linux".format(version)
        # Map names must follow the pack name: create's positionals stop at the first option.
        cmd = [sys.executable, str(tool), "create", a.name] + [m.strip() for m in (a.maps or "").split(",") if m.strip()]
        cmd += ["--base", str(ROOT / "Unreal" / "CarlaUnreal" / "Releases" / release),
                "--config", "Shipping", "--pack-version", version, "--out", str(build / "Package"),
                "--description", "CARLA towns not shipped in the default package"]
        cmd += ["--maps-file", a.maps_file] if a.maps_file else []
        if get("CARLA_UNREAL_ENGINE_PATH"):
            cmd += ["--engine", get("CARLA_UNREAL_ENGINE_PATH")]
    print("[maps] " + " ".join(cmd))
    if subprocess.run(cmd).returncode != 0:
        die("building the pack failed")
    tars = sorted((build / "Package").glob("{}-*.tar.gz".format(a.name)), key=lambda p: p.stat().st_mtime)
    if not tars:
        die("no {}-*.tar.gz in {}".format(a.name, build / "Package"))
    print("[maps] built {} ({:.2f} GB)".format(tars[-1], tars[-1].stat().st_size / 1e9))


def registry_names(src):
    """FName set of an AssetRegistry.bin, from a file, a folder or a metadata/pack tarball."""
    for d in (PKG / "Tools", HERE.parents[3] / "Util" / "ContentPacks", ROOT / "Util" / "ContentPacks"):
        if (d / "carla_pack.py").is_file():
            sys.path.insert(0, str(d))
    import carla_pack
    src = Path(src)
    if src.is_dir():
        found = sorted(src.rglob("AssetRegistry.bin"))
        if not found:
            die("no AssetRegistry.bin under {}".format(src))
        return set(carla_pack.registry_names(found[0]))
    if src.suffix == ".bin":
        return set(carla_pack.registry_names(src))
    with tarfile.open(str(src)) as t, tempfile.TemporaryDirectory() as tmp:
        for m in t:
            if m.isfile() and m.name.endswith("/AssetRegistry.bin") and "/Metadata/" not in m.name:
                t.extract(m, tmp)
                return set(carla_pack.registry_names(Path(tmp) / m.name))
    die("no AssetRegistry.bin in {}".format(src))


def cmd_pair(a):
    """Pack references that the pack's base had and the target package's base lacks."""
    if not a.pack_base or not a.package_base:
        tar = Path(a.tarball)
        meta = sorted(PKG.parent.glob("*-release-metadata.tar.gz"))
        pkg_meta = Path(a.package_base) if a.package_base else (meta[-1] if meta else None)
        if pkg_meta and pkg_meta.stat().st_mtime > tar.stat().st_mtime:
            print("[maps] WARN {} is older than the package's metadata {}: rebuild the pack after every package build".format(tar.name, pkg_meta.name))
        if not a.pack_base:
            print("[maps] no --pack-base: give the metadata the pack was cooked against for the reference check")
            return
        a.package_base = str(pkg_meta)
    pack = {n for n in registry_names(a.tarball) if n.startswith("/Game/")}
    old, new = registry_names(a.pack_base), registry_names(a.package_base)
    missing = sorted(pack & (old - new))
    print("[maps] pack names found in its base but not in the package's: {}".format(len(missing)))
    for n in missing[:15]:
        print("[maps]   " + n)
    if missing:
        print("[maps] FAIL the package was cooked with less content than the pack expects: its towns load with content missing")
        sys.exit(1)
    print("[maps] OK every base asset the pack names is in the package")


def cmd_install(a):
    script = HERE.parents[1] / "install-carla-content-pack" / "scripts" / "packs.py"
    r = subprocess.run([sys.executable, str(script), "install", a.tarball] + (["--replace"] if a.replace else []))
    sys.exit(r.returncode)


def main():
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    plat = "Windows" if sys.platform.startswith("win") else "Linux"
    for name, fn in (("urls", cmd_urls), ("download", cmd_download)):
        p = sub.add_parser(name)
        p.add_argument("--channel", choices=("release", "weekly"), default="release")
        p.add_argument("--version", default="1.0")
        p.add_argument("--platform", choices=("Linux", "Windows"), default=plat)
        if name == "download":
            p.add_argument("--dest", default=".")
            p.add_argument("--with-package", action="store_true", help="also download and extract the matching package")
        p.set_defaults(func=fn)
    p = sub.add_parser("build")
    p.add_argument("--maps-file")
    p.add_argument("--maps", help="comma-separated map names")
    p.add_argument("--name", default="AdditionalMaps")
    p.add_argument("--fresh", action="store_true", help="delete Plugins/Packs/<name> first so only the given maps ship")
    p.set_defaults(func=cmd_build)
    p = sub.add_parser("pair")
    p.add_argument("tarball")
    p.add_argument("--pack-base", help="release metadata the pack was cooked against")
    p.add_argument("--package-base", help="release metadata of the target package (default: next to it)")
    p.set_defaults(func=cmd_pair)
    p = sub.add_parser("install")
    p.add_argument("tarball")
    p.add_argument("--replace", action="store_true")
    p.set_defaults(func=cmd_install)
    a = ap.parse_args()
    a.func(a)


if __name__ == "__main__":
    main()
