#!/usr/bin/env python3
# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Execute the production override generator and its generated Bash script."""

import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest
import xml.etree.ElementTree as ET


@unittest.skipIf(os.name == "nt" or not shutil.which("bash"), "Requires Linux Bash and GNU tools")
class TestAutowareOverrides(unittest.TestCase):
    """The fixture owns every configuration file changed by the generated script."""

    @classmethod
    def setUpClass(cls):
        launcher = (Path(__file__).resolve().parents[2] / "examples" / "av_stacks" /
                    "autoware" / "run" / "run_carla_autoware.sh")
        source = launcher.read_text(encoding="utf-8")
        match = re.search(r"(?ms)^write_override_script\(\) \{\n.*?^\}", source)
        if match is None:
            raise AssertionError("Production override generator was not found")
        cls.generator = match.group(0)

    def setUp(self):
        # addCleanup retains ownership through each test, including failures.
        temporary = tempfile.TemporaryDirectory(  # pylint: disable=consider-using-with
            prefix="carla autoware ")
        self.addCleanup(temporary.cleanup)
        self.directory = Path(temporary.name)
        self.root = self.directory / "workspace root"
        self.root.mkdir()
        self.script = self.directory / "apply overrides.sh"

    def write(self, relative, content):
        """Create a configuration file under the owned temporary root."""
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")
        return path

    def run_overrides(self, dry=False):
        """Run the production generator and its generated script through Bash."""
        source = ("set -euo pipefail\n" + self.generator +
                  '\nOVERRIDE_SCRIPT="$1"\nDRY_RUN=' + str(dry).lower() +
                  "\nwrite_override_script\n")
        result = subprocess.run(["bash", "--noprofile", "--norc", "-s", "--", str(self.script)],
                                input=source, capture_output=True, text=True, check=False, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        if dry:
            return result
        syntax = subprocess.run(["bash", "-n", str(self.script)], capture_output=True,
                                text=True, check=False, timeout=10)
        self.assertEqual(syntax.returncode, 0, syntax.stderr)
        result = subprocess.run(["bash", str(self.script)], capture_output=True, text=True,
                                env=dict(os.environ, OVERRIDE_ROOT=str(self.root)),
                                check=False, timeout=10)
        self.assertEqual(result.returncode, 0, result.stderr)
        return result

    def test_all_ndt_copies_are_patched(self):
        """Update both the package template and localization configuration."""
        original = ("/**:\n  ros__parameters:\n"
                    "    converged_param_nearest_voxel_transformation_likelihood: 2.3\n"
                    "    another_threshold: 2.3\n")
        paths = [self.write(package + "/config/ndt_scan_matcher.param.yaml", original)
                 for package in ("autoware_ndt_scan_matcher", "autoware_localization_config")]
        unrelated = self.write("other.param.yaml", original)
        self.run_overrides()
        for path in paths:
            self.assertEqual(path.read_text(), original.replace("likelihood: 2.3", "likelihood: 1.0"))
        self.assertEqual(unrelated.read_text(), original)

    def test_current_localization_launch_is_patched(self):
        """Update the default and forwarded vehicle stop check argument."""
        path = self.write("autoware_localization_launch/launch/util/util.launch.xml",
                          '<launch><arg name="stop_check_enabled" default="true"/>'
                          '<include><arg name="stop_check_enabled" value="$(var stop_check_enabled)"/>'
                          '</include><arg name="unrelated" default="true"/></launch>')
        self.run_overrides()
        root = ET.parse(path).getroot()
        self.assertEqual(root.find("arg").get("default"), "false")
        self.assertEqual(root.find("include/arg").get("value"), "false")
        self.assertEqual(root.findall("arg")[1].get("default"), "true")

    def test_all_legacy_launch_copies_and_symlinks_are_patched(self):
        """Update all legacy copies while retaining source workspace symlinks."""
        original = '<launch><arg name="stop_check_enabled" default="true"/></launch>'
        paths = [self.write(package + "/pose_twist_estimator.launch.xml", original)
                 for package in ("tier4_localization_launch", "overlay/tier4_localization_launch")]
        target = self.write("shared/source.xml", original)
        link = self.root / "linked/tier4_localization_launch/pose_twist_estimator.launch.xml"
        link.parent.mkdir(parents=True)
        link.symlink_to(target)
        unrelated = self.write("other_package/util.launch.xml", original)
        self.run_overrides()
        for path in paths + [target]:
            self.assertEqual(ET.parse(path).getroot().find("arg").get("default"), "false")
        self.assertTrue(link.is_symlink())
        self.assertEqual(unrelated.read_text(), original)

    def test_ndt_symlinks_and_newline_paths_are_patched(self):
        """Find complete paths and follow the installed configuration symlink."""
        original = "converged_param_nearest_voxel_transformation_likelihood: 2.3\n"
        target = self.write("shared/source.yaml", original)
        link = self.root / "symlink/ndt_scan_matcher.param.yaml"
        link.parent.mkdir()
        link.symlink_to(target)
        newline = self.write("overlay\nworkspace/ndt_scan_matcher.param.yaml", original)
        self.run_overrides()
        self.assertEqual(target.read_text(), original.replace("2.3", "1.0"))
        self.assertEqual(newline.read_text(), original.replace("2.3", "1.0"))
        self.assertTrue(link.is_symlink())

    def test_combined_overrides_are_idempotent_and_preserve_diagnostics(self):
        """Retain diagnostic behavior and stable output on repeated runs."""
        ndt = self.write("config/ndt_scan_matcher.param.yaml",
                         "converged_param_nearest_voxel_transformation_likelihood: 2.3\n")
        launch = self.write("autoware_localization_launch/util.launch.xml",
                            '<launch><arg name="stop_check_enabled" default="true"/></launch>')
        diagnostics = self.write("autoware_launch/diagnostics/autoware-carla.yaml",
                                 "- path: /autoware/localization/state\n  timeout: 3.0\n"
                                 "- path: /adapi/mrm_request/delegate\n  timeout: 3.0\n"
                                 "- path: /sensor/ordinary\n  timeout: 3.0\n")
        self.run_overrides()
        paths = [ndt, launch, diagnostics]
        expected = {path: path.read_bytes() for path in paths}
        self.assertIn("likelihood: 1.0", ndt.read_text())
        self.assertEqual(ET.parse(launch).getroot().find("arg").get("default"), "false")
        self.assertEqual(diagnostics.read_text(),
                         "- path: /autoware/localization/state\n  timeout: 30.0\n"
                         "- path: /adapi/mrm_request/delegate\n  timeout: 30.0\n"
                         "- path: /sensor/ordinary\n  timeout: 3.0\n")
        self.run_overrides()
        self.assertEqual({path: path.read_bytes() for path in paths}, expected)

    def test_missing_configuration_warns_and_dry_run_writes_nothing(self):
        """Retain missing-file diagnostics and the generator's dry-run behavior."""
        dry = self.run_overrides(dry=True)
        self.assertIn("[dry-run]", dry.stdout)
        self.assertFalse(self.script.exists())
        result = self.run_overrides()
        self.assertIn("WARNING: ndt_scan_matcher.param.yaml not found", result.stderr)
        self.assertEqual(result.stderr.count("WARNING:"), 3)
        self.assertNotIn("patched:", result.stdout)


if __name__ == "__main__":
    unittest.main()
