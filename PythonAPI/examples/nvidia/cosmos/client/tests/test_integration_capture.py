"""End-to-end capture against a live CARLA server.

Run with ``CARLA_COSMOS_TEST_PORT=<port> pytest -m integration``.  Keeps the
load small (one hero, 8 frames, single camera) because the GPU is shared.
"""

import numpy as np
import pyarrow.parquet as pq
import pytest

pytestmark = pytest.mark.integration


@pytest.fixture(scope="module")
def hero(carla_client):
    world = carla_client.get_world()
    bps = world.get_blueprint_library()
    candidates = list(bps.filter("vehicle.lincoln.mkz*"))
    if not candidates:
        candidates = [b for b in bps.filter("vehicle.*")
                      if b.has_attribute("base_type") and b.get_attribute("base_type").as_str() == "car"]
    bp = candidates[0]
    bp.set_attribute("role_name", "hero")
    spawn = world.get_map().get_spawn_points()[0]
    actor = world.try_spawn_actor(bp, spawn)
    assert actor is not None, "could not spawn the hero"
    # Let the world observer publish snapshots containing the new actor: the actor
    # transform is identity until the first one, and the skeleton the server reports
    # lags it by a tick (see rear_axle_local_ue), so one wait is a race under load.
    for _ in range(5):
        world.wait_for_tick()
    yield actor
    actor.destroy()


def test_rear_axle_from_bones(carla_client, hero):
    from carla_cosmos.rig import rear_axle_local_ue

    axle = rear_axle_local_ue(hero)
    assert axle[0] < 0, "rear axle must be behind the actor origin"
    assert abs(axle[1]) < 0.2, "rear axle must be laterally centred"
    assert abs(axle[2]) < 0.3, "actor origin is at ground level on ue58-dev"


def test_capture_small_clip(carla_client, hero, tmp_path):
    from carla_cosmos import COSMOS3_NANO, Capture, Clip, Rig

    world = carla_client.get_world()
    cap = Capture(world, hero, Rig.single(), COSMOS3_NANO, frames=8, fps=30, edge=True)
    clip = cap.run(tmp_path, "itest_clip")
    assert clip.validate() == []

    loaded = Clip.load(clip.path)
    assert loaded.manifest.frames == 8
    assert loaded.manifest.rig.cameras[0].name == "camera:front:wide:120fov"

    ego = pq.read_table(clip.scene_dir / "itest_clip.egomotion_estimate.parquet")
    assert ego.num_rows == 8
    first = ego.to_pylist()[0]["egomotion_estimate"]["location"]
    assert np.allclose([first["x"], first["y"], first["z"]], 0, atol=0.05), \
        "first ego pose must be the world-frame origin"
    cal = pq.read_table(clip.scene_dir / "itest_clip.calibration_estimate.parquet")
    assert cal.num_rows == 1

    # world settings restored
    assert world.get_settings().synchronous_mode is False


def test_semantic_aov_survives_the_codec_and_drives_masking(carla_client, hero, tmp_path):
    """The real thing: a live capture's semantic AOV must decode back to exact
    class ids (nearest palette, inside ``mask.MAX_CODEC_L1``) and mask a class out."""
    from carla_cosmos import COSMOS3_NANO, Capture, Rig, controls, mask

    camera = "camera:front:wide:120fov"
    world = carla_client.get_world()
    cap = Capture(world, hero, Rig.single(), COSMOS3_NANO, frames=8, fps=30)
    clip = cap.run(tmp_path, "itest_semantic")

    assert clip.manifest.video("semantic", camera) == "semantic_camera_front_wide_120fov.mp4"
    assert clip.manifest.video("seg", camera) == "seg_camera_front_wide_120fov.mp4"
    assert clip.validate() == [] and clip.validate(for_masking=True) == []
    counts = {k: controls.probe_video(clip.path / n)["frames"] for k, n in clip.manifest.videos.items()}
    assert set(counts.values()) == {8}, counts

    semantic = mask.semantic_video(clip, camera)
    assert semantic.name == "semantic_camera_front_wide_120fov.mp4"
    assert not mask.is_palette_video(clip.video("seg", camera)), "seg must keep the instance colouring"

    worst = 0
    with controls.VideoReader(semantic) as reader:
        for frame in reader:
            worst = max(worst, int(mask._palette_distance(frame).max()))
            mask.tags_from_frame(frame)  # raises if a pixel is beyond the guard
    assert worst <= mask.MAX_CODEC_L1, f"semantic codec round trip L1 {worst}"

    # the ego's own body is tagged 'car', so 'vehicle' always has something to remove
    classes = mask.resolve_classes(["vehicle"])
    masks = list(mask.iter_frame_masks(semantic, classes))
    assert len(masks) == 8 and masks[0].any()
    written = mask.masked_clip_videos(clip, camera, classes, ["depth"], tmp_path / "masked")

    def first_frame(path):
        with controls.VideoReader(path) as r:
            return next(iter(r))

    masked, plain = first_frame(written["depth"]), first_frame(clip.video("depth", camera))
    m = masks[0]
    assert masked[m].max() == 0, "masked region must be black"
    assert (masked[~m] == plain[~m]).all(), "everything else must be untouched"
