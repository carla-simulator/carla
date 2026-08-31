#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.
"""carla-pack: author, build, inspect, verify and install CARLA content packs.

A content pack is a content-only Unreal plugin cooked as DLC against a CARLA
base release, wrapped with a ``carla-pack.json`` manifest.  See
``Docs/content_packs.md`` for the user-facing description.

Source (pre-cook) layout, inside the CARLA project::

    Unreal/CarlaUnreal/Plugins/Packs/<Pack>/
        <Pack>.uplugin
        carla-pack.json
        Config/PluginSettings.ini           (stages the sidecar dirs loose)
        Content/{Maps, Maps/OpenDrive, Maps/Nav, Maps/TM, Config, Static, Blueprints}

Distributed (post-cook) layout, ``<Pack>-<version>-<base_release>.tar.gz``::

    <Pack>/
        carla-pack.json
        <Pack>.uplugin
        AssetRegistry.bin
        Content/Paks/<Platform>/<Pack>CarlaUnreal-<Platform>.pak|.utoc|.ucas   (<Pack>-<Platform>.* with --rename)
        Content/Config/*.json, Content/Maps/OpenDrive/*.xodr, Content/Maps/Nav/*.bin   (loose sidecars)

``check-base`` reads a base release's AssetRegistry.bin and fails when a pack's
mount root shows up in it (the package target's post-cook leak guard).

Only the Python standard library is used (3.8+).
"""

import argparse
import hashlib
import json
import os
import re
import shutil
import struct
import subprocess
import sys
import tarfile
import tempfile
from pathlib import Path

__version__ = "0.1.0"

MANIFEST_NAME = "carla-pack.json"
BASE_RELEASE_FILE = "BaseRelease"
ASSET_REGISTRY = "AssetRegistry.bin"
ENGINE_ENV = "CARLA_UNREAL_ENGINE_PATH"
DEFAULT_PLATFORM = "Linux"
DEFAULT_CONFIG = "Development"
DEFAULT_PACKS_ROOT = "Plugins/Packs"  # relative to the Unreal project dir
PACKS_INSTALL_DIR = "Packs"           # <server>/CarlaUnreal/Packs/<Pack>
UPLUGIN_CATEGORY = "CARLA Content Packs"
CONTENT_SUBDIRS = (
    "Maps",
    "Maps/OpenDrive",
    "Maps/Nav",
    "Maps/TM",
    "Config",
    "Static",
    "Static/Static",
    "Blueprints",
)
# Non-asset files (catalog JSON, xodr, nav, TM) are not part of a DLC cook; the
# plugin's Config/PluginSettings.ini [StageSettings] makes UAT stage them as loose
# files next to the pak (CopyBuildToStagingDirectory.Automation.cs:1658-1672), which is
# where the runtime reads them. UFS files inside a DLC pak are addressed engine-root
# relative and are unreachable once the pack lives elsewhere, so nothing non-asset
# may go into the pak (spike REPORT.md, Lane A).
SIDECAR_DIRS = ("Content/Config", "Content/Maps/OpenDrive", "Content/Maps/Nav", "Content/Maps/TM")
PLUGIN_SETTINGS_INI = "Config/PluginSettings.ini"
PACK_NAME_RE = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
# Mount points / folders a pack name must not shadow (compared case-insensitively),
# plus every plugin name found under <project>/Plugins at init time.
RESERVED_PACK_NAMES = ("Game", "Engine", "Carla", "CarlaTools", "CarlaExporter", "CarlaUnreal",
                       "Script", "Memory", "Temp", "Config", "Paks", "Packs", "Content", "Plugins",
                       "Saved", "Releases")
KNOWN_PLATFORMS = ("Linux", "Win64", "Mac", "LinuxArm64")
# Semantic tagging (Tagger.cpp GetLabelByFolderName): a mesh is tagged after the folder
# name that follows "Static/" in its path, so pack meshes must live under
# Content/Static/<Label>/... with one of these labels; anything else is tagged None.
SEMANTIC_LABELS = ("Building", "Fence", "Pedestrian", "Pole", "Other", "Road", "RoadLine", "SideWalk",
                   "TrafficSign", "Vegetation", "Car", "Wall", "Sky", "Ground", "Bridge", "RailTrack",
                   "GuardRail", "TrafficLight", "Static", "Dynamic", "Water", "Terrain", "Truck",
                   "Motorcycle", "Bicycle", "Bus", "Rider", "Train", "Rock", "Stone", "Bush")
VERSION_RE = re.compile(r"^\d+\.\d+\.\d+([-+][0-9A-Za-z.-]+)?$")

TOOL_DIR = Path(__file__).resolve().parent
REPO_ROOT = TOOL_DIR.parents[1]  # Util/ContentPacks -> repo root
DEFAULT_PROJECT = REPO_ROOT / "Unreal" / "CarlaUnreal" / "CarlaUnreal.uproject"


class PackError(Exception):
    """A user-facing error; the message is printed and the tool exits 1."""


# --------------------------------------------------------------------------
# Small helpers
# --------------------------------------------------------------------------

def info(msg):
    print("carla-pack: " + msg)


def warn(msg):
    print("carla-pack: warning: " + msg, file=sys.stderr)


def sha256_file(path, chunk=1 << 20):
    h = hashlib.sha256()
    with open(str(path), "rb") as f:
        while True:
            block = f.read(chunk)
            if not block:
                break
            h.update(block)
    return "sha256:" + h.hexdigest()


def load_json(path):
    try:
        with open(str(path), "r", encoding="utf-8") as f:
            return json.load(f)
    except OSError as e:
        raise PackError("cannot read {}: {}".format(path, e))
    except ValueError as e:
        raise PackError("{} is not valid JSON: {}".format(path, e))


def write_bytes_atomic(path, data):
    """Write <path> via a temp file + os.replace so a failure never leaves it truncated."""
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp = path.with_name(path.name + ".carla-pack.tmp")
    try:
        with open(str(tmp), "wb") as f:
            f.write(data)
        os.replace(str(tmp), str(path))
    finally:
        if tmp.exists():
            try:
                tmp.unlink()
            except OSError:
                pass


def save_json(path, data):
    write_bytes_atomic(path, (json.dumps(data, indent=2) + "\n").encode("utf-8"))


def rmtree_force(path):
    """rmtree that first makes the tree writable (hostile archives can leave mode-0 dirs)."""
    path = str(path)
    if not os.path.lexists(path):
        return
    for dirpath, dirnames, filenames in os.walk(path):
        for d in dirnames:
            try:
                os.chmod(os.path.join(dirpath, d), 0o700)
            except OSError:
                pass
        for f in filenames:
            try:
                os.chmod(os.path.join(dirpath, f), 0o600)
            except OSError:
                pass
    shutil.rmtree(path, ignore_errors=True)


def posix(path):
    return str(path).replace(os.sep, "/")


def shell_quote(arg):
    if re.match(r"^[A-Za-z0-9_./:=+,@%-]+$", arg):
        return arg
    return "'" + arg.replace("'", "'\\''") + "'"


def copy_any(src, dst):
    """Copy a file or a directory tree (replacing an existing destination)."""
    src, dst = Path(src), Path(dst)
    if not src.exists():
        raise PackError("no such file or directory: {}".format(src))
    dst.parent.mkdir(parents=True, exist_ok=True)
    if src.is_dir():
        if dst.exists():
            shutil.rmtree(str(dst))
        shutil.copytree(str(src), str(dst), symlinks=False)
    else:
        shutil.copy2(str(src), str(dst))


def carla_version_from_cmake():
    """Read CARLA_VERSION_{MAJOR,MINOR,PATCH} from the repo's CMakeLists.txt."""
    cmake = REPO_ROOT / "CMakeLists.txt"
    if not cmake.is_file():
        return None
    parts = {}
    try:
        for line in cmake.read_text(encoding="utf-8", errors="replace").splitlines():
            m = re.match(r"\s*set\s*\(\s*CARLA_VERSION_(MAJOR|MINOR|PATCH)\s+(\d+)\s*\)", line)
            if m:
                parts[m.group(1)] = m.group(2)
    except OSError:
        return None
    if len(parts) == 3:
        return "{MAJOR}.{MINOR}.{PATCH}".format(**parts)
    return None


# --------------------------------------------------------------------------
# Manifest
# --------------------------------------------------------------------------

def new_manifest(name, version, carla_version, platform=DEFAULT_PLATFORM):
    return {
        "name": name,
        "version": version,
        "carla_version": carla_version,
        "base_release": "",
        "engine": {"version": "", "commit": ""},
        "platform": platform,
        "maps": [],
        "catalogs": [],
        "files": {},
    }


def check_manifest(manifest, where="manifest"):
    required = ("name", "version", "carla_version", "base_release", "engine",
                "platform", "maps", "catalogs", "files")
    missing = [k for k in required if k not in manifest]
    if missing:
        raise PackError("{}: missing manifest keys: {}".format(where, ", ".join(missing)))
    if not PACK_NAME_RE.match(str(manifest["name"])):
        raise PackError("{}: invalid pack name {!r}".format(where, manifest["name"]))
    if not isinstance(manifest["maps"], list) or not isinstance(manifest["catalogs"], list):
        raise PackError("{}: 'maps' and 'catalogs' must be lists".format(where))
    if not isinstance(manifest["files"], dict):
        raise PackError("{}: 'files' must be an object".format(where))
    for key in manifest["files"]:
        if not is_clean_relpath(key):
            raise PackError("{}: files key {!r} is not a normalised relative path".format(where, key))
    return manifest


def is_clean_relpath(rel):
    """True for 'a/b/c' style keys: relative, no '..', no backslashes, no leading './'."""
    if not isinstance(rel, str) or not rel or "\\" in rel or rel.startswith("/") or ":" in rel.split("/")[0]:
        return False
    parts = rel.split("/")
    return all(part not in ("", ".", "..") for part in parts)


