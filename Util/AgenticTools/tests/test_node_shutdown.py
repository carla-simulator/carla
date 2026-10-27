# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB). Licensed under the MIT license.

"""Exercise complete Node stdio responses when the client closes its input."""

import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest


ROOT = Path(__file__).resolve().parents[1]
NODE = shutil.which("node")


@unittest.skipUnless(NODE, "Node is required for the stdio server")
class TestNodeShutdown(unittest.TestCase):
    """EOF must allow pending stdout writes to drain before the process exits."""

    def run_requests(self, requests, document=None):
        """Send newline-delimited valid requests and immediately close stdin."""
        request_bytes = "".join(json.dumps(item) + "\n" for item in requests)
        with tempfile.TemporaryDirectory() as directory:
            skills = Path(directory) / "skills"
            skill = skills / "large-procedure"
            skill.mkdir(parents=True)
            if document is not None:
                (skill / "SKILL.md").write_bytes(document.encode("utf-8"))
            environment = dict(os.environ)
            environment["CARLA_SKILLS_DIR"] = str(skills)
            environment["CARLA_TOOLS_CONFIG"] = str(Path(directory) / "config.env")
            result = subprocess.run(
                [NODE, str(ROOT / "bin" / "carla-agentic-tools.js")],
                input=request_bytes.encode("utf-8"), stdout=subprocess.PIPE,
                stderr=subprocess.PIPE, env=environment, check=False,
                timeout=20)
        self.assertEqual(result.returncode, 0, result.stderr.decode("utf-8"))
        self.assertEqual(result.stderr, b"")
        if result.stdout:
            self.assertTrue(result.stdout.endswith(b"\n"),
                            "EOF truncated the final JSON response")
        return [json.loads(line) for line in result.stdout.splitlines()]

    @staticmethod
    def request(identifier, method, params=None):
        """Construct a JSON-RPC request without touching server internals."""
        return {"jsonrpc": "2.0", "id": identifier,
                "method": method, "params": params or {}}

    def test_empty_input_exits(self):
        """An idle server exits successfully at EOF without a response."""
        self.assertEqual(self.run_requests([]), [])

    def test_ping_response_survives_eof(self):
        """Small responses retain their ordinary JSON-RPC shape."""
        self.assertEqual(self.run_requests([self.request(1, "ping")]),
                         [{"jsonrpc": "2.0", "id": 1, "result": {}}])

    def test_large_skill_response_survives_eof(self):
        """Real skill I/O returns the complete UTF-8 document before exit."""
        for repeats in (8192, 65536):
            with self.subTest(repeats=repeats):
                document = "---\nname: large-procedure\n---\n" + "Map café 歩道\n" * repeats
                response, = self.run_requests([self.request(
                    2, "tools/call", {"name": "read_skill", "arguments": {
                        "name": "large-procedure"}})], document)
                result = response["result"]
                self.assertEqual(response["id"], 2)
                self.assertFalse(result["isError"])
                text = result["content"][0]["text"]
                self.assertTrue(text.endswith(document))
                self.assertEqual(result["structuredContent"]["result"], text)

    def test_queued_responses_keep_order_and_contents(self):
        """Several pending responses and a trailing ping all reach the client."""
        document = "# Procedure\n" + "unchanged step\n" * 32768
        requests = [self.request(0, "initialize", {
            "protocolVersion": "2025-06-18"})]
        requests.extend(self.request(identifier, "tools/call", {
            "name": "read_skill", "arguments": {"name": "large-procedure"}})
                        for identifier in range(1, 5))
        requests.append(self.request(5, "ping"))
        responses = self.run_requests(requests, document)
        self.assertEqual([item["id"] for item in responses], list(range(6)))
        self.assertEqual(responses[0]["result"]["protocolVersion"], "2025-06-18")
        for response in responses[1:5]:
            self.assertTrue(response["result"]["content"][0]["text"].endswith(document))
        self.assertEqual(responses[-1]["result"], {})


if __name__ == "__main__":
    unittest.main()
