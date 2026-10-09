"""carla-agentic-tools MCP server.

Skills-first design: capabilities live in ``skills/<group>/<name>/`` as a ``SKILL.md``
plus executable scripts. This server exposes the skill registry over MCP so an
agent can discover skills, read their procedure, and check their prerequisites.
Mutating steps (a cook/package is a long, ~30GB operation) are intentionally NOT
auto-run here — the server surfaces the procedure and prerequisite checks, and
the agent/user drives execution deliberately.

This repo is standalone and targets a CARLA instance chosen at runtime: the
skills read CARLA_UE4_ROOT / UE4_ROOT from the environment (see the skill's
env.sh and README), so one install can drive any checkout.

Run:  carla-agentic-tools   (stdio transport)
"""
from __future__ import annotations

import inspect
import os
import re
import subprocess
from pathlib import Path

from . import config as _cfg

# The MCP SDK renamed the high-level server class in 2.0 (FastMCP -> MCPServer)
# and dropped the old import path. Both expose the same surface used here
# (constructor kwargs, the .tool() decorator, .run() defaulting to stdio), so
# support either: a published package is installed fresh by `uvx`, which resolves
# the newest SDK, while existing checkouts still have 1.x.
try:  # mcp >= 2.0
    from mcp.server import MCPServer as _MCPServer
except ImportError:  # mcp 1.x
    from mcp.server.fastmcp import FastMCP as _MCPServer

def _resolve_skills_dir() -> Path:
    """`Skills/` at the carla repo root, or CARLA_SKILLS_DIR when set."""
    override = os.environ.get("CARLA_SKILLS_DIR")
    if override:
        return Path(override).expanduser().resolve()
    # The server lives at Util/AgenticTools/src/carla_agentic_tools/.
    return Path(__file__).resolve().parents[4] / "Skills"


SKILLS_DIR = _resolve_skills_dir()

# Which env var makes a group's skills usable. A group with no entry here (or an
# unset var) is still listed — it is annotated `available: false` rather than
# hidden, because the agent may be about to create that very environment (asking
# to build CARLA is the normal way CARLA_UE4_ROOT comes to exist).
# A group is satisfied when ANY of its variables is set: `ue4` covers both a
# source checkout and a downloaded release, because run-carla-server can serve
# either — the build/import skills inside it still fail loudly on their own if the
# source is missing, which is what check_prerequisites is for.
# `setup` is deliberately absent: download-carla and install-python-api are the
# bootstrap pair, and gating them on an existing CARLA would hide exactly the two
# skills a user with nothing needs.
GROUP_REQUIREMENTS = {
    "ue4": (("CARLA_UE4_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT"),
            "a CARLA ue4-dev checkout or an extracted release"),
    # ue5 and ue58 are deliberately separate groups, not one engine-agnostic group:
    # they track different CARLA branches (ue5-dev / ue58-dev) against different
    # engine forks (ue5-dev-carla = UE 5.5, ue58-dev-carla = UE 5.8), and a skill
    # that names the wrong engine branch sends the user into a multi-hour build of
    # the wrong thing. If the branches converge later, merge the groups then.
    "ue5": (("CARLA_UE5_ROOT",), "a CARLA ue5-dev checkout (UE 5.5)"),
    "ue58": (("CARLA_UE58_ROOT",), "a CARLA ue58-dev checkout (UE 5.8)"),
    "scenario-runner": (("SCENARIO_RUNNER_ROOT",), "a scenario_runner checkout"),
    "leaderboard": (("LEADERBOARD_ROOT",), "a leaderboard checkout (plus its matching scenario_runner)"),
    "scenic": (("SCENIC_ROOT",), "a Scenic install"),
}

