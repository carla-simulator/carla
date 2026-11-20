#!/usr/bin/env python3
"""Minimal client for the Unreal Editor's MCP server (unreal-mcp), for agents and
shells that have no MCP client connected to it.

Uses curl, not urllib: the server answers tools/call as text/event-stream and
urllib returns an empty body for it.

  python3 unreal_mcp.py ping
  python3 unreal_mcp.py call <toolset> <tool> '<json args>'
  python3 unreal_mcp.py script <file.py>        # ProgrammaticToolset.execute_tool_script
"""
import json
import os
import subprocess
import sys

URL = os.environ.get("UNREAL_MCP_URL", "http://localhost:6256/mcp")
PROGRAMMATIC = "editor_toolset.toolsets.programmatic.ProgrammaticToolset"


def _post(body, sid=None, timeout=1800):
    cmd = ["curl", "-s", "-i", "-m", str(timeout), "-X", "POST", URL,
           "-H", "Content-Type: application/json",
           "-H", "Accept: application/json, text/event-stream", "--data-binary", "@-"]
    if sid:
        cmd += ["-H", "Mcp-Session-Id: " + sid]
    out = subprocess.run(cmd, input=json.dumps(body).encode(), capture_output=True).stdout
    out = out.decode(errors="replace")
    head, _, payload = out.partition("\r\n\r\n")
    for line in head.splitlines():
        if line.lower().startswith("mcp-session-id:"):
            sid = line.split(":", 1)[1].strip()
    if payload.lstrip().startswith("event"):
        payload = "\n".join(l[5:] for l in payload.splitlines() if l.startswith("data:"))
    return sid, (json.loads(payload) if payload.strip() else None)


def session():
    sid, r = _post({"jsonrpc": "2.0", "id": 1, "method": "initialize",
                    "params": {"protocolVersion": "2025-06-18", "capabilities": {},
                               "clientInfo": {"name": "restyle-carla-ue58-signs", "version": "1"}}},
                   timeout=30)
    if not r or "result" not in r:
        raise SystemExit("unreal-mcp did not answer at %s: is the editor open and done loading?" % URL)
    _post({"jsonrpc": "2.0", "method": "notifications/initialized"}, sid, timeout=30)
    return sid


def _text(r):
    if r is None:
        return ""
    res = r.get("result", r)
    if isinstance(res, dict) and "content" in res:
        return "\n".join(c.get("text", "") for c in res["content"] if c.get("type") == "text")
    return json.dumps(r)


def call(sid, toolset, tool, args):
    """Calls a toolset tool through the server's call_tool meta-tool; returns its text."""
    _, r = _post({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                  "params": {"name": "call_tool",
                             "arguments": {"toolset_name": toolset, "tool_name": tool, "arguments": args}}}, sid)
    return _text(r)


def run_script(sid, script):
    """Runs a sandbox script; returns its run() dict, or raises with the editor's error text."""
    out = call(sid, PROGRAMMATIC, "execute_tool_script", {"script": script})
    try:
        return json.loads(json.loads(out)["returnValue"])
    except (ValueError, KeyError, TypeError):
        raise RuntimeError(out[-3000:])


if __name__ == "__main__":
    if len(sys.argv) < 2:
        raise SystemExit(__doc__)
    s = session()
    if sys.argv[1] == "ping":
        print("unreal-mcp answering at", URL)
    elif sys.argv[1] == "call":
        print(call(s, sys.argv[2], sys.argv[3], json.loads(sys.argv[4]) if len(sys.argv) > 4 else {}))
    elif sys.argv[1] == "script":
        print(json.dumps(run_script(s, open(sys.argv[2]).read()), indent=1))
    else:
        raise SystemExit(__doc__)
