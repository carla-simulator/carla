"""Test registry, per-test context and process helpers for the system tests."""

import os
import re
import signal
import subprocess
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[4]
SYSTEM_DIR = REPO / "PythonAPI" / "test" / "system"
SKILLS = REPO / "Skills"
EXAMPLES = REPO / "PythonAPI" / "examples"
PYTHON = "/usr/bin/python3"

# Default RPC port per tier: the CI smoke suite hardcodes 3654; several examples
# hardcode localhost:2000, so everything after T2 runs there.
TIER_PORT = {"T0": None, "T2": 3654, "T3": 2000, "T4": 2000, "T5": 2000, "SELF": 2000}
TM_PORT = 8000

TESTS = []


class Fail(Exception):
    pass


class Skip(Exception):
    pass


class Test:
    def __init__(self, fn, id, tier, port, restart, xfail):
        self.fn, self.id, self.tier = fn, id, tier
        self.port, self.restart, self.xfail = port, restart, xfail


def test(id, restart=False, xfail=None):
    """Register a test. The tier is the id prefix (T0..T5).

    restart: start a fresh server before the test (stress tests).
    xfail:   reason the test is expected to fail (a known, documented bug).
    """
    tier = id.split(".")[0]

    def deco(fn):
        TESTS.append(Test(fn, id, tier, TIER_PORT[tier], restart, xfail))
        return fn
    return deco


def check(cond, msg):
    if not cond:
        raise Fail(msg)


def has_traceback(out):
    """A Python traceback other than the KeyboardInterrupt our own SIGINT causes."""
    if "Traceback (most recent call last)" not in out:
        return False
    # Tracebacks from worker processes interleave, so look for the exception
    # line itself rather than splitting into blocks. KeyboardInterrupt never matches.
    return re.search(r"^\s*[A-Za-z_][\w.]*(Error|Exception)\b", out, re.M) is not None


class Proc:
    """A command running in the background; output goes to a file."""

    def __init__(self, cmd, env, cwd, out_path):
        self.cmd, self.out_path = cmd, out_path
        self._fh = open(out_path, "w")
        self.p = subprocess.Popen(cmd, env=env, cwd=cwd, stdout=self._fh,
                                  stderr=subprocess.STDOUT, start_new_session=True)

    def output(self):
        self._fh.flush()
        return Path(self.out_path).read_text(errors="replace")

    def running(self):
        return self.p.poll() is None

    def wait_for(self, text, timeout):
        """Block until text appears in the output; Fail if the process dies or times out."""
        end = time.time() + timeout
        while time.time() < end:
            if text in self.output():
                return
            if not self.running():
                raise Fail(f"{self.cmd[1:3]} exited (rc={self.p.returncode}) before "
                           f"printing {text!r}:\n{self.output()[-1500:]}")
            time.sleep(0.5)
        raise Fail(f"timed out after {timeout}s waiting for {text!r}")

    def stop(self, grace=30):
        """SIGINT (lets the script's finally-block clean up), then SIGKILL."""
        if self.running():
            try:
                os.killpg(self.p.pid, signal.SIGINT)
            except ProcessLookupError:
                pass
            try:
                self.p.wait(grace)
            except subprocess.TimeoutExpired:
                os.killpg(self.p.pid, signal.SIGKILL)
                self.p.wait()
        self._fh.close()
        return self.p.returncode, Path(self.out_path).read_text(errors="replace")


class Ctx:
    """What a test function receives."""

    def __init__(self, test, run_dir, server, opts):
        self.test, self.server, self.opts = test, server, opts
        self.port = test.port
        self.out = Path(run_dir) / "artifacts" / test.id
        self.out.mkdir(parents=True, exist_ok=True)
        self.log_path = Path(run_dir) / "logs" / f"{test.id}.log"
        self.log_path.parent.mkdir(parents=True, exist_ok=True)
        self._log = open(self.log_path, "a")
        self.metrics = {}
        self.notes = []
        self.subresults = []
        self._procs = []
        self._n = 0

    # -- environment -------------------------------------------------------
    def env(self, **extra):
        e = dict(os.environ)
        pypath = [str(REPO / "PythonAPI" / "carla"), e.get("PYTHONPATH", "")]
        e.update(CARLA_HOST="127.0.0.1", CARLA_TIMEOUT="60", TM_PORT=str(TM_PORT),
                 CARLA_ROOT=str(REPO), SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy",
                 PYTHONPATH=os.pathsep.join(p for p in pypath if p),
                 PYTHONUNBUFFERED="1")
        if self.port:
            e["CARLA_PORT"] = str(self.port)
        e.update({k: str(v) for k, v in extra.items()})
        return e

    def log(self, msg):
        self._log.write(msg.rstrip("\n") + "\n")
        self._log.flush()

    # -- processes ---------------------------------------------------------
    def sh(self, cmd, timeout=300, cwd=None, env=None, ok_rc=(0,), allow_timeout=False):
        """Run a command to completion. Returns (rc, output).

        allow_timeout: the command runs forever by design; on timeout it gets
        SIGINT (then SIGKILL) and the timeout is not a failure.
        Raises Fail when rc is not in ok_rc (ok_rc=None accepts any rc).
        """
        cmd = [str(c) for c in cmd]
        self._n += 1
        out_path = self.out / f"cmd{self._n:02d}.out"
        self.log(f"$ {' '.join(cmd)}")
        p = Proc(cmd, env or self.env(), str(cwd or REPO), out_path)
        timed_out = False
        try:
            p.p.wait(timeout)
        except subprocess.TimeoutExpired:
            timed_out = True
        rc, out = p.stop()
        self.last_timed_out = timed_out
        self.log(out[-20000:])
        self.log(f"[rc={rc}{' timeout' if timed_out else ''}]")
        if timed_out and not allow_timeout:
            raise Fail(f"{' '.join(cmd[:3])} timed out after {timeout}s:\n{out[-1500:]}")
        if not timed_out and ok_rc is not None and rc not in ok_rc:
            raise Fail(f"{' '.join(cmd[:4])} exited rc={rc}:\n{out[-1500:]}")
        return rc, out

    def bg(self, cmd, cwd=None, env=None):
        cmd = [str(c) for c in cmd]
        self._n += 1
        out_path = self.out / f"cmd{self._n:02d}.out"
        self.log(f"$ {' '.join(cmd)} &")
        p = Proc(cmd, env or self.env(), str(cwd or REPO), out_path)
        self._procs.append(p)
        return p

    def stop_bg(self, p):
        rc, out = p.stop()
        self.log(f"--- background {' '.join(p.cmd[:3])} [rc={rc}]\n{out[-20000:]}")
        return rc, out

    def close(self):
        for p in self._procs:
            if p.running():
                self.stop_bg(p)
        self._log.close()

    # -- CARLA ---------------------------------------------------------------
    def client(self, timeout=30.0):
        if getattr(self, "_client", None) is None:
            import carla
            self._client = carla.Client("127.0.0.1", self.port)
        self._client.set_timeout(timeout)
        return self._client

    def world(self):
        """The current world, after one tick so the actor list reflects the server.

        Actor lists come from the last snapshot the client received; reading one
        right after connecting (or after another process spawned) can be stale.
        """
        w = self.client().get_world()
        try:
            w.wait_for_tick(3.0)
        except RuntimeError:
            pass   # synchronous with nobody ticking: nothing newer will arrive anyway
        return w
