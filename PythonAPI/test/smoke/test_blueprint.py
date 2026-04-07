# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import re

from . import SmokeTest


class TestBlueprintLibrary(SmokeTest):
    def test_blueprint_ids(self):
        print('TestBlueprintLibrary.test_blueprint_ids')
        library = self.client.get_world().get_blueprint_library()
        self.assertTrue(list(library))
        self.assertTrue(list(library.filter('sensor.*')))
        self.assertTrue(list(library.filter('static.*')))
        self.assertTrue(list(library.filter('vehicle.*')))
        self.assertTrue(list(library.filter('walker.*')))
        rgx = re.compile(r'\S+\.\S+\.\S+')
        for bp in library:
            self.assertTrue(rgx.match(bp.id))
        rgx = re.compile(r'(vehicle)\.\S+\.\S+')
        for bp in library.filter('vehicle.*'):
            self.assertTrue(rgx.match(bp.id))
