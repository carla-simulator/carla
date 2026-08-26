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
    # let the world observer publish a snapshot containing the new actor,
    # otherwise get_transform() is still the identity (see rear_axle_local_ue)
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
