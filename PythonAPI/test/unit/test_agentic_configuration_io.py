"""Real Node processes and Python configuration I/O, isolated from user settings."""
import importlib.util
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

ROOT = Path(__file__).resolve().parents[3]
TOOLS = ROOT / "Util/AgenticTools"
NODE = shutil.which("node")


def config_module():
    """Load configuration without requiring the optional Python MCP SDK."""
    spec = importlib.util.spec_from_file_location("agentic_config", TOOLS / "src/carla_agentic_tools/config.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


@unittest.skipUnless(NODE, "Node is required for cross-runtime tests")
class TestAgenticConfigurationIO(unittest.TestCase):
    """Keep persisted values and protocol responses consistent across runtimes."""

    def test_python_command_name_can_be_persisted(self):
        """A working command name must survive the Node configuration setter."""
        command = Path(sys.executable).name
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ, CARLA_TOOLS_CONFIG=str(Path(tmp) / "config.env"))
            env.pop("PYTHON", None)
            env["PATH"] = str(Path(sys.executable).parent) + os.pathsep + env["PATH"]
            script = ("const s=require('./lib/skills'); s.setConfig({PYTHON:process.argv[1]});"
                      "console.log(JSON.stringify(s.getConfig().keys.PYTHON));")
            result = subprocess.run([NODE, "-e", script, command], cwd=TOOLS,
                                    env=env, capture_output=True, text=True, timeout=15, check=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            value = json.loads(result.stdout)
            self.assertEqual(value["value"], command)
            self.assertTrue(value["usable"])

    def test_absolute_python_control(self):
        """An absolute interpreter path continues to validate."""
        with tempfile.TemporaryDirectory() as tmp:
            env = dict(os.environ, CARLA_TOOLS_CONFIG=str(Path(tmp) / "config.env"))
            script = "require('./lib/skills').setConfig({PYTHON:process.argv[1]})"
            result = subprocess.run([NODE, "-e", script, sys.executable], cwd=TOOLS,
                                    env=env, capture_output=True, text=True, timeout=15, check=False)
            self.assertEqual(result.returncode, 0, result.stderr)

    def test_node_written_utf8_path_can_be_read_by_python(self):
        """Read a Node-written Unicode path through the public Python reader."""
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "config.env"
            value = str(Path(tmp) / "Straße-道路")
            script = "require('./lib/config').writeConfig({SCENIC_ROOT:process.argv[1]})"
            env = dict(os.environ, CARLA_TOOLS_CONFIG=str(target))
            result = subprocess.run([NODE, "-e", script, value], cwd=TOOLS,
                                    env=env, capture_output=True, text=True, timeout=15, check=False)
            self.assertEqual(result.returncode, 0, result.stderr)
            with patch.dict(os.environ, {"CARLA_TOOLS_CONFIG": str(target)}):
                self.assertEqual(config_module().read_config(Path(tmp))["SCENIC_ROOT"], value)

    def test_python_writes_portable_utf8_config(self):
        """Python writes the same UTF-8 representation consumed by Node."""
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "config.env"
            value = str(Path(tmp) / "Straße-道路")
            config_module().write_config({"SCENIC_ROOT": value}, path=target)
            self.assertIn(value, target.read_text(encoding="utf-8"))

    def test_invalid_jsonrpc_envelope_does_not_end_stdio_session(self):
        """Reject invalid envelopes and process a later valid request."""
        valid = {"jsonrpc": "2.0", "id": 41, "method": "ping"}
        for invalid in (None, 3, [], {"jsonrpc": "2.0", "method": 17, "id": 4},
                        *({"jsonrpc": "2.0", "method": "ping", "id": value}
                          for value in (True, {}, [], None, 1.5)),
                        {"jsonrpc": "2.0", "method": "ping", "params": 7}):
            with self.subTest(invalid=invalid):
                result = subprocess.run([NODE, str(TOOLS / "bin/carla-agentic-tools.js")],
                                        input=json.dumps(invalid)+"\n"+json.dumps(valid)+"\n",
                                        capture_output=True, text=True, encoding="utf-8", timeout=15, check=False)
                self.assertEqual(result.returncode, 0, result.stderr)
                messages = [json.loads(line) for line in result.stdout.splitlines()]
                self.assertEqual(messages[0]["error"]["code"], -32600)
                self.assertEqual(messages[-1], {"jsonrpc": "2.0", "id": 41, "result": {}})

    def test_notification_and_ping_control(self):
        """A notification stays silent while an ordinary request is answered."""
        requests = [{"jsonrpc": "2.0", "method": "notifications/initialized"},
                    {"jsonrpc": "2.0", "id": 42, "method": "ping"}]
        result = subprocess.run([NODE, str(TOOLS / "bin/carla-agentic-tools.js")],
                                input="".join(json.dumps(r)+"\n" for r in requests),
                                capture_output=True, text=True, encoding="utf-8", timeout=15, check=False)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual([json.loads(s) for s in result.stdout.splitlines()],
                         [{"jsonrpc": "2.0", "id": 42, "result": {}}])


if __name__ == "__main__":
    unittest.main()
