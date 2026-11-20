#!/usr/bin/env python3
"""Runs editor Python with a pack mounted, and imports FBX meshes into a pack as props.

  pack_editor.py import-meshes <Pack> <file.fbx|dir>... [--label Static] [--size Small] [--scale 1.0] [--no-register]
  pack_editor.py run <Pack> <script.py>

Needs CARLA_UE58_ROOT and CARLA_UNREAL_ENGINE_PATH (source scripts/env.sh first).
"""
import argparse
import json
import os
import subprocess
import sys
import tempfile
import time
from pathlib import Path

ROOT = Path(os.environ.get("CARLA_UE58_ROOT") or ".").resolve()
ENGINE = Path(os.environ.get("CARLA_UNREAL_ENGINE_PATH") or "/nonexistent")
sys.path.insert(0, str(ROOT / "Util" / "ContentPacks"))
try:
    import carla_pack  # noqa: E402  the descriptor toggle and catalog checks live there
except ImportError:
    sys.exit("[pack-editor] no Util/ContentPacks/carla_pack.py under CARLA_UE58_ROOT={}".format(ROOT))

PROJECT = ROOT / "Unreal" / "CarlaUnreal" / "CarlaUnreal.uproject"
PROP_EDITOR_SCRIPT = ROOT / "Skills" / "ue58" / "import-carla-ue58-prop" / "scripts" / "editor" / "import_prop_editor.py"
SIZES = ("Tiny", "Small", "Medium", "Big", "Huge")


def pack_dir(name):
    d = ROOT / "Unreal" / "CarlaUnreal" / "Plugins" / "Packs" / name
    if not (d / (name + ".uplugin")).is_file():
        sys.exit("[pack-editor] no pack {} at {} — run 'carla-pack init {}' first".format(name, d, name))
    return d


def run_editor(name, script, env_extra, log):
    """Headless editor with the pack enabled; the .uplugin is restored even on failure."""
    editor = ENGINE / "Engine" / "Binaries" / "Linux" / "UnrealEditor-Cmd"
    if not editor.is_file():
        sys.exit("[pack-editor] no {} (export CARLA_UNREAL_ENGINE_PATH)".format(editor))
    cmd = [str(editor), str(PROJECT), "-run=pythonscript", "-script={}".format(script),
           "-EnablePlugins={}".format(name), "-NullRHI", "-unattended", "-nosplash", "-nopause", "-stdout"]
    print("[pack-editor] {}".format(" ".join(cmd)))
    print("[pack-editor] log {} (the editor takes ~1 min to boot the project)".format(log))
    t0 = time.time()
    # A file, not a pipe: shader workers inherit the pipe and keep it open after exit.
    with carla_pack.cook_time_descriptor(pack_dir(name) / (name + ".uplugin")), open(log, "w") as lf:
        rc = subprocess.run(cmd, env=dict(os.environ, **env_extra), stdout=lf,
                            stderr=subprocess.STDOUT, timeout=3600).returncode
    print("[pack-editor] editor exited {} after {:.0f} s".format(rc, time.time() - t0))
    return rc


def cmd_run(a):
    log = pack_dir(a.pack) / "Saved" / "CarlaPack" / "editor-run.log"
    log.parent.mkdir(parents=True, exist_ok=True)
    rc = run_editor(a.pack, Path(a.script).resolve(), {}, log)
    sys.exit(rc)


def collect_fbx(paths):
    out = []
    for p in map(Path, paths):
        if p.is_dir():
            out += sorted(p.rglob("*.fbx")) + sorted(p.rglob("*.FBX"))
        elif p.is_file():
            out.append(p)
        else:
            sys.exit("[pack-editor] not found: {}".format(p))
    if not out:
        sys.exit("[pack-editor] no .fbx files in {}".format(paths))
    return [p.resolve() for p in out]