# The `instructions` field is returned in the MCP initialize handshake; clients
# MAY add it to the system prompt. It makes the skill library self-advertising in
# an agent-agnostic way — any MCP client gets the routing rule from the server
# alone, with no client-side configuration.
SERVER_INSTRUCTIONS = """\
CARLA build, packaging, cooking, map, vehicle, asset, and server/simulation
tasks have vetted procedures in this server's skill library, each with its
failure modes encoded. The skills are the source of truth for these tasks;
the raw Makefile and Util/BuildTools scripts are a fallback when none matches.
list_skills finds a skill, read_skill(name) returns its procedure,
check_prerequisites(name) verifies its environment.

Paths are configured on first need, not up front. When check_prerequisites
reports a "needs" section, ask the user which path to use — offering each
candidate with its flavor and branch, the install_skill that would obtain it,
and typing a path — then record the answer with set_config so it survives the
session. Never guess a path: several CARLA checkouts on one machine is normal
and the wrong one fails slowly. CARLA_ROOT is the only CARLA path to ask for;
set_config derives the engine-specific variable that gates ue4/ue5/ue58.
"""

def _version() -> str:
    """The version clients are shown; without it FastMCP reports the SDK's."""
    try:
        from . import __version__

        return __version__
    except Exception:
        return ""


# Clients display serverInfo.version, and it identifies which skill library a run
# used. Accept it on the constructor where the SDK offers it (mcp 2.x, and the
# low-level Server), rather than branching on SDK version numbers.
_kwargs: dict = {"name": "carla-agentic-tools", "instructions": SERVER_INSTRUCTIONS}
if "version" in inspect.signature(_MCPServer.__init__).parameters:
    _kwargs["version"] = _version()
mcp = _MCPServer(**_kwargs)

# FastMCP (mcp 1.x) takes no `version` and wraps a low-level Server that does,
# leaving serverInfo.version reporting the *SDK* release. Set it through the
# wrapper so every client sees the skill library's version instead.
if "version" not in _kwargs:
    _low = getattr(mcp, "_mcp_server", None)
    if _low is not None and hasattr(_low, "version"):
        _low.version = _version()


def _skill_dirs() -> list[Path]:
    """Every skill directory, in either layout.

    Skills live in groups (`skills/<group>/<name>/SKILL.md`); the flat form
    (`skills/<name>/SKILL.md`) is still accepted so an ungrouped checkout or a
    user's own drop-in directory keeps working.
    """
    if not SKILLS_DIR.is_dir():
        return []
    found = {p.parent for p in SKILLS_DIR.glob("*/SKILL.md")}
    found |= {p.parent for p in SKILLS_DIR.glob("*/*/SKILL.md")}
    return sorted(found, key=lambda p: (p.parent.name, p.name))


def _group_of(d: Path) -> str:
    """The skill's group: its parent directory, or "" when stored flat."""
    return d.parent.name if d.parent != SKILLS_DIR else ""


#: Engine groups are additionally satisfied by the flavor detected at CARLA_ROOT,
#: so a user is asked "where is your CARLA?" once instead of having to know which
#: of five variable names describes their checkout.
_ENGINE_GROUPS = ("ue4", "ue5", "ue58")


def _group_available(group: str) -> tuple[bool, str]:
    vars_, what = GROUP_REQUIREMENTS.get(group, ((), ""))
    if not vars_:
        return True, ""
    broken: list[str] = []
    for v in vars_:
        value, _, why = _cfg.resolve_valid(v)
        if value and not why:
            return True, ""
        if value:
            broken.append(f"{v}: {why}")
    if group in _ENGINE_GROUPS:
        root, _, why = _cfg.resolve_valid("CARLA_ROOT")
        if root and not why:
            if _cfg.detect_carla(root)["flavor"] == group:
                return True, ""
            broken.append(f"CARLA_ROOT is a {_cfg.detect_carla(root)['flavor'] or 'unrecognised'} "
                          f"CARLA, not {group}")
        elif root:
            broken.append(f"CARLA_ROOT: {why}")
    if broken:
        # A configured-but-unusable path is a different problem from an unset one,
        # and saying which stops the user re-entering the same wrong answer.
        return False, "configured but unusable — " + "; ".join(broken) + f" (needs {what})"
    return False, (f"not configured: set CARLA_ROOT to your CARLA, or {'/'.join(vars_)} "
                   f"directly (needs {what})" if group in _ENGINE_GROUPS
                   else f"not configured: {'/'.join(vars_)} is unset (needs {what})")


