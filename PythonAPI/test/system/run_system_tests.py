#!/usr/bin/env python3
"""CARLA system tests: offline checks, CI smoke suite, skills, examples and stress.

    python3 PythonAPI/test/system/run_system_tests.py                 # T0,T2,T3,T4,T5
    python3 PythonAPI/test/system/run_system_tests.py --tiers T0,T3
    python3 PythonAPI/test/system/run_system_tests.py --only T3.set-weather,T5.churn
    python3 PythonAPI/test/system/run_system_tests.py --resume Build/test-results/system/<run>
    python3 PythonAPI/test/system/run_system_tests.py --list

See README.md for what each tier covers and how verdicts are decided.
"""

import argparse
import datetime
import json
import os
import signal
import subprocess
import sys
import time
import traceback
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

from harness import core, health, report                       # noqa: E402
from harness.core import Fail, Skip, REPO, check              # noqa: E402
from harness.server import Server                             # noqa: E402
import tests  # noqa: E402,F401  (registers every test)

CHILD_TIMEOUT = 6 * 3600   # backstop; tests time-box their own commands


def select(args):
    tiers = set(args.tiers.split(","))
    chosen = [t for t in core.TESTS if t.tier in tiers]
    if args.only:
        want = args.only.split(",")
        chosen = [t for t in core.TESTS if any(t.id == w or t.id.startswith(w + ".")
                                               or t.id.startswith(w + "-") for w in want)]
    return chosen


def ensure_server(server, port, fresh):
    if server.port == port and not fresh and server.alive():
        return
    if server.pid:
        server.stop()
    elif not fresh and server.adopt(port):
        print(f"[server] reusing the server already on port {port} (pid {server.pid})", flush=True)
        return
    print(f"[server] starting on port {port} ...", flush=True)
    t = time.time()
    server.start(port)
    print(f"[server] ready in {time.time() - t:.0f}s (pid {server.pid}, log {server.log})",
          flush=True)


def run_one(t, run_dir, server, opts):
    """Parent side: make sure the server is up, run the test in a child process."""
    row = {"id": t.id, "tier": t.tier, "log": str(Path(run_dir) / "logs" / f"{t.id}.log")}
    if t.port:
        try:
            ensure_server(server, t.port, t.restart)
        except Exception as e:
            row.update(status="FAIL", msg=f"server failed to start: {e}", secs=0,
                       metrics={}, notes=[], server_start_failed=True)
            return row, []

    # A carla.Client that hits a timeout on a background thread aborts its whole
    # process; isolating each test keeps that from taking the runner down.
    out_json = Path(run_dir) / "artifacts" / t.id / "result.json"
    out_json.parent.mkdir(parents=True, exist_ok=True)
    out_json.unlink(missing_ok=True)
    child_log = Path(run_dir) / "logs" / f"{t.id}.child.log"
    child_log.parent.mkdir(parents=True, exist_ok=True)
    cmd = [sys.executable, __file__, "--child", t.id, "--run-dir", str(run_dir),
           "--soak-minutes", str(opts.soak_minutes)]
    start = time.time()
    with open(child_log, "w") as fh:
        p = subprocess.Popen(cmd, stdout=fh, stderr=subprocess.STDOUT, start_new_session=True)
        try:
            rc = p.wait(CHILD_TIMEOUT)
        except subprocess.TimeoutExpired:
            os.killpg(p.pid, signal.SIGKILL)
            rc = p.wait()
    if out_json.exists():
        res = json.loads(out_json.read_text())
        row, subs = res["row"], res["subs"]
    else:
        tail = "\n".join(child_log.read_text(errors="replace").splitlines()[-15:])
        row.update(status="FAIL", secs=round(time.time() - start, 1), metrics={}, notes=[],
                   msg=f"test process died (rc={rc}) before reporting:\n{tail}")
        if t.xfail:
            row["status"] = "XFAIL"
        subs = []
    if t.port and not server.alive():
        print("[server] dead after test — restarting", flush=True)
        try:
            server.restart(t.port)
        except Exception as e:
            print(f"[server] restart failed: {e}", flush=True)
    return row, subs


