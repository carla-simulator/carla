"""Static-obstacle export: what the level bounding boxes are allowed to become.

No CARLA server - only ``carla``'s value types (``BoundingBox``, ``Location``) plus a stub world
whose ``get_level_bbs`` returns whatever the test wants.
"""

import types

import numpy as np
import pytest

import carla

from carla_cosmos.contracts import CameraManifest
from carla_cosmos.scene import STATIC_OBSTACLE_LABELS, SceneExporter, WorldFrame
from carla_cosmos.visibility import VisibilityParams

CAR = carla.CityObjectLabel.Car


def bbox(x, y, z=0.8, ext=(2.5, 1.05, 0.77), yaw=0.0):
    bb = carla.BoundingBox(carla.Location(x=x, y=y, z=z), carla.Vector3D(*ext))
    bb.rotation = carla.Rotation(pitch=0.0, yaw=yaw, roll=0.0)
    return bb


class FakeActor:
    def __init__(self, actor_id, x, y, z=0.0):
        self.id = actor_id
        self._tf = carla.Transform(carla.Location(x=x, y=y, z=z))

    def get_transform(self):
        return self._tf


class FakeWorld:
    def __init__(self, level_bbs, actors=()):
        self._bbs = level_bbs
        self._actors = {a.id: a for a in actors}

    def get_level_bbs(self, label):
        return list(self._bbs.get(label, []))

    def get_actor(self, actor_id):
        return self._actors.get(actor_id)


def exporter(world, hero, actors=()):
    exp = SceneExporter(world, "clip", hero, np.zeros(3))
    exp.frame = WorldFrame(np.eye(4))
    exp._actors = [(a.id, "automobile", None) for a in actors]
    return exp


def track_ids(exp):
    return [o["trackline_id"] for o in exp._static_obstacles]


def test_the_bigger_of_two_boxes_at_the_same_centre_wins():
    """Every parked vehicle mesh carries a second, smaller box for its interior/glass."""
    body = bbox(10.0, 4.0, ext=(2.50, 1.05, 0.77))
    glass = bbox(10.02, 4.01, z=0.91, ext=(2.38, 0.86, 0.57))
    hero = FakeActor(1, 0.0, 0.0)
    exp = exporter(FakeWorld({CAR: [body, glass]}), hero)
    exp._export_static_obstacles()
    assert track_ids(exp) == ["static:car:0"]
    size = exp._static_obstacles[0]["size"]
    assert size["x"] == pytest.approx(5.0) and size["y"] == pytest.approx(2.1)


def test_the_bigger_box_wins_whichever_order_it_comes_in():
    glass = bbox(10.0, 4.0, z=0.91, ext=(2.38, 0.86, 0.57))
    body = bbox(10.0, 4.0, ext=(2.50, 1.05, 0.77))
    exp = exporter(FakeWorld({CAR: [glass, body]}), FakeActor(1, 0.0, 0.0))
    exp._export_static_obstacles()
    assert track_ids(exp) == ["static:car:1"]
    assert exp._static_obstacles[0]["size"]["x"] == pytest.approx(5.0)


def test_distinct_parked_cars_are_kept():
    exp = exporter(FakeWorld({CAR: [bbox(10.0, 4.0), bbox(16.0, 4.0)]}), FakeActor(1, 0.0, 0.0))
    exp._export_static_obstacles()
    assert track_ids(exp) == ["static:car:0", "static:car:1"]


def test_the_hero_mesh_is_not_exported_as_a_parked_car():
    """The hero is not in ``_actors`` (it is the rig) but its mesh *is* in ``get_level_bbs``."""
    hero = FakeActor(7, 30.0, -2.0)
    exp = exporter(FakeWorld({CAR: [bbox(30.1, -2.0, ext=(2.45, 0.92, 0.76)), bbox(10.0, 4.0)]}), hero)
    exp._export_static_obstacles()
    assert track_ids(exp) == ["static:car:1"]


def test_live_vehicle_actors_are_still_skipped():
    other = FakeActor(9, 20.0, 4.0)
    exp = exporter(FakeWorld({CAR: [bbox(20.0, 4.0), bbox(10.0, 4.0)]}, actors=[other]),
                   FakeActor(1, 0.0, 0.0), actors=[other])
    exp._export_static_obstacles()
    assert track_ids(exp) == ["static:car:1"]


def test_flat_boxes_are_dropped():
    exp = exporter(FakeWorld({CAR: [bbox(10.0, 4.0, ext=(2.5, 1.05, 0.05))]}), FakeActor(1, 0.0, 0.0))
    exp._export_static_obstacles()
    assert track_ids(exp) == []


