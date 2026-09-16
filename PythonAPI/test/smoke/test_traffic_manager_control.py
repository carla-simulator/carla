# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Smoke tests for the Traffic Manager control loop.

Three behaviours that need a live server tick loop and cannot be reached from
the LibCarla suite, because they only appear once the controller output has
been through the physics and back:

* pulling away from rest and from a junction happens at a comfortable rate
  rather than with the throttle on its bound from the first frame,
* corners are taken at a comfortable lateral acceleration rather than at the
  tyre-grip limit, and the speed limiter does not brake traffic to a halt,
* the automatic vehicle lights never switch on a light the stage does not
  manage, which is what deriving a command from the "state unknown" sentinel
  used to do -- reachable only in asynchronous mode, where the cached light
  state list can predate a vehicle.

The comfort tests run in synchronous mode so the sampling period is exact.
The lights test runs in asynchronous mode because that is the only mode where
the cached world queries are paced instead of read every step.
"""

import math
import time

from . import SmokeTest, SyncSmokeTest

import carla


# Sampling window used to differentiate speed. One tick of 0.05 s resolves the
# transient but also every suspension blip; 0.2 s smooths those out and still
# sits well inside a pull-away, which takes about 2 s.
ACCELERATION_WINDOW_TICKS = 4

# A comfortable urban pull-away is 1-2 m/s2. Measured on this branch: 1.8-2.0
# mean and 2.6-2.9 peak. Before the reference shaper: 5.2-5.6 median and 8.4
# peak. The bound sits between the two with margin for physics noise.
MAX_PULL_AWAY_ACCELERATION = 4.5

# The turn speed limiter asks for LATERAL_COMFORT_ACCELERATION (3.0 m/s2).
# Measured on this branch: 3.8-4.0 m/s2 at the 99th percentile. The tyre-grip
# model it replaced asked for FRICTION * GRAVITY (5.89 m/s2) and measured over
# 9.8 m/s2 at the peak, so this bound separates the two.
MAX_CORNERING_ACCELERATION = 5.0

# The limiter must not brake traffic to a standstill: a regression that makes
# the path speed limit collapse would otherwise pass every bound above.
MIN_FLEET_MEAN_SPEED = 2.0

# Bits the VehicleLightStage does not manage. The "state unknown" sentinel is
# every bit set, so a command derived from it switches these on and the
# write-back then holds them on for good.
UNMANAGED_LIGHTS = (
    int(carla.VehicleLightState.Reverse) |
    int(carla.VehicleLightState.Interior) |
    int(carla.VehicleLightState.Special1) |
    int(carla.VehicleLightState.Special2))

MANAGED_LIGHTS = (
    int(carla.VehicleLightState.Position) |
    int(carla.VehicleLightState.LowBeam) |
    int(carla.VehicleLightState.HighBeam) |
    int(carla.VehicleLightState.Brake) |
    int(carla.VehicleLightState.RightBlinker) |
    int(carla.VehicleLightState.LeftBlinker) |
    int(carla.VehicleLightState.Fog))


def _magnitude(vector):
    return math.sqrt(vector.x * vector.x + vector.y * vector.y + vector.z * vector.z)


def _forward_speed(actor_snapshot):
    velocity = actor_snapshot.get_velocity()
    forward = actor_snapshot.get_transform().get_forward_vector()
    return velocity.x * forward.x + velocity.y * forward.y + velocity.z * forward.z


def _lateral_acceleration(actor_snapshot):
    # v * yaw_rate is the centripetal acceleration of a vehicle following its
    # own heading, which is what the turn speed limiter bounds.
    yaw_rate = math.radians(actor_snapshot.get_angular_velocity().z)
    return abs(_magnitude(actor_snapshot.get_velocity()) * yaw_rate)


def _percentile(samples, fraction):
    ordered = sorted(samples)
    index = min(len(ordered) - 1, int(fraction * len(ordered)))
    return ordered[index]


class TestTrafficManagerComfort(SyncSmokeTest):

    def setUp(self):
        super(TestTrafficManagerComfort, self).setUp()
        self.tm = self.client.get_trafficmanager()
        self.tm.set_synchronous_mode(True)
        self.tm_port = self.tm.get_port()
        self.spawned = []

    def tearDown(self):
        for actor in self.spawned:
            try:
                actor.set_autopilot(False, self.tm_port)
                actor.destroy()
            except Exception:
                pass
        self.spawned = []
        try:
            self.tm.set_synchronous_mode(False)
        except Exception:
            pass
        self.tm = None
        # The packaged build only ships Town10HD_Opt, so the base class's
        # Town03 is not available here.
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn(self, count):
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = self.filter_vehicles_for_old_towns(list(bp_lib.filter("vehicle.*")))
        spawn_points = self.world.get_map().get_spawn_points()
        for index in range(min(count, len(spawn_points))):
            actor = self.world.try_spawn_actor(
                vehicle_bps[index % len(vehicle_bps)], spawn_points[index])
            if actor is not None:
                self.spawned.append(actor)
        return self.spawned

    def _settle(self, ticks):
        # A vehicle spawns above the road and drops onto it. Hold it on the
        # handbrake until the suspension has settled, so the landing is not
        # measured as a pull-away.
        for actor in self.spawned:
            actor.apply_control(carla.VehicleControl(hand_brake=True))
        for _ in range(ticks):
            self.world.tick()

    def test_pull_away_from_rest_is_comfortable(self):
        vehicles = self._spawn(20)
        self.assertGreater(len(vehicles), 0)
        self._settle(20)

        for actor in vehicles:
            actor.set_autopilot(True, self.tm_port)
        # The first cycle after registration localizes the vehicle and issues
        # no control, so do not sample across it.
        for _ in range(5):
            self.world.tick()

        delta_seconds = 0.05
        history = {actor.id: [] for actor in vehicles}
        peak_acceleration = 0.0
        worst_actor = None
        for _ in range(240):
            self.world.tick()
            snapshot = self.world.get_snapshot()
            for actor in vehicles:
                actor_snapshot = snapshot.find(actor.id)
                if actor_snapshot is None:
                    continue
                speeds = history[actor.id]
                speeds.append(_forward_speed(actor_snapshot))
                if len(speeds) <= ACCELERATION_WINDOW_TICKS:
                    continue
                acceleration = (
                    (speeds[-1] - speeds[-1 - ACCELERATION_WINDOW_TICKS]) /
                    (ACCELERATION_WINDOW_TICKS * delta_seconds))
                if acceleration > peak_acceleration:
                    peak_acceleration = acceleration
                    worst_actor = actor.type_id

        self.assertLess(
            peak_acceleration, MAX_PULL_AWAY_ACCELERATION,
            "peak longitudinal acceleration {:.2f} m/s2 on {}; the reference "
            "shaper and the launch ramp should keep a pull-away near "
            "COMFORT_ACCELERATION".format(peak_acceleration, worst_actor))

    def test_cornering_is_comfortable_without_stalling_traffic(self):
        vehicles = self._spawn(20)
        self.assertGreater(len(vehicles), 0)
        self._settle(20)

        for actor in vehicles:
            actor.set_autopilot(True, self.tm_port)
        for _ in range(5):
            self.world.tick()

        lateral_accelerations = []
        speeds = []
        for _ in range(600):
            self.world.tick()
            snapshot = self.world.get_snapshot()
            for actor in vehicles:
                actor_snapshot = snapshot.find(actor.id)
                if actor_snapshot is None:
                    continue
                lateral_accelerations.append(_lateral_acceleration(actor_snapshot))
                speeds.append(_magnitude(actor_snapshot.get_velocity()))

        self.assertGreater(len(lateral_accelerations), 0)
        p99 = _percentile(lateral_accelerations, 0.99)
        self.assertLess(
            p99, MAX_CORNERING_ACCELERATION,
            "lateral acceleration at the 99th percentile is {:.2f} m/s2; the "
            "turn speed limiter should keep corners near "
            "LATERAL_COMFORT_ACCELERATION".format(p99))

        mean_speed = sum(speeds) / len(speeds)
        self.assertGreater(
            mean_speed, MIN_FLEET_MEAN_SPEED,
            "the fleet averaged {:.2f} m/s; the turn speed limiter should slow "
            "traffic for corners, not stop it".format(mean_speed))


class TestTrafficManagerVehicleLights(SmokeTest):
    """Asynchronous mode, where the cached light state list is paced.

    Synchronous mode reads the list on every step, so the case a stale or
    incomplete cache produces is only reachable here.
    """

    def setUp(self):
        super(TestTrafficManagerVehicleLights, self).setUp()
        self.tm = self.client.get_trafficmanager()
        self.tm_port = self.tm.get_port()
        self.spawned = []

    def tearDown(self):
        for actor in self.spawned:
            try:
                actor.set_autopilot(False, self.tm_port)
                actor.destroy()
            except Exception:
                pass
        self.spawned = []
        self.tm = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def _spawn_one(self, spawn_point, blueprint):
        actor = self.world.try_spawn_actor(blueprint, spawn_point)
        if actor is None:
            return None
        actor.set_autopilot(True, self.tm_port)
        self.tm.update_vehicle_lights(actor, True)
        self.spawned.append(actor)
        return actor

    def test_automatic_lights_never_switch_on_an_unmanaged_light(self):
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = self.filter_vehicles_for_old_towns(list(bp_lib.filter("vehicle.*")))
        spawn_points = self.world.get_map().get_spawn_points()
        self.assertGreater(len(spawn_points), 12)

        for index in range(8):
            self._spawn_one(spawn_points[index], vehicle_bps[index % len(vehicle_bps)])
        self.assertGreater(len(self.spawned), 0)

        observed_managed = 0
        next_spawn = 8
        for step in range(40):
            # Registering a vehicle between two refreshes of the cached list is
            # the case that used to read the "state unknown" sentinel.
            if step % 8 == 0 and next_spawn < min(12, len(spawn_points)):
                self._spawn_one(
                    spawn_points[next_spawn], vehicle_bps[next_spawn % len(vehicle_bps)])
                next_spawn += 1
            time.sleep(0.5)
            for actor in list(self.spawned):
                if not actor.is_alive:
                    continue
                light_state = int(actor.get_light_state())
                observed_managed |= light_state & MANAGED_LIGHTS
                self.assertEqual(
                    light_state & UNMANAGED_LIGHTS, 0,
                    "{} has an unmanaged light on (state 0x{:x}); the vehicle "
                    "light stage must skip a vehicle whose current state it "
                    "does not know instead of deriving a command from the "
                    "sentinel".format(actor.type_id, light_state))

        self.assertNotEqual(
            observed_managed, 0,
            "no managed light was ever driven, so the assertion above never "
            "exercised the stage")

    def test_destroyed_vehicle_does_not_hold_a_light_state(self):
        bp_lib = self.world.get_blueprint_library()
        vehicle_bps = self.filter_vehicles_for_old_towns(list(bp_lib.filter("vehicle.*")))
        spawn_points = self.world.get_map().get_spawn_points()

        first = self._spawn_one(spawn_points[0], vehicle_bps[0])
        self.assertIsNotNone(first)
        time.sleep(3.0)

        self.spawned.remove(first)
        first.destroy()
        time.sleep(1.0)

        # The cached list outlives the response it came from, so a destroyed
        # vehicle has to be dropped from it explicitly. A replacement must get
        # its own state rather than the one left behind.
        replacement = self._spawn_one(spawn_points[1], vehicle_bps[1])
        self.assertIsNotNone(replacement)
        for _ in range(10):
            time.sleep(0.5)
            self.assertEqual(
                int(replacement.get_light_state()) & UNMANAGED_LIGHTS, 0,
                "a vehicle spawned after another was destroyed picked up an "
                "unmanaged light state")
