#!/usr/bin/env python3
"""
End-to-End (E2E) Verification Test Suite for Milestone 10 (M10).

Verifies Photorealistic PBR Materials, Multi-Slot Shading & UE5 Reference Matching:
  - DoD 10.1: Multi-slot building extrusion separates vertical facade walls from horizontal roofs.
  - DoD 10.2: Facade UV coordinates have modular per-story scaling (dv/dz = 1.0 / 3.5m) preventing texture stretching.
  - DoD 10.3: Roof UV coordinates follow planar top-down grid scaling (5.0m tile repeat).
  - DoD 10.4: OSM building:material, roof:material, and building functional tags drive material classification.
  - DoD 10.5: Multi-slot architectural layer builder partitions footprints into distinct architectural layers.
  - DoD 10.6: Bundle compilation packages separated architectural layers with semantic label 'Buildings'.
  - DoD 10.7: CLI inspection and verification confirm valid CMesh binaries, headers, and material presets.
  - DoD 10.8: Mathematical UV audit tool validates non-stretching modular story alignment.
  - DoD 10.9: Hero showcase comparison suite and PBR inspection collage artifacts are generated and valid.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import socket
import sys
import tempfile
import unittest

from PIL import Image

E2E_DIR = Path(__file__).resolve().parent
REPO_ROOT = E2E_DIR.parent.parent
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
tempfile.tempdir = str(LOCAL_TMP_DIR)

sys.path.insert(0, str(REPO_ROOT))
sys.path.insert(0, str(REPO_ROOT / "Util" / "MapBuilder"))

from Util.MapBuilder.carla_map import cmd_build, cmd_inspect, cmd_verify
from Util.MapBuilder.cmesh import read_cmesh_file
from Util.MapBuilder.inspect_pbr_materials import audit_mesh_uv_metrics
from Util.MapBuilder.osm_buildings import (
    BuildingFootprint,
    build_osm_buildings_layer,
    build_osm_buildings_multislot_layers,
    classify_building_material,
    extract_osm_building_footprints,
    extrude_building_mesh,
    extrude_building_mesh_multislot,
    parse_building_colour,
)

MUNICH_OSM = REPO_ROOT / "Docs" / "map-maker" / "output" / "munich_stachus.osm"
if not MUNICH_OSM.is_file():
    MUNICH_OSM = REPO_ROOT / "munich_stachus.osm"

BUNDLE_TAR = REPO_ROOT / "Docs" / "map-maker" / "output" / "MunichStachus.carlamap.tar.gz"
if not BUNDLE_TAR.is_file():
    BUNDLE_TAR = REPO_ROOT / "MunichStachus.carlamap.tar.gz"

IMAGES_DIR = REPO_ROOT / "Docs" / "map-maker" / "images"


def is_port_open(host: str, port: int, timeout: float = 1.0) -> bool:
    """Checks whether the CARLA RPC port is open."""
    try:
        with socket.create_connection((host, port), timeout=timeout):
            return True
    except OSError:
        return False


class TestMilestone10Photorealism(unittest.TestCase):
    """Milestone 10 E2E Test Suite for Photorealistic PBR Materials & Multi-Slot Shading."""

    def test_01_multislot_building_extrusion_separation(self) -> None:
        """DoD 10.1: Verify extrude_building_mesh_multislot outputs separated walls and roofs."""
        poly = [(0.0, 0.0), (20.0, 0.0), (20.0, 15.0), (0.0, 15.0)]
        slots = extrude_building_mesh_multislot(
            poly,
            height=14.0,  # 4 stories (14m / 3.5m = 4.0)
            base_z=1.0,
            uv_tile_u=4.0,
            uv_floor_height=3.5,
        )

        self.assertIn("walls", slots, "Extruded slots must contain 'walls'")
        self.assertIn("roofs", slots, "Extruded slots must contain 'roofs'")

        w_verts, w_tris, w_norms, w_uvs = slots["walls"]
        r_verts, r_tris, r_norms, r_uvs = slots["roofs"]

        self.assertGreater(len(w_verts), 0, "Walls must have vertices")
        self.assertGreater(len(w_tris), 0, "Walls must have triangles")
        self.assertGreater(len(r_verts), 0, "Roofs must have vertices")
        self.assertGreater(len(r_tris), 0, "Roofs must have triangles")

        # Normals verification: roof normals point up (+Z), wall normals are horizontal (Z == 0)
        for _, _, nz in r_norms:
            self.assertAlmostEqual(nz, 1.0, places=4, msg="Roof normals must point upwards (+Z)")
        for _, _, nz in w_norms:
            self.assertAlmostEqual(nz, 0.0, places=4, msg="Wall normals must be horizontal (Z == 0)")

    def test_02_modular_story_uv_mapping_no_stretching(self) -> None:
        """DoD 10.2 & 10.3: Verify modular story UV mapping has 1.0 repeat per 3.5m story."""
        poly = [(0.0, 0.0), (20.0, 0.0), (20.0, 10.0), (0.0, 10.0)]
        height = 10.5  # Exactly 3 stories of 3.5m each
        slots = extrude_building_mesh_multislot(
            poly,
            height=height,
            base_z=0.0,
            uv_tile_u=4.0,
            uv_floor_height=3.5,
        )

        w_verts, w_tris, _, w_uvs = slots["walls"]
        r_verts, _, _, r_uvs = slots["roofs"]

        # Check maximum V coordinate equals height / 3.5 = 3.0
        max_v = max(uv[1] for uv in w_uvs)
        self.assertAlmostEqual(max_v, 3.0, places=4, msg="Modular story UV max V must equal 3.0 for 10.5m height")

        # Check roof planar UV coordinates follow x/5.0 and y/5.0
        for (x, y, _), (u, v) in zip(r_verts, r_uvs):
            self.assertAlmostEqual(u, x / 5.0, places=4, msg="Roof U coordinate must match x / 5.0")
            self.assertAlmostEqual(v, y / 5.0, places=4, msg="Roof V coordinate must match y / 5.0")

        # Check audit_mesh_uv_metrics calculates average dv/dz close to 1.0/3.5 = 0.2857
        dummy_geom = {
            "vertices": w_verts,
            "triangles": w_tris,
            "uvs": w_uvs,
        }
        metrics = audit_mesh_uv_metrics(dummy_geom)
        self.assertAlmostEqual(metrics["avg_dv_dz"], 1.0 / 3.5, places=3, msg="avg_dv_dz must match 1.0 / 3.5")

    def test_03_tag_driven_material_classification(self) -> None:
        """DoD 10.4: Verify tag-driven architectural material and color classification."""
        # 1. Explicit building:material
        facade, roof = classify_building_material({"building:material": "brick", "roof:material": "tiles"})
        self.assertEqual(facade, "building_brick")
        self.assertEqual(roof, "roof_tiles")

        facade, roof = classify_building_material({"building:material": "glass", "roof:material": "metal"})
        self.assertEqual(facade, "building_concrete")
        self.assertEqual(roof, "roof_metal")

        facade, roof = classify_building_material({"building:material": "stone", "roof:material": "bitumen"})
        self.assertEqual(facade, "building_stone")
        self.assertEqual(roof, "roof_bitumen")

        facade, roof = classify_building_material({"building:material": "plaster"})
        self.assertEqual(facade, "building_plaster")

        # 2. Functional building tags
        facade, roof = classify_building_material({"building": "commercial"})
        self.assertEqual(facade, "building_concrete")
        self.assertEqual(roof, "roof_bitumen")

        facade, roof = classify_building_material({"building": "church"})
        self.assertEqual(facade, "building_stone")

        # 3. Colour parsing
        col = parse_building_colour({"building:colour": "cream"})
        self.assertEqual(col, (245, 235, 215))

        col_hex = parse_building_colour({"colour": "#1A2B3C"})
        self.assertEqual(col_hex, (0x1A, 0x2B, 0x3C))

    def test_04_munich_multislot_layers_generation(self) -> None:
        """DoD 10.5: Verify multi-slot architectural layers generation from munich_stachus.osm."""
        self.assertTrue(MUNICH_OSM.is_file(), f"Munich OSM file '{MUNICH_OSM}' must exist")
        footprints = extract_osm_building_footprints(MUNICH_OSM)
        self.assertGreaterEqual(len(footprints), 50, "Expected at least 50 building footprints in Munich OSM")

        def dummy_proj(lat: float, lon: float) -> tuple[float, float, float]:
            return (lon * 1000.0, lat * 1000.0, 0.0)

        layers = build_osm_buildings_multislot_layers(footprints, dummy_proj)

        # Must have separated walls, roofs, and modern glass facades
        self.assertIn("walls", layers, "Must contain 'walls' layer")
        self.assertIn("roofs", layers, "Must contain 'roofs' layer")
        self.assertIn("modern_walls", layers, "Must contain 'modern_walls' layer for commercial/retail buildings")

        self.assertEqual(layers["walls"]["material_preset"], "building_plaster")
        self.assertEqual(layers["roofs"]["material_preset"], "roof_tiles")
        self.assertEqual(layers["modern_walls"]["material_preset"], "building_concrete")

        # All layers must have CityObjectLabel Buildings
        for l_name, l_data in layers.items():
            self.assertEqual(l_data["semantic_label"], "Buildings", f"Layer {l_name} must have label Buildings")
            self.assertGreater(len(l_data["vertices"]), 0, f"Layer {l_name} must have non-empty vertices")
            self.assertGreater(len(l_data["triangles"]), 0, f"Layer {l_name} must have non-empty triangles")

    def test_05_bundle_inspection_and_verification(self) -> None:
        """DoD 10.6 & 10.7: Verify bundle compilation with multi-slot layers passes verification."""
        self.assertTrue(BUNDLE_TAR.is_file(), f"Bundle archive '{BUNDLE_TAR}' must exist")

        with tempfile.TemporaryDirectory(prefix="e2e_m10_bundle_") as tmp_dir:
            import tarfile
            with tarfile.open(BUNDLE_TAR, "r:*") as tar:
                tar.extractall(tmp_dir)

            inner_dirs = [d for d in Path(tmp_dir).iterdir() if d.is_dir() and (d / "carla-map.json").is_file()]
            bundle_dir = inner_dirs[0] if inner_dirs else Path(tmp_dir)

            manifest_file = bundle_dir / "carla-map.json"
            self.assertTrue(manifest_file.is_file(), "carla-map.json must exist in bundle")

            manifest = json.loads(manifest_file.read_text(encoding="utf-8"))
            layer_names = [l["name"] for l in manifest.get("layers", [])]

            # Confirm multi-slot layers are present in manifest
            self.assertIn("OSM_Buildings", layer_names)
            self.assertIn("OSM_Buildings_Roofs", layer_names)
            self.assertIn("OSM_Buildings_Modern", layer_names)

            # Confirm material presets
            layer_map = {l["name"]: l for l in manifest["layers"]}
            self.assertEqual(layer_map["OSM_Buildings_Roofs"]["material_preset"], "roof_tiles")
            self.assertIn(layer_map["OSM_Buildings_Modern"]["material_preset"], ("building_concrete", "/Game/Carla/Static/GenericMaterials/Facade/MI_Facade01.MI_Facade01"))

            # Run carla_map verify
            args = argparse.Namespace(target=str(BUNDLE_TAR))
            res = cmd_verify(args)
            self.assertEqual(res, 0, "carla-map verify must exit with code 0 on multi-slot bundle")

    def test_06_hero_showcase_artifacts_generated(self) -> None:
        """DoD 10.9: Verify all hero showcase and visual inspection artifacts exist and have valid dimensions."""
        required_images = [
            ("munich_hero_comparison.png", 3000, 1000),  # Side-by-side comparison canvas
            ("munich_hero_boulevard.png", 1920, 1080),   # Hero street boulevard
            ("munich_hero_aerial.png", 2560, 1440),      # Panoramic 4K aerial
            ("munich_hero_intersection.png", 1920, 1080),# Signalized intersection
            ("munich_hero_cockpit.png", 1920, 1080),     # Driver cockpit POV
            ("pbr_material_inspection.png", 1280, 960),  # PBR macro material collage
        ]

        for fname, min_w, min_h in required_images:
            img_path = IMAGES_DIR / fname
            self.assertTrue(img_path.is_file(), f"Required hero artifact '{fname}' must exist in {IMAGES_DIR}")
            self.assertGreater(img_path.stat().st_size, 10_000, f"Artifact '{fname}' must be non-empty (>10KB)")

            with Image.open(str(img_path)) as im:
                w, h = im.size
                self.assertGreaterEqual(w, min_w, f"'{fname}' width {w} must be >= {min_w}")
                self.assertGreaterEqual(h, min_h, f"'{fname}' height {h} must be >= {min_h}")


def main() -> int:
    suite = unittest.TestLoader().loadTestsFromTestCase(TestMilestone10Photorealism)
    runner = unittest.TextTestRunner(verbosity=2)
    res = runner.run(suite)
    return 0 if res.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