def child_main(test_id, run_dir, opts):
    """Child side: run one test plus its health check; write result.json."""
    t = next(x for x in core.TESTS if x.id == test_id)
    server = Server(run_dir)
    ctx = core.Ctx(t, run_dir, server, opts)
    row = {"id": t.id, "tier": t.tier, "log": str(ctx.log_path)}
    start = time.time()
    before = None
    try:
        if t.port:
            check(server.adopt(t.port), f"no server answering on port {t.port}")
            client = ctx.client()
            server.initial_settings = client.get_world().get_settings()
            before = health.snapshot(server, client)
            # Don't keep an idle client through the test: if the server dies or
            # stalls, its background timeout aborts this process.
            del client
            ctx._client = None
        ctx.metrics.update(t.fn(ctx) or {})
        status, msg = "PASS", ""
    except Skip as e:
        status, msg = "SKIP", str(e)
    except Fail as e:
        status, msg = "FAIL", str(e)
    except Exception:  # an unexpected error in the test or the harness
        status, msg = "FAIL", traceback.format_exc()
    finally:
        ctx.close()   # stops any background process the test left running

    if t.port and before is not None:
        try:
            issues, notes, m = health.check(server, ctx.client(), before)
        except Exception as e:
            issues, notes, m = [f"health check itself failed: {e}"], [], {}
        ctx.metrics.update(m)
        ctx.notes += notes
        if issues:
            if status in ("PASS", "SKIP"):
                status, msg = "FAIL", ""
            msg = (msg + "\n" if msg else "") + "health: " + "; ".join(issues)

    if t.xfail:
        status = {"FAIL": "XFAIL", "PASS": "XPASS"}.get(status, status)
        msg = f"[expected failure: {t.xfail}] " + msg
    row.update(status=status, msg=msg, secs=round(time.time() - start, 1),
               metrics=ctx.metrics, notes=ctx.notes)
    out = Path(run_dir) / "artifacts" / t.id / "result.json"
    out.write_text(json.dumps({"row": row, "subs": ctx.subresults}))
    # Skip interpreter teardown: the client can segfault destroying its objects.
    sys.stdout.flush()
    os._exit(0)


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--tiers", default="T0,T2,T3,T4,T5")
    ap.add_argument("--only", help="comma-separated test ids or prefixes (overrides --tiers)")
    ap.add_argument("--resume", help="run directory to continue; finished ids are skipped")
    ap.add_argument("--soak-minutes", type=float, default=60.0)
    ap.add_argument("--no-ros2", action="store_true", help="start the server without -ros2")
    ap.add_argument("--keep-server", action="store_true",
                    help="leave the server running at the end; the next run reuses it")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("--child", help=argparse.SUPPRESS)
    ap.add_argument("--run-dir", help=argparse.SUPPRESS)
    args = ap.parse_args()
    if args.child:
        return child_main(args.child, args.run_dir, args)

    chosen = select(args)
    if args.list:
        for t in chosen:
            print(f"{t.id:40s} port={t.port} {'restart ' if t.restart else ''}"
                  f"{'xfail' if t.xfail else ''}")
        return 0

    if args.resume:
        run_dir = Path(args.resume).resolve()
    else:
        stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S")
        run_dir = REPO / "Build" / "test-results" / "system" / stamp
    run_dir.mkdir(parents=True, exist_ok=True)
    done = {r["id"] for r in report.load(run_dir)} if args.resume else set()
    results = open(run_dir / "results.jsonl", "a")
    print(f"[run] {len(chosen)} tests -> {run_dir}", flush=True)

    server = Server(run_dir, ros2=not args.no_ros2)
    server_failures = 0
    try:
        for t in chosen:
            if t.id in done:
                continue
            if t.port and server_failures >= 2:
                row, subs = ({"id": t.id, "tier": t.tier, "status": "SKIP", "secs": 0,
                              "msg": "server unavailable (failed to start twice)",
                              "metrics": {}, "notes": [], "log": ""}, [])
            else:
                print(f"[test] {t.id} ...", flush=True)
                row, subs = run_one(t, run_dir, server, args)
                server_failures = server_failures + 1 if row.get("server_start_failed") else 0
            for r in subs + [row]:
                results.write(json.dumps(r) + "\n")
            results.flush()
            print(f"[{row['status']:5s}] {t.id} ({row['secs']:.0f}s) "
                  f"{row['msg'].splitlines()[0][:160] if row['msg'] else ''}", flush=True)
    except KeyboardInterrupt:
        print("[run] interrupted — resume with --resume", run_dir, flush=True)
    finally:
        if not args.keep_server:
            server.stop()
        results.close()
        out, total = report.write(run_dir)
        print(f"[run] report: {out}")
        print("[run] " + ", ".join(f"{k}={v}" for k, v in sorted(total.items())))
    return 1 if total.get("FAIL") or total.get("XPASS") else 0


if __name__ == "__main__":
    sys.exit(main())
