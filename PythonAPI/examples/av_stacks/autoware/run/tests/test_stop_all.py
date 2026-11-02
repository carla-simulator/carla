#!/usr/bin/env python3
"""Regression test for stop_all.sh: it must kill a whole process group, also
after the group leader exits. Runs with plain python (about 15 s):

    python3 PythonAPI/examples/av_stacks/autoware/run/tests/test_stop_all.py
"""

import os
import signal
import subprocess
import tempfile
import time

STOP_ALL = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "stop_all.sh")


def alive(pid):
    """True while pid exists and is not a zombie."""
    try:
        with open(f"/proc/{pid}/stat", encoding="ascii") as f:
            return f.read().split()[2] != "Z"
    except FileNotFoundError:
        return False


def test_kills_member_that_outlives_the_leader():
    """stop_all.sh kills a group member after the group leader exits."""
    # Same shape as the CARLA server: the leader shell (CarlaUnreal.sh) exits
    # on SIGTERM, the engine binary in its group ignores SIGTERM.
    with tempfile.TemporaryDirectory() as d:
        child_file = os.path.join(d, "child.pid")
        cmd = f"bash -c 'trap \"\" TERM; echo $$ > {child_file}; exec sleep 300' & wait"
        with subprocess.Popen(["bash", "-c", cmd], start_new_session=True) as leader:
            try:
                for _ in range(50):
                    if os.path.exists(child_file) and os.path.getsize(child_file):
                        break
                    time.sleep(0.1)
                with open(child_file, encoding="ascii") as f:
                    child = int(f.read())
                pidfile = os.path.join(d, "carla_autoware.pids")
                with open(pidfile, "w", encoding="ascii") as f:
                    f.write(f"carla_server {leader.pid}\n")

                with subprocess.Popen(["bash", STOP_ALL, "--pidfile", pidfile]) as stop:
                    # Reap the leader as soon as it exits, as its real parent
                    # (the launcher) does. An unreaped zombie answers kill -0.
                    leader.wait(timeout=10)
                    assert stop.wait(timeout=60) == 0
                for _ in range(50):
                    if not alive(child):
                        break
                    time.sleep(0.1)
                assert not alive(child), f"group member {child} survived stop_all.sh"
            finally:
                try:
                    os.killpg(leader.pid, signal.SIGKILL)
                except ProcessLookupError:
                    pass

if __name__ == "__main__":
    test_kills_member_that_outlives_the_leader()
    print("ok")
