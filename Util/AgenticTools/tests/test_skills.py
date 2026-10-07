"""Structural tests for the skill library and the MCP surface.

These guard the invariants a published package must not break: every skill is
discoverable, its frontmatter is complete, its group matches where it lives, its
in-repo links resolve, and its scripts parse. Run with `pytest -q`.

Deliberately no CARLA/UE4 required — these are static checks plus the three MCP
tools against the local tree, so they run in CI on a bare runner.
"""
from __future__ import annotations

import ast
import re
import subprocess
import sys
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[1]
SKILLS = REPO / "skills"
GROUPS = {"setup", "python-api", "ue4", "ue5", "ue58", "ros2",
          "scenario-runner", "leaderboard", "scenic"}

sys.path.insert(0, str(REPO / "src"))


def skill_dirs() -> list[Path]:
    return sorted(p.parent for p in SKILLS.glob("*/*/SKILL.md"))


def frontmatter(skill: Path) -> str:
    text = (skill / "SKILL.md").read_text()
    assert text.startswith("---\n"), f"{skill.name}: no YAML frontmatter"
    return text.split("---", 2)[1]


def test_library_is_not_empty():
    assert skill_dirs(), "no skills discovered under skills/<group>/<name>/"


@pytest.mark.parametrize("skill", skill_dirs(), ids=lambda p: p.name)
def test_frontmatter_is_complete(skill: Path):
    fm = frontmatter(skill)
    name = re.search(r"^name:\s*(\S+)", fm, re.M)
    desc = re.search(r"^description:\s*(.+)$", fm, re.M)
    assert name, f"{skill.name}: missing name"
    assert desc, f"{skill.name}: missing description"
    # The router only ever sees name + description, so a stub description makes a
    # skill unreachable however good its body is.
    assert len(desc.group(1)) >= 40, f"{skill.name}: description too short to route on"
    assert name.group(1) == skill.name, f"{skill.name}: name != directory"


@pytest.mark.parametrize("skill", skill_dirs(), ids=lambda p: p.name)
def test_group_matches_directory(skill: Path):
    group = skill.parent.name
    assert group in GROUPS, f"{skill.name}: unknown group directory {group!r}"
    declared = re.search(r"^\s+group:\s*(\S+)", frontmatter(skill), re.M)
    assert declared, f"{skill.name}: no metadata.group"
    assert declared.group(1) == group, (
        f"{skill.name}: metadata.group={declared.group(1)!r} but lives in {group!r}"
    )


@pytest.mark.parametrize("skill", skill_dirs(), ids=lambda p: p.name)
def test_relative_links_resolve(skill: Path):
    """Every markdown link to a repo path must exist (anchors stripped)."""
    body = (skill / "SKILL.md").read_text()
    for target in re.findall(r"\]\((references/[^)]+|scripts/[^)]+|\.\./[^)]+)\)", body):
        path = (skill / target.split("#", 1)[0]).resolve()
        assert path.exists(), f"{skill.name}: dead link -> {target}"


@pytest.mark.parametrize("skill", skill_dirs(), ids=lambda p: p.name)
def test_declared_prerequisites_exist(skill: Path):
    m = re.search(r"^\s+prerequisites:\s*(\S+)", frontmatter(skill), re.M)
    if not m:
        return
    assert (skill / m.group(1)).is_file(), f"{skill.name}: prerequisites path missing"


@pytest.mark.parametrize("skill", skill_dirs(), ids=lambda p: p.name)
def test_scripts_parse(skill: Path):
    """Shell scripts pass `bash -n`; Python scripts parse. Catches broken edits."""
    for script in sorted((skill / "scripts").glob("*")) if (skill / "scripts").is_dir() else []:
        if script.suffix == ".sh":
            r = subprocess.run(["bash", "-n", str(script)], capture_output=True, text=True)
            assert r.returncode == 0, f"{script}: {r.stderr}"
        elif script.suffix == ".py":
            ast.parse(script.read_text())


def test_skill_names_are_unique():
    names = [d.name for d in skill_dirs()]
    dupes = {n for n in names if names.count(n) > 1}
    assert not dupes, f"duplicate skill names across groups: {dupes}"


