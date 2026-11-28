"""Start, stop and watch the CARLA UE 5.8 server through the run-carla-ue58-server skill."""

import os
import re
import signal
import subprocess
import sys
import time
from pathlib import Path

from .core import REPO

RUN_SERVER = REPO / "Skills" / "ue58" / "run-carla-ue58-server" / "scripts" / "run_server.sh"
MAP = "Town10HD_Opt"
CRASH_MARKERS = ("Signal 11", "Fatal error", "Assertion failed", "Segmentation fault",
                 "SIGSEGV", "Unhandled Exception")


# Liveness probes run in a throwaway interpreter: a carla.Client that hits a
# timeout on a background thread aborts its whole process (libc++abi), so the
# long-lived runner process never holds one.
_PROBE = """
import sys, carla
c = carla.Client("127.0.0.1", int(sys.argv[1]))
c.set_timeout(float(sys.argv[2]))
c.get_server_version()
if sys.argv[3] == "ready":
    w = c.get_world()
    assert sys.argv[4] in w.get_map().name, w.get_map().name
    w.wait_for_tick(10.0)
"""


def _probe(port, kind, timeout=10.0):
    try:
        r = subprocess.run([sys.executable, "-c", _PROBE, str(port), str(timeout), kind, MAP],
                           capture_output=True, timeout=timeout * 3 + 30)
        return r.returncode == 0
    except subprocess.TimeoutExpired:
        return False


def _prefer_oom_kill(pid):
    """Make the server the kernel's first OOM victim.

    A stress test that exhausts RAM otherwise gets the desktop session killed
    (editors run with oom_score_adj 300; the server with 0). Raising our own
    process's score needs no privileges.
    """
    try:
        Path(f"/proc/{pid}/oom_score_adj").write_text("1000")
    except OSError:
        pass


def _find_pid(port):
    """The UnrealEditor process serving this RPC port, or None."""
    tag = f"-carla-rpc-port={port}".encode()
    for d in Path("/proc").iterdir():
        if not d.name.isdigit():
            continue
        try:
            argv = (d / "cmdline").read_bytes().split(b"\0")
        except OSError:
            continue
        if argv and argv[0].endswith(b"UnrealEditor") and tag in argv:
            return int(d.name)
    return None


def rss_mb(pid):
    try:
        for line in Path(f"/proc/{pid}/status").read_text().splitlines():
            if line.startswith("VmRSS:"):
                return int(line.split()[1]) // 1024
    except OSError:
        pass
    return None


class Server:
    def __init__(self, run_dir, ros2=True):
        self.root = Path(run_dir) / "server"
        self.ros2 = ros2
        self.port = self.pid = self.log = None
        self.starts = 0
        self.initial_settings = None

    # -- lifecycle -----------------------------------------------------------
    def start(self, port, timeout=1800):
        """Launch headless in -game mode and block until a client can talk to it."""
        if _find_pid(port):
            raise RuntimeError(f"a server already serves port {port}; stop it first")
        self.starts += 1
        d = self.root / f"{self.starts:02d}-port{port}"
        d.mkdir(parents=True, exist_ok=True)
        # run_server.sh derives the tree from $PWD unless told; pin both.
        env = dict(os.environ, PORT=str(port), DETACH="1", TMPDIR=str(d),
                   CARLA_UE58_ROOT=str(REPO))
        if self.ros2:
            env["ROS2"] = "1"
        with open(d / "launch.out", "w") as fh:
            subprocess.run(["bash", str(RUN_SERVER), "game", MAP], env=env, cwd=str(REPO),
                           stdout=fh, stderr=subprocess.STDOUT, timeout=300)
        self.port, self.log = port, d / f"carla-ue58-{port}.log"
        end = time.time() + timeout
        while time.time() < end:
            self.pid = _find_pid(port)
            if self.pid is None:
                raise RuntimeError(f"server process gone during startup; see {self.log}")
            _prefer_oom_kill(self.pid)
            if _probe(port, "ready"):
                return
            time.sleep(5)
        raise RuntimeError(f"server on {port} not ready after {timeout}s; see {self.log}")

    def adopt(self, port):
        """Take over a server already running on `port` (left by --keep-server)."""
        pid = _find_pid(port)
        if pid is None:
            return False
        self.port, self.pid = port, pid
        try:
            self.log = Path(os.readlink(f"/proc/{pid}/fd/1"))
        except OSError:
            self.log = None
        if not _probe(port, "alive"):
            self.pid = self.port = None
            return False
        return True

    def stop(self):
        """Kill only our server's process group (run_server.sh stop would pkill every editor)."""
        if self.pid:
            try:
                os.killpg(os.getpgid(self.pid), signal.SIGTERM)
            except ProcessLookupError:
                pass
            end = time.time() + 60
            while time.time() < end and Path(f"/proc/{self.pid}").exists():
                time.sleep(1)
            if Path(f"/proc/{self.pid}").exists():
                os.killpg(os.getpgid(self.pid), signal.SIGKILL)
                time.sleep(2)
        # Wait for the RPC/streaming ports to be released before any restart.
        end = time.time() + 30
        while self.port and time.time() < end and _port_busy(self.port):
            time.sleep(1)
        self.pid = self.port = None

    def restart(self, port=None):
        port = port or self.port
        self.stop()
        self.start(port)

    # -- observation -----------------------------------------------------------
    def alive(self):
        if not self.pid or not Path(f"/proc/{self.pid}").exists():
            return False
        return _probe(self.port, "alive")

    def rss_mb(self):
        return rss_mb(self.pid) if self.pid else None

    def log_size(self):
        try:
            return self.log.stat().st_size
        except (OSError, AttributeError):
            return 0

    def log_since(self, offset):
        try:
            with open(self.log, "rb") as fh:
                fh.seek(offset)
                return fh.read().decode(errors="replace")
        except (OSError, TypeError):
            return ""

    def crash_lines(self, offset):
        return [l for l in self.log_since(offset).splitlines()
                if any(m in l for m in CRASH_MARKERS)]


def _port_busy(port):
    out = subprocess.run(["ss", "-ltn"], capture_output=True, text=True).stdout
    return re.search(rf":{port}\b", out) is not None