def load_manifest(pack_dir):
    path = Path(pack_dir) / MANIFEST_NAME
    if not path.is_file():
        raise PackError("{} is not a content pack (no {})".format(pack_dir, MANIFEST_NAME))
    return check_manifest(load_json(path), str(path))


def uplugin_descriptor(name, version, description, explicitly_loaded=True):
    """Plugin descriptor (authored convention: ExplicitlyLoaded=true).

    The editor and the DLC cooker only mount the content of an *enabled, not
    ExplicitlyLoaded* plugin (PluginManager.cpp:1898/2001), so `build` rewrites the
    authored .uplugin with ExplicitlyLoaded=false for the duration of the cook and
    restores it byte-for-byte afterwards (see cook_time_descriptor); the distributed
    copy keeps ExplicitlyLoaded=true, which is what lets the packaged server mount
    the pack on demand (IPluginManager::MountExplicitlyLoadedPlugin).  Verified by
    the phase-0 spike (REPORT.md, Task 3).

    What the packaged server actually reads from the descriptor (UE 5.8,
    Engine/Source/Runtime/Projects/Private/{PluginDescriptor,PluginManager}.cpp):

    - ``FileVersion`` (3) - required, ``FPluginDescriptor::Read`` fails without it
      (PluginDescriptor.cpp:263-267) and ``AddToPluginsList`` then refuses the file;
    - ``ExplicitlyLoaded`` true - ``MountExplicitlyLoadedPlugin`` (PluginManager.cpp:3309)
      returns false for anything else;
    - ``CanContainContent`` true - ``MountPluginFromExternalSource`` (:3560) registers
      the ``/<Pack>/`` mount point only for a plugin that can contain content;
    - ``Modules`` absent or empty - a non-empty list makes the mount load plugin
      binaries (:3585-3600), which a content pack does not ship;
    - ``EnabledByDefault`` false - not consulted for a pack mounted through
      ``AddToPluginsList``; kept so the engine's own ``Plugins/`` discovery never
      enables a pack that lands there by mistake;
    - ``Installed`` - read (PluginDescriptor.cpp:373) but only consumed by the editor's
      project-file bookkeeping (ProjectManager.cpp:301/384/437, "installed on top of
      the engine", marketplace semantics).  Neither ``AddToPluginsList`` nor the mount
      look at it, so the tool neither writes nor forces it.
    The remaining keys are cosmetic (plugin browser).
    """
    return {
        "FileVersion": 3,
        "Version": 1,
        "VersionName": version,
        "FriendlyName": name,
        "Description": description,
        "Category": UPLUGIN_CATEGORY,
        "CreatedBy": "",
        "CreatedByURL": "",
        "DocsURL": "",
        "MarketplaceURL": "",
        "SupportURL": "",
        "CanContainContent": True,
        "ExplicitlyLoaded": bool(explicitly_loaded),
        "EnabledByDefault": False,
    }


def runtime_uplugin(source_uplugin):
    """The distributed descriptor: the authored one, minimally corrected for the
    packaged server (see uplugin_descriptor for the engine evidence).

    Forces FileVersion (when missing), CanContainContent, ExplicitlyLoaded and
    EnabledByDefault; drops an empty ``Modules`` list (equivalent to absent) and
    refuses a non-empty one (a pack is content-only: the mount would try to load
    binaries the pack does not ship).  Every other authored key, ``Installed``
    included, is passed through untouched.
    """
    desc = load_json(source_uplugin)
    if not isinstance(desc, dict):
        raise PackError("{} is not a plugin descriptor".format(source_uplugin))
    modules = desc.pop("Modules", None)
    if modules:
        raise PackError("{} lists {} code module(s) ({}); a content pack is content-only and "
                        "cannot ship modules".format(source_uplugin, len(modules),
                                                     ", ".join(str(m.get("Name", "?")) for m in modules
                                                               if isinstance(m, dict))))
    if not isinstance(desc.get("FileVersion"), int):
        desc = dict([("FileVersion", 3)] + list(desc.items()))
    desc["CanContainContent"] = True
    desc["ExplicitlyLoaded"] = True
    desc["EnabledByDefault"] = False
    return desc


class cook_time_descriptor(object):
    """Context manager: ExplicitlyLoaded=false / EnabledByDefault=false in the source
    .uplugin while the cooker runs, original bytes restored on exit - also on UAT
    failure and Ctrl-C."""

    def __init__(self, uplugin):
        self.uplugin = Path(uplugin)
        self.original = None

    def __enter__(self):
        desc = load_json(self.uplugin)
        if not isinstance(desc, dict):
            raise PackError("{} is not a plugin descriptor".format(self.uplugin))
        with open(str(self.uplugin), "rb") as f:
            self.original = f.read()
        desc["ExplicitlyLoaded"] = False
        desc["EnabledByDefault"] = False
        try:
            save_json(self.uplugin, desc)   # atomic: temp file + os.replace
        except BaseException:
            self.restore()
            raise
        return self

    def restore(self):
        if self.original is not None:
            write_bytes_atomic(self.uplugin, self.original)

    def __exit__(self, *exc):
        self.restore()
        return False


PLUGIN_SETTINGS_HEADER = "; Written by carla-pack"
STAGE_SECTION = "[StageSettings]"
STAGE_LINES = tuple("+AdditionalNonUSFDirectories={}".format(d) for d in SIDECAR_DIRS)


def merge_plugin_settings(text, wanted=STAGE_LINES):
    """Merge the stage lines into an existing PluginSettings.ini text.

    Returns (new_text, added_lines).  The author's sections, keys, comments and
    blank lines are kept verbatim; the lines of ``wanted`` that are not already in
    the ``[StageSettings]`` section (compared case-insensitively, whitespace
    stripped) are inserted at the end of that section, or the section is appended
    when the file has none.  Nothing is ever removed or reordered.
    """
    eol = "\r\n" if "\r\n" in text else "\n"
    lines = text.splitlines()
    start = -1                       # index of the [StageSettings] header, -1 = no section
    end = len(lines)                 # index of the next section header (or EOF)
    for i, line in enumerate(lines):
        stripped = line.strip()
        if start < 0:
            if stripped.lower() == STAGE_SECTION.lower():
                start = i
        elif stripped.startswith("[") and stripped.endswith("]"):
            end = i
            break
    present = set()
    if start >= 0:
        present = {l.strip().lower() for l in lines[start + 1:end]}
    missing = [w for w in wanted if w.lower() not in present]
    if not missing:
        return text, []
    if start < 0:
        if lines and lines[-1].strip():
            lines.append("")
        lines.append(STAGE_SECTION)
        lines.extend(missing)
    else:
        insert_at = end
        while insert_at > start + 1 and not lines[insert_at - 1].strip():
            insert_at -= 1              # before the blank lines that separate sections
        lines[insert_at:insert_at] = missing
    return eol.join(lines) + eol, missing


def write_plugin_settings(pack_dir):
    """Ensure <pack>/Config/PluginSettings.ini stages the sidecar dirs loose.

    A missing file is created; an existing one - the tool's own or the author's -
    is merged, never overwritten: its sections, keys and comments stay, only the
    ``+AdditionalNonUSFDirectories`` lines that are absent from ``[StageSettings]``
    are added (see merge_plugin_settings).  Returns True when the file changed.
    """
    ini = Path(pack_dir) / PLUGIN_SETTINGS_INI
    if not ini.is_file():
        lines = [PLUGIN_SETTINGS_HEADER + ": stage the non-asset sidecar files as loose files next to",
                 "; the pak (they are read from the pack folder by the CARLA runtime).",
                 STAGE_SECTION] + list(STAGE_LINES)
        write_bytes_atomic(ini, ("\n".join(lines) + "\n").encode("utf-8"))
        return True
    # bytes, not read_text(): universal newlines would turn CRLF into LF
    current = ini.read_bytes().decode("utf-8", "replace")
    merged, added = merge_plugin_settings(current)
    if not added:
        return False
    info("{}: added {} stage line(s) to {}: {}".format(ini, len(added), STAGE_SECTION, ", ".join(added)))
    write_bytes_atomic(ini, merged.encode("utf-8"))
    return True


# --------------------------------------------------------------------------
# Catalogs (the JSON files under Content/Config the CARLA factories read)
# --------------------------------------------------------------------------

# kind -> (top-level key, required keys per item, keys holding asset paths,
#          destination file name under Content/Config)
CATALOG_KINDS = {
    "props-package": ("props", ("name", "path"), ("path",), "{pack}.Package.json"),
    "props": ("Props", ("Name", "Mesh"), ("Mesh",), "PropParameters.json"),
    "vehicles": ("Vehicles", ("Make", "Model", "Class"), ("Class",), "Vehicles.json"),
    "walkers": ("Walkers", ("Id", "Class"), ("Class",), "WalkerParameters.json"),
    "blueprints": ("Blueprints", ("Name", "Path"), ("Path",), "BlueprintParameters.json"),
}
PROP_SIZES = ("Tiny", "Small", "Medium", "Big", "Huge")


def detect_props_kind(data):
    if isinstance(data, dict) and "props" in data:
        return "props-package"
    if isinstance(data, dict) and "Props" in data:
        return "props"
    raise PackError("props catalog must have a top-level 'props' (<Pack>.Package.json shape) "
                    "or 'Props' (PropParameters.json shape) list")


def check_asset_path(value, pack_name, where, warnings):
    if not isinstance(value, str) or not value:
        warnings.append("{}: empty asset path".format(where))
        return
    if not value.startswith("/"):
        warnings.append("{}: {!r} is not a package path (expected /{}/... or /Game/...)"
                        .format(where, value, pack_name))
        return
    if not (value.startswith("/" + pack_name + "/") or value.startswith("/Game/")):
        warnings.append("{}: {!r} is neither under /{}/ nor /Game/ - it will not resolve "
                        "once the pack is mounted".format(where, value, pack_name))