def test_no_scratch_files_in_library():
    junk = [p for p in SKILLS.rglob("*")
            if p.is_file() and (p.suffix in {".pyc", ".log", ".orig"} or "__pycache__" in p.parts)]
    assert not junk, f"scratch files would ship in the wheel: {junk[:5]}"


# --- MCP surface -------------------------------------------------------------

def test_mcp_tools_work_against_the_tree(monkeypatch):
    monkeypatch.setenv("CARLA_SKILLS_DIR", str(SKILLS))
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server

    listed = server.list_skills()
    assert len(listed) == len(skill_dirs())
    assert {e["group"] for e in listed} <= GROUPS
    assert all(e["description"] for e in listed)

    one = listed[0]["name"]
    body = server.read_skill(one)
    # An MCP client only ever sees this text, so the absolute skill directory has
    # to be in it — the document's own `scripts/...` paths are relative to it.
    first = body.splitlines()[0]
    assert first.startswith("Skill directory: /"), f"no absolute anchor: {first!r}"
    assert Path(first.split(": ", 1)[1], "SKILL.md").is_file()
    assert "\n---\n" in body, "frontmatter missing after the anchor"
    with pytest.raises(ValueError):
        server.read_skill("no-such-skill")

def test_unconfigured_group_is_flagged_not_hidden(monkeypatch, tmp_path):
    """A group with nothing configured stays listed, with an actionable reason.

    Building or checking out the missing thing is itself a valid next step, so
    hiding the skill would hide the fix. Every path var is cleared and the config
    pointed at an empty file, or the result depends on the developer's machine.
    """
    monkeypatch.setenv("CARLA_SKILLS_DIR", str(SKILLS))
    monkeypatch.setenv("CARLA_TOOLS_CONFIG", str(tmp_path / "config.env"))
    monkeypatch.chdir(tmp_path)
    for var in ("CARLA_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT", "CARLA_UE4_ROOT",
                "CARLA_UE5_ROOT", "CARLA_UE58_ROOT", "SCENARIO_RUNNER_ROOT",
                "LEADERBOARD_ROOT", "SCENIC_ROOT"):
        monkeypatch.delenv(var, raising=False)
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server

    ue4 = server.list_skills(group="ue4")
    assert ue4, "ue4 group vanished when nothing was configured"
    assert all(e["available"] is False for e in ue4)
    # The reason has to name what to do, and CARLA_ROOT is the only path a user
    # is ever asked for, so it must appear rather than just the derived vars.
    assert all("CARLA_ROOT" in e["unavailable_reason"] for e in ue4)

    # A group needing no checkout stays available on a bare machine, except for
    # the one skill that imports `agents` — see the navigate-to case below.
    api = {e["name"]: e for e in server.list_skills(group="python-api")}
    assert all(e["available"] for n, e in api.items() if n != "navigate-to")

    # download-carla must never be gated: it is the way out of an empty machine.
    setup = {e["name"]: e for e in server.list_skills(group="setup")}
    assert setup["download-carla"]["available"]
    assert setup["install-python-api"]["available"]


def test_navigate_to_needs_carla_root_though_its_group_does_not(monkeypatch, tmp_path):
    """`agents` ships only inside a CARLA tree, not in the carla wheel.

    Gating on the group alone reported this skill ready with just a wheel and a
    running server, which is the one case in `python-api` where that is false.
    """
    monkeypatch.setenv("CARLA_SKILLS_DIR", str(SKILLS))
    monkeypatch.setenv("CARLA_TOOLS_CONFIG", str(tmp_path / "config.env"))
    monkeypatch.chdir(tmp_path)
    for var in ("CARLA_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT", "CARLA_UE4_ROOT",
                "CARLA_UE5_ROOT", "CARLA_UE58_ROOT"):
        monkeypatch.delenv(var, raising=False)
    for mod in [m for m in list(sys.modules) if m.startswith("carla_agentic_tools")]:
        del sys.modules[mod]
    import carla_agentic_tools.server as server

    nav = next(e for e in server.list_skills(group="python-api") if e["name"] == "navigate-to")
    assert nav["available"] is False
    assert "CARLA_ROOT" in nav["unavailable_reason"]

    spawn = next(e for e in server.list_skills(group="python-api") if e["name"] == "spawn-vehicles")
    assert spawn["available"], "a wheel-only skill must not be gated on CARLA_ROOT"
