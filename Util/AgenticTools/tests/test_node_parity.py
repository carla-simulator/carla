"""The Node and Python servers must answer alike.

Two implementations of one protocol drift silently, and the symptom is a user on
`npx` being told a skill is available when the `uvx` user is told it is not. So
run both over real stdio against the same skills and the same config, and diff
what comes back.

Skipped when no `node` is on PATH — a Python-only contributor should not be
blocked by it, and CI runs both.
"""
from __future__ import annotations

import json
import os
import shutil
import subprocess
import sys
import threading
import queue
from pathlib import Path

import pytest

REPO = Path(__file__).resolve().parents[1]
NODE = shutil.which("node")

pytestmark = pytest.mark.skipif(NODE is None, reason="no node on PATH")


class Server:
    """A minimal MCP stdio client, enough to compare two servers."""

    def __init__(self, cmd: list[str], env: dict, name: str = "server"):
        self.name = name
        self.proc = subprocess.Popen(
            cmd, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, text=True, bufsize=1, env=env, cwd=str(REPO),
        )
        self.lines: queue.Queue = queue.Queue()
        threading.Thread(target=self._pump, daemon=True).start()
        self._id = 0

    def _pump(self):
        for line in self.proc.stdout:
            self.lines.put(line)

    def call(self, method: str, params: dict | None = None) -> dict:
        self._id += 1
        self.proc.stdin.write(json.dumps(
            {"jsonrpc": "2.0", "id": self._id, "method": method, "params": params or {}}) + "\n")
        self.proc.stdin.flush()
        while True:
            try:
                raw = self.lines.get(timeout=30).strip()
            except queue.Empty:
                # A server that died on import would otherwise hang the suite;
                # its stderr says why, so surface that instead of a timeout.
                self.proc.poll()
                err = (self.proc.stderr.read() or "").strip() if self.proc.stderr else ""
                raise AssertionError(
                    f"{self.name} did not answer {method} "
                    f"(exit={self.proc.returncode})\n{err[-1500:]}")
            if not raw:
                continue
            msg = json.loads(raw)
            if msg.get("id") == self._id:
                return msg

    def notify(self, method: str):
        self.proc.stdin.write(json.dumps({"jsonrpc": "2.0", "method": method}) + "\n")
        self.proc.stdin.flush()

    def tool(self, name: str, args: dict | None = None) -> dict:
        return self.call("tools/call", {"name": name, "arguments": args or {}})["result"]

    def close(self):
        try:
            self.proc.stdin.close()
        finally:
            self.proc.terminate()


def _env(tmp_path: Path) -> dict:
    """One environment for both: same skills, same fresh config, no ambient CARLA
    variables — otherwise the two servers are answering different questions.

    HOME is left alone deliberately. Python resolves its user site-packages from
    it, so redirecting it hides `mcp` and the Python server dies on import;
    CARLA_TOOLS_CONFIG is what isolates the config, and it is enough.
    """
    env = dict(os.environ)
    for key in ("CARLA_ROOT", "CARLA_TARGET", "CARLA_PACKAGE_ROOT", "CARLA_UE4_ROOT",
                "CARLA_UE5_ROOT", "CARLA_UE58_ROOT", "SCENARIO_RUNNER_ROOT",
                "LEADERBOARD_ROOT", "SCENIC_ROOT", "PYTHON",
                "CARLA_UNREAL_ENGINE_PATH", "UE4_ROOT"):
        env.pop(key, None)
    env["CARLA_SKILLS_DIR"] = str(REPO / "skills")
    env["CARLA_TOOLS_CONFIG"] = str(tmp_path / "config.env")
    env["PYTHONPATH"] = str(REPO / "src") + os.pathsep + env.get("PYTHONPATH", "")
    return env


