"""Persisted paths for the skill library.

A newcomer cannot fill in an `mcp add -e` block: the paths do not exist yet, and
the skill that creates them runs later. So paths are recorded at runtime instead
of at registration, in a file both this server and the skills' `env.sh` read.

Resolution order for every key, highest first:

    1. an explicit environment variable   one-off override, CI, back-compat
    2. ./.carla-tools.env                 a repo carrying its own CARLA
    3. the user config                    the normal case
    4. each env.sh's own search list      last resort

The config outranks the search lists on purpose. Once a user has confirmed which
of several checkouts to use, detection must not silently pick the other one.

Format is `KEY=value` lines, not JSON, so `env.sh` can source the file with no
dependency: parsing JSON in shell needs `jq`, which is not guaranteed, and
routing it through Python is circular when `PYTHON` is itself a key.
"""
from __future__ import annotations

import os
import re
import shutil
import subprocess
from pathlib import Path

#: Keys a user is ever asked about, and what each one means. `CARLA_ROOT` is the
#: only CARLA path anyone is prompted for; the engine-specific keys below it are
#: derived by `detect_carla` and kept because the skills' scripts read them.
CONFIG_KEYS: dict[str, str] = {
    "CARLA_ROOT": "a CARLA release or source checkout (holds PythonAPI/carla)",
    "PYTHON": "the interpreter that imports the carla wheel",
    "SCENARIO_RUNNER_ROOT": "a scenario_runner checkout",
    "LEADERBOARD_ROOT": "a leaderboard checkout",
    "SCENIC_ROOT": "a Scenic checkout or installed package",
    "CARLA_UNREAL_ENGINE_PATH": "the Unreal Engine fork CARLA builds against",
    "UE4_ROOT": "the built CarlaUnreal UE 4.26 fork, for the ue4 editor",
}

#: Written by `detect_carla`, never prompted for. Each names the same tree as
#: CARLA_ROOT; which one gets written is what gates the ue4/ue5/ue58 groups.
DERIVED_KEYS = (
    "CARLA_UE4_ROOT", "CARLA_UE5_ROOT", "CARLA_UE58_ROOT",
    "CARLA_PACKAGE_ROOT", "CARLA_TARGET",
)

PROJECT_CONFIG = ".carla-tools.env"
_LINE = re.compile(r"^\s*(?:export\s+)?([A-Z][A-Z0-9_]*)\s*=\s*(.*?)\s*$")


def user_config_path() -> Path:
    """The per-user config file, overridable for tests and odd setups."""
    override = os.environ.get("CARLA_TOOLS_CONFIG")
    if override:
        return Path(override).expanduser()
    base = os.environ.get("XDG_CONFIG_HOME") or "~/.config"
    return Path(base).expanduser() / "carla-agentic-tools" / "config.env"


def _parse(path: Path) -> dict[str, str]:
    out: dict[str, str] = {}
    if not path.is_file():
        return out
    for line in path.read_text().splitlines():
        if not line.strip() or line.lstrip().startswith("#"):
            continue
        m = _LINE.match(line)
        if m:
            out[m.group(1)] = m.group(2).strip("'\"")
    return out


def read_config(cwd: Path | None = None) -> dict[str, str]:
    """User config overlaid by a project config, if the directory has one."""
    merged = _parse(user_config_path())
    merged.update(_parse((cwd or Path.cwd()) / PROJECT_CONFIG))
    return merged


def resolve(key: str, cwd: Path | None = None) -> tuple[str, str]:
    """The effective value of `key` and where it came from."""
    if os.environ.get(key):
        return os.environ[key], "environment"
    cfg = read_config(cwd)
    if cfg.get(key):
        project = _parse((cwd or Path.cwd()) / PROJECT_CONFIG)
        return cfg[key], "project config" if key in project else "user config"
    return "", "unset"


