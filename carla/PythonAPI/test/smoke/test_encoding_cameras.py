# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Pixel-content regression test for the encoding cameras.

Validates that the depth, semantic-segmentation and instance-segmentation
cameras emit data with the expected encoding shape, not just non-empty
buffers. Designed to run in isolation without the rest of the smoke
suite, and overrides ``tearDown`` to skip the inherited
``client.load_world("Town03")`` call (Town03 is not present in the
packaged build; only Town10HD_Opt, Mine_01 and Town15 ship).

Run standalone:

    docker exec -d carla-development-ue5-22.04 \\
        ./Build/Package/Carla-0.10.0-Linux-Shipping/Linux/CarlaUnreal.sh \\
        -RenderOffScreen -carla-rpc-port=3654
    # wait for the listener
    conda activate carla5
    cd PythonAPI/test
    python -m nose2 -v smoke.test_encoding_cameras
"""

from queue import Queue, Empty

import carla
import numpy as np

from . import SyncSmokeTest


def _frame_to_bgra(image):
    """View the raw image bytes as a (H, W, 4) BGRA uint8 array."""
    return np.frombuffer(image.raw_data, dtype=np.uint8).reshape(
        (image.height, image.width, 4))


class TestEncodingCameras(SyncSmokeTest):

    def tearDown(self):
        # Override SyncSmokeTest.tearDown to skip the chained
        # SmokeTest.tearDown call into client.load_world("Town03"). That
        # map is not in the packaged build and the load throws, masking
        # any earlier assertion failure.
        if getattr(self, "settings", None) is not None:
            self.world.apply_settings(self.settings)
            self.world.tick()
            self.settings = None
        self.world = None
        self.client = None

    # -- helpers --------------------------------------------------------

    def _spawn_camera(self, sensor_id, width=320, height=240, fov=90):
        bp = self.world.get_blueprint_library().find(sensor_id)
        bp.set_attribute("image_size_x", str(width))
        bp.set_attribute("image_size_y", str(height))
        bp.set_attribute("fov", str(fov))
        # Use the first map spawn point lifted by 1.5 m. In Town10HD_Opt
        # this points the camera along a city street with road, building
        # facades and sky in view, which gives the encoding sensors a
        # rich multi-class scene.
        spawn_points = self.world.get_map().get_spawn_points()
        self.assertTrue(spawn_points, "no spawn points in current world")
        anchor = spawn_points[0]
        transform = carla.Transform(
            anchor.location + carla.Location(z=1.5),
            anchor.rotation)
        return self.world.spawn_actor(bp, transform)

    def _capture_frames(self, sensor_id, num_frames=6, warmup=4):
        camera = self._spawn_camera(sensor_id)
        try:
            queue = Queue()
            camera.listen(lambda image: queue.put(image))
            # Warm-up ticks: texture streaming, Lumen surface cache and
            # the encoding post-process pipeline all need a few frames
            # before the output is representative. Drain whatever the
            # listener emits during warm-up so the measured frames come
            # from a settled scene.
            for _ in range(warmup):
                self.world.tick()
                try:
                    queue.get(timeout=2.0)
                except Empty:
                    pass
            frames = []
            for _ in range(num_frames):
                self.world.tick()
                frames.append(queue.get(timeout=2.0))
            return frames
        finally:
            camera.stop()
            camera.destroy()

    def _frame_to_frame_diff_mean(self, frames):
        """Mean absolute pixel diff between consecutive frames over BGR."""
        arrays = [_frame_to_bgra(f)[:, :, :3].astype(np.int16) for f in frames]
        diffs = [np.abs(arrays[i] - arrays[i - 1]).mean()
                 for i in range(1, len(arrays))]
        return float(np.mean(diffs))

    # -- tests ----------------------------------------------------------

    def test_depth_camera_emits_stable_encoded_pixels(self):
        frames = self._capture_frames("sensor.camera.depth")
        self.assertGreaterEqual(len(frames), 5)
        union = np.concatenate([
            _frame_to_bgra(f).reshape(-1, 4) for f in frames])
        # CARLA packs depth (in metres, [0, 1000]) across the BGR
        # channels: depth_m = (R + G*256 + B*65536) / (256**3 - 1) * 1000.
        # Decode and assert on the metres-domain distribution rather
        # than on raw channel statistics, which depend on the camera
        # placement (close-up scenes keep R near zero, distant city
        # views spread across all 24 bits).
        b = union[:, 0].astype(np.uint32)
        g = union[:, 1].astype(np.uint32)
        r = union[:, 2].astype(np.uint32)
        normalised = (r + g * 256 + b * 65536) / float((256 ** 3) - 1)
        depth_m = normalised * 1000.0
        depth_min = float(depth_m.min())
        depth_med = float(np.median(depth_m))
        depth_max = float(depth_m.max())
        # Spawn points in Town10HD_Opt sit on a road with the camera
        # raised 1.5 m. The closest pixels (vehicle / road just below
        # the camera) are within tens of metres, the median is on
        # street-scale, and sky pixels saturate near the 1000 m far clip.
        self.assertLess(depth_min, 25.0,
            f"depth min {depth_min:.1f} m is too far; near-field road / "
            f"vehicle should be within 25 m of the camera")
        self.assertGreater(depth_med, 5.0,
            f"depth median {depth_med:.1f} m is implausibly close; "
            f"working depth on a street view should be tens of metres")
        self.assertGreater(depth_max, 500.0,
            f"depth max {depth_max:.1f} m never reaches far range; "
            f"sky pixels should saturate near 1000 m")
        # In sync mode against a static scene consecutive frames are
        # essentially bit-identical. A broken capture pipeline drifts
        # significantly per frame.
        diff = self._frame_to_frame_diff_mean(frames)
        self.assertLess(diff, 5.0,
            f"depth output drifts {diff:.2f}/channel between sync-mode "
            f"frames; expected < 5.0 on a static scene")

    def test_semantic_segmentation_emits_valid_labels(self):
        frames = self._capture_frames(
            "sensor.camera.semantic_segmentation")
        self.assertGreaterEqual(len(frames), 5)
        union = np.concatenate([
            _frame_to_bgra(f).reshape(-1, 4) for f in frames])
        r_channel = union[:, 2]
        # The CARLA UE5 GTMaterial encodes the CityObjectLabel id across
        # the BGRA output. Valid ids span 0..29 with 255 reserved for
        # `Any`. The pipeline's tone-curve / sRGB-write step pushes a
        # tail of pixels to intermediate values, so we don't assert on
        # exact range; instead we require that the *dominant* labels
        # are valid (a broken frame has no dominant concentration at
        # all). A working Town10HD_Opt spawn view shows at least
        # Buildings, Walls, Roads, Sidewalks, etc.
        labels, counts = np.unique(r_channel, return_counts=True)
        top5 = labels[np.argsort(counts)[-5:]].tolist()
        for label in top5:
            self.assertTrue(int(label) <= 29 or int(label) == 255,
                f"semseg top-5 most-common label {label} is outside "
                f"CityObjectLabel range (top-5 = {top5})")
        # The top labels combined must cover the bulk of the frame --
        # a broken frame would have a flat histogram with no top-K
        # dominance.
        top5_coverage = float(np.sort(counts)[-5:].sum() / r_channel.size)
        self.assertGreater(top5_coverage, 0.50,
            f"semseg top-5 labels cover only {top5_coverage:.1%} of the "
            f"frame; broken output would have no dominant top-K. "
            f"Top-5 labels = {top5}")
        # And we want at least 3 distinct dominant labels (>=1% of the
        # frame each) so we know multiple scene classes are encoded,
        # not a single washed-out colour.
        dominant = labels[counts > r_channel.size * 0.01]
        self.assertGreaterEqual(len(dominant), 3,
            f"semseg has only {len(dominant)} dominant labels (>=1% "
            f"of pixels); Town10HD_Opt spawn-point view should yield "
            f"3+ (saw labels with >=1% coverage = {dominant.tolist()})")
        diff = self._frame_to_frame_diff_mean(frames)
        self.assertLess(diff, 5.0,
            f"semantic segmentation drifts {diff:.2f}/channel between "
            f"sync-mode frames; expected < 5.0 on a static scene")

    def test_instance_segmentation_emits_unique_actor_ids(self):
        frames = self._capture_frames(
            "sensor.camera.instance_segmentation")
        self.assertGreaterEqual(len(frames), 5)
        flat = _frame_to_bgra(frames[-1]).reshape(-1, 4)[:, :3].copy()
        unique_ids = np.unique(flat, axis=0)
        # Multiple distinct (B, G, R) tuples means multiple actor ids
        # are visible. Combined with the frame-to-frame stability check
        # below, this rules out a noise frame (which has many unique
        # tuples but no spatial structure).
        self.assertGreater(len(unique_ids), 2,
            f"instance segmentation has only {len(unique_ids)} unique "
            f"pixel ids; expected >= 3")
        diff = self._frame_to_frame_diff_mean(frames)
        self.assertLess(diff, 5.0,
            f"instance segmentation drifts {diff:.2f}/channel between "
            f"sync-mode frames; expected < 5.0 on a static scene")