def semantic_label_of(path):
    """Label the tagger derives from a mesh path (folder after the first 'Static'), or None."""
    parts = str(path).split("/")
    for i, part in enumerate(parts[1:], 1):
        if part == "Static" and i + 1 < len(parts):
            return parts[i + 1] if parts[i + 1] in SEMANTIC_LABELS else None
    return None


def validate_catalog(data, kind, pack_name, allow_untagged=False):
    """Return a list of warnings; raise PackError on a shape error or an untagged pack mesh."""
    top, required, path_keys, _ = CATALOG_KINDS[kind]
    warnings = []
    if not isinstance(data, dict) or top not in data:
        raise PackError("{} catalog: expected a JSON object with a {!r} list".format(kind, top))
    items = data[top]
    if not isinstance(items, list):
        raise PackError("{} catalog: {!r} must be a list".format(kind, top))
    for i, item in enumerate(items):
        where = "{}[{}]".format(top, i)
        if not isinstance(item, dict):
            raise PackError("{} catalog: {} is not an object".format(kind, where))
        missing = [k for k in required if k not in item]
        if missing:
            raise PackError("{} catalog: {} is missing {}".format(kind, where, ", ".join(missing)))
        for key in path_keys:
            check_asset_path(item.get(key), pack_name, "{}.{}".format(where, key), warnings)
        size = item.get("size", item.get("Size"))
        if kind in ("props", "props-package"):
            if size not in PROP_SIZES:
                warnings.append("{}: size {!r} is not one of {}".format(where, size, "/".join(PROP_SIZES)))
            mesh = item.get("path", item.get("Mesh"))
            if isinstance(mesh, str) and mesh.startswith("/" + pack_name + "/") and semantic_label_of(mesh) is None:
                msg = ("{}: {!r} gets no semantic segmentation label; pack meshes are tagged by the "
                       "folder after 'Static' (/{}/Static/<Label>/..., Label one of {})"
                       .format(where, mesh, pack_name, ", ".join(SEMANTIC_LABELS)))
                if not allow_untagged:
                    raise PackError(msg + "; move the mesh or pass --allow-untagged")
                warnings.append(msg)
        if kind == "vehicles":
            for key in ("RecommendedColors", "SupportedDrivers"):
                if key in item and not isinstance(item[key], list):
                    warnings.append("{}: {} must be a list".format(where, key))
            if "NumberOfWheels" in item and not isinstance(item["NumberOfWheels"], int):
                warnings.append("{}: NumberOfWheels must be an integer".format(where))
        if kind == "walkers" and "Speeds" in item:
            if not isinstance(item["Speeds"], list) or any(
                    not isinstance(s, dict) or "Speed" not in s for s in item["Speeds"]):
                warnings.append("{}: Speeds must be a list of {{\"Speed\": <m/s>}}".format(where))
    if kind == "props-package" and "maps" in data:
        if not isinstance(data["maps"], list):
            raise PackError("props catalog: 'maps' must be a list")
        for i, m in enumerate(data["maps"]):
            if not isinstance(m, dict) or "path" not in m or "name" not in m:
                raise PackError("props catalog: maps[{}] needs 'name' and 'path'".format(i))
            check_asset_path(m["path"], pack_name, "maps[{}].path".format(i), warnings)
    return warnings


# --------------------------------------------------------------------------
# Locating things
# --------------------------------------------------------------------------

def project_file(args):
    project = Path(getattr(args, "project", None) or DEFAULT_PROJECT).expanduser()
    if project.is_dir():
        candidates = list(project.glob("*.uproject"))
        if len(candidates) == 1:
            project = candidates[0]
    return project.resolve() if project.exists() else project.absolute()


def packs_root(args):
    root = Path(getattr(args, "root", None) or DEFAULT_PACKS_ROOT).expanduser()
    if not root.is_absolute():
        root = project_file(args).parent / root
    return root


def resolve_pack_dir(spec, args, must_exist=True):
    """<spec> is a pack name under --root, or a path to a pack directory."""
    p = Path(spec).expanduser()
    if (p / MANIFEST_NAME).is_file():
        return p.resolve()
    candidate = packs_root(args) / spec
    if (candidate / MANIFEST_NAME).is_file():
        return candidate.resolve()
    if must_exist:
        raise PackError("pack {!r} not found (looked in {} and {}); run 'carla-pack init' first"
                        .format(spec, p, candidate))
    return candidate


def engine_root(args):
    engine = getattr(args, "engine", None) or os.environ.get(ENGINE_ENV)
    if not engine:
        raise PackError("no Unreal Engine location: set {} or pass --engine".format(ENGINE_ENV))
    engine = Path(engine).expanduser()
    if not engine.is_dir():
        raise PackError("Unreal Engine directory does not exist: {}".format(engine))
    return engine.resolve()


def engine_info(engine):
    """{'version': '5.8.0', 'commit': '<git short hash or changelist or empty>'}"""
    version, commit = "", ""
    build_version = Path(engine) / "Engine" / "Build" / "Build.version"
    if build_version.is_file():
        try:
            bv = load_json(build_version)
            version = "{}.{}.{}".format(bv.get("MajorVersion", 0), bv.get("MinorVersion", 0),
                                        bv.get("PatchVersion", 0))
            if bv.get("Changelist"):
                commit = str(bv["Changelist"])
        except PackError:
            pass
    try:
        out = subprocess.run(["git", "-C", str(engine), "rev-parse", "--short=12", "HEAD"],
                             stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, timeout=10)
        if out.returncode == 0 and out.stdout.strip():
            commit = out.stdout.decode("utf-8", "replace").strip()
    except (OSError, subprocess.SubprocessError):
        pass
    return {"version": version, "commit": commit}


def find_dirs_named(root, name, prune=None):
    """All directories called <name> below root (not descending into matches)."""
    found = []
    root = str(root)
    if not os.path.isdir(root):
        return found
    for dirpath, dirnames, _ in os.walk(root):
        if os.path.basename(dirpath) == name:
            found.append(Path(dirpath))
            dirnames[:] = []
            continue
        if prune:
            dirnames[:] = [d for d in dirnames if d not in prune]
    return found


def find_external_folders(map_path):
    """Find the World Partition __ExternalActors__/__ExternalObjects__ folders of a map.

    A map at <Mount>/A/B/<Map>.umap keeps them at <Mount>/__External*__/A/B/<Map>/;
    CARLA's Content/Carla bundle mounts them at Content/Carla/__External*__/Carla/Maps/<Map>/.
    Walk up from the map, try every ancestor that has an __ExternalActors__ dir and
    look for a <Map> directory inside it.
    """
    map_path = Path(map_path).resolve()
    map_name = map_path.stem
    result = {}
    for kind in ("__ExternalActors__", "__ExternalObjects__"):
        for ancestor in [map_path.parent] + list(map_path.parent.parents):
            ext_root = ancestor / kind
            if not ext_root.is_dir():
                continue
            rel = map_path.parent.relative_to(ancestor)
            # exact: <root>/__External*__/<rel dir of map>/<Map>, then the bundle-name form
            # CARLA uses for Content/Carla (<root>/__External*__/<bundle>/<rel>/<Map>)
            exact = [ext_root / rel / map_name, ext_root / ancestor.name / rel / map_name]
            matches = [d for d in exact if d.is_dir()]
            if not matches:
                matches = [d for d in find_dirs_named(ext_root, map_name) if d.parent.name == "Maps"] \
                    or find_dirs_named(ext_root, map_name)
            if matches:
                result[kind] = matches[0]
                break
    return result


# --------------------------------------------------------------------------
# init
# --------------------------------------------------------------------------

def cmd_init(args):
    name = args.pack
    if not PACK_NAME_RE.match(name):
        raise PackError("invalid pack name {!r}: use letters, digits and '_' and start with a letter"
                        .format(name))
    reserved = {n.lower() for n in RESERVED_PACK_NAMES}
    plugins_dir = project_file(args).parent / "Plugins"
    for up in plugins_dir.rglob("*.uplugin") if plugins_dir.is_dir() else []:
        if "Packs" not in up.relative_to(plugins_dir).parts[:1]:
            reserved.add(up.stem.lower())
    if name.lower() in reserved:
        raise PackError("{!r} is a reserved name (an engine/project mount point or an existing plugin); "
                        "pick another pack name".format(name))
    if not VERSION_RE.match(args.version):
        raise PackError("invalid --version {!r}: expected MAJOR.MINOR.PATCH".format(args.version))
    pack_dir = packs_root(args) / name
    if (pack_dir / MANIFEST_NAME).exists():
        raise PackError("pack already exists: {}".format(pack_dir))
    carla_version = args.carla_version or carla_version_from_cmake() or "0.0.0"
    description = args.description or "CARLA content pack {}".format(name)

    for sub in CONTENT_SUBDIRS:
        (pack_dir / "Content" / sub).mkdir(parents=True, exist_ok=True)
    save_json(pack_dir / (name + ".uplugin"), uplugin_descriptor(name, args.version, description))
    write_plugin_settings(pack_dir)
    save_json(pack_dir / MANIFEST_NAME, new_manifest(name, args.version, carla_version, args.platform))
    info("created {}".format(pack_dir))
    info("author your content in the CARLA editor under {}/Content, then 'carla-pack add {}'"
         .format(pack_dir, name))
    return 0


# --------------------------------------------------------------------------
# add
# --------------------------------------------------------------------------