def write_config(updates: dict[str, str], path: Path | None = None) -> Path:
    """Merge `updates` into the config, preserving any keys already there."""
    target = path or user_config_path()
    target.parent.mkdir(parents=True, exist_ok=True)
    merged = _parse(target)
    for k, v in updates.items():
        if v:
            merged[k] = str(v)
        else:
            merged.pop(k, None)  # empty value clears a key
    body = "\n".join(f"{k}={merged[k]}" for k in sorted(merged))
    target.write_text(
        "# Written by carla-agentic-tools. An exported environment variable of\n"
        "# the same name still wins over anything here.\n"
        f"{body}\n"
    )
    return target


# --- validation -------------------------------------------------------------
# A key that merely *has* a value is not configured: a path can be deleted,
# renamed, or point at the wrong kind of tree. Every marker below is the one the
# corresponding `carla_*_is_root` helper in the skills' env.sh tests for, so the
# gate and the script that runs afterwards agree on what counts.

def _has(root: Path, *rel: str) -> bool:
    return any((root / r).exists() for r in rel)


#: key -> (predicate, what a valid value looks like)
VALIDATORS: dict[str, tuple] = {
    "CARLA_ROOT": (lambda p: (p / "PythonAPI/carla").is_dir(),
                   "a CARLA root holding PythonAPI/carla"),
    "SCENARIO_RUNNER_ROOT": (lambda p: (p / "scenario_runner.py").is_file()
                             and (p / "srunner").is_dir(),
                             "a scenario_runner checkout (scenario_runner.py + srunner/)"),
    "LEADERBOARD_ROOT": (lambda p: (p / "leaderboard/leaderboard_evaluator.py").is_file(),
                         "a leaderboard checkout (leaderboard/leaderboard_evaluator.py)"),
    "SCENIC_ROOT": (lambda p: _has(p, "src/scenic", "examples/carla", "simulators/carla"),
                    "a Scenic checkout or the installed scenic package"),
    "CARLA_UE4_ROOT": (lambda p: (p / "Unreal/CarlaUE4/CarlaUE4.uproject").is_file(),
                       "a ue4-dev checkout"),
    "CARLA_UE5_ROOT": (lambda p: (p / "CMakePresets.json").is_file(),
                       "a ue5-dev checkout"),
    "CARLA_UE58_ROOT": (lambda p: (p / "CMakePresets.json").is_file(),
                        "a ue58-dev checkout"),
    "CARLA_UNREAL_ENGINE_PATH": (lambda p: _has(p, "Engine/Build/BatchFiles", "Setup.sh"),
                                 "an Unreal Engine source tree"),
    "UE4_ROOT": (lambda p: _has(p, "Engine/Build/BatchFiles", "Setup.sh"),
                 "an Unreal Engine 4.26 source tree"),
}


def validate(key: str, value: str) -> tuple[bool, str]:
    """Whether `value` is usable for `key`, and why not when it is not.

    PYTHON is a command rather than a tree, so it is checked for being runnable;
    whether it imports `carla` is left to the skill's check_env.sh, which can say
    so precisely instead of guessing from a path.
    """
    if not value:
        return False, "unset"
    if key == "PYTHON":
        if Path(value).is_file() and os.access(value, os.X_OK):
            return True, ""
        if shutil.which(value):
            return True, ""
        return False, f"{value} is not an executable on PATH"
    p = Path(value).expanduser()
    if not p.is_dir():
        return False, f"{p} is not a directory"
    check = VALIDATORS.get(key)
    if check and not check[0](p):
        return False, f"{p} is not {check[1]}"
    return True, ""


def resolve_valid(key: str, cwd: Path | None = None) -> tuple[str, str, str]:
    """`resolve`, plus why the value cannot be used. Empty reason means usable."""
    value, source = resolve(key, cwd)
    ok, why = validate(key, value)
    return (value, source, "" if ok else why)


# --- CARLA detection --------------------------------------------------------
# Structural markers only, so a tarball, a detached HEAD or a renamed directory
# still resolves. These mirror the `carla_*_is_root` helpers in the skills'
# env.sh files; tests/test_config.py asserts the two stay in agreement.

