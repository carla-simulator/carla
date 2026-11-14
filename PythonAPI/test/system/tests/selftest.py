"""SELF — tests that must FAIL, to prove the verdict machinery is not vacuous.

Run with:  run_system_tests.py --tiers SELF   (all four should report FAIL)
"""

import os
import signal

from harness.core import test, check, has_traceback, PYTHON


@test("SELF.leaked-actor")
def leaked_actor(ctx):
    w = ctx.world()
    bp = w.get_blueprint_library().filter("vehicle.*")[0]
    w.spawn_actor(bp, w.get_map().get_spawn_points()[3])


@test("SELF.traceback")
def traceback_(ctx):
    _, out = ctx.sh([PYTHON, "-c", "raise ValueError('boom')"], ok_rc=None)
    check(not has_traceback(out), "traceback seen")


@test("SELF.sync-left-on")
def sync_left_on(ctx):
    w = ctx.world()
    s = w.get_settings()
    s.synchronous_mode, s.fixed_delta_seconds = True, 0.05
    w.apply_settings(s)


@test("SELF.client-abort")
def client_abort(ctx):
    os.kill(os.getpid(), signal.SIGABRT)


@test("SELF.server-killed")
def server_killed(ctx):
    os.kill(ctx.server.pid, signal.SIGKILL)