def add_map(pack_dir, manifest, args):
    src = Path(args.map).expanduser().resolve()
    if not src.is_file() or src.suffix.lower() != ".umap":
        raise PackError("--map must point to an existing .umap file: {}".format(src))
    name = manifest["name"]
    map_name = src.stem
    content = pack_dir / "Content"
    maps_dir = content / "Maps"
    dst = maps_dir / (map_name + ".umap")
    inside = content.resolve() in src.parents
    if not inside:
        # A .umap copied from another content root keeps its internal package name
        # (/Game/...) and its World Partition actors reference that path: it will not
        # load as /<Pack>/Maps/<Map>.  Maps must be saved/duplicated into the pack root
        # from the editor; `add --map` then registers them and attaches the sidecars.
        if not args.allow_cross_root:
            raise PackError("{} is not inside {}: a map must be saved or duplicated into the pack's "
                            "Content/Maps from the CARLA editor (its package name and World Partition "
                            "actor references stay /Game/... when the file is copied). Use "
                            "--allow-cross-root to copy it anyway.".format(src, content))
        warn("copying {} from another content root: its package name and World Partition actor "
             "references stay /Game/... and the map will most likely not load as /{}/Maps/{}; "
             "duplicate it into the pack from the editor instead".format(src, name, map_name))
    if src != dst:
        copy_any(src, dst)
        info("map {} -> {}".format(src, dst))
    else:
        info("map {} is already in the pack; registering it".format(dst))
    built = src.with_name(map_name + "_BuiltData.uasset")
    if built.is_file() and built != maps_dir / built.name:
        copy_any(built, maps_dir / built.name)

    external = find_external_folders(src)
    world_partition = bool(args.world_partition or external)
    if world_partition and not external and src != dst:
        warn("--world-partition given but no __ExternalActors__/{} folder found next to {}"
             .format(map_name, src))
    for kind, folder in external.items():
        target = content / kind / "Maps" / map_name
        if folder.resolve() != target.resolve():
            copy_any(folder, target)
            info("{} -> {}".format(folder, target))

    entry = {
        "name": map_name,
        "package": "/{}/Maps/{}".format(name, map_name),
        "xodr": "",
        "world_partition": world_partition,
    }
    if args.xodr:
        xodr = Path(args.xodr).expanduser()
        if not xodr.is_file():
            raise PackError("--xodr file not found: {}".format(xodr))
        dst_xodr = content / "Maps" / "OpenDrive" / (map_name + ".xodr")
        if xodr.resolve() != dst_xodr.resolve():
            copy_any(xodr, dst_xodr)
        entry["xodr"] = "Maps/OpenDrive/{}.xodr".format(map_name)
    else:
        warn("map {} has no --xodr: the CARLA map API (waypoints, traffic manager) will not work "
             "for it".format(map_name))
    if args.nav:
        nav = Path(args.nav).expanduser()
        if not nav.is_file():
            raise PackError("--nav file not found: {}".format(nav))
        dst_nav = content / "Maps" / "Nav" / (map_name + ".bin")
        if nav.resolve() != dst_nav.resolve():
            copy_any(nav, dst_nav)
        entry["nav"] = "Maps/Nav/{}.bin".format(map_name)
    if args.tm:
        tm = Path(args.tm).expanduser()
        if not tm.is_dir():
            raise PackError("--tm must be a directory: {}".format(tm))
        dst_tm = content / "Maps" / "TM" / map_name
        if tm.resolve() != dst_tm.resolve():
            copy_any(tm, dst_tm)
        entry["tm"] = "Maps/TM/{}".format(map_name)

    manifest["maps"] = [m for m in manifest["maps"] if m.get("name") != map_name] + [entry]
    info("registered map {} as {}{}".format(map_name, entry["package"],
                                          " (World Partition)" if world_partition else ""))


def add_catalog(pack_dir, manifest, kind, src, allow_untagged=False):
    src = Path(src).expanduser()
    data = load_json(src)
    if kind == "props":
        kind = detect_props_kind(data)
    for w in validate_catalog(data, kind, manifest["name"], allow_untagged):
        warn("{}: {}".format(src, w))
    file_name = CATALOG_KINDS[kind][3].format(pack=manifest["name"])
    dst = pack_dir / "Content" / "Config" / file_name
    rel = "Config/" + file_name
    if dst.exists() and src.resolve() != dst.resolve():
        warn("replacing existing {}".format(rel))
    save_json(dst, data)
    if rel not in manifest["catalogs"]:
        manifest["catalogs"].append(rel)
    info("catalog {} -> {}".format(src, rel))


def add_asset(pack_dir, manifest, src, dest):
    src = Path(src).expanduser()
    if not src.exists():
        raise PackError("--asset not found: {}".format(src))
    dest = dest.strip("/").replace("\\", "/")
    if not dest or dest.startswith("..") or "/../" in dest:
        raise PackError("--dest must be a path relative to the pack's Content folder")
    target = pack_dir / "Content" / dest
    if src.is_file() and (target.is_dir() or dest.endswith("/") or not target.suffix):
        target = target / src.name
    copy_any(src, target)
    info("asset {} -> Content/{}".format(src, posix(target.relative_to(pack_dir / "Content"))))


def validate_add_inputs(args):
    """Check every input before anything is copied or registered.

    cmd_add copies files and edits the manifest as it goes and saves once at the
    end, so a bad argument used to leave copied files behind with no manifest
    entry for them."""
    for flag, value in (("--map", args.map), ("--xodr", args.xodr),
                        ("--nav", args.nav)):
        if value and not Path(value).expanduser().is_file():
            raise PackError("{} not found or not a file: {}".format(flag, value))
    if args.tm and not Path(args.tm).expanduser().is_dir():
        raise PackError("--tm must be a directory: {}".format(args.tm))
    for flag, value in (("--props", args.props), ("--vehicles", args.vehicles),
                        ("--walkers", args.walkers), ("--blueprints", args.blueprints)):
        if not value:
            continue
        path = Path(value).expanduser()
        if path.is_dir():
            raise PackError(
                "{0} takes a catalog JSON file, not a directory: {1}\n"
                "       (a catalog lists the assets; to copy files into the pack use "
                "--asset {1} --dest <path under Content>)".format(flag, value))
        if not path.is_file():
            raise PackError("{} not found: {}".format(flag, value))
    for a in (args.asset or []):
        if not Path(a).expanduser().exists():
            raise PackError("--asset not found: {}".format(a))


def cmd_add(args):
    pack_dir = resolve_pack_dir(args.pack, args)
    validate_add_inputs(args)
    manifest = load_manifest(pack_dir)
    did = False
    if args.map:
        add_map(pack_dir, manifest, args)
        did = True
    elif args.xodr or args.nav or args.tm or args.world_partition:
        raise PackError("--xodr/--nav/--tm/--world-partition need --map")
    for kind, src in (("props", args.props), ("vehicles", args.vehicles),
                      ("walkers", args.walkers), ("blueprints", args.blueprints)):
        if src:
            add_catalog(pack_dir, manifest, kind, src, args.allow_untagged)
            did = True
    if args.asset:
        if not args.dest:
            raise PackError("--asset needs --dest <path relative to Content>")
        for a in args.asset:
            add_asset(pack_dir, manifest, a, args.dest)
        did = True
    elif args.dest:
        raise PackError("--dest needs --asset")
    if not did:
        raise PackError("nothing to add: give --map, --props, --vehicles, --walkers, --blueprints "
                        "or --asset")
    save_json(pack_dir / MANIFEST_NAME, manifest)
    return 0


# --------------------------------------------------------------------------
# build
# --------------------------------------------------------------------------

def member_relpath(name):
    """Normalised member path, or None when it escapes the extraction root."""
    raw = name.replace("\\", "/")
    if ".." in raw.split("/"):
        return None                       # any '..' component, even one that would normalise away
    norm = os.path.normpath(raw)
    if norm in (".", ""):
        return ""
    if os.path.isabs(norm) or norm.startswith("/") or norm.startswith("\\"):
        return None
    parts = norm.split("/")
    if any(part == ".." for part in parts) or (":" in parts[0]):
        return None
    return norm


def safe_members(tar):
    for m in tar.getmembers():
        rel = member_relpath(m.name)
        if rel is None:
            raise PackError("refusing to extract unsafe archive member {!r}".format(m.name))
        if not (m.isfile() or m.isdir()):
            raise PackError("refusing to extract non-regular archive member {!r} (link/device/fifo)"
                            .format(m.name))
        if rel == "":
            continue
        yield m


def sanitize_member_mode(member):
    """What tarfile's 'data' filter does to permissions, for interpreters without it:
    directories get the default mode, files lose setuid/setgid/sticky and group/other
    write, keep owner read+write, and are executable for others only when the owner is."""
    if member.isdir():
        member.mode = 0o755
    else:
        mode = (member.mode & 0o777) | 0o600
        mode &= ~0o022
        if not mode & 0o100:
            mode &= ~0o011
        member.mode = mode
    return member


def extract_tar(path, dest):
    """Extract a pack tarball safely.

    Members are vetted first (safe_members: no absolute paths, no '..', no links,
    devices or FIFOs).  The extraction then uses tarfile's 'data' filter when the
    interpreter has it (Python >= 3.12 and the 3.8.17 / 3.9.17 / 3.10.12 / 3.11.4
    backports; it is the default in 3.14 and avoids the 3.12/3.13 DeprecationWarning)
    and otherwise falls back to the vetted member list with the same permission
    sanitising applied by hand (sanitize_member_mode).
    """
    dest = Path(dest)
    dest.mkdir(parents=True, exist_ok=True)
    try:
        with tarfile.open(str(path), "r:*") as tar:
            members = list(safe_members(tar))
            if getattr(tarfile, "data_filter", None) is not None:
                tar.extractall(str(dest), members=members, filter="data")
            else:
                tar.extractall(str(dest), members=[sanitize_member_mode(m) for m in members])
    except (tarfile.TarError, OSError) as e:
        raise PackError("cannot extract {}: {}".format(path, e))


