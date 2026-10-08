"""The version is written in three places; make drift a test failure."""
import json
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(REPO / "src"))


def _plugin_manifest() -> dict:
    return json.loads((REPO / ".claude-plugin" / "plugin.json").read_text())


def _release() -> str:
    """The Claude Code plugin manifest is the reference version."""
    return _plugin_manifest()["version"]


def test_package_version_matches_plugin():
    from carla_agentic_tools import __version__

    assert __version__ == _release(), (
        f"__init__.py says {__version__}, .claude-plugin/plugin.json says {_release()}"
    )


def _reported_version(server) -> str:
    """The version a client sees, however this SDK exposes it.

    mcp 2.x takes `version` on the constructor and keeps it on the server object;
    1.x's FastMCP has no such parameter and holds a low-level Server that does.
    """
    low = getattr(server.mcp, "_mcp_server", None)
    if low is not None and hasattr(low, "create_initialization_options"):
        return low.create_initialization_options().server_version
    return getattr(server.mcp, "version", "")


def test_server_reports_its_own_version():
    """serverInfo.version must be the skill library's, not the MCP SDK's.

    FastMCP silently drops a `version` kwarg it does not declare, which reports
    the SDK release to every client instead.
    """
    sys.path.insert(0, str(REPO / "src"))
    import carla_agentic_tools.server as server

    reported = _reported_version(server)
    assert reported == _release(), \
        f"serverInfo.version is {reported!r}, expected {_release()}"


# --- the Claude Code plugin ------------------------------------------------

def test_plugin_runs_a_file_that_ships():
    """The plugin starts the Node server from its own checkout. If the path drifts
    the server fails to start with nothing but a client-side error to go on."""
    servers = _plugin_manifest()["mcpServers"]
    assert servers, "the plugin must declare the MCP server"
    for name, spec in servers.items():
        target = next(a for a in spec["args"] if "${CLAUDE_PLUGIN_ROOT}" in a)
        rel = target.replace("${CLAUDE_PLUGIN_ROOT}/", "")
        assert (REPO / rel).exists(), f"{name} runs {rel}, which is missing"


# --- the portable Agent Plugin manifests (Cursor, Codex) -------------------

def _portable_manifest() -> dict:
    return json.loads((REPO / "plugin.json").read_text())


def _portable_mcp() -> dict:
    return json.loads((REPO / "mcp.json").read_text())


def test_portable_version_matches_plugin():
    """Cursor and Codex read the root plugin.json; Claude Code reads
    .claude-plugin/plugin.json. Both must name the same release."""
    got = _portable_manifest()["version"]
    assert got == _release(), (
        f"plugin.json says {got}, .claude-plugin/plugin.json says {_release()}"
    )


def test_portable_and_claude_manifests_agree():
    """Same plugin, two manifest dialects — a name split would install the same
    server under two identities."""
    portable, claude = _portable_manifest(), _plugin_manifest()
    assert portable["name"] == claude["name"], (
        f"plugin.json is {portable['name']!r}, "
        f".claude-plugin/plugin.json is {claude['name']!r}"
    )


def test_portable_mcp_runs_a_file_that_ships():
    """The portable manifest resolves the server relative to ${PLUGIN_ROOT}, so
    the path is checked against the repo the same way."""
    servers = _portable_mcp()["mcpServers"]
    assert servers, "mcp.json must declare the server"
    for name, spec in servers.items():
        assert spec["type"] == "stdio", f"{name} must be a stdio server"
        assert spec["cwd"] == "${PLUGIN_ROOT}", \
            f"{name} must resolve from ${{PLUGIN_ROOT}}, got {spec['cwd']!r}"
        script = next(a for a in spec["args"] if a.endswith(".js"))
        assert (REPO / script).exists(), f"{name} runs {script}, which is missing"
