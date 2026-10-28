"""Run python inside the running UE editor (Python Remote Execution).

    python3 ue.py "print(1)"      or      python3 ue.py -f script.py

On Linux the editor binds its UDP socket to 127.0.0.1:6766 and never sees
the 239.0.0.1 multicast discovery, so this client skips discovery: it sends
an "open_connection" unicast to 127.0.0.1:6766 with no "dest" (the editor
accepts messages with an empty dest) and serves the TCP command channel.
"""
import json
import os
import socket
import sys
import uuid

UDP_EP = ("127.0.0.1", 6766)
TCP_EP = ("127.0.0.1", 6776)
NODE = str(uuid.uuid4())


def _msg(type_, data=None):
    m = {"version": 1, "magic": "ue_py", "type": type_, "source": NODE}
    if data is not None:
        m["data"] = data
    return json.dumps(m).encode()


def run(code, timeout=120):
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind(TCP_EP)
    srv.listen(1)
    srv.settimeout(10)
    udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    udp.sendto(_msg("open_connection", {"command_ip": TCP_EP[0], "command_port": TCP_EP[1]}), UDP_EP)
    try:
        conn, _ = srv.accept()
    except socket.timeout:
        raise RuntimeError("editor did not connect back (remote execution off?)")
    conn.settimeout(timeout)
    try:
        conn.sendall(_msg("command", {"command": code, "unattended": True, "exec_mode": "ExecuteFile"}))
        buf = b""
        while True:
            chunk = conn.recv(1 << 20)
            if not chunk:
                break
            buf += chunk
            try:
                m = json.loads(buf.decode())
            except ValueError:
                continue
            if m.get("type") == "command_result":
                return m["data"]
            buf = b""
    finally:
        udp.sendto(_msg("close_connection"), UDP_EP)
        conn.close()
        srv.close()
        udp.close()


if __name__ == "__main__":
    code = open(sys.argv[2]).read() if sys.argv[1] == "-f" else sys.argv[1]
    res = run(code)
    for o in res.get("output", []):
        s = o["output"]
        print(s, end="" if s.endswith("\n") else "\n")
    if not res.get("success"):
        print("FAILED:", res.get("result"))
        sys.exit(1)