def _branch(path: Path) -> str:
    try:
        out = subprocess.run(["git", "-C", str(path), "rev-parse", "--abbrev-ref", "HEAD"],
                             capture_output=True, text=True, timeout=10)
        return out.stdout.strip() if out.returncode == 0 else ""
    except Exception:
        return ""


def detect_carla(path: str | Path) -> dict:
    """Classify a directory as a CARLA release or checkout, and say which vars it sets.

    Returns `kind` ("release" / "source" / "none"), `flavor` ("ue4" / "ue5" /
    "ue58" / ""), a human `detail` for a chooser, and the `vars` to persist.
    A `flavor` is what gates the matching skill group, so it is derived here
    rather than inferred from which variable name the user happened to set.
    """
    p = Path(path).expanduser()
    info: dict = {"path": str(p), "kind": "none", "flavor": "", "detail": "", "vars": {}}
    if not p.is_dir():
        info["detail"] = "not a directory"
        return info

    has_api = (p / "PythonAPI" / "carla").is_dir()

    if (p / "CMakePresets.json").is_file() and (p / "Unreal/CarlaUnreal").is_dir():
        # ue5-dev and ue58-dev share this shape; the Autoware plugin and the DLSS
        # CMake module arrived with 5.8 and are what tell them apart.
        is_58 = ((p / "Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Autoware").is_dir()
                 and (p / "CMake/DLSS.cmake").is_file())
        info["kind"] = "source"
        info["flavor"] = "ue58" if is_58 else "ue5"
        info["vars"]["CARLA_UE58_ROOT" if is_58 else "CARLA_UE5_ROOT"] = str(p)
    elif (p / "Unreal/CarlaUE4/CarlaUE4.uproject").is_file():
        info["kind"] = "source"
        info["flavor"] = "ue4"
        info["vars"]["CARLA_UE4_ROOT"] = str(p)
    else:
        launcher = next((c for c in ("CarlaUE4.sh", "CarlaUnreal.sh",
                                     "LinuxNoEditor/CarlaUE4.sh")
                         if (p / c).is_file()), "")
        if launcher:
            info["kind"] = "release"
            info["vars"]["CARLA_PACKAGE_ROOT"] = str(p)
            info["vars"]["CARLA_TARGET"] = str(p)
        elif not has_api:
            info["detail"] = "no CARLA here (no launcher, no Unreal project, no PythonAPI)"
            return info

    if has_api:
        info["vars"]["CARLA_ROOT"] = str(p)

    bits = [info["kind"]]
    if info["flavor"]:
        bits.append(info["flavor"])
    br = _branch(p)
    if br and br != "HEAD":
        bits.append(f"branch {br}")
    if not has_api:
        bits.append("no PythonAPI/carla — build or install it before the API skills work")
    info["detail"] = ", ".join(bits)
    return info


#: Where a CARLA is plausibly found when the user has not said. Ordered: an
#: explicit cwd beats a guess, and a guess is only ever offered as a candidate
#: to confirm, never selected silently.
_SEARCH = (
    "{cwd}", "{cwd}/carla", "{home}/carla", "{home}/CARLA",
    "{home}/UE58/carla", "{home}/carla-ue58", "{home}/carla-downloads",
    "/opt/carla", "/workspace/carla",
)


def carla_candidates(cwd: Path | None = None) -> list[dict]:
    """Every plausible CARLA on this machine, classified, newest-looking first.

    Offered to the user to choose from. Several checkouts of different branches
    on one machine is normal, and picking the wrong one silently is the failure
    this exists to prevent — so each candidate carries its flavor and branch.
    """
    home = Path.home()
    here = cwd or Path.cwd()
    seen: dict[str, dict] = {}
    for tmpl in _SEARCH:
        base = Path(tmpl.format(cwd=here, home=home))
        for cand in (base, *sorted(base.glob("CARLA_*"))) if base.is_dir() else ():
            key = str(cand.resolve())
            if key in seen:
                continue
            info = detect_carla(cand)
            if info["kind"] != "none":
                seen[key] = info
    return list(seen.values())
