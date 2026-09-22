#!/usr/bin/env python3

# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Unit tests for the ROS2 example launch scripts.

These tests deliberately replace Docker and the image build with small local
stubs.  They validate the command-line contract without requiring a CARLA
server, a ROS installation, or a container runtime.
"""

import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile
import types
import unittest
import importlib.util


class TestMapAndLidarDemoLaunchScript(unittest.TestCase):
    def test_ros_domain_id_is_forwarded_to_demo_container(self):
        source_script = (
            Path(__file__).resolve().parents[2]
            / 'examples' / 'ros2' / 'run_map_and_lidar_demo.sh')

        with tempfile.TemporaryDirectory() as temp_dir:
            temp_path = Path(temp_dir)
            script_dir = temp_path / 'ros2'
            demo_dir = script_dir / 'map_and_lidar_demo'
            bin_dir = temp_path / 'bin'
            script_dir.mkdir()
            demo_dir.mkdir()
            bin_dir.mkdir()
            shutil.copy2(source_script, script_dir / source_script.name)

            build_script = demo_dir / 'build.sh'
            build_script.write_text('#!/bin/bash\nexit 0\n')
            build_script.chmod(0o755)

            capture = temp_path / 'docker-arguments.txt'
            docker = bin_dir / 'docker'
            docker.write_text(
                '#!/bin/bash\nprintf "%s\\n" "$@" > "$DOCKER_ARGUMENTS"\n')
            docker.chmod(0o755)

            environment = os.environ.copy()
            environment['PATH'] = '{}:{}'.format(bin_dir, environment['PATH'])
            environment['DOCKER_ARGUMENTS'] = str(capture)
            subprocess.run(
                ['bash', str(script_dir / source_script.name),
                 '--ros-domain-id=42', '--map-only'],
                check=True,
                capture_output=True,
                text=True,
                env=environment)

            self.assertIn('--env=ROS_DOMAIN_ID=42',
                          capture.read_text().splitlines())

    def test_ros2_controller_never_claims_an_existing_sync_clock(self):
        source = (
            Path(__file__).resolve().parents[2]
            / 'examples' / 'ros2' / 'ros2_native.py')
        carla_stub = types.ModuleType('carla')
        previous_carla = sys.modules.get('carla')
        sys.modules['carla'] = carla_stub
        try:
            spec = importlib.util.spec_from_file_location('ros2_native_test', source)
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
        finally:
            if previous_carla is None:
                sys.modules.pop('carla', None)
            else:
                sys.modules['carla'] = previous_carla

        self.assertFalse(module._should_take_sync_ownership(True, False, True))
        self.assertTrue(module._should_take_sync_ownership(True, False, False))
        self.assertTrue(module._should_take_sync_ownership(False, True, True))
        self.assertFalse(module._should_take_sync_ownership(False, False, False))


if __name__ == '__main__':
    unittest.main()