def test_every_label_maps_to_a_category():
    assert set(STATIC_OBSTACLE_LABELS.values()) <= {"automobile", "heavy_truck", "bus", "rider"}


# ----------------------------------------------------------------------------- occlusion filter

class FakeSnapshot:
    """Just enough of ``carla.WorldSnapshot`` for :meth:`SceneExporter.record_tick`."""

    def __init__(self, seconds, transforms):
        self.timestamp = types.SimpleNamespace(elapsed_seconds=seconds)
        self.frame = int(seconds * 30)
        self._transforms = transforms

    def find(self, actor_id):
        tf = self._transforms.get(actor_id)
        return types.SimpleNamespace(get_transform=lambda tf=tf: tf) if tf is not None else None


W, H, FAR = 320, 180, 1000.0


def camera_manifest(name="camera:front:wide:120fov"):
    return CameraManifest(name=name, hfov=90.0, width=W, height=H, lens="pinhole",
                          t_flu=[0.0, 0.0, 0.0], rpy_flu=[0.0, 0.0, 0.0],
                          attach_ue={"x": 0.0, "y": 0.0, "z": 0.0, "pitch": 0.0, "yaw": 0.0, "roll": 0.0},
                          shifted=False)


def depth(value=FAR):
    return {"camera:front:wide:120fov": np.full((H, W), float(value), np.float32)}


def occlusion_exporter(actors=(), statics=(), **params):
    """An exporter with the world frame at the origin, the filter on and nothing else set up.

    The hero sits at the UE origin looking down +x, one camera at the rig origin, so an actor at
    UE ``(d, 0, 0)`` is straight ahead at ``d`` metres in every frame — the same geometry the
    pure tests in ``test_visibility.py`` use."""
    hero = FakeActor(1, 0.0, 0.0)
    exp = SceneExporter(FakeWorld({}), "clip", hero, np.zeros(3),
                        cameras=[camera_manifest()], visibility=VisibilityParams(**params))
    exp.frame = WorldFrame(np.eye(4))
    exp._actors = list(actors)
    exp._static_obstacles = list(statics)
    exp._begin_visibility()
    return exp


def moving_actor(actor_id=7, category="automobile"):
    bb = carla.BoundingBox(carla.Location(0.0, 0.0, 0.0), carla.Vector3D(2.0, 1.0, 0.75))
    bb.rotation = carla.Rotation()
    return (actor_id, category, bb)


def parked(track="static:car:0", x=20.0):
    return {"trackline_id": track, "center": {"x": x, "y": 0.0, "z": 0.0},
            "size": {"x": 4.0, "y": 2.0, "z": 1.5},
            "orientation": {"x": 0.0, "y": 0.0, "z": 0.0, "w": 1.0}, "category": "automobile"}


def tick(exp, seconds, depths, actor_x=20.0, actor_id=7):
    snap = FakeSnapshot(seconds, {1: carla.Transform(),
                                  actor_id: carla.Transform(carla.Location(x=actor_x))})
    exp.record_tick(snap, depth=depths)


def emitted(exp):
    rows, _occluded, _decisions = exp._obstacle_rows()
    ids = {}
    for r in rows:
        ids.setdefault(str(r["obstacle"]["trackline_id"]), 0)
        ids[str(r["obstacle"]["trackline_id"])] += 1
    return ids


def test_an_obstacle_behind_a_wall_is_not_exported():
    exp = occlusion_exporter(actors=[moving_actor()])
    for i in range(6):
        tick(exp, i / 30, depth(8.0))  # a building 8 m ahead, the car 20 m behind it
    assert emitted(exp) == {}


def test_an_obstacle_in_the_open_is_exported_at_every_tick():
    exp = occlusion_exporter(actors=[moving_actor()])
    for i in range(6):
        tick(exp, i / 30, depth())
    assert emitted(exp) == {"7": 6}


def test_a_track_that_disappears_and_comes_back_is_split():
    """The loader interpolates across any hole inside a track, so the two halves must not share
    a ``trackline_id``."""
    exp = occlusion_exporter(actors=[moving_actor()], hysteresis_frames=3)
    for i in range(3):
        tick(exp, i / 30, depth())
    for i in range(3, 9):
        tick(exp, i / 30, depth(8.0))
    for i in range(9, 12):
        tick(exp, i / 30, depth())
    assert emitted(exp) == {"7#0": 3, "7#1": 3}


