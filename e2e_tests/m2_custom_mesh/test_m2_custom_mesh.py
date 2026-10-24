#!/usr/bin/env python3
"""
Milestone 2 End-to-End Verification Suite:
Enhanced Runtime Custom Mesh, Material & Semantic Tagging Engine.

Verifies all Definition-of-Done (DoD) items for Milestone 2:
  - DoD 2.1: Extended Custom Mesh RPC (`spawn_custom_mesh_ex` & batch `spawn_custom_meshes`)
             + backward compatibility of `spawn_custom_mesh`.
  - DoD 2.2: Environment Object & Bounding Box Registration (`get_environment_objects`,
             `get_level_bbs`, `enable_environment_objects`).
  - DoD 2.3: Semantic Segmentation & Raycast Ground-Truth Test (`cast_ray` +
             `sensor.camera.semantic_segmentation` for Roads, Sidewalks, Buildings, Vegetation).
  - DoD 2.4: Physics Collision & Vehicle Drivability Test on a custom road mesh.
  - DoD 2.5: Pre-Cooked Material Catalog (30+ presets) & Hybrid OpenDRIVE Mode
             (`generate_road_mesh=False`, `generate_ground_mesh=False`).
"""

from __future__ import annotations

import argparse
import os
from pathlib import Path
import queue
import subprocess
import sys
import time
import unittest

import carla


REPO_ROOT = Path(__file__).resolve().parents[2]
LOCAL_TMP_DIR = REPO_ROOT / ".tmp"
LOCAL_TMP_DIR.mkdir(parents=True, exist_ok=True)
os.environ["TMPDIR"] = str(LOCAL_TMP_DIR)
import tempfile
tempfile.tempdir = str(LOCAL_TMP_DIR)

FIXTURES_DIR = Path(__file__).resolve().parent / "fixtures"
SIMPLE_ROAD_XODR = FIXTURES_DIR / "simple_road.xodr"

sys.path.insert(0, str(REPO_ROOT / "e2e_tests" / "m1_osm2odr"))
from test_m1_osm2odr import find_unreal_editor_binary, is_port_open  # noqa: E402


def make_quad_vertices(cx: float, cy: float, cz: float, half_x: float, half_y: float) -> list[carla.Vector3D]:
    """Creates a horizontal quad centered at (cx, cy, cz) with upward normal in UE left-handed frame."""
    return [
        carla.Vector3D(cx - half_x, cy - half_y, cz),
        carla.Vector3D(cx - half_x, cy + half_y, cz),
        carla.Vector3D(cx + half_x, cy + half_y, cz),
        carla.Vector3D(cx + half_x, cy - half_y, cz),
    ]


def make_box_mesh(
    half_x: float, half_y: float, height: float
) -> tuple[list[carla.Vector3D], list[int]]:
    """Creates a closed 3D box from z=0 to z=height centered at local (0, 0)."""
    hx, hy, h = half_x, half_y, height
    verts = [
        carla.Vector3D(-hx, -hy, 0.0),
        carla.Vector3D(hx, -hy, 0.0),
        carla.Vector3D(hx, hy, 0.0),
        carla.Vector3D(-hx, hy, 0.0),
        carla.Vector3D(-hx, -hy, h),
        carla.Vector3D(hx, -hy, h),
        carla.Vector3D(hx, hy, h),
        carla.Vector3D(-hx, hy, h),
    ]
    tris = [
        # Top (+Z)
        4, 7, 6, 4, 6, 5,
        # Bottom (-Z)
        0, 1, 2, 0, 2, 3,
        # Front (+X)
        1, 5, 6, 1, 6, 2,
        # Back (-X)
        0, 3, 7, 0, 7, 4,
        # Right (+Y)
        3, 2, 6, 3, 6, 7,
        # Left (-Y)
        0, 4, 5, 0, 5, 1,
    ]
    return verts, tris


