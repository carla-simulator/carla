"""Static-obstacle export: what the level bounding boxes are allowed to become.

No CARLA server - only ``carla``'s value types (``BoundingBox``, ``Location``) plus a stub world
whose ``get_level_bbs`` returns whatever the test wants.
"""

import numpy as np
import pytest

import carla

from carla_cosmos.scene import STATIC_OBSTACLE_LABELS, SceneExporter, WorldFrame

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