#: What each installer skill creates. Two uses: an unset key can be offered as
#: "get it for me" rather than only "type a path", and a skill is never gated on
#: something it exists to produce — install-leaderboard clones the matching
#: scenario_runner as part of its job, so an absent one must not hide it.
_PROVIDES: dict[str, tuple[str, ...]] = {
    "download-carla": ("CARLA_ROOT",),
    "install-python-api": ("PYTHON",),
    "install-scenario-runner": ("SCENARIO_RUNNER_ROOT",),
    "install-leaderboard": ("LEADERBOARD_ROOT", "SCENARIO_RUNNER_ROOT"),
    "install-scenic": ("SCENIC_ROOT", "PYTHON"),
}

#: key -> the skill to suggest for it. First provider wins, so the skill whose
#: primary job is that key is the one offered.
_PROVIDERS = {key: name for name, keys in _PROVIDES.items() for key in reversed(keys)}
_PROVIDERS.update({keys[0]: name for name, keys in _PROVIDES.items()})


def _skill_available(skill: Path, group: str) -> tuple[bool, str]:
    """Group requirement, plus every required key this one skill declares.

    Gating on the group alone marks a skill ready when it is not: `navigate-to`
    sits in the ungated `python-api` group but imports `agents`, which ships only
    inside a CARLA tree — so it needs CARLA_ROOT while its neighbours need only
    the carla wheel.
    """
    ok, why = _group_available(group)
    if not ok:
        return ok, why
    for key, (doc, required) in _declared_vars(skill).items():
        if not required or key not in _cfg.CONFIG_KEYS:
            continue
        # Never gate a skill on the thing it exists to produce: install-leaderboard
        # takes LEADERBOARD_ROOT as an optional input for verify mode, and hiding
        # it until a leaderboard exists hides the way to get one.
        if key in _PROVIDES.get(skill.name, ()):
            continue
        value, _, bad = _cfg.resolve_valid(key)
        if not value:
            return False, f"not configured: {key} is unset ({doc})"
        if bad:
            return False, f"configured but unusable — {key}: {bad}"
    return True, ""


def _find_skill(name: str) -> Path | None:
    for d in _skill_dirs():
        if d.name == name:
            return d
    return None


@mcp.tool()
def list_skills(group: str | None = None) -> list[dict]:
    """List the available CARLA skills as {name, group, description, available} entries.

    The library is the source of procedure for CARLA tasks — match a skill by
    its description and call read_skill(name), rather than improvising from the
    Makefile.

    Groups say what a skill binds to: `python-api` drives any running server,
    `ue4`/`ue5`/`ue58` need that engine's checkout, `ros2` covers the native ROS 2
    interface. `available: false` means something it needs is not configured yet
    (see `unavailable_reason`) — the skill is still listed, because obtaining that
    thing is itself a valid next step. Pass `group` to list one group only.
    """
    out: list[dict] = []
    for d in _skill_dirs():
        g = _group_of(d)
        if group is not None and g != group:
            continue
        desc = ""
        for line in (d / "SKILL.md").read_text().splitlines():
            if line.startswith("description:"):
                desc = line.split(":", 1)[1].strip()
                break
        ok, why = _skill_available(d, g)
        entry = {"name": d.name, "group": g, "description": desc, "available": ok}
        if why:
            entry["unavailable_reason"] = why
        out.append(entry)
    return out