def find_release_dirs(root, platform):
    """Yield (release_name, releases_root) for every <root>/**/<rel>/<platform>/AssetRegistry.bin."""
    root = Path(root)
    hits = []
    for reg in root.rglob(ASSET_REGISTRY):
        if reg.parent.name == platform and reg.parent.parent != root.parent:
            hits.append((reg.parent.parent.name, reg.parent.parent.parent))
    return hits


def resolve_base(base, platform, work):
    """Return (release_name, releases_root) so that
    <releases_root>/<release_name>/<platform>/AssetRegistry.bin exists.

    Accepts the `<release>-release-metadata.tar.gz` the package target writes
    (members `<release>/<Platform>/...` or `Releases/<release>/<Platform>/...`),
    an extracted `Releases/<release>` directory, a `Releases` directory with a
    single release, or a `Releases/<release>/<Platform>` directory.
    """
    base = Path(base).expanduser()
    if not base.exists():
        raise PackError("--base not found: {}".format(base))
    if base.is_file():
        extracted = Path(work) / "Releases"
        if extracted.exists():
            shutil.rmtree(str(extracted))
        extract_tar(base, extracted)
        hits = find_release_dirs(extracted, platform)
        if not hits:
            raise PackError("{} does not contain <release>/{}/{}".format(base, platform, ASSET_REGISTRY))
        release, root = hits[0]
        return release, root.resolve()
    # directory
    if (base / platform / ASSET_REGISTRY).is_file():          # Releases/<rel>
        return base.name, base.parent.resolve()
    if base.name == platform and (base / ASSET_REGISTRY).is_file():  # Releases/<rel>/<Platform>
        return base.parent.name, base.parent.parent.resolve()
    hits = find_release_dirs(base, platform)
    if len(hits) == 1:
        return hits[0][0], hits[0][1].resolve()
    if len(hits) > 1:
        raise PackError("{} holds several releases ({}); point --base at one of them"
                        .format(base, ", ".join(sorted(set(h[0] for h in hits)))))
    raise PackError("{} is not a release-metadata tarball or a Releases/<release> directory "
                    "(no <release>/{}/{} inside)".format(base, platform, ASSET_REGISTRY))


def uat_command(engine, project, uplugin, release, releases_root, platform, config, maps,
                staging_dir, extra):
    """The DLC cook + stage line, exactly as verified by the phase-0 spike (REPORT.md, Task 3).

    - -dlcname takes the absolute .uplugin path so UAT does not fall back to
      Plugins/<Name>/<Name>.uplugin (ProjectParams.cs:764-766);
    - -basedonreleaseversionroot is the Releases directory itself: UAT joins
      <root>/<release>/<Platform> (ProjectParams.cs GetBasedOnReleaseVersionPath);
    - -EnablePlugins=<Pack> is forwarded to the cooker, which only mounts /<Pack>/ for an
      enabled, non-ExplicitlyLoaded plugin;
    - never -iterate (refused together with -basedonreleaseversion);
    - -DLCIncludeEngineContent only when the cook fails with -errorOnEngineContentUse
      (the pack references engine content the base did not cook): --uat-arg=-DLCIncludeEngineContent.
    """
    runuat = Path(engine) / "Engine" / "Build" / "BatchFiles" / (
        "RunUAT.bat" if os.name == "nt" else "RunUAT.sh")
    cmd = [
        str(runuat),
        "BuildCookRun",
        "-project={}".format(project),
        "-nocompileeditor",
        "-nop4",
        "-skipbuild",
        "-cook",
        "-stage",
        "-pak",
        "-iostore",
        "-clientconfig={}".format(config),
        "-TargetPlatform={}".format(platform),
        "-Platform={}".format(platform),
        "-dlcname={}".format(uplugin),
        "-basedonreleaseversion={}".format(release),
        "-basedonreleaseversionroot={}".format(releases_root),
        "-stagingdirectory={}".format(staging_dir),
        "-AdditionalCookerOptions=-EnablePlugins={}".format(Path(uplugin).stem),
    ]
    if maps:
        cmd.append("-MapsToCook={}".format("+".join(maps)))
    cmd.extend(extra or [])
    return cmd


def find_staged_pack(staged_root, name, platform):
    """The staged plugin folder: <staged_root>/**/<name>/Content/Paks/<platform>/."""
    for d in find_dirs_named(staged_root, name):
        if (d / "Content" / "Paks" / platform).is_dir():
            return d
    raise PackError("no staged output for {} under {} (expected .../{}/Content/Paks/{}/)"
                    .format(name, staged_root, name, platform))


def find_asset_registry(staged_pack, pack_dir, name, platform, override=None):
    """Locate the pack's cooked asset registry.

    The DLC cook writes <pack>/Saved/Cooked/<Platform>/CarlaUnreal/Plugins/Packs/<Pack>/AssetRegistry.bin
    (the full registry); the copy inside the pak is stripped and only reachable at an
    engine-root-relative path, so the cooked-dir one is what ships loose in the pack
    (spike REPORT.md, Task 3).  Order: --asset-registry, cooked dir, staged loose copy,
    Metadata/DevelopmentAssetRegistry.bin as a last resort.
    """
    if override:
        p = Path(override).expanduser()
        if not p.is_file():
            raise PackError("--asset-registry not found: {}".format(p))
        return p
    candidates = []
    cooked_root = Path(pack_dir) / "Saved" / "Cooked" / platform
    cooked_dirs = find_dirs_named(cooked_root, name, prune={"Content", "Paks"})
    for d in cooked_dirs:
        candidates.append(d / ASSET_REGISTRY)
    candidates.append(Path(staged_pack) / ASSET_REGISTRY)
    for d in cooked_dirs:
        candidates.append(d / "Metadata" / "DevelopmentAssetRegistry.bin")
    candidates.append(Path(staged_pack) / "Metadata" / "DevelopmentAssetRegistry.bin")
    for c in candidates:
        if c.is_file():
            if c.name != ASSET_REGISTRY:
                warn("using {} as {} (no cooked {} found)".format(c, ASSET_REGISTRY, ASSET_REGISTRY))
            return c
    raise PackError("no {} for {} (looked at {}); pass --asset-registry <file>"
                    .format(ASSET_REGISTRY, name, ", ".join(str(c) for c in candidates)))


def collect_pak_sets(paks_dir):
    """Group <base>.pak/.utoc/.ucas(+ _sN.ucas partitions) by base name."""
    sets = {}
    for f in sorted(Path(paks_dir).iterdir()):
        if not f.is_file():
            continue
        base = f.name
        for ext in (".pak", ".utoc", ".ucas"):
            if base.endswith(ext):
                base = base[: -len(ext)]
                break
        else:
            continue
        base = re.sub(r"_s\d+$", "", base)
        sets.setdefault(base, []).append(f)
    return sets


def assemble_pack(pack_dir, manifest, staged_pack, out_dir, platform, release, eng, registry,
                  rename=False):
    """Normalise the staged DLC output into the distributed pack folder and tar it.

    Pack layout (verified in the spike):
        <Pack>.uplugin (ExplicitlyLoaded:true), carla-pack.json, AssetRegistry.bin,
        Content/Paks/<Platform>/<Pack>CarlaUnreal-<Platform>.{pak,utoc,ucas},
        Content/Config/*.json, Content/Maps/OpenDrive/*.xodr, Content/Maps/Nav/*.bin, Content/Maps/TM/**
    The .upluginmanifest UAT writes is not shipped: the CARLA Packs/ loader does not use it
    and with the engine's own Plugins/ drop-in it would enable the pack at startup.
    """
    name = manifest["name"]
    staged_pack = Path(staged_pack)
    out_pack = Path(out_dir) / name
    if out_pack.exists():
        # only ever delete a previous build output (a built manifest lists files)
        previous = out_pack / MANIFEST_NAME
        try:
            is_output = previous.is_file() and bool(load_json(previous).get("files"))
        except PackError:
            is_output = False
        if not is_output:
            raise PackError("{} exists and is not a previous carla-pack build output; refusing to delete it"
                            .format(out_pack))
        shutil.rmtree(str(out_pack))
    out_paks = out_pack / "Content" / "Paks" / platform
    out_paks.mkdir(parents=True)

    # .uplugin: the authored descriptor, flipped to ExplicitlyLoaded for runtime mounting
    uplugin_src = pack_dir / (name + ".uplugin")
    if not uplugin_src.is_file():
        raise PackError("missing {}".format(uplugin_src))
    save_json(out_pack / uplugin_src.name, runtime_uplugin(uplugin_src))

    # containers: UAT names them <Pack><Project>-<Platform>.*; kept by default.  --rename
    # gives <Pack>-<Platform>.* - the server globs Content/Paks/<Platform>/*.pak and the
    # pak platform file finds the .utoc/.ucas by the .pak's base name, so any name works
    # as long as the triple shares it (verified 2026-08-30 on the packaged server:
    # TestPack-Linux.* mounted, TestMap loaded, pack prop and vehicle spawned).
    staged_paks = staged_pack / "Content" / "Paks" / platform
    sets = collect_pak_sets(staged_paks)
    if not sets:
        raise PackError("no .pak/.utoc/.ucas in {}".format(staged_paks))
    target_base = "{}-{}".format(name, platform)
    if rename and len(sets) == 1:
        base, files = next(iter(sets.items()))
        for f in files:
            copy_any(f, out_paks / (target_base + f.name[len(base):]))
        if base != target_base:
            info("renamed containers {}.* -> {}.*".format(base, target_base))
    else:
        if rename:
            warn("{} container sets staged; --rename ignored".format(len(sets)))
        for files in sets.values():
            for f in files:
                copy_any(f, out_paks / f.name)

    copy_any(registry, out_pack / ASSET_REGISTRY)

    # loose files the stage copied next to the pak (sidecars via PluginSettings.ini, Config/)
    skip_suffixes = (".uplugin", ".upluginmanifest")
    for f in staged_pack.rglob("*"):
        if not f.is_file() or f.suffix in skip_suffixes or staged_paks in f.parents:
            continue
        rel = f.relative_to(staged_pack)
        if rel.name.startswith("Manifest_") or posix(rel) == ASSET_REGISTRY:
            continue  # UAT bookkeeping / the stripped in-pak registry copy
        copy_any(f, out_pack / rel)

    # sidecars the stage missed (older cook without PluginSettings.ini): take the authored ones
    for rel in sidecar_files(pack_dir, manifest):
        if not (out_pack / rel).exists() and (pack_dir / rel).is_file():
            warn("{} was not staged; copying the authored file (check {})".format(rel, PLUGIN_SETTINGS_INI))
            copy_any(pack_dir / rel, out_pack / rel)
    for rel in sidecar_files(pack_dir, manifest):
        if not (out_pack / rel).is_file():
            raise PackError("{} listed in the manifest is missing from the pack".format(rel))

    out_manifest = json.loads(json.dumps(manifest))
    out_manifest["base_release"] = release
    out_manifest["platform"] = platform
    out_manifest["engine"] = eng
    files = {}
    for f in sorted(out_pack.rglob("*")):
        if f.is_file():
            files[posix(f.relative_to(out_pack))] = sha256_file(f)
    out_manifest["files"] = files
    save_json(out_pack / MANIFEST_NAME, out_manifest)

    tar_path = Path(out_dir) / "{}-{}-{}.tar.gz".format(name, manifest["version"], release)
    with tarfile.open(str(tar_path), "w:gz") as tar:
        tar.add(str(out_pack), arcname=name)
    return out_pack, tar_path


