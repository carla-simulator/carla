import unittest

import carla
import numpy as np

from nurec_integration import NurecActor, NurecScenario


class _ActorInstance:
    def __init__(self, x=0.0, y=0.0):
        self.autopilot = False
        self.location = carla.Location(x=x, y=y)
        self.bounding_box = carla.BoundingBox(
            carla.Location(), carla.Vector3D(x=2.0, y=1.0, z=1.0)
        )

    def set_autopilot(self, enabled, traffic_manager_port=8000):
        self.autopilot = enabled
        self.traffic_manager_port = traffic_manager_port

    def apply_control(self, control):
        self.control = control

    def set_collisions(self, enabled):
        self.collisions = enabled

    def set_simulate_physics(self, enabled):
        self.simulate_physics = enabled

    def set_target_velocity(self, velocity):
        self.target_velocity = velocity

    def set_target_angular_velocity(self, velocity):
        self.target_angular_velocity = velocity

    def get_location(self):
        return self.location

    def get_transform(self):
        return carla.Transform(self.location, carla.Rotation())


class _Track:
    track_id = "traffic"

    def get_path(self, spacing, start_time):
        return ["route-start", "route-end"]


class _MovingTrack(_Track):
    def start_time(self):
        return 1_000_000

    def end_time(self):
        return 2_000_000

    def interpolate_pose_matrix(self, timestamp):
        pose = np.eye(4)
        elapsed = (timestamp - self.start_time()) / 1_000_000
        pose[0, 3] = 10.0 * elapsed
        pose[1, 3] = 2.0 * elapsed
        return pose


class _Actor:
    def __init__(self, x=0.0, y=0.0):
        self.actor_inst = _ActorInstance(x=x, y=y)
        self.track = _Track()
        self.tm_route_end = None
        self.tm_route_done = False
        self.tm_controlled = False

    def set_physics(
        self, enabled, current_time, seed_recorded_velocity=True
    ):
        self.physics = enabled
        if enabled and not seed_recorded_velocity:
            self.actor_inst.set_target_velocity(carla.Vector3D())
            self.actor_inst.set_target_angular_velocity(carla.Vector3D())


class _TrafficManager:
    def __init__(self):
        self.following_distance = None
        self.ignore_vehicles = None

    def set_path(self, actor, path):
        self.path = path

    def set_desired_speed(self, actor, speed):
        self.speed = speed

    def update_vehicle_lights(self, actor, enabled):
        pass

    def random_left_lanechange_percentage(self, actor, percentage):
        pass

    def random_right_lanechange_percentage(self, actor, percentage):
        pass

    def auto_lane_change(self, actor, enabled):
        pass

    def distance_to_leading_vehicle(self, actor, distance):
        self.following_distance = distance

    def ignore_lights_percentage(self, actor, percentage):
        pass

    def ignore_vehicles_percentage(self, actor, percentage):
        self.ignore_vehicles = percentage