@mcp.tool()
def read_skill(name: str) -> str:
    """Return a skill's full SKILL.md — the step-by-step procedure and its gotchas.

    Call after list_skills once a skill matches the task, and read it before running
    the commands it describes. The first line is the skill's absolute directory: the
    document's own `scripts/...` paths are relative to it, and your working
    directory is not. `name` is a name from list_skills.
    """
    d = _find_skill(name)
    if d is None:
        raise ValueError(f"unknown skill {name!r}; see list_skills()")
    # The document reaches its scripts and references by paths relative to its
    # own directory, and the client's working directory is the user's project.
    # An MCP client has never seen a filesystem path for this skill, so state it
    # here or every `bash scripts/check_env.sh` in the body is unrunnable.
    # The announce line covers the case where the body's prerequisite step is
    # skipped: scripts/check_env.sh prints the same banner itself, but a skill
    # that needs no preflight would otherwise do its work with nothing on
    # screen naming it.
    header = (
        f"Skill directory: {d}\n"
        f"Every `scripts/...` and `references/...` path below is relative to that "
        f"directory. Prefix them with it before running anything.\n"
        f"Announce yourself first: run `printf '\\n\\033[1;36m>> using skill: "
        f"{name}\\033[0m\\n'` before the first step, so a terminal recording "
        f"shows which skill is acting.\n\n"
    )
    return header + (d / "SKILL.md").read_text()


@mcp.tool()
def check_prerequisites(name: str) -> str:
    """Run a skill's read-only prerequisite checks and return its PASS/WARN/FAIL report.

    Call before executing a skill to confirm the environment is ready. Read-only;
    does not modify the system. When the check fails, the report ends in a `needs`
    section naming the paths that are missing or unusable, with candidates found on
    this machine — ask the user to choose and record it with set_config.
    `name` is a name from list_skills.
    """
    d = _find_skill(name)
    if d is None:
        raise ValueError(f"unknown skill {name!r}; see list_skills()")
    script = d / "scripts" / "check_env.sh"
    if not script.is_file():
        raise ValueError(f"skill {name!r} has no prerequisite checks (scripts/check_env.sh)")
    proc = subprocess.run(
        ["bash", str(script)],
        capture_output=True, text=True, timeout=120,
    )
    report = f"exit={proc.returncode}\n--- stdout ---\n{proc.stdout}\n--- stderr ---\n{proc.stderr}"
    # Only when the preflight actually failed. Several keys have a working default
    # (PYTHON falls back to python3), so asking about one the check just passed
    # with would be a prompt for nothing.
    if proc.returncode == 0:
        return report
    needs = _unmet_keys(d)
    return report + ("\n--- needs ---\n" + needs if needs else "")


def _unmet_keys(skill: Path) -> str:
    """The configurable keys this skill declares that have no value yet.

    Only the keys this one skill reads, because prompting for the whole set on
    first contact is worse than the status quo: most skills need one or two, and
    a skill that drives an already-running server needs none.
    """
    lines: list[str] = []
    for key, (what, _required) in _declared_vars(skill).items():
        if key not in _cfg.CONFIG_KEYS:
            continue
        value, _, bad = _cfg.resolve_valid(key)
        if value and not bad:
            continue
        lines.append(f"key: {key}")
        lines.append(f"  what: {what}")
        if bad and value:
            lines.append(f"  current value is unusable: {bad}")
        if key in _PROVIDERS:
            lines.append(f"  install_skill: {_PROVIDERS[key]}")
        if key == "CARLA_ROOT":
            for c in _cfg.carla_candidates():
                lines.append(f"  candidate: {c['path']}  ({c['detail']})")
    if not lines:
        return ""
    return ("Ask the user to choose, then call set_config. Offer each candidate with\n"
            "its flavor and branch, the install_skill, and typing a path.\n"
            + "\n".join(lines))


#: Each env.sh documents the variables it reads in a header block. Parsing that
#: is what lets check_prerequisites report exactly which keys a given skill needs
#: instead of prompting for the whole set.
_VAR_DOC = re.compile(r"^#\s{2,}([A-Z][A-Z0-9_]{2,})\s{2,}(\S.*)$")