def guard_out_dir(out_dir, name, pack_dir, root, work):
    """Refuse an --out that would make assemble_pack delete authored content."""
    out_pack = (Path(out_dir) / name).resolve()
    pack_dir, root, work = Path(pack_dir).resolve(), Path(root).resolve(), Path(work).resolve()
    scratch = pack_dir / "Saved"          # the tool's / UAT's own scratch area, never authored content
    if out_pack == scratch or scratch in out_pack.parents:
        if out_pack == work or out_pack in work.parents:
            raise PackError("--out {} contains the work dir {}; choose another directory".format(out_dir, work))
        return
    for bad, why in ((pack_dir, "the authored pack itself"),
                     (root, "the packs root {}".format(root))):
        if out_pack == bad or bad in out_pack.parents:
            raise PackError("--out {} resolves inside {}; the output <out>/{} would replace authored "
                            "content. Choose a directory outside Plugins/Packs (or under the pack's "
                            "Saved/).".format(out_dir, why, name))
    if out_pack in pack_dir.parents or out_pack == work or out_pack in work.parents:
        raise PackError("--out {} contains the pack sources or the work dir; choose another directory"
                        .format(out_dir))


def sidecar_files(pack_dir, manifest):
    """Content-relative sidecar files the manifest promises (catalogs, xodr, nav)."""
    rels = ["Content/" + c for c in manifest.get("catalogs", [])]
    for m in manifest.get("maps", []):
        for key in ("xodr", "nav"):
            if m.get(key):
                rels.append("Content/" + m[key])
    return rels


def cmd_build(args):
    pack_dir = resolve_pack_dir(args.pack, args)
    manifest = load_manifest(pack_dir)
    name = manifest["name"]
    platform = args.platform
    uplugin = pack_dir / (name + ".uplugin")
    if not uplugin.is_file():
        raise PackError("missing {}".format(uplugin))
    if not isinstance(load_json(uplugin), dict):
        raise PackError("{} is not a plugin descriptor".format(uplugin))
    if not args.dry_run and write_plugin_settings(pack_dir):
        info("wrote {} (stages the sidecar files loose next to the pak)".format(pack_dir / PLUGIN_SETTINGS_INI))
    work = Path(args.work or pack_dir / "Saved" / "CarlaPack").expanduser().resolve()
    work.mkdir(parents=True, exist_ok=True)
    out_dir = Path(args.out or work / "out").expanduser().resolve()
    guard_out_dir(out_dir, name, pack_dir, packs_root(args), work)
    release, releases_root = resolve_base(args.base, platform, work)

    engine = None
    eng = {"version": "", "commit": ""}
    if not args.skip_cook or args.engine or os.environ.get(ENGINE_ENV):
        try:
            engine = engine_root(args)
            eng = engine_info(engine)
        except PackError:
            if not args.skip_cook:
                raise
    project = project_file(args)
    if not project.is_file() and not (args.dry_run or args.skip_cook):
        raise PackError("Unreal project not found: {} (pass --project)".format(project))

    # A DLC cook cooks everything under /<Pack>/ (verified); -MapsToCook is only added on request.
    maps = [m.strip() for m in args.maps.split(",")] if args.maps else []
    known = {m["name"]: m["package"] for m in manifest["maps"]}
    maps = [known.get(m, m) for m in maps if m]
    for m in maps:
        if not m.startswith("/"):
            raise PackError("--maps entries must be map names from the manifest or package paths "
                            "(/{}/Maps/<Map>): {!r}".format(name, m))

    staging_dir = Path(args.staged).expanduser().resolve() if args.staged else work / "Staged"
    cmd = uat_command(engine or "$" + ENGINE_ENV, project, uplugin, release, releases_root,
                      platform, args.config, maps, staging_dir, args.uat_arg)
    if args.dry_run:
        print(" ".join(shell_quote(c) for c in cmd))
        return 0
    if not args.skip_cook:
        info("cooking {} as DLC based on {} ...".format(name, release))
        info(" ".join(shell_quote(c) for c in cmd))
        # the cooker only mounts /<Pack>/ while the descriptor is not ExplicitlyLoaded
        with cook_time_descriptor(uplugin):
            try:
                rc = subprocess.run(cmd, cwd=str(engine)).returncode
            except OSError as e:
                raise PackError("cannot run UAT: {}".format(e))
        if rc != 0:
            raise PackError("UAT BuildCookRun failed with exit code {} (see the log under {}/Engine/"
                            "Programs/AutomationTool/Saved/Logs)".format(rc, engine))

    staged_pack = find_staged_pack(staging_dir, name, platform)
    registry = find_asset_registry(staged_pack, pack_dir, name, platform, args.asset_registry)
    info("asset registry: {}".format(registry))
    out_pack, tar_path = assemble_pack(pack_dir, manifest, staged_pack, out_dir, platform,
                                       release, eng, registry, rename=args.rename)
    size = tar_path.stat().st_size
    info("pack folder: {}".format(out_pack))
    info("wrote {} ({:.1f} MB)".format(tar_path, size / 1e6))
    return 0


# --------------------------------------------------------------------------
# inspect / verify / install
# --------------------------------------------------------------------------

def read_manifest_from_tar(path):
    """Return (manifest, top_dir, sizes) for a pack tarball (members `<Pack>/...` or `./<Pack>/...`);
    sizes maps top-relative member paths to their sizes."""
    manifest, top, sizes = None, None, {}
    try:
        with tarfile.open(str(path), "r:*") as tar:
            for m in tar.getmembers():
                rel = member_relpath(m.name)
                if rel is None or not m.isfile():
                    continue
                parts = rel.split("/")
                if manifest is None and parts[-1] == MANIFEST_NAME and len(parts) <= 2:
                    stream = tar.extractfile(m)
                    if stream is None:          # cannot happen for a regular file; keeps the type sound
                        continue
                    data = stream.read().decode("utf-8")
                    try:
                        manifest = check_manifest(json.loads(data), "{}:{}".format(path, m.name))
                    except ValueError as e:
                        raise PackError("{}:{} is not valid JSON: {}".format(path, m.name, e))
                    top = "/".join(parts[:-1])
                sizes[rel] = m.size
    except (tarfile.TarError, OSError) as e:
        raise PackError("cannot read {}: {}".format(path, e))
    if manifest is None:
        raise PackError("{} is not a content pack archive (no <Pack>/{} inside)".format(path, MANIFEST_NAME))
    prefix = top + "/" if top else ""
    sizes = {k[len(prefix):]: v for k, v in sizes.items() if k.startswith(prefix)}
    return manifest, top, sizes


def open_pack(path):
    """Return (manifest, source_kind, sizes) for a pack tarball or folder."""
    path = Path(path).expanduser()
    if path.is_dir():
        manifest = load_manifest(path)
        sizes = {}
        for rel in manifest["files"]:
            f = path / rel
            if f.is_file():
                sizes[rel] = f.stat().st_size
        return manifest, "dir", sizes
    if path.is_file():
        manifest, _, sizes = read_manifest_from_tar(path)
        return manifest, "tar", sizes
    raise PackError("no such pack: {}".format(path))


def resolve_server(server):
    """Return the packaged project dir for a CARLA package.

    Accepts the package root (<Package>, holding Linux/CarlaUnreal), the platform dir
    (<Package>/Linux) or the project dir itself (<Package>/Linux/CarlaUnreal).
    """
    server = Path(server).expanduser()
    if not server.is_dir():
        raise PackError("--server is not a directory: {}".format(server))
    if (server / "CarlaUnreal").is_dir():
        return (server / "CarlaUnreal").resolve()
    if server.name == "CarlaUnreal":
        return server.resolve()
    nested = [p for p in server.glob("*/CarlaUnreal") if p.is_dir()]
    if len(nested) == 1:
        return nested[0].resolve()
    raise PackError("{} does not look like a CARLA package (no CarlaUnreal/ inside)".format(server))


def server_base_release(project_dir):
    for candidate in (project_dir / BASE_RELEASE_FILE, project_dir.parent / BASE_RELEASE_FILE,
                      project_dir.parent.parent / BASE_RELEASE_FILE):
        if candidate.is_file():
            text = candidate.read_text(encoding="utf-8", errors="replace").strip()
            if text:
                return text
    return None


