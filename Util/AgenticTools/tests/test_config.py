"""The persisted-path layer.

These guard the properties a new user depends on: a path recorded once survives
the session, an exported variable still wins, and the config beats a search list
so that a machine with several CARLA checkouts cannot silently get the wrong one.
"""
from __future__ import annotations

import re
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "src"))

from carla_agentic_tools import config as cfg  # noqa: E402

COMMON_SH = REPO / "skills" / "_common" / "env_common.sh"


@pytest.fixture()
def clean(monkeypatch, tmp_path):
    """No ambient CARLA variables and a config file of our own."""
    monkeypatch.setenv("CARLA_TOOLS_CONFIG", str(tmp_path / "config.env"))
    monkeypatch.chdir(tmp_path)
    for key in (*cfg.CONFIG_KEYS, *cfg.DERIVED_KEYS):
        monkeypatch.delenv(key, raising=False)
    return tmp_path


def _fake_ue58(root: Path) -> Path:
    """The structural markers detect_carla keys on for a 5.8 checkout."""
    (root / "Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Autoware").mkdir(parents=True)
    (root / "CMake").mkdir()
    (root / "CMake/DLSS.cmake").write_text("")
    (root / "CMakePresets.json").write_text("{}")
    (root / "PythonAPI/carla").mkdir(parents=True)
    return root


# --- resolution order -------------------------------------------------------

def test_config_value_survives(clean):
    cfg.write_config({"SCENIC_ROOT": str(clean)})
    value, source = cfg.resolve("SCENIC_ROOT")
    assert value == str(clean)
    assert source == "user config"


def test_environment_beats_config(clean, monkeypatch):
    cfg.write_config({"SCENIC_ROOT": "/from/config"})
    monkeypatch.setenv("SCENIC_ROOT", "/from/env")
    assert cfg.resolve("SCENIC_ROOT") == ("/from/env", "environment")


def test_project_config_beats_user_config(clean):
    cfg.write_config({"SCENIC_ROOT": "/from/user"})
    (clean / cfg.PROJECT_CONFIG).write_text("SCENIC_ROOT=/from/project\n")
    assert cfg.resolve("SCENIC_ROOT") == ("/from/project", "project config")


def test_write_config_merges_and_clears(clean):
    cfg.write_config({"SCENIC_ROOT": "/a", "LEADERBOARD_ROOT": "/b"})
    cfg.write_config({"SCENIC_ROOT": ""})
    kept = cfg.read_config()
    assert "SCENIC_ROOT" not in kept
    assert kept["LEADERBOARD_ROOT"] == "/b"


def test_config_file_cannot_execute_commands(clean):
    """The loader parses KEY=value; it must never run what the file contains."""
    path = Path(cfg.user_config_path())
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text("SCENIC_ROOT=$(touch pwned)\n")
    assert cfg.resolve("SCENIC_ROOT")[0] == "$(touch pwned)"
    assert not (clean / "pwned").exists()


# --- detection --------------------------------------------------------------

def test_detects_ue58_and_derives_vars(clean):
    info = cfg.detect_carla(_fake_ue58(clean / "carla"))
    assert (info["kind"], info["flavor"]) == ("source", "ue58")
    # A user is asked for CARLA_ROOT only; the engine var is what gates the group.
    assert info["vars"]["CARLA_UE58_ROOT"] == str(clean / "carla")
    assert info["vars"]["CARLA_ROOT"] == str(clean / "carla")


def test_ue5_is_not_mistaken_for_ue58(clean):
    root = clean / "carla5"
    (root / "Unreal/CarlaUnreal").mkdir(parents=True)
    (root / "CMakePresets.json").write_text("{}")
    info = cfg.detect_carla(root)
    assert info["flavor"] == "ue5", "the Autoware/DLSS markers are what separate 5.8"
    assert "CARLA_UE58_ROOT" not in info["vars"]


def test_detects_release_package(clean):
    root = clean / "CARLA_0.9.16"
    (root / "PythonAPI/carla").mkdir(parents=True)
    (root / "CarlaUE4.sh").write_text("")
    info = cfg.detect_carla(root)
    assert info["kind"] == "release"
    assert info["vars"]["CARLA_PACKAGE_ROOT"] == str(root)
    assert info["vars"]["CARLA_TARGET"] == str(root)


def test_non_carla_directory_is_rejected(clean):
    info = cfg.detect_carla(clean)
    assert info["kind"] == "none" and info["vars"] == {}


def test_candidates_report_flavor_for_the_chooser(clean, monkeypatch):
    """Several checkouts on one machine is normal, so each candidate must carry
    enough detail for the user to pick — that is the whole point of asking."""
    _fake_ue58(clean / "carla")
    monkeypatch.chdir(clean)
    found = {c["path"]: c for c in cfg.carla_candidates(cwd=clean)}
    picked = found.get(str(clean / "carla"))
    assert picked, f"cwd/carla not offered: {list(found)}"
    assert picked["flavor"] == "ue58"
    assert "ue58" in picked["detail"]


# --- the two implementations must agree -------------------------------------