@pytest.fixture()
def pair(tmp_path):
    env = _env(tmp_path)
    py = Server([sys.executable, "-m", "carla_agentic_tools.server"], env, "python")
    js = Server([NODE, str(REPO / "bin" / "carla-agentic-tools.js")], env, "node")
    for s in (py, js):
        s.call("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                              "clientInfo": {"name": "parity", "version": "0"}})
        s.notify("notifications/initialized")
    yield py, js
    py.close()
    js.close()


def test_server_identity_matches(pair):
    py, js = pair
    a = py.call("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                               "clientInfo": {"name": "p", "version": "0"}})["result"]
    b = js.call("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                               "clientInfo": {"name": "p", "version": "0"}})["result"]
    assert a["serverInfo"] == b["serverInfo"], "the two servers report different identities"
    assert a["protocolVersion"] == b["protocolVersion"]


def test_same_tools_are_offered(pair):
    py, js = pair
    a = sorted(t["name"] for t in py.call("tools/list")["result"]["tools"])
    b = sorted(t["name"] for t in js.call("tools/list")["result"]["tools"])
    assert a == b, "the two servers offer different tools"


def test_list_skills_agrees_exactly(pair):
    """Names, groups and — the part that matters — availability and its reason."""
    py, js = pair
    a = py.tool("list_skills")["structuredContent"]["result"]
    b = js.tool("list_skills")["structuredContent"]["result"]
    assert [e["name"] for e in a] == [e["name"] for e in b], "different skills or order"
    for x, y in zip(a, b):
        assert x == y, f"{x['name']} differs:\n  python={x}\n  node  ={y}"


def test_read_skill_is_byte_identical(pair):
    py, js = pair
    for name in ("download-carla", "run-scenic-scenario", "navigate-to"):
        a = py.tool("read_skill", {"name": name})["content"][0]["text"]
        b = js.tool("read_skill", {"name": name})["content"][0]["text"]
        assert a == b, f"{name}: read_skill output differs"


def test_unknown_skill_fails_the_same_way(pair):
    py, js = pair
    a = py.tool("read_skill", {"name": "no-such-skill"})
    b = js.tool("read_skill", {"name": "no-such-skill"})
    assert a["isError"] is True and b["isError"] is True
    assert a["content"][0]["text"] == b["content"][0]["text"], "different error text"


def test_get_config_agrees(pair):
    # An object return lands in structuredContent unwrapped, unlike a list.
    py, js = pair
    a = py.tool("get_config")["structuredContent"]
    b = js.tool("get_config")["structuredContent"]
    assert a["config_file"] == b["config_file"]
    assert a["keys"] == b["keys"], "the two servers resolve the keys differently"


def test_a_path_recorded_by_one_is_seen_by_the_other(pair, tmp_path):
    """One config, either writer. This is why the file format is shared."""
    py, js = pair
    root = tmp_path / "carla"
    (root / "Unreal/CarlaUnreal/Plugins/Carla/Source/Carla/Autoware").mkdir(parents=True)
    (root / "CMake").mkdir()
    (root / "CMake/DLSS.cmake").write_text("")
    (root / "CMakePresets.json").write_text("{}")
    (root / "PythonAPI/carla").mkdir(parents=True)

    written = js.tool("set_config", {"paths": {"CARLA_ROOT": str(root)}})
    assert written["isError"] is False, written["content"][0]["text"]
    assert "also set CARLA_UE58_ROOT" in written["content"][0]["text"]

    # The Python server was started before the file existed, so this also pins
    # that neither side caches the config.
    seen = py.tool("get_config")["structuredContent"]["keys"]
    assert seen["CARLA_UE58_ROOT"]["value"] == str(root)
    assert seen["CARLA_ROOT"]["usable"] is True

    ue58_py = py.tool("list_skills", {"group": "ue58"})["structuredContent"]["result"]
    ue58_js = js.tool("list_skills", {"group": "ue58"})["structuredContent"]["result"]
    assert ue58_py == ue58_js


def test_set_config_rejects_the_same_paths(pair, tmp_path):
    py, js = pair
    a = py.tool("set_config", {"paths": {"CARLA_ROOT": str(tmp_path)}})
    b = js.tool("set_config", {"paths": {"CARLA_ROOT": str(tmp_path)}})
    assert a["isError"] is True and b["isError"] is True
    for r in (a, b):
        assert "PythonAPI/carla" in r["content"][0]["text"], r["content"][0]["text"]


def test_group_requirements_are_declared_in_both(pair):
    """The gating tables are hand-maintained in two languages.

    A group added to one and not the other makes the same skill usable on `uvx`
    and unusable on `npx` — so compare the tables directly, not just their
    effect on the skills that happen to exist today.
    """
    py_src = (REPO / "src/carla_agentic_tools/server.py").read_text()
    js_src = (REPO / "lib/skills.js").read_text()

    py_block = py_src.split("GROUP_REQUIREMENTS = {", 1)[1].split("\n}", 1)[0]
    js_block = js_src.split("const GROUP_REQUIREMENTS = {", 1)[1].split("\n};", 1)[0]

    import re
    # Python quotes every key; JS quotes only the hyphenated ones.
    py_groups = set(re.findall(r'"([a-z0-9-]+)":', py_block))
    js_groups = {a or b for a, b in
                 re.findall(r'(?:"([a-z0-9-]+)"|\b([a-z0-9]+)):\s*\[\[', js_block)}
    assert py_groups == js_groups, (
        f"gated groups differ — only in python: {py_groups - js_groups}; "
        f"only in node: {js_groups - py_groups}"
    )

    # Every variable named on one side must be named on the other, too.
    py_vars = set(re.findall(r'"(CARLA_[A-Z0-9_]+|[A-Z]+_ROOT)"', py_block))
    js_vars = set(re.findall(r'"(CARLA_[A-Z0-9_]+|[A-Z]+_ROOT)"', js_block))
    assert py_vars == js_vars, (
        f"gating variables differ — only in python: {py_vars - js_vars}; "
        f"only in node: {js_vars - py_vars}"
    )


def test_tool_descriptions_agree(pair):
    """The descriptions are what the model routes on, so drift is a real defect.

    Comparing names only let three of them fall out of step: the Python
    docstrings still said `available: false` meant a group's environment variable
    was unset, named no ue58 group, and did not mention read_skill's absolute
    first line or check_prerequisites' `needs` section.
    """
    def normalise(tools):
        # Python docstrings arrive indented and hard-wrapped differently; compare
        # the words, not the layout.
        return {t["name"]: " ".join(t["description"].split()) for t in tools}

    a = normalise(pair[0].call("tools/list")["result"]["tools"])
    b = normalise(pair[1].call("tools/list")["result"]["tools"])
    assert set(a) == set(b)
    for name in sorted(a):
        assert a[name] == b[name], (
            f"{name} description differs:\n  python: {a[name][:300]}\n  node:   {b[name][:300]}"
        )


def test_handshake_instructions_agree(pair):
    """`instructions` rides the initialize handshake and clients may put it in the
    system prompt, so it is the routing rule for every MCP client. Both servers
    must state the same one."""
    a = pair[0].call("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                                    "clientInfo": {"name": "p", "version": "0"}})["result"]
    b = pair[1].call("initialize", {"protocolVersion": "2025-06-18", "capabilities": {},
                                    "clientInfo": {"name": "p", "version": "0"}})["result"]
    assert " ".join((a.get("instructions") or "").split()) \
        == " ".join((b.get("instructions") or "").split())
    assert "set_config" in (a.get("instructions") or ""), "the config rule must be in it"