def test_a_short_dropout_does_not_split_the_track():
    exp = occlusion_exporter(actors=[moving_actor()], hysteresis_frames=3)
    for i in range(6):
        tick(exp, i / 30, depth(8.0 if i == 3 else FAR))
    assert emitted(exp) == {"7": 6}


def test_a_one_tick_segment_is_dropped():
    """NVIDIA's loader skips tracks with fewer than two observations anyway."""
    exp = occlusion_exporter(actors=[moving_actor()], hysteresis_frames=0)
    for i in range(6):
        tick(exp, i / 30, depth(FAR if i == 2 else 8.0))
    assert emitted(exp) == {}


def test_a_parked_obstacle_hidden_in_the_middle_becomes_one_track_per_segment():
    """A clip-wide first+last pair would make the loader hold the box through the occlusion."""
    exp = occlusion_exporter(statics=[parked()], hysteresis_frames=3)
    for i in range(12):
        tick(exp, i / 30, depth(8.0 if 3 <= i < 9 else FAR))
    assert emitted(exp) == {"static:car:0#0": 2, "static:car:0#1": 2}


def test_a_parked_obstacle_that_stays_visible_keeps_its_two_clip_wide_rows():
    exp = occlusion_exporter(statics=[parked()])
    for i in range(6):
        tick(exp, i / 30, depth())
    rows, _occ, _dec = exp._obstacle_rows()
    assert [r["key"]["timestamp_micros"] for r in rows] == [exp.timestamps[0], exp.timestamps[-1]]
    assert rows[0]["obstacle"]["trackline_id"] == "static:car:0"


def test_the_sidecar_carries_the_dropped_boxes_and_the_decisions():
    exp = occlusion_exporter(actors=[moving_actor()], statics=[parked(x=60.0)], hysteresis_frames=0)
    for i in range(4):
        tick(exp, i / 30, depth(8.0))
    _rows, occluded, decisions = exp._obstacle_rows()
    assert {e["ticks"][1] - e["ticks"][0] for e in occluded if e["obstacle"]["trackline_id"] == "7"} == {1}
    assert [e["ticks"] for e in occluded if e["obstacle"]["trackline_id"].startswith("static:")] == [[0, 4]]
    assert decisions["7"]["segments"] == [] and decisions["7"]["state"] == [0, 0, 0, 0]
    assert decisions["static:car:0"]["static"] is True


def test_visibility_none_exports_every_obstacle_as_before():
    exp = occlusion_exporter(actors=[moving_actor()], statics=[parked()], mode="none")
    for i in range(4):
        tick(exp, i / 30, depth(8.0))
    assert emitted(exp) == {"7": 4, "static:car:0": 2}


def test_the_filter_switches_itself_off_without_a_calibration():
    exp = SceneExporter(FakeWorld({}), "clip", FakeActor(1, 0.0, 0.0), np.zeros(3),
                        visibility="depth")
    exp.frame = WorldFrame(np.eye(4))
    exp._begin_visibility()
    assert exp.visibility.mode == "none"


class FakeVehicle(FakeActor):
    """A live actor as ``_scan_actors`` reads it."""

    def __init__(self, actor_id, x, y, type_id="vehicle.audi.tt", attributes=None):
        super().__init__(actor_id, x, y)
        self.type_id = type_id
        self.attributes = attributes or {"base_type": "car", "number_of_wheels": "4"}
        self.bounding_box = bbox(0.0, 0.0)


class ActorList(list):
    def filter(self, _pattern):
        return []


def test_the_hero_is_not_a_dynamic_obstacle():
    """The ego is the rig, not an obstacle.

    NVIDIA's ClipGT keeps the ego in ``egomotion_estimate`` only - their reference package
    (``multiview_example1``) has no obstacle track that follows the ego pose - and the
    world-scenario renderer draws every row of the obstacle table, so an ego row would put a
    box around the camera in every frame.  ``_scan_actors`` skips it by actor id.
    """
    hero = FakeVehicle(7, 0.0, 0.0)
    other = FakeVehicle(9, 20.0, 4.0)
    walker = FakeVehicle(11, 5.0, 1.0, type_id="walker.pedestrian.0001")
    world = FakeWorld({})
    world.get_actors = lambda: ActorList([hero, other, walker])
    exp = SceneExporter(world, "clip", hero, np.zeros(3))
    exp.frame = WorldFrame(np.eye(4))
    exp._scan_actors()
    assert [aid for aid, _cat, _bb in exp._actors] == [9, 11]