def cmd_import(a):
    if not PROP_EDITOR_SCRIPT.is_file():
        sys.exit("[pack-editor] missing {} (the import-carla-ue58-prop skill's editor half)".format(PROP_EDITOR_SCRIPT))
    if a.label not in carla_pack.SEMANTIC_LABELS:
        sys.exit("[pack-editor] --label {} is not a CARLA semantic label: {}".format(a.label, ", ".join(carla_pack.SEMANTIC_LABELS)))
    pdir = pack_dir(a.pack)
    work = pdir / "Saved" / "CarlaPack"
    work.mkdir(parents=True, exist_ok=True)
    fbxs = collect_fbx(a.paths)
    job = {"scale": a.scale, "combine": True, "collision": True, "result": str(work / "import-meshes.json"),
           "props": [{"fbx": str(f), "dest": "/{}/Static/{}/{}".format(a.pack, a.label, f.stem),
                      "size": a.size} for f in fbxs]}
    job_file = work / "import-meshes-job.json"
    job_file.write_text(json.dumps(job, indent=2))
    res_file = Path(job["result"])
    if res_file.exists():
        res_file.unlink()
    rc = run_editor(a.pack, PROP_EDITOR_SCRIPT, {"CARLA_PROP_JOB": str(job_file)}, work / "import-meshes.log")
    if not res_file.is_file():
        sys.exit("[pack-editor] FAIL: no result file (editor exit {}); see {}".format(rc, work / "import-meshes.log"))
    res = json.loads(res_file.read_text())
    for e in res.get("errors", []):
        print("[pack-editor] ERROR {}\n{}".format(e.get("fbx"), e.get("traceback")))
    props = []
    for item in res.get("imported", []):
        mesh = item.get("mesh")
        if not mesh:
            print("[pack-editor] WARN {}: no StaticMesh imported (assets: {})".format(item["fbx"], item["assets"]))
            continue
        uasset = pdir / "Content" / Path(mesh["path"].split(".")[0]).relative_to("/" + a.pack).with_suffix(".uasset")
        if not uasset.is_file():
            sys.exit("[pack-editor] FAIL: the editor reported {} but {} was not written".format(mesh["path"], uasset))
        print("[pack-editor] {} -> {}  {} m, size {}".format(Path(item["fbx"]).name, mesh["path"], mesh["dimensions_m"], mesh["size"]))
        stem = Path(item["fbx"]).stem
        props.append({"name": stem[3:] if stem.lower().startswith("sm_") else stem, "path": mesh["path"], "size": mesh["size"]})
    if not props:
        sys.exit("[pack-editor] FAIL: nothing imported")
    if a.no_register:
        return
    # Merge with the pack's existing props catalog: re-adding a catalog replaces it.
    catalog = pdir / "Content" / "Config" / "{}.Package.json".format(a.pack)
    data = json.loads(catalog.read_text()) if catalog.is_file() else {}
    keep = [p for p in data.get("props", []) if p["name"] not in {q["name"] for q in props}]
    data["props"] = keep + props
    with tempfile.NamedTemporaryFile("w", suffix=".json", delete=False) as tf:
        json.dump(data, tf, indent=2)
    r = subprocess.run([sys.executable, str(ROOT / "Util" / "ContentPacks" / "carla_pack.py"),
                        "add", a.pack, "--props", tf.name])
    os.unlink(tf.name)
    if r.returncode != 0:
        sys.exit("[pack-editor] FAIL: carla-pack add --props refused the catalog")
    print("[pack-editor] registered {} prop(s): {}".format(
        len(props), ", ".join("static.prop." + p["name"].lower() for p in props)))


def main():
    sys.stdout.reconfigure(line_buffering=True)
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = ap.add_subparsers(dest="cmd", required=True)
    p = sub.add_parser("import-meshes", help="import FBX static meshes into /<Pack>/Static/<Label>/ and register them as props")
    p.add_argument("pack")
    p.add_argument("paths", nargs="+")
    p.add_argument("--label", default="Static", help="semantic label folder (default Static)")
    p.add_argument("--size", choices=SIZES, help="prop size (default: from the mesh bounds)")
    p.add_argument("--scale", type=float, default=1.0)
    p.add_argument("--no-register", action="store_true", help="import only, leave the props catalog alone")
    p.set_defaults(func=cmd_import)
    p = sub.add_parser("run", help="run an editor Python script with the pack mounted")
    p.add_argument("pack")
    p.add_argument("script")
    p.set_defaults(func=cmd_run)
    a = ap.parse_args()
    a.func(a)


if __name__ == "__main__":
    main()