def _declared_vars(skill: Path) -> dict[str, tuple[str, bool]]:
    """`{KEY: (doc, required)}` from the skill's env.sh header.

    A key whose doc names a default is optional — `PYTHON` falls back to python3,
    `CARLA_HOST` to 127.0.0.1 — so only the rest can make a skill unavailable.
    """
    env_sh = skill / "scripts" / "env.sh"
    if not env_sh.is_file():
        return {}
    out: dict[str, tuple[str, bool]] = {}
    for line in env_sh.read_text().splitlines()[:40]:
        m = _VAR_DOC.match(line)
        if m:
            doc = m.group(2).strip()
            has_default = "(default" in doc.lower() or "default:" in doc.lower()
            out[m.group(1)] = (doc, not has_default)
    return out


@mcp.tool()
def get_config() -> dict[str, object]:
    """Report every configurable path, its value, and where that value came from.

    Call this before asking the user anything: a key already set in the
    environment or the config file needs no prompt. `candidates` lists the CARLA
    installs found on this machine, each with its flavor and branch, for when
    CARLA_ROOT is unset — offer them as choices rather than picking one.
    """
    entries = {}
    for key, what in {**_cfg.CONFIG_KEYS,
                      **{k: "derived from CARLA_ROOT" for k in _cfg.DERIVED_KEYS}}.items():
        value, source, problem = _cfg.resolve_valid(key)
        entries[key] = {"value": value, "source": source, "what": what,
                        "usable": bool(value) and not problem}
        if problem and value:
            entries[key]["problem"] = problem
    out: dict = {"config_file": str(_cfg.user_config_path()), "keys": entries}
    if not entries["CARLA_ROOT"]["value"]:
        out["candidates"] = _cfg.carla_candidates()
    return out


@mcp.tool()
def set_config(paths: dict[str, str]) -> str:
    """Persist configured paths so they survive the session. Ask the user first.

    `paths` maps a key from get_config to an absolute path; an empty value clears
    it. Setting CARLA_ROOT also writes the engine-specific variable for whatever
    that directory turns out to be, which is what makes the matching ue4/ue5/ue58
    skills available — so ask only for CARLA_ROOT, never for those.

    Never guess a path here. Present get_config's `candidates` to the user with
    their flavor and branch and let them choose, because several checkouts of
    different branches on one machine is normal and the wrong one fails slowly.
    """
    updates: dict[str, str] = {}
    notes: list[str] = []
    for key, raw in paths.items():
        if key not in _cfg.CONFIG_KEYS and key not in _cfg.DERIVED_KEYS:
            raise ValueError(f"unknown key {key!r}; see get_config()")
        if not raw:
            updates[key] = ""
            notes.append(f"{key} cleared")
            continue
        p = Path(raw).expanduser()
        # Reject here rather than storing it: a path that fails validation would
        # otherwise leave the group unavailable with no sign the answer was wrong.
        ok, why = _cfg.validate(key, str(p))
        if not ok:
            raise ValueError(f"{key}: {why}")
        updates[key] = str(p)
        if key == "CARLA_ROOT":
            info = _cfg.detect_carla(p)
            if info["kind"] == "none":
                raise ValueError(f"CARLA_ROOT: {info['detail']} at {p}")
            updates.update(info["vars"])
            notes.append(f"CARLA_ROOT is {info['detail']}; "
                         f"also set {', '.join(k for k in info['vars'] if k != 'CARLA_ROOT')}")
        else:
            notes.append(f"{key} = {p}")

    written = _cfg.write_config(updates)
    shadowed = [k for k in updates if os.environ.get(k) and os.environ[k] != updates[k]]
    if shadowed:
        notes.append("NOTE: an exported environment variable still overrides "
                     f"{', '.join(shadowed)} — unset it or the config is ignored")
    return f"wrote {written}\n" + "\n".join(f"  {n}" for n in notes)


def main() -> None:
    """Serve over stdio."""
    mcp.run()


if __name__ == "__main__":
    main()