class TrafficControlTest(unittest.TestCase):
    def _scenario(self):
        scenario = object.__new__(NurecScenario)
        scenario.traffic_manager = _TrafficManager()
        scenario.traffic_manager_port = 8001
        scenario.actor_mapping = {"traffic": _Actor()}
        scenario._enable_traffic_manager = lambda: None
        scenario.get_sim_time = lambda: 0
        scenario._track_travel_stats = lambda track, start: (20.0, 4.0)
        return scenario

    def test_collision_aware_handoff_respects_vehicles(self):
        scenario = self._scenario()
        scenario.set_follow_path("traffic", collision_aware=True)

        self.assertEqual(scenario.traffic_manager.following_distance, 10.0)
        self.assertEqual(scenario.traffic_manager.ignore_vehicles, 0)
        self.assertEqual(scenario.actor_mapping["traffic"].tm_follow_distance, 10.0)
        self.assertTrue(scenario.actor_mapping["traffic"].tm_controlled)
        self.assertEqual(scenario.actor_mapping["traffic"].actor_inst.target_velocity.x, 0.0)
        self.assertTrue(scenario.actor_mapping["traffic"].actor_inst.collisions)
        self.assertEqual(
            scenario.actor_mapping["traffic"].actor_inst.traffic_manager_port,
            scenario.traffic_manager_port,
        )

    def test_physics_can_start_without_recorded_velocity_seed(self):
        instance = _ActorInstance()
        actor = NurecActor(instance, _Track())

        actor.set_physics(True, 1_000_000, seed_recorded_velocity=False)

        self.assertTrue(instance.simulate_physics)
        self.assertEqual(instance.target_velocity.x, 0.0)
        self.assertEqual(instance.target_angular_velocity.z, 0.0)

    def test_physics_at_track_start_seeds_forward_recorded_velocity(self):
        instance = _ActorInstance()
        actor = NurecActor(instance, _MovingTrack())

        actor.set_physics(True, 1_000_000)

        self.assertAlmostEqual(instance.target_velocity.x, 10.0)
        # NuRec is right-handed and CARLA is left-handed.
        self.assertAlmostEqual(instance.target_velocity.y, -2.0)

    def test_dataset_replay_handoff_retains_legacy_timing(self):
        scenario = self._scenario()
        scenario.set_follow_path("traffic", collision_aware=False)

        self.assertEqual(scenario.traffic_manager.following_distance, 0.0)
        self.assertEqual(scenario.traffic_manager.ignore_vehicles, 100)

    def test_following_gap_scales_with_recorded_speed(self):
        scenario = self._scenario()
        scenario._track_travel_stats = lambda track, start: (80.0, 4.0)

        scenario.set_follow_path("traffic", collision_aware=True)

        self.assertEqual(scenario.traffic_manager.following_distance, 40.0)

    def test_close_same_lane_rear_actor_is_unsafe(self):
        scenario = self._scenario()
        scenario.actor_mapping["ego"] = _Actor(x=0.0, y=0.0)
        scenario.actor_mapping["traffic"] = _Actor(x=-8.0, y=0.0)

        safe, gap, required = scenario._rear_handoff_status(
            scenario.actor_mapping["traffic"]
        )

        self.assertFalse(safe)
        self.assertAlmostEqual(gap, 4.0)
        self.assertEqual(required, 10.0)

    def test_distant_same_lane_rear_actor_is_safe(self):
        scenario = self._scenario()
        scenario.actor_mapping["ego"] = _Actor(x=0.0, y=0.0)
        scenario.actor_mapping["traffic"] = _Actor(x=-20.0, y=0.0)

        safe, gap, required = scenario._rear_handoff_status(
            scenario.actor_mapping["traffic"]
        )

        self.assertTrue(safe)
        self.assertAlmostEqual(gap, 16.0)
        self.assertEqual(required, 10.0)

    def test_future_handoff_initializes_tm_before_actors_enter(self):
        scenario = object.__new__(NurecScenario)
        scenario.actor_mapping = {}
        tm_initializations = []
        scenario._enable_traffic_manager = lambda: tm_initializations.append(True)

        handed_over = scenario.set_all_actors_carla_controlled(
            include_future_actors=True, collision_aware=True
        )

        self.assertEqual(handed_over, 0)
        self.assertEqual(tm_initializations, [True])
        self.assertTrue(scenario.carla_controls_new_actors)
        self.assertTrue(scenario.carla_traffic_collision_aware)

    def test_route_less_vehicle_is_held_under_physics(self):
        scenario = self._scenario()
        actor = scenario.actor_mapping["traffic"]

        scenario._hold_actor_with_physics(actor)

        self.assertTrue(actor.physics)
        self.assertTrue(actor.tm_route_done)
        self.assertFalse(actor.tm_controlled)
        self.assertEqual(actor.actor_inst.control.brake, 1.0)
        self.assertTrue(actor.actor_inst.control.hand_brake)


if __name__ == "__main__":
    unittest.main()
