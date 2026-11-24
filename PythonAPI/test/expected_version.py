# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""The version string carla::version() should report for this checkout.

Mirrors CMake/CarlaVersion.cmake:

  CARLA_RELEASE_VERSION set      -> that version, e.g. "1.0" (release builds)
  HEAD is exactly at a git tag    -> the tag, e.g. "1.0"
  otherwise (nightly, local work) -> <CARLA_VERSION>-<short hash>[-dirty]

The build passes CARLA_RELEASE_VERSION to CMake, so a test of a release build
reads it from the environment variable of the same name.
"""

import os
import re
import subprocess

REPO_ROOT = os.path.abspath(
    os.path.join(os.path.dirname(__file__), "..", ".."))


def _git(*args):
    try:
        out = subprocess.check_output(
            ("git",) + args, cwd=REPO_ROOT, stderr=subprocess.DEVNULL)
    except (OSError, subprocess.CalledProcessError):
        return None
    return out.decode("utf8").strip() or None


def base_version():
    """CARLA_VERSION (<major>.<minor>) from the top-level CMakeLists.txt."""
    parts = {}
    with open(os.path.join(REPO_ROOT, "CMakeLists.txt")) as f:
        for line in f:
            m = re.match(
                r"\s*set\s*\(\s*CARLA_VERSION_(MAJOR|MINOR)\s+(\d+)\s*\)", line)
            if m:
                parts[m.group(1)] = m.group(2)
    return "{}.{}".format(parts["MAJOR"], parts["MINOR"])


def expected_version_pattern():
    """A regex the reported version must match in full, or None outside git.

    "-dirty" is optional: whether the tree had local changes when it was built
    is not something the test can know afterwards.

    In a checkout where git cannot read HEAD (not installed, or it refuses the
    directory as not owned by the user) this raises rather than returning
    None: that is when the build drops the commit from the version, so the
    check must fail there, not be skipped.
    """
    release = os.environ.get("CARLA_RELEASE_VERSION", "").strip()
    if release:
        return re.escape(release)
    if not os.path.exists(os.path.join(REPO_ROOT, ".git")):
        return None
    tag = _git("describe", "--tags", "--exact-match", "HEAD")
    if tag:
        return re.escape(tag)
    commit = _git("rev-parse", "--short=9", "HEAD")
    if commit is None:
        raise RuntimeError(
            "git cannot read HEAD of {}, so the built commit is unknown".format(REPO_ROOT))
    return re.escape("{}-{}".format(base_version(), commit)) + "(-dirty)?"
