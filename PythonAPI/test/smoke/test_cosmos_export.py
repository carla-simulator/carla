# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import json
import os
import tempfile
import unittest

import carla

from . import TESTING_ADDRESS

# The session id is parsed by every exporter as "uuid_start_end"; the two
# trailing underscore-separated parts are timestamps echoed into the JSON.
SESSION_ID = "smokeuuid_1700000000000000_1700000001000000"

# Each RPC writes one or more JSON files under <output_path><subdir>/<sid><suffix>.
# The RPC itself returns the literal string "Success"; the exporter writes the
# file server-side, so (server on localhost) we reconstruct and read the paths.
# (subdir, suffix) tuples per method:
COSMOS_EXPORTS = {
    "export_cosmos_crosswalks": [("3d_crosswalks/", ".crosswalks.json")],
    "export_cosmos_road_boundaries": [("3d_road_boundaries/", ".road_boundaries.json")],
    "export_cosmos_lane_lines": [("3d_lanelines/", ".lanelines.json")],
    "export_cosmos_traffic_signs": [
        ("3d_poles/", ".poles.json"),
        ("3d_traffic_signs/", ".traffic_signs.json"),
        ("3d_traffic_lights/", ".traffic_lights.json"),
    ],
    "export_cosmos_wait_lines": [("3d_wait_lines/", ".wait_lines.json")],
    "export_cosmos_road_markings": [("3d_road_markings/", ".road_markings.json")],
}

# road_markings is sourced from OpenDRIVE stencil <object>s (PR1 Stencil
# subsystem). Stock shipped towns may carry none, so an empty labels array is a
# valid pass for this exporter only.
ALLOW_EMPTY = {"export_cosmos_road_markings"}


class TestCosmosExport(unittest.TestCase):
    """Runtime smoke test for the six world.export_cosmos_* RPCs.

    Drives every exporter against a real town and asserts each writes
    schema-correct JSON. This is a localhost-only test: the exporter writes the
    JSON server-side, so we reconstruct the on-disk path and read it back. It
    self-skips when no CARLA server is reachable.
    """

    def setUp(self):
        self.client = carla.Client(*TESTING_ADDRESS)
        self.client.set_timeout(120.0)
        try:
            self.world = self.client.get_world()
        except RuntimeError as error:
            self.skipTest("no CARLA server at %s:%d (%s)" % (
                TESTING_ADDRESS[0], TESTING_ADDRESS[1], error))
        # The cosmos exporters read the road network / scene actors, so make
        # sure we run on a town that actually carries crosswalks, lanes, signs.
        if "Town10HD" not in self.world.get_map().name:
            self.world = self.client.load_world("Town10HD_Opt")

    def tearDown(self):
        self.world = None
        self.client = None

    def _check_json(self, path, allow_empty):
        self.assertTrue(os.path.isfile(path), "missing output file: %s" % path)
        # The exporters write with UE's ForceUTF8 option, which prepends a UTF-8
        # BOM (this matches the ue4-dev byte-for-byte output). Read with
        # utf-8-sig so the BOM is stripped before parsing.
        with open(path, encoding="utf-8-sig") as handle:
            doc = json.load(handle)
        self.assertIn("labels", doc, "top-level 'labels' missing in %s" % path)
        self.assertIsInstance(doc["labels"], list,
                              "'labels' is not an array in %s" % path)
        if not allow_empty:
            self.assertGreater(len(doc["labels"]), 0,
                               "expected non-empty 'labels' in %s" % path)
        return len(doc["labels"])

    def test_export_cosmos(self):
        print("TestCosmosExport.test_export_cosmos")
        tmp_dir = tempfile.mkdtemp(prefix="cosmos_smoke_")
        # The exporter concatenates output_path with the subdir, so it must end
        # with a path separator.
        output_path = tmp_dir + os.sep

        for method, files in COSMOS_EXPORTS.items():
            exporter = getattr(self.world, method)
            # The RPC returns "Success"; failures surface as a RuntimeError.
            exporter(SESSION_ID, output_path)
            allow_empty = method in ALLOW_EMPTY
            for subdir, suffix in files:
                path = output_path + subdir + SESSION_ID + suffix
                count = self._check_json(path, allow_empty)
                print("    %s%s -> %d labels" % (subdir, suffix, count))