class Milestone2CustomMeshE2ETest(unittest.TestCase):
    host: str = os.environ.get("CARLA_E2E_HOST", "127.0.0.1")
    port: int = int(os.environ.get("CARLA_E2E_PORT", "2000"))
    offline_only: bool = os.environ.get("CARLA_E2E_OFFLINE_ONLY", "0") == "1"

    def test_dod_2_1_python_api_bindings_and_types(self) -> None:
        """DoD 2.1 (Offline): Verify CustomMeshDescription, CustomMaterialParameters, and OpendriveGenerationParameters bindings."""
        mat = carla.CustomMaterialParameters()
        self.assertEqual((mat.base_color.r, mat.base_color.g, mat.base_color.b), (200, 200, 200))
        mat.base_color = carla.Color(180, 60, 40)
        mat.roughness = 0.65
        mat.metallic = 0.1
        mat.specular = 0.4
        mat.uv_scale = carla.Vector2D(2.0, 3.0)
        self.assertEqual((mat.base_color.r, mat.base_color.g, mat.base_color.b), (180, 60, 40))
        self.assertAlmostEqual(mat.roughness, 0.65, places=4)
        self.assertAlmostEqual(mat.uv_scale.x, 2.0, places=4)

        desc = carla.CustomMeshDescription()
        desc.name = "TestBuilding_01"
        desc.transform = carla.Transform(carla.Location(10.0, 20.0, 0.0), carla.Rotation(0.0, 45.0, 0.0))
        desc.scale = carla.Vector3D(1.0, 1.0, 1.0)
        verts, tris = make_box_mesh(5.0, 5.0, 10.0)
        desc.vertices = verts
        desc.triangles = tris
        desc.semantic_label = carla.CityObjectLabel.Buildings
        desc.enable_collision = True
        desc.material_preset = "building_brick"
        desc.material_params = mat

        self.assertEqual(desc.name, "TestBuilding_01")
        self.assertEqual(len(desc.vertices), 8)
        self.assertEqual(len(desc.triangles), 36)
        self.assertEqual(desc.semantic_label, carla.CityObjectLabel.Buildings)
        self.assertEqual(desc.material_preset, "building_brick")

        # Also verify flat float/tuple vertex & triangle assignment
        desc2 = carla.CustomMeshDescription()
        desc2.vertices = [0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0]
        desc2.triangles = [(0, 1, 2)]
        self.assertEqual(len(desc2.vertices), 3)
        self.assertEqual(list(desc2.triangles), [0, 1, 2])

        # Verify OpendriveGenerationParameters hybrid flags
        od_params = carla.OpendriveGenerationParameters()
        self.assertTrue(od_params.generate_road_mesh)
        self.assertTrue(od_params.generate_ground_mesh)
        self.assertTrue(od_params.generate_street_furniture)
        self.assertTrue(od_params.generate_traffic_signals)
        od_params.generate_road_mesh = False
        od_params.generate_ground_mesh = False
        self.assertFalse(od_params.generate_road_mesh)
        self.assertFalse(od_params.generate_ground_mesh)

    def test_dod_2_1_to_2_5_live_custom_mesh_engine(self) -> None:
        """DoD 2.1 - 2.5 (Live Server): Verify Hybrid OpenDRIVE, Custom Mesh RPC, Env Objects, BBs, Semantic Seg, Raycast, Physics, and Material Presets."""
        if self.offline_only:
            self.skipTest("Skipping live server test (--offline-only set)")

        server_proc: subprocess.Popen[bytes] | None = None
        log_handle = None

        try:
            if not is_port_open(self.host, self.port):
                editor_bin = find_unreal_editor_binary(REPO_ROOT)
                uproject = REPO_ROOT / "Unreal/CarlaUnreal/CarlaUnreal.uproject"
                if editor_bin is None or not uproject.is_file():
                    self.skipTest("CARLA server not running and UnrealEditor binary not found")

                log_path = LOCAL_TMP_DIR / f"carla_m2_e2e_{self.port}.log"
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
                        f"-carla-rpc-port={self.port}",
                    ],
                    stdout=log_handle,
                    stderr=subprocess.STDOUT,
                )
                for _ in range(90):
                    if server_proc.poll() is not None:
                        self.fail(f"UnrealEditor exited early with code {server_proc.returncode}")
                    if is_port_open(self.host, self.port):
                        break
                    time.sleep(1.0)
                else:
                    self.fail(f"Timed out waiting for CARLA server on {self.host}:{self.port}")

            client = carla.Client(self.host, self.port)
            client.set_timeout(30.0)

            # ------------------------------------------------------------------
            # DoD 2.5 (Part A): Hybrid OpenDRIVE Mode (generate_road_mesh=False, generate_ground_mesh=False)
            # ------------------------------------------------------------------
            xodr_str = SIMPLE_ROAD_XODR.read_text(encoding="utf-8")
            od_params = carla.OpendriveGenerationParameters(
                vertex_distance=2.0,
                max_road_length=50.0,
                wall_height=0.0,
                additional_width=0.0,
                smooth_junctions=True,
                enable_mesh_visibility=True,
                enable_pedestrian_navigation=False,
                generate_road_mesh=False,
                generate_ground_mesh=False,
                generate_street_furniture=False,
                generate_traffic_signals=True,
            )
            world = client.generate_opendrive_world(xodr_str, od_params)
            time.sleep(1.0)

            settings = world.get_settings()
            settings.synchronous_mode = True
            settings.fixed_delta_seconds = 0.05
            world.apply_settings(settings)
            world.tick()

            # Verify logical OpenDRIVE waypoints exist even though procedural road/ground meshes were skipped
            carla_map = world.get_map()
            waypoints = carla_map.generate_waypoints(5.0)
            self.assertGreater(len(waypoints), 10, "Hybrid OpenDRIVE mode should still populate logical waypoints")

            # Verify no procedural road mesh is present under (50, 0) before spawning custom meshes
            pre_hits = world.cast_ray(carla.Location(50.0, 0.0, 10.0), carla.Location(50.0, 0.0, -10.0))
            road_pre_hits = [h for h in pre_hits if h.label == carla.CityObjectLabel.Roads]
            self.assertEqual(len(road_pre_hits), 0, "Expected 0 procedural road meshes when generate_road_mesh=False")

            # ------------------------------------------------------------------
            # DoD 2.1: Legacy `spawn_custom_mesh` backward compatibility + `spawn_custom_mesh_ex` + `spawn_custom_meshes`
            # ------------------------------------------------------------------
            legacy_verts = [
                -20.0, -20.0, 0.0,
                -20.0, -10.0, 0.0,
                -10.0, -10.0, 0.0,
                -10.0, -20.0, 0.0,
            ]
            legacy_tris = [0, 1, 2, 0, 2, 3]
            world.spawn_custom_mesh(legacy_verts, legacy_tris, "grass")

            # Spawn 4 distinct semantic custom meshes for DoD 2.2, 2.3, and 2.4:
            # 1. Road mesh along x in [0, 100], y in [-10, 10], z = 0
            road_desc = carla.CustomMeshDescription()
            road_desc.name = "CustomRoad_Main"
            road_desc.vertices = make_quad_vertices(50.0, 0.0, 0.0, 50.0, 10.0)
            road_desc.triangles = [0, 1, 2, 0, 2, 3]
            road_desc.semantic_label = carla.CityObjectLabel.Roads
            road_desc.enable_collision = True
            road_desc.material_preset = "road_asphalt"

            # 2. Sidewalk mesh at x in [10, 30], y in [12, 22], z = 0.2
            sidewalk_desc = carla.CustomMeshDescription()
            sidewalk_desc.name = "CustomSidewalk_North"
            sidewalk_desc.vertices = make_quad_vertices(20.0, 17.0, 0.2, 10.0, 5.0)
            sidewalk_desc.triangles = [0, 1, 2, 0, 2, 3]
            sidewalk_desc.semantic_label = carla.CityObjectLabel.Sidewalks
            sidewalk_desc.enable_collision = True
            sidewalk_desc.material_preset = "sidewalk_tiles"

            # 3. Building 3D box at (20, 32, 0), size 10x10x8m
            bldg_verts, bldg_tris = make_box_mesh(5.0, 5.0, 8.0)
            bldg_desc = carla.CustomMeshDescription()
            bldg_desc.name = "CustomBuilding_HQ"
            bldg_desc.transform = carla.Transform(carla.Location(20.0, 32.0, 0.0))
            bldg_desc.vertices = bldg_verts
            bldg_desc.triangles = bldg_tris
            bldg_desc.semantic_label = carla.CityObjectLabel.Buildings
            bldg_desc.enable_collision = True
            bldg_desc.material_preset = "building_brick"

            # 4. Vegetation patch at x in [10, 30], y in [-22, -12], z = 0.1
            veg_desc = carla.CustomMeshDescription()
            veg_desc.name = "CustomVegetation_Park"
            veg_desc.vertices = make_quad_vertices(20.0, -17.0, 0.1, 10.0, 5.0)
            veg_desc.triangles = [0, 1, 2, 0, 2, 3]
            veg_desc.semantic_label = carla.CityObjectLabel.Vegetation
            veg_desc.enable_collision = True
            veg_desc.material_preset = "grass_yard"

            road_id = world.spawn_custom_mesh_ex(road_desc)
            self.assertGreater(road_id, 0, "spawn_custom_mesh_ex should return a non-zero uint64 ID")

            batch_ids = world.spawn_custom_meshes([sidewalk_desc, bldg_desc, veg_desc])
            self.assertEqual(len(batch_ids), 3)
            sidewalk_id, bldg_id, veg_id = batch_ids
            self.assertEqual(len({road_id, sidewalk_id, bldg_id, veg_id}), 4, "All spawned mesh IDs must be unique")

            world.tick()

            # ------------------------------------------------------------------
            # DoD 2.2: Environment Object & Bounding Box Registration + Toggle
            # ------------------------------------------------------------------
            bldg_env_objs = world.get_environment_objects(carla.CityObjectLabel.Buildings)
            bldg_by_id = {obj.id: obj for obj in bldg_env_objs}
            self.assertIn(bldg_id, bldg_by_id, "CustomBuilding_HQ ID must be in get_environment_objects(Buildings)")
            self.assertEqual(bldg_by_id[bldg_id].name, "CustomBuilding_HQ")
            self.assertGreater(bldg_by_id[bldg_id].bounding_box.extent.z, 3.5)

            bldg_bbs_before = world.get_level_bbs(carla.CityObjectLabel.Buildings)
            self.assertGreaterEqual(len(bldg_bbs_before), 1, "get_level_bbs(Buildings) must include CustomBuilding_HQ")

            # Disable building via enable_environment_objects([bldg_id], False)
            world.enable_environment_objects([bldg_id], False)
            world.tick()

            bldg_bbs_disabled = world.get_level_bbs(carla.CityObjectLabel.Buildings)
            self.assertEqual(
                len(bldg_bbs_disabled),
                len(bldg_bbs_before) - 1,
                "Disabling CustomBuilding_HQ should remove its bounding box from get_level_bbs(Buildings)",
            )
            hits_disabled = world.cast_ray(carla.Location(20.0, 32.0, 20.0), carla.Location(20.0, 32.0, -5.0))
            self.assertEqual(
                [h for h in hits_disabled if h.label == carla.CityObjectLabel.Buildings],
                [],
                "Disabled environment object must not block raycasts",
            )

            # Re-enable building via enable_environment_objects([bldg_id], True)
            world.enable_environment_objects([bldg_id], True)
            world.tick()

            bldg_bbs_reenabled = world.get_level_bbs(carla.CityObjectLabel.Buildings)
            self.assertEqual(len(bldg_bbs_reenabled), len(bldg_bbs_before))

            # ------------------------------------------------------------------
            # DoD 2.3: Semantic Segmentation & Raycast Ground-Truth Test
            # ------------------------------------------------------------------
            expected_ray_targets = [
                (carla.Location(20.0, 0.0, 15.0), carla.Location(20.0, 0.0, -5.0), carla.CityObjectLabel.Roads),
                (carla.Location(20.0, 17.0, 15.0), carla.Location(20.0, 17.0, -5.0), carla.CityObjectLabel.Sidewalks),
                (carla.Location(20.0, 32.0, 15.0), carla.Location(20.0, 32.0, -5.0), carla.CityObjectLabel.Buildings),
                (carla.Location(20.0, -17.0, 15.0), carla.Location(20.0, -17.0, -5.0), carla.CityObjectLabel.Vegetation),
            ]
            for start_loc, end_loc, expected_label in expected_ray_targets:
                hits = world.cast_ray(start_loc, end_loc)
                self.assertGreater(len(hits), 0, f"Raycast missed target for {expected_label}")
                hit_labels = [h.label for h in hits]
                self.assertIn(
                    expected_label,
                    hit_labels,
                    f"Expected raycast label {expected_label}, got {hit_labels}",
                )

            # Also test sensor.camera.semantic_segmentation looking straight down over all 4 meshes
            bp_lib = world.get_blueprint_library()
            sem_bp = bp_lib.find("sensor.camera.semantic_segmentation")
            sem_bp.set_attribute("image_size_x", "256")
            sem_bp.set_attribute("image_size_y", "256")
            sem_bp.set_attribute("fov", "90")
            cam_transform = carla.Transform(
                carla.Location(20.0, 5.0, 35.0),
                carla.Rotation(pitch=-90.0, yaw=0.0, roll=0.0),
            )
            sem_cam = world.spawn_actor(sem_bp, cam_transform)
            img_queue: queue.Queue[carla.Image] = queue.Queue()
            sem_cam.listen(img_queue.put)
            try:
                sem_img = None
                for _ in range(5):
                    world.tick()
                    try:
                        sem_img = img_queue.get(timeout=5.0)
                    except queue.Empty:
                        pass
                while not img_queue.empty():
                    sem_img = img_queue.get_nowait()

                if sem_img is not None:
                    # Raw BGRA image stores the CityObjectLabel tag in the Red channel (every 4th byte + 2)
                    raw = bytes(sem_img.raw_data)
                    red_channel_tags = set(raw[2::4])
                    # If RHI rendering is active (non-nullrhi), verify all 4 semantic tag IDs appear in image
                    if len(red_channel_tags) > 1:
                        for tag_id, label_name in [
                            (int(carla.CityObjectLabel.Roads), "Roads"),
                            (int(carla.CityObjectLabel.Sidewalks), "Sidewalks"),
                            (int(carla.CityObjectLabel.Buildings), "Buildings"),
                            (int(carla.CityObjectLabel.Vegetation), "Vegetation"),
                        ]:
                            self.assertIn(
                                tag_id,
                                red_channel_tags,
                                f"Semantic segmentation camera image missing tag {tag_id} ({label_name}); "
                                f"found tags: {sorted(red_channel_tags)}",
                            )
            finally:
                sem_cam.stop()
                sem_cam.destroy()

            # ------------------------------------------------------------------
            # DoD 2.4: Physics Collision & Vehicle Drivability Test
            # ------------------------------------------------------------------
            vehicles = bp_lib.filter("vehicle.*")
            vehicle_bp = next(
                (bp for bp in vehicles if int(bp.get_attribute("number_of_wheels")) == 4),
                vehicles[0],
            )
            spawn_tf = carla.Transform(
                carla.Location(10.0, -1.75, 1.0),
                carla.Rotation(pitch=0.0, yaw=0.0, roll=0.0),
            )
            vehicle = world.try_spawn_actor(vehicle_bp, spawn_tf)
            self.assertIsNotNone(vehicle, "Failed to spawn vehicle 1m above custom road mesh")
            try:
                # Tick 60 frames (~3.0s) to let vehicle settle on custom road mesh
                for _ in range(60):
                    world.tick()
                settled_loc = vehicle.get_location()
                self.assertGreater(
                    settled_loc.z,
                    -0.5,
                    f"Vehicle fell through custom road mesh (z={settled_loc.z:.2f})",
                )
                self.assertLess(
                    settled_loc.z,
                    1.5,
                    f"Vehicle did not settle onto custom road mesh (z={settled_loc.z:.2f})",
                )

                # Apply throttle=0.5 for 100 ticks (5.0s) and verify forward progress
                start_x = settled_loc.x
                vehicle.apply_control(carla.VehicleControl(throttle=0.5, steer=0.0, brake=0.0))
                for _ in range(100):
                    world.tick()
                driven_loc = vehicle.get_location()
                self.assertGreater(
                    driven_loc.x - start_x,
                    2.0,
                    f"Vehicle failed to drive forward on custom road mesh (dx={driven_loc.x - start_x:.2f}m)",
                )
                self.assertGreater(driven_loc.z, -0.5, "Vehicle fell through custom road mesh while driving")
            finally:
                vehicle.destroy()

            # ------------------------------------------------------------------
            # DoD 2.5 (Part B): Pre-Cooked Material Catalog (20+ presets) & Custom PBR
            # ------------------------------------------------------------------
            presets = [
                "road", "road_asphalt", "road_worn", "road_wet", "road_rural",
                "sidewalk", "sidewalk_concrete", "sidewalk_tiles", "sidewalk_residential",
                "crosswalk", "curb", "gutter", "road_line_white", "road_line_yellow",
                "grass", "grass_yard", "dirt", "gravel", "rock", "water",
                "building_concrete", "building_brick", "building_plaster", "building_stone", "building_glass",
                "roof_tiles", "roof_metal", "wall_concrete", "wall_brick",
                "fence_wood", "fence_metal", "guardrail", "bridge_steel", "custom_pbr",
            ]
            preset_descs = []
            for idx, preset in enumerate(presets):
                d = carla.CustomMeshDescription()
                d.name = f"MatPreset_{preset}"
                d.vertices = make_quad_vertices(-50.0, float(idx * 2), 0.0, 0.5, 0.5)
                d.triangles = [0, 1, 2, 0, 2, 3]
                d.semantic_label = carla.CityObjectLabel.Buildings
                d.enable_collision = False
                d.material_preset = preset
                if preset == "custom_pbr":
                    mp = carla.CustomMaterialParameters()
                    mp.base_color = carla.Color(120, 180, 220)
                    mp.roughness = 0.35
                    mp.metallic = 0.2
                    d.material_params = mp
                preset_descs.append(d)

            preset_ids = world.spawn_custom_meshes(preset_descs)
            self.assertEqual(len(preset_ids), len(presets))
            self.assertTrue(all(pid > 0 for pid in preset_ids))
            world.tick()

            settings.synchronous_mode = False
            world.apply_settings(settings)

        finally:
            if server_proc is not None:
                server_proc.terminate()
                try:
                    server_proc.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    server_proc.kill()
            if log_handle is not None:
                log_handle.close()


def main() -> int:
    parser = argparse.ArgumentParser(description="Milestone 2 Custom Mesh Engine E2E Test Suite")
    parser.add_argument("--host", default=os.environ.get("CARLA_E2E_HOST", "127.0.0.1"))
    parser.add_argument("--port", type=int, default=int(os.environ.get("CARLA_E2E_PORT", "2000")))
    parser.add_argument("--offline-only", action="store_true")
    args, remaining = parser.parse_known_args()

    Milestone2CustomMeshE2ETest.host = args.host
    Milestone2CustomMeshE2ETest.port = args.port
    if args.offline_only:
        Milestone2CustomMeshE2ETest.offline_only = True

    suite = unittest.defaultTestLoader.loadTestsFromTestCase(Milestone2CustomMeshE2ETest)
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)
    return 0 if result.wasSuccessful() else 1


if __name__ == "__main__":
    sys.exit(main())