def server_platform(project_dir):
    """The packaged platform from the layout <Package>/<Platform>/CarlaUnreal (None if unknown)."""
    name = Path(project_dir).parent.name
    return name if name in KNOWN_PLATFORMS else None


def compatibility(manifest, project_dir):
    """Return (ok, base_release_of_server, list of problems)."""
    problems = []
    platform = server_platform(project_dir)
    if platform and manifest.get("platform") != platform:
        problems.append("platform mismatch: pack is for {!r}, server is {!r}"
                        .format(manifest.get("platform"), platform))
    server_release = server_base_release(project_dir)
    if server_release is None:
        problems.append("server has no {} stamp (built without content-pack support?)"
                        .format(BASE_RELEASE_FILE))
    elif server_release != manifest["base_release"]:
        problems.append("base release mismatch: pack built against {!r}, server is {!r}; "
                        "re-run 'carla-pack build' with this server's release metadata"
                        .format(manifest["base_release"], server_release))
    if not manifest["base_release"]:
        problems.append("pack was never built (empty base_release)")
    return not problems, server_release, problems


def verify_dir(pack_dir):
    """Return (missing, mismatched, extra) relative paths for a pack folder."""
    manifest = load_manifest(pack_dir)
    missing, mismatched, extra = [], [], []
    for rel, digest in sorted(manifest["files"].items()):
        f = Path(pack_dir) / rel
        if not f.is_file():
            missing.append(rel)
        elif sha256_file(f) != digest:
            mismatched.append(rel)
    listed = set(manifest["files"]) | {MANIFEST_NAME}
    for f in Path(pack_dir).rglob("*"):
        if f.is_file():
            rel = posix(f.relative_to(pack_dir))
            if rel not in listed:
                extra.append(rel)
    if not manifest["files"]:
        missing.append("<manifest lists no files: not a built pack>")
    return missing, mismatched, sorted(extra)


def cmd_inspect(args):
    manifest, kind, sizes = open_pack(args.pack)
    report = {"source": str(args.pack), "kind": kind, "manifest": manifest, "sizes": sizes}
    if args.server:
        project_dir = resolve_server(args.server)
        ok, server_release, problems = compatibility(manifest, project_dir)
        report["server"] = {"path": str(project_dir), "base_release": server_release,
                            "compatible": ok, "problems": problems}
    if args.json:
        print(json.dumps(report, indent=2))
    else:
        m = manifest
        print("Pack:          {} {}".format(m["name"], m["version"]))
        print("CARLA:         {}".format(m["carla_version"]))
        print("Base release:  {}".format(m["base_release"] or "(not built)"))
        print("Engine:        {} {}".format(m["engine"].get("version", ""),
                                            m["engine"].get("commit", "")).rstrip())
        print("Platform:      {}".format(m["platform"]))
        print("Maps ({}):".format(len(m["maps"])))
        for mp in m["maps"]:
            flags = []
            if mp.get("world_partition"):
                flags.append("world-partition")
            if mp.get("xodr"):
                flags.append("xodr")
            if mp.get("nav"):
                flags.append("nav")
            print("  {:<24} {}  [{}]".format(mp.get("name", "?"), mp.get("package", "?"),
                                             ", ".join(flags)))
        print("Catalogs ({}):".format(len(m["catalogs"])))
        for c in m["catalogs"]:
            print("  " + c)
        total = sum(sizes.get(rel, 0) for rel in m["files"])
        print("Files ({}, {:.1f} MB):".format(len(m["files"]), total / 1e6))
        for rel in sorted(m["files"]):
            print("  {}  {:>10}  {}".format(m["files"][rel][:19] + "...",
                                          "{:.1f} MB".format(sizes[rel] / 1e6) if rel in sizes else "?", rel))
        if "server" in report:
            s = report["server"]
            print("Server:        {} (base release {})".format(s["path"], s["base_release"] or "?"))
            print("Compatible:    {}".format("yes" if s["compatible"] else "NO"))
            for p in s["problems"]:
                print("  - " + p)
    if args.server and not report["server"]["compatible"]:
        return 3
    return 0


def cmd_verify(args):
    path = Path(args.pack).expanduser()
    tmp = None
    try:
        if path.is_file():
            tmp = tempfile.mkdtemp(prefix="carla-pack-verify-")
            # a hostile tarball must not leave anything behind
            manifest, top, _ = read_manifest_from_tar(path)
            extract_tar(path, tmp)
            pack_dir = Path(tmp) / top if top else Path(tmp)
        elif path.is_dir():
            pack_dir = path
        else:
            raise PackError("no such pack: {}".format(path))
        missing, mismatched, extra = verify_dir(pack_dir)
        for rel in missing:
            print("MISSING    " + rel)
        for rel in mismatched:
            print("MISMATCH   " + rel)
        for rel in extra:
            print("UNLISTED   " + rel)
        n = len(load_manifest(pack_dir)["files"])
        if missing or mismatched or extra:
            print("FAILED: {} missing, {} mismatched, {} unlisted of {} files"
                  .format(len(missing), len(mismatched), len(extra), n))
            return 1
        print("OK: {} files verified".format(n))
        return 0
    finally:
        if tmp:
            rmtree_force(tmp)


def cmd_install(args):
    src = Path(args.pack).expanduser()
    if not src.is_file():
        raise PackError("install needs a pack tarball: {}".format(src))
    manifest, top, _ = read_manifest_from_tar(src)
    name = manifest["name"]
    project_dir = resolve_server(args.server)
    ok, server_release, problems = compatibility(manifest, project_dir)
    if not ok:
        if args.force:
            for p in problems:
                warn(p + " (ignored: --force)")
        else:
            raise PackError("{} is not compatible with {}:\n  - {}"
                            .format(src.name, project_dir, "\n  - ".join(problems)))
    packs_dir = project_dir / PACKS_INSTALL_DIR
    target = packs_dir / name
    if target.exists() and not args.force:
        raise PackError("{} is already installed at {} (use --force to replace it; restart the "
                        "server or unmount it first if it is loaded)".format(name, target))

    packs_dir.mkdir(parents=True, exist_ok=True)
    tmp = Path(tempfile.mkdtemp(prefix=".{}.".format(name), dir=str(packs_dir)))
    try:
        extract_tar(src, tmp)
        extracted = tmp / top if top else tmp
        if not (extracted / MANIFEST_NAME).is_file():
            raise PackError("archive layout unexpected: no {}/{}".format(top, MANIFEST_NAME))
        missing, mismatched, extra = verify_dir(extracted)
        if missing or mismatched:
            raise PackError("checksum verification failed for {}: missing {}, mismatched {}"
                            .format(src.name, missing or "-", mismatched or "-"))
        if extra:
            raise PackError("{} contains files not listed in its manifest: {}"
                            .format(src.name, ", ".join(extra)))
        if target.exists():
            rmtree_force(target)
        shutil.move(str(extracted), str(target))
    finally:
        rmtree_force(tmp)
    info("installed {} {} -> {}".format(name, manifest["version"], target))
    info("it is picked up at the next server start, or now with "
         "client.mount_content_pack('{}')".format(target))
    return 0


# --------------------------------------------------------------------------
# check-base: did a pack leak into a base release?
# --------------------------------------------------------------------------

# FNameHash::AlgorithmId, the hash version SaveNameBatch writes (UnrealNames.cpp; unchanged
# since UE 4.26).  Used to locate the name batch at the top of AssetRegistry.bin.
NAME_BATCH_HASH_VERSION = 0xC1640000


def registry_names(path):
    """The FName table of a cooked AssetRegistry.bin / DevelopmentAssetRegistry.bin.

    FAssetRegistryWriter (AssetRegistryArchive.cpp) starts the file with the registry
    version header followed by a SaveNameBatch block: [Num u32][NumStringBytes u32]
    [HashVersion u64][Num x u64 hashes][Num x 2-byte headers][the strings, back to back].
    A header is (bIsWide << 7 | Len >> 8, Len & 0xFF).  Package names (/Game/Carla/...,
    /<Pack>/Maps/<Map>) are single names in that table, which is why a mount-root
    check needs the table and not a byte search: the strings are not separated.
    """
    data = Path(path).read_bytes()
    start, num, string_bytes = -1, 0, 0
    for off in range(0, min(len(data) - 16, 128), 4):
        num, string_bytes, hash_version = struct.unpack_from("<IIQ", data, off)
        if (hash_version == NAME_BATCH_HASH_VERSION and 0 < num < (1 << 24)
                and off + 16 + 10 * num + string_bytes <= len(data)):
            start = off
            break
    if start < 0:
        raise PackError("{} is not a cooked asset registry (no FName batch found)".format(path))
    headers = start + 16 + 8 * num
    pos = headers + 2 * num
    names = []
    for i in range(num):
        b0, b1 = data[headers + 2 * i], data[headers + 2 * i + 1]
        wide = b0 & 0x80
        length = ((b0 & 0x7F) << 8) | b1
        nbytes = length * 2 if wide else length
        chunk = data[pos:pos + nbytes]
        pos += nbytes
        names.append(chunk.decode("utf-16-le", "replace") if wide else chunk.decode("latin-1"))
    if pos - (headers + 2 * num) != string_bytes:
        raise PackError("{}: FName table is inconsistent ({} string bytes announced, {} read)"
                        .format(path, string_bytes, pos - (headers + 2 * num)))
    return names


def registry_files_of(base, platform):
    """AssetRegistry.bin files behind --base: a registry file, a Releases/<rel>[/<Platform>] dir
    or a Releases dir (every <rel>/<Platform>/AssetRegistry.bin and Metadata/Development... below)."""
    base = Path(base).expanduser()
    if base.is_file():
        if base.suffix.lower() != ".bin":
            raise PackError("{}: expected an AssetRegistry.bin or a Releases/<release> directory "
                            "(a metadata tarball must be extracted first)".format(base))
        return [base]
    if not base.is_dir():
        raise PackError("--base not found: {}".format(base))
    found = sorted(set(list(base.rglob(ASSET_REGISTRY)) + list(base.rglob("DevelopmentAssetRegistry.bin"))))
    found = [f for f in found if platform in f.parts or f.parent == base]
    if not found:
        raise PackError("no {} under {} for platform {}".format(ASSET_REGISTRY, base, platform))
    return found