def test_shell_loader_honours_precedence(clean):
    """env_common.sh is what the skills use; it must resolve as this module does."""
    cfg.write_config({"SCENIC_ROOT": "/from/config"})
    script = f'. "{COMMON_SH}"; printf "%s" "${{SCENIC_ROOT:-}}"'
    out = subprocess.run(["bash", "-c", script], capture_output=True, text=True,
                         env={"HOME": str(clean), "PATH": "/usr/bin:/bin",
                              "CARLA_TOOLS_CONFIG": str(cfg.user_config_path()),
                              "PWD": str(clean)}, cwd=clean)
    assert out.stdout == "/from/config", out.stderr

    out = subprocess.run(["bash", "-c", script], capture_output=True, text=True,
                         env={"HOME": str(clean), "PATH": "/usr/bin:/bin",
                              "CARLA_TOOLS_CONFIG": str(cfg.user_config_path()),
                              "SCENIC_ROOT": "/from/env", "PWD": str(clean)}, cwd=clean)
    assert out.stdout == "/from/env", "an exported variable must still win"


def test_detection_markers_match_the_shell_helpers():
    """detect_carla duplicates markers the env.sh helpers also test for.

    Two implementations of the same rule drift silently, and the symptom is a
    group that gates differently from the script it gates — so pin the markers.
    """
    py = (REPO / "src/carla_agentic_tools/config.py").read_text()
    shell = "\n".join(p.read_text() for p in REPO.glob("skills/*/*/scripts/env.sh"))
    for marker in ("CMakePresets.json", "Unreal/CarlaUnreal",
                   "CMake/DLSS.cmake", "Carla/Autoware", "CarlaUE4.uproject"):
        assert marker in py, f"{marker} missing from detect_carla"
        assert re.search(re.escape(marker), shell), f"{marker} no longer used by any env.sh"


def test_every_skill_can_reach_the_config():
    """A skill that never loads the loader ignores everything the user recorded.

    Most skills get it through their env.sh. A skill with no env.sh (install-scenic
    has nothing to resolve before it runs) has to load it from check_env.sh
    instead, or the one skill whose job is picking an interpreter cannot see the
    interpreter that was recorded for it.
    """
    unreachable = []
    for skill in sorted(REPO.glob("skills/*/*/SKILL.md")):
        scripts = skill.parent / "scripts"
        loaded = any("_common/env_common.sh" in p.read_text()
                     for p in scripts.glob("*.sh")) if scripts.is_dir() else False
        if not loaded:
            unreachable.append(skill.parent.name)
    assert not unreachable, f"skills that cannot see the config: {unreachable}"


def test_no_needs_block_when_the_preflight_passes(clean, monkeypatch):
    """A key with a working default must not be prompted for on a passing check.

    PYTHON falls back to python3; if that interpreter already imports carla the
    skill is ready, and asking anyway is a prompt the user cannot act on.
    """
    monkeypatch.setenv("CARLA_SKILLS_DIR", str(REPO / "skills"))
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server

    skill = server._find_skill("spawn-vehicles")
    assert "PYTHON" in server._declared_vars(skill), "test no longer covers a defaulted key"
    assert "PYTHON" in server._unmet_keys(skill), "an unset defaulted key is still reportable"

    monkeypatch.setattr(server.subprocess, "run",
                        lambda *a, **k: type("R", (), {"returncode": 0, "stdout": "all PASS",
                                                       "stderr": ""})())
    assert "--- needs ---" not in server.check_prerequisites("spawn-vehicles")


# --- validation -------------------------------------------------------------

def test_validate_rejects_the_wrong_kind_of_tree(clean):
    """A set key is not a configured key; a plausible-looking wrong path is the
    expensive mistake, because the failure surfaces much later."""
    sr = clean / "scenario_runner"
    (sr / "srunner").mkdir(parents=True)
    (sr / "scenario_runner.py").write_text("")
    assert cfg.validate("SCENARIO_RUNNER_ROOT", str(sr))[0]
    # The same directory is not a leaderboard, and must not pass as one.
    ok, why = cfg.validate("LEADERBOARD_ROOT", str(sr))
    assert not ok and "leaderboard_evaluator.py" in why


def test_validate_rejects_missing_and_empty(clean):
    assert cfg.validate("CARLA_ROOT", "")[0] is False
    ok, why = cfg.validate("CARLA_ROOT", str(clean / "nope"))
    assert not ok and "not a directory" in why


def test_validate_python_is_a_command_not_a_tree(clean):
    assert cfg.validate("PYTHON", "sh")[0], "a name on PATH is valid"
    assert not cfg.validate("PYTHON", str(clean))[0], "a directory is not an interpreter"


def test_gating_rejects_a_configured_but_deleted_path(clean, monkeypatch):
    """The case that motivated validation: the path was right when recorded."""
    monkeypatch.setenv("CARLA_SKILLS_DIR", str(REPO / "skills"))
    gone = _fake_ue58(clean / "carla")
    # The ue58 skills build the editor, so they declare the engine path too.
    engine = clean / "UnrealEngine"
    (engine / "Engine/Build/BatchFiles").mkdir(parents=True)
    cfg.write_config({"CARLA_ROOT": str(gone),
                      "CARLA_UNREAL_ENGINE_PATH": str(engine)})
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server
    assert all(e["available"] for e in server.list_skills(group="ue58"))

    import shutil
    shutil.rmtree(gone)
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server2
    ue58 = server2.list_skills(group="ue58")
    assert not any(e["available"] for e in ue58)
    # "configured but unusable" is a different problem from "unset", and saying
    # which is what stops the user re-entering the same answer.
    assert "unusable" in ue58[0]["unavailable_reason"]


def test_set_config_refuses_an_unusable_path(clean, monkeypatch):
    monkeypatch.setenv("CARLA_SKILLS_DIR", str(REPO / "skills"))
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server

    with pytest.raises(ValueError, match="PythonAPI/carla"):
        server.set_config({"CARLA_ROOT": str(clean)})
    assert "CARLA_ROOT" not in cfg.read_config(), "a rejected value must not be stored"
