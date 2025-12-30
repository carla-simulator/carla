# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import carla

import sys
import unittest

from subprocess import check_output


class TestClient(unittest.TestCase):
    def test_client_version(self):
        c = carla.Client('localhost', 8080)
        v = c.get_client_version()

        out = ""
        branch = check_output(['git', 'rev-parse', '--abbrev-ref',  'HEAD', ])
        if sys.version_info > (3, 0):
            branch = branch.decode('utf8').strip()

        if branch.startswith("ue4/"):
            out = branch.split("/")[1]
        else:
            commit = check_output(['git', 'rev-parse', '--short',  'HEAD', ])
            dirty = check_output(['git', 'diff-index', 'HEAD', ])
            if sys.version_info > (3, 0):
                commit = commit.decode('utf8').strip()
                dirty = dirty.decode('utf8').strip()
            
            out = commit
            if dirty:
                out += "-dirty"

        self.assertEqual(str(v), str(out))