def packs_under(root):
    """Pack names under a Plugins/Packs directory (a <Pack>.uplugin or carla-pack.json inside)."""
    root = Path(root)
    names = []
    if root.is_dir():
        for d in sorted(root.iterdir()):
            if d.is_dir() and ((d / (d.name + ".uplugin")).is_file() or (d / MANIFEST_NAME).is_file()):
                if PACK_NAME_RE.match(d.name):
                    names.append(d.name)
    return names


def cmd_check_base(args):
    """Fail (rc 1) when any pack's mount root /<Pack>/ has package names in a base registry."""
    names = list(args.pack or [])
    if not names:
        names = packs_under(packs_root(args))
    for n in names:
        if not PACK_NAME_RE.match(n):
            raise PackError("invalid pack name {!r}".format(n))
    if not names:
        info("no packs under {}: nothing to check".format(packs_root(args)))
        return 0
    registries = registry_files_of(args.base, args.platform)
    leaks = 0
    for reg in registries:
        table = registry_names(reg)
        for pack in names:
            prefix = "/{}/".format(pack)
            hits = [s for s in table if s.startswith(prefix)]
            if hits:
                leaks += 1
                print("LEAK  {}: {} package name(s) under {} (e.g. {})".format(
                    reg, len(hits), prefix, ", ".join(hits[:3])))
            else:
                print("ok    {}: no {} names ({} names checked)".format(reg, prefix, len(table)))
    if leaks:
        print("FAILED: {} pack/registry combination(s) leaked; the pack(s) were cooked into the base "
              "(enabled in the .uproject, ExplicitlyLoaded:false with EnabledByDefault:true, or "
              "-EnablePlugins on the base cook)".format(leaks))
        return 1
    print("OK: no /<Pack>/ names for {} in {} registry file(s)".format(", ".join(names), len(registries)))
    return 0


# --------------------------------------------------------------------------
# CLI
# --------------------------------------------------------------------------

def build_parser():
    p = argparse.ArgumentParser(
        prog="carla-pack",
        description="Author, build, inspect and install CARLA content packs "
                    "(content-only plugins cooked as DLC against a CARLA base release).",
        epilog="exit codes: 0 ok, 1 error, 2 usage, 3 pack not compatible with --server (inspect)")
    p.add_argument("--version", action="version", version="carla-pack " + __version__)
    sub = p.add_subparsers(dest="cmd", metavar="<command>")
    sub.required = True

    def project_opts(sp):
        sp.add_argument("--root", default=None,
                        help="packs folder, absolute or relative to the Unreal project "
                             "(default: {})".format(DEFAULT_PACKS_ROOT))
        sp.add_argument("--project", default=None,
                        help="CarlaUnreal.uproject (default: this checkout's)")

    sp = sub.add_parser("init", help="create a pack skeleton under Plugins/Packs/<Pack>")
    sp.add_argument("pack", metavar="<Pack>")
    project_opts(sp)
    sp.add_argument("--carla-version", default=None, help="CARLA version the pack targets "
                    "(default: from CMakeLists.txt)")
    sp.add_argument("--pack-version", dest="version", default="1.0.0", help="pack version (default 1.0.0)")
    sp.add_argument("--description", default=None)
    sp.add_argument("--platform", default=DEFAULT_PLATFORM)
    sp.set_defaults(func=cmd_init)

    sp = sub.add_parser("add", help="copy maps, sidecar files, catalogs and assets into a pack",
                        formatter_class=argparse.RawDescriptionHelpFormatter,
                        epilog="semantic segmentation: a pack mesh is labelled by the folder after 'Static' in "
                               "its path,\n  /<Pack>/Static/<Label>/..., with <Label> one of:\n  "
                               + ", ".join(SEMANTIC_LABELS) + "\n"
                               "--props refuses a mesh under any other folder unless --allow-untagged is given.")
    sp.add_argument("pack", metavar="<Pack>")
    project_opts(sp)
    sp.add_argument("--map", help=".umap to add (World Partition external folders are copied too)")
    sp.add_argument("--xodr", help="OpenDRIVE file for --map")
    sp.add_argument("--nav", help="pedestrian navigation .bin for --map")
    sp.add_argument("--tm", help="Traffic Manager data directory for --map")
    sp.add_argument("--world-partition", action="store_true",
                    help="mark --map as a World Partition map (auto-detected when the "
                         "__ExternalActors__ folder is found)")
    sp.add_argument("--allow-cross-root", action="store_true",
                    help="copy a .umap that lives outside the pack's Content folder anyway "
                         "(it keeps its /Game/... package name and will most likely not load)")
    sp.add_argument("--props", help="props catalog (<Pack>.Package.json or PropParameters.json shape)")
    sp.add_argument("--allow-untagged", action="store_true",
                    help="accept prop meshes outside /<Pack>/Static/<Label>/ (they get no semantic label)")
    sp.add_argument("--vehicles", help="vehicles catalog (Vehicles.json shape)")
    sp.add_argument("--walkers", help="walkers catalog (WalkerParameters.json shape)")
    sp.add_argument("--blueprints", help="blueprints catalog (BlueprintParameters.json shape)")
    sp.add_argument("--asset", action="append", help="file or directory to copy (repeatable)")
    sp.add_argument("--dest", help="destination of --asset, relative to the pack's Content folder")
    sp.set_defaults(func=cmd_add)

    sp = sub.add_parser("build", help="cook the pack as DLC against a base release and tar it")
    sp.add_argument("pack", metavar="<Pack>")
    project_opts(sp)
    sp.add_argument("--base", required=True,
                    help="<release>-release-metadata.tar.gz or an extracted Releases/<release> dir")
    sp.add_argument("--maps", default=None, help="comma-separated map names/packages to pass as "
                    "-MapsToCook (default: none, the DLC cook takes everything under /<Pack>/)")
    sp.add_argument("--config", default=DEFAULT_CONFIG, choices=["Development", "Shipping", "Test",
                                                                  "DebugGame", "Debug"])
    sp.add_argument("--platform", default=DEFAULT_PLATFORM)
    sp.add_argument("--engine", default=None, help="Unreal Engine root (default: ${})".format(ENGINE_ENV))
    sp.add_argument("--out", default=None, help="output dir (default: <pack>/Saved/CarlaPack/out)")
    sp.add_argument("--work", default=None, help="scratch dir (default: <pack>/Saved/CarlaPack)")
    sp.add_argument("--staged", default=None, help="UAT staging dir (default: <work>/Staged)")
    sp.add_argument("--asset-registry", default=None, help="use this AssetRegistry.bin instead "
                    "of the one found in the cook output")
    sp.add_argument("--uat-arg", action="append", default=[], metavar="FLAG",
                    help="extra BuildCookRun flag, written as --uat-arg=-Flag (repeatable)")
    sp.add_argument("--rename", action="store_true",
                    help="name the containers <Pack>-<Platform>.pak/.utoc/.ucas instead of keeping "
                         "UAT's <Pack>CarlaUnreal-<Platform>.* (the server mounts either; off by default)")
    sp.add_argument("--skip-cook", action="store_true", help="do not run UAT; package what is "
                    "already under --staged")
    sp.add_argument("--dry-run", action="store_true",
                    help="print the UAT command line and exit (a tarball --base is still extracted "
                         "into the work dir; the pack sources are not touched)")
    sp.set_defaults(func=cmd_build)

    sp = sub.add_parser("inspect", help="print a pack's manifest and check it against a server")
    sp.add_argument("pack", metavar="<tar.gz|dir>")
    sp.add_argument("--server", default=None,
                    help="CARLA package to check against: <Package>, <Package>/Linux or <Package>/Linux/CarlaUnreal")
    sp.add_argument("--json", action="store_true")
    sp.set_defaults(func=cmd_inspect)

    sp = sub.add_parser("verify", help="check a pack's files against the manifest checksums")
    sp.add_argument("pack", metavar="<tar.gz|dir>")
    sp.set_defaults(func=cmd_verify)

    sp = sub.add_parser("install", help="verify and extract a pack into <server>/CarlaUnreal/Packs/")
    sp.add_argument("pack", metavar="<tar.gz>")
    sp.add_argument("--server", required=True,
                    help="CARLA package: <Package> (holds Linux/), <Package>/Linux (holds CarlaUnreal.sh) "
                         "or <Package>/Linux/CarlaUnreal")
    sp.add_argument("--force", action="store_true", help="replace an installed pack / ignore a "
                    "base release mismatch")
    sp.set_defaults(func=cmd_install)

    sp = sub.add_parser("check-base", help="fail if a pack's content (/<Pack>/...) was cooked into a "
                        "base release's AssetRegistry.bin (the package target runs this after its cook)")
    sp.add_argument("base", metavar="<AssetRegistry.bin|Releases/<release>>",
                    help="a cooked registry file, a Releases/<release> directory or the Releases directory")
    project_opts(sp)
    sp.add_argument("--pack", action="append", metavar="<Pack>",
                    help="pack name to look for (repeatable; default: every pack under --root)")
    sp.add_argument("--platform", default=DEFAULT_PLATFORM)
    sp.set_defaults(func=cmd_check_base)
    return p


def main(argv=None):
    parser = build_parser()
    args = parser.parse_args(argv)
    try:
        return int(args.func(args) or 0)
    except PackError as e:
        print("carla-pack: error: {}".format(e), file=sys.stderr)
        return 1
    except OSError as e:
        print("carla-pack: error: {}".format(e), file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        return 130


if __name__ == "__main__":
    sys.exit(main())
