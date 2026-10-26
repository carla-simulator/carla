# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Offline import metadata regressions; no FBX parser or editor is invoked."""

import importlib.util
import json
import os
from pathlib import Path
import sys
import tempfile
import types
import unittest
from unittest import mock


IMPORT_PATH = Path(__file__).resolve().parents[3] / "Util" / "Tools" / "Import.py"
SPEC = importlib.util.spec_from_file_location("carla_asset_import", IMPORT_PATH)
IMPORTER = importlib.util.module_from_spec(SPEC)
# These metadata functions use only the standard library. Import.py also imports
# the native client for other operations; leave that unused boundary inert.
with mock.patch.dict(sys.modules, {"carla": types.ModuleType("carla")}):
    SPEC.loader.exec_module(IMPORTER)


class TestImportMapTiles(unittest.TestCase):
    def _generate(self, filenames):
        with tempfile.TemporaryDirectory(prefix="carla-map-tiles-") as directory:
            folder = Path(directory)
            for name in filenames:
                path = folder / name
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_bytes(b"")

            packages = IMPORTER.generate_json_package(directory, "TestPackage", True)
            IMPORTER.generate_decals_file(directory)
            descriptor_path = folder / "TestPackage.json"
            decals_path = folder / "roadpainter_decals.json"
            descriptor = json.loads(descriptor_path.read_text()) if descriptor_path.exists() else {}
            decals = json.loads(decals_path.read_text()) if decals_path.exists() else {}
            return packages, descriptor.get("maps", []), decals.get("decals", [])

    def test_multiple_maps_keep_their_own_tiles(self):
        _, maps, decals = self._generate([
            "North.xodr", "South.xodr",
            "North_Tile_0_0.fbx", "North_Tile_1_0.fbx",
            "South_Tile_0_0.fbx", "South_Tile_1_0.fbx",
        ])
        self.assertEqual({item["name"] for item in maps}, {"North", "South"})
        for item in maps:
            self.assertEqual(set(item["tiles"]), {
                "./" + item["name"] + "_Tile_0_0.fbx",
                "./" + item["name"] + "_Tile_1_0.fbx",
            })
        self.assertEqual({item["map_name"] for item in decals}, {"North", "South"})

    def test_common_name_prefix_does_not_share_tiles(self):
        _, maps, _ = self._generate([
            "Town.xodr", "TownExtra.xodr",
            "Town_Tile_0_0.fbx", "Town_Tile_1_0.fbx",
            "TownExtra_Tile_0_0.fbx", "TownExtra_Tile_1_0.fbx",
        ])
        by_name = {item["name"]: set(item["tiles"]) for item in maps}
        self.assertEqual(by_name["Town"], {"./Town_Tile_0_0.fbx", "./Town_Tile_1_0.fbx"})
        self.assertEqual(by_name["TownExtra"], {
            "./TownExtra_Tile_0_0.fbx", "./TownExtra_Tile_1_0.fbx"})

    def test_map_name_can_contain_the_tile_marker(self):
        _, maps, _ = self._generate([
            "Town.xodr", "Town_Tile_North.xodr",
            "Town_Tile_0_0.fbx", "Town_Tile_1_0.fbx",
            "Town_Tile_North_Tile_0_0.fbx", "Town_Tile_North_Tile_1_0.fbx",
        ])
        by_name = {item["name"]: set(item["tiles"]) for item in maps}
        self.assertEqual(by_name["Town"], {"./Town_Tile_0_0.fbx", "./Town_Tile_1_0.fbx"})
        self.assertEqual(by_name["Town_Tile_North"], {
            "./Town_Tile_North_Tile_0_0.fbx", "./Town_Tile_North_Tile_1_0.fbx"})

    def test_map_name_is_literal(self):
        _, maps, _ = self._generate([
            "Town[1].xodr", "Town[1]_Tile_0_0.fbx", "Town[1]_Tile_1_0.fbx",
            "Town1_Tile_0_0.fbx",
        ])
        self.assertEqual(set(maps[0]["tiles"]), {
            "./Town[1]_Tile_0_0.fbx", "./Town[1]_Tile_1_0.fbx"})

    def test_unmatched_xodr_does_not_borrow_another_maps_tiles(self):
        _, maps, decals = self._generate([
            "North.xodr", "Missing.xodr", "North_Tile_0_0.fbx", "North_Tile_1_0.fbx",
        ])
        self.assertEqual([item["name"] for item in maps], ["North"])
        self.assertEqual([item["map_name"] for item in decals], ["North"])

    def test_single_map_ignores_unrelated_files(self):
        _, maps, _ = self._generate([
            "North.xodr", "North_Tile_0_0.fbx", "North_Tile_1_0.fbx",
            "Other_Tile_0_0.fbx", "North.fbx.bak", "North_Tile_0_0.obj",
            "North_Tile_0_0.fbx.bak", "notes.txt",
        ])
        self.assertEqual(set(maps[0]["tiles"]), {
            "./North_Tile_0_0.fbx", "./North_Tile_1_0.fbx"})
        self.assertEqual(maps[0]["tile_size"], 2000)
        self.assertEqual(maps[0]["xodr"], "./North.xodr")
        self.assertTrue(maps[0]["use_carla_materials"])

    def test_valid_single_tiled_map_is_unchanged(self):
        _, maps, decals = self._generate([
            "North.xodr", "North_Tile_0_0.fbx", "North_Tile_1_0.fbx",
        ])
        self.assertEqual(set(maps[0]["tiles"]), {
            "./North_Tile_0_0.fbx", "./North_Tile_1_0.fbx"})
        self.assertEqual([item["map_name"] for item in decals], ["North"])

    def test_single_fbx_still_takes_precedence(self):
        _, maps, decals = self._generate([
            "North.xodr", "North.fbx", "North_Tile_0_0.fbx", "Other_Tile_0_0.fbx",
        ])
        self.assertEqual(maps[0]["source"], "./North.fbx")
        self.assertNotIn("tiles", maps[0])
        self.assertEqual([item["map_name"] for item in decals], ["North"])

    def test_only_unrelated_tiles_produce_no_package(self):
        packages, maps, decals = self._generate([
            "Missing.xodr", "Other_Tile_0_0.fbx", "Other_Tile_1_0.fbx",
        ])
        self.assertEqual(packages, [])
        self.assertEqual(maps, [])
        self.assertEqual(decals, [])

    def test_case_matching_keeps_platform_convention(self):
        _, maps, _ = self._generate([
            "North.xodr", "north_tile_0_0.FBX", "north_tile_1_0.FBX",
        ])
        if os.name == "nt":
            self.assertEqual(set(maps[0]["tiles"]), {
                "./north_tile_0_0.FBX", "./north_tile_1_0.FBX"})
        else:
            self.assertEqual(maps, [])


if __name__ == "__main__":
    unittest.main()
