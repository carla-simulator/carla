#!/usr/bin/env python3
"""
Master End-to-End (E2E) Regression Test Runner for CARLA Map Maker.

Discovers and runs all milestone E2E test suites (`e2e_tests/m*/test_*.py`) in chronological
milestone order. Automatically manages a single headless CARLA server instance if one is not
already running on the target RPC port (unless `--offline-only` is passed).
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import subprocess
import sys
import time

E2E_DIR = Path(__file__).resolve().parent
REPO_ROOT = E2E_DIR.parent
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
import tempfile
tempfile.tempdir = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(E2E_DIR / "m1_osm2odr"))

from test_m1_osm2odr import find_unreal_editor_binary, is_port_open  # noqa: E402


def main() -> int:
    parser = argparse.ArgumentParser(description="Run all CARLA Map Maker E2E milestone test suites.")
    parser.add_argument("--host", default="127.0.0.1", help="CARLA server host (default: 127.0.0.1)")
    parser.add_argument("--port", type=int, default=2000, help="CARLA server RPC port (default: 2000)")
    parser.add_argument(
        "--offline-only",
        action="store_true",
        help="Skip live CARLA simulator tests and run only offline tests.",
    )
    parser.add_argument(
        "--milestone",
        default="",
        help="Optional milestone prefix filter, e.g. 'm1' or 'm2'.",
    )
    args = parser.parse_args()

    import re

    def milestone_sort_key(p: Path) -> tuple[int, str]:
        m = re.match(r"^m(\d+)", p.parent.name.lower())
        return (int(m.group(1)) if m else 999, p.name)

    test_scripts = sorted(E2E_DIR.glob("m*/test_*.py"), key=milestone_sort_key)
    if args.milestone:
        test_scripts = [
            p for p in test_scripts if p.parent.name.lower().startswith(args.milestone.lower())
        ]

    if not test_scripts:
        print("No matching E2E test scripts found.")
        return 1

    server_proc: subprocess.Popen[bytes] | None = None
    log_handle = None

    try:
        if not args.offline_only and not is_port_open(args.host, args.port):
            editor_bin = find_unreal_editor_binary(REPO_ROOT)
            uproject = REPO_ROOT / "Unreal/CarlaUnreal/CarlaUnreal.uproject"
            if editor_bin is not None and uproject.is_file():
                log_path = LOCAL_TMP_DIR / f"carla_e2e_suite_{args.port}.log"
                print(f"[e2e] Starting headless CARLA server on {args.host}:{args.port} (log: {log_path})...")
                log_handle = log_path.open("w", encoding="utf-8")
                server_proc = subprocess.Popen(
                    [
                        str(editor_bin),
                        str(uproject),
                        "OpenDriveMap",
                        "-game",
                        "-RenderOffScreen",
                        "-notrace",
                        "-nosound",
                        "-stdout",
                        f"-carla-rpc-port={args.port}",
                    ],
                    stdout=log_handle,
                    stderr=subprocess.STDOUT,
                )
                for _ in range(120):
                    if server_proc.poll() is not None:
                        print(f"[e2e] ERROR: Server exited early with code {server_proc.returncode}")
                        return 1
                    if is_port_open(args.host, args.port):
                        break
                    time.sleep(1.0)

        env = os.environ.copy()
        env["CARLA_E2E_HOST"] = args.host
        env["CARLA_E2E_PORT"] = str(args.port)
        env["CARLA_E2E_OFFLINE_ONLY"] = "1" if args.offline_only else "0"

        results: list[tuple[str, int, float]] = []
        for script in test_scripts:
            suite_name = f"{script.parent.name}/{script.name}"
            print(f"\n{'=' * 72}\n[e2e] Running {suite_name}\n{'=' * 72}")
            t0 = time.monotonic()
            cmd = [sys.executable, str(script), "--host", args.host, "--port", str(args.port)]
            if args.offline_only:
                cmd.append("--offline-only")
            rc = subprocess.call(cmd, cwd=str(REPO_ROOT), env=env)
            elapsed = time.monotonic() - t0
            results.append((suite_name, rc, elapsed))

        print(f"\n{'=' * 72}\nCARLA Map Maker E2E Regression Summary\n{'=' * 72}")
        all_passed = True
        for suite_name, rc, elapsed in results:
            status = "PASS" if rc == 0 else f"FAIL (rc={rc})"
            if rc != 0:
                all_passed = False
            print(f"  {suite_name:<45} {status:<12} ({elapsed:.1f}s)")
        print("=" * 72)
        return 0 if all_passed else 1
    finally:
        if server_proc is not None:
            print("[e2e] Stopping managed CARLA server...")
            server_proc.terminate()
            try:
                server_proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                server_proc.kill()
        if log_handle is not None:
            log_handle.close()


if __name__ == "__main__":
    sys.exit(main())

