#!/usr/bin/env python3
"""Checks how run_carla_autoware.sh picks the server and the wheels to check
against when --carla-root names a release package. Uses --dry-run, so it
starts nothing and needs neither CARLA nor Autoware. Runs with plain python:

    python3 PythonAPI/examples/av_stacks/autoware/run/tests/test_carla_root.py
"""

import os
import subprocess
import tempfile

LAUNCHER = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "run_carla_autoware.sh")


def dry_run(carla_root, log_dir):
    """Output of a classical dry run with --carla-root, $CARLA_ROOT unset."""
    env = {k: v for k, v in os.environ.items() if k != "CARLA_ROOT"}
    proc = subprocess.run(
        ["bash", LAUNCHER, "--dry-run", "--mode", "classical", "--stack", "docker",
         "--carla-root", carla_root, "--log-dir", log_dir],
        capture_output=True, text=True, env=env, timeout=300, check=False)
    return proc.stdout + proc.stderr


def test_package_root_selects_its_server_and_wheels():
    """The package's Linux/CarlaUnreal.sh runs, and its dist holds the wheels."""
    with tempfile.TemporaryDirectory() as pkg:
        launcher = os.path.join(pkg, "Linux", "CarlaUnreal.sh")
        os.makedirs(os.path.dirname(launcher))
        with open(launcher, "w", encoding="ascii") as f:
            f.write("#!/bin/sh\n")
        os.chmod(launcher, 0o755)
        out = dry_run(pkg, os.path.join(pkg, "logs"))
    assert f"CARLA server: packaged {launcher}" in out, out
    # The package has no wheel, so the check names the package's dist folder,
    # not this tree's Build/ folder.
    assert f"no wheel under {pkg}/PythonAPI/carla/dist" in out, out


def test_explicit_root_without_launcher_fails():
    """An explicit --carla-root without a launcher is a preflight failure."""
    with tempfile.TemporaryDirectory() as empty:
        out = dry_run(empty, os.path.join(empty, "logs"))
    assert "holds neither CarlaUnreal.sh nor Linux/CarlaUnreal.sh" in out, out


if __name__ == "__main__":
    test_package_root_selects_its_server_and_wheels()
    test_explicit_root_without_launcher_fails()
    print("ok")
