# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest
from . import SmokeTest

import carla
import time
import math
import numpy as np
from enum import Enum

def list_equal_tol(objs, tol = 1e-5):
    if (len(objs) < 2):
        return True

    for i in range(1, len(objs)):
        equal = equal_tol(objs[0], objs[i], tol)
        if not equal:
            return False

    return True

def equal_tol(obj_a, obj_b, tol = 1e-5):
    if isinstance(obj_a, list):
        return obj_a == obj_b

    if isinstance(obj_a, carla.Vector3D):
        diff = abs(obj_a - obj_b)
        return diff.x < tol and diff.y < tol and diff.z < tol

    if isinstance(obj_a, carla.Vector2D):
        return abs(obj_a.x - obj_b.x) < tol and abs(obj_a.y - obj_b.y) < tol

    # Curves and wheel arrays come back as bound vectors, which have no
    # subtraction operator; compare them element by element.
    if hasattr(obj_a, '__len__'):
        return len(obj_a) == len(obj_b) and all(
            equal_tol(x, y, tol) for x, y in zip(obj_a, obj_b))

    return abs(obj_a - obj_b) < tol

def equal_physics_control(pc_a, pc_b):
    error_msg = ""

    for key in dir(pc_a):
        if key.startswith('__') or key == "wheels":
            continue

        if not equal_tol(getattr(pc_a, key), getattr(pc_b, key), 1e-3):
            error_msg = "Car property: '%s' in VehiclePhysicsControl does not match: %.4f %.4f" \
              % (key, getattr(pc_a, key), getattr(pc_b, key))
            return False, error_msg

    if len(pc_a.wheels) != len(pc_b.wheels):
        error_msg = "The number of wheels does not match %d, %d" \
            % (len(pc_a.wheels) != len(pc_b.wheels))
        return False, error_msg

    for w in range(0, len(pc_a.wheels)):
        for key in dir(pc_a.wheels[w]):
            if key.startswith('__') or key == "position":
                continue

            if not equal_tol(getattr(pc_a.wheels[w], key), getattr(pc_b.wheels[w], key), 1e-3):
                error_msg = "Wheel property: '%s' in VehiclePhysicsControl does not match: %.4f %.4f" \
                % (key, getattr(pc_a.wheels[w], key), getattr(pc_b.wheels[w], key))
                return False, error_msg

    return True, error_msg

def change_physics_control(vehicle, friction_force_multiplier = None, drag = None, wheel_sweep = None):
    # Change Vehicle Physics Control parameters of the vehicle
    physics_control = vehicle.get_physics_control()

    if drag is not None:
        physics_control.drag_coefficient = drag

    if wheel_sweep is not None:
        physics_control.use_sweep_wheel_collision = wheel_sweep

    front_left_wheel = physics_control.wheels[0]
    front_right_wheel = physics_control.wheels[1]
    rear_left_wheel = physics_control.wheels[2]
    rear_right_wheel = physics_control.wheels[3]

    if friction_force_multiplier is not None:
        front_left_wheel.friction_force_multiplier = friction_force_multiplier
        front_right_wheel.friction_force_multiplier = friction_force_multiplier
        rear_left_wheel.friction_force_multiplier = friction_force_multiplier
        rear_right_wheel.friction_force_multiplier = friction_force_multiplier

    wheels = [front_left_wheel, front_right_wheel, rear_left_wheel, rear_right_wheel]
    physics_control.wheels = wheels

    return physics_control

SpawnActor = carla.command.SpawnActor
FutureActor = carla.command.FutureActor
ApplyTargetVelocity = carla.command.ApplyTargetVelocity
SetEnableGravity = carla.command.SetEnableGravity
ApplyVehicleControl = carla.command.ApplyVehicleControl
ApplyVehiclePhysicsControl = carla.command.ApplyVehiclePhysicsControl


class TestApplyVehiclePhysics(SyncSmokeTest):
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()

    def check_single_physics_control(self, bp_vehicle):
        veh_tranf = self.world.get_map().get_spawn_points()[0]

        vehicle = self.world.spawn_actor(bp_vehicle, veh_tranf)

        # Checking the setting of car variables (drag coefficient)
        pc_a = change_physics_control(vehicle, drag=5)
        vehicle.apply_physics_control(pc_a)
        self.wait(2)
        pc_b = vehicle.get_physics_control()

        equal, msg = equal_physics_control(pc_a, pc_b)
        if not equal:
            self.fail("%s: %s" % (bp_vehicle.id, msg))

        self.wait(2)

        # Checking the setting of wheel variables (tire friction)
        pc_a = change_physics_control(vehicle, friction_force_multiplier=5)
        vehicle.apply_physics_control(pc_a)
        self.wait(2)
        pc_b = vehicle.get_physics_control()

        equal, msg = equal_physics_control(pc_a, pc_b)
        if not equal:
            self.fail("%s: %s" % (bp_vehicle.id, msg))

        vehicle.destroy()

    def check_multiple_physics_control(self, bp_vehicles, index_bp = None):
        num_veh = 10
        vehicles = []
        pc_a = []
        pc_b = []
        for i in range(0, num_veh):
            veh_tranf = self.world.get_map().get_spawn_points()[i]
            bp_vehicle = bp_vehicles[index_bp] if index_bp is not None else bp_vehicles[i]
            vehicles.append(self.world.spawn_actor(bp_vehicle, veh_tranf))
            drag_coeff = 3.0 + 0.1*i
            pc_a.append(change_physics_control(vehicles[i], drag=drag_coeff))
            vehicles[i].apply_physics_control(pc_a[i])

        self.wait(2)

        for i in range(0, num_veh):
            pc_b.append(vehicles[i].get_physics_control())

        for i in range(0, num_veh):
            equal, msg = equal_physics_control(pc_a[i], pc_b[i])
            if not equal:
                self.fail("%s: %s" % (bp_vehicle.id, msg))

        pc_a = []
        pc_b = []
        for i in range(0, num_veh):
            friction = 1.0 + 0.1*i
            pc_a.append(change_physics_control(vehicles[i], friction_force_multiplier=friction))
            vehicles[i].apply_physics_control(pc_a[i])

        self.wait(2)

        for i in range(0, num_veh):
            pc_b.append(vehicles[i].get_physics_control())

        for i in range(0, num_veh):
            equal, msg = equal_physics_control(pc_a[i], pc_b[i])
            if not equal:
                self.fail("%s: %s" % (bp_vehicle.id, msg))

        for i in range(0, num_veh):
            vehicles[i].destroy()

    def test_single_physics_control(self):
        print("TestApplyVehiclePhysics.test_single_physics_control")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = self.filter_vehicles_for_old_towns(bp_vehicles)
        for bp_veh in bp_vehicles:
            self.check_single_physics_control(bp_veh)

    def test_multiple_physics_control(self):
        print("TestApplyVehiclePhysics.test_multiple_physics_control")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = self.filter_vehicles_for_old_towns(bp_vehicles)
        for idx in range(0, len(bp_vehicles)):
            self.check_multiple_physics_control(bp_vehicles, idx)

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = self.filter_vehicles_for_old_towns(bp_vehicles)
        self.check_multiple_physics_control(bp_vehicles)

class TestVehicleTireConfig(SyncSmokeTest):
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()

    def test_vehicle_wheel_collision(self):
        print("TestVehicleTireConfig.test_vehicle_wheel_collision")

        self.load_world("Town05_Opt", False)

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = [x for x in bp_vehicles if int(x.get_attribute('number_of_wheels')) == 4]

        for bp_veh in bp_vehicles:
            veh_transf_00 = carla.Transform(carla.Location(36, -200, 0.2), carla.Rotation(yaw=91))
            veh_transf_01 = carla.Transform(carla.Location(31, -200, 0.7), carla.Rotation(yaw=91))

            batch = [
                    SpawnActor(bp_veh, veh_transf_00)
                    .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(0, 0, 0))),
                    SpawnActor(bp_veh, veh_transf_01)
                    .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(0, 0, 0)))
                ]

            responses = self.client.apply_batch_sync(batch)

            veh_ids = [x.actor_id for x in responses]
            veh_refs = [self.world.get_actor(x) for x in veh_ids]

            if (0 in veh_ids) or (None in veh_refs):
                self.fail("%s: The test cars could not be correctly spawned" % (bp_veh.id))

            self.wait(10)

            vel_ref = 100.0 / 3.6
            self.client.apply_batch_sync([
                ApplyVehiclePhysicsControl(veh_refs[0], change_physics_control(veh_refs[0], wheel_sweep = False)),
                ApplyVehiclePhysicsControl(veh_refs[1], change_physics_control(veh_refs[1], wheel_sweep = True))])
            self.wait(1)

            self.client.apply_batch_sync([
                ApplyTargetVelocity(veh_refs[0], carla.Vector3D(0, vel_ref, 0)),
                ApplyTargetVelocity(veh_refs[1], carla.Vector3D(0, vel_ref, 0))
            ])
            self.wait(150)

            loc_veh_00 = veh_refs[0].get_location().y
            loc_veh_01 = veh_refs[1].get_location().y
            vel_veh_00 = veh_refs[0].get_velocity().y
            vel_veh_01 = veh_refs[1].get_velocity().y

            if not list_equal_tol([vel_veh_00, vel_veh_01], 0.5):
                self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in veh_ids])
                self.fail("%s: Velocities are not equal after simulation. [%.3f, %.3f]"
                  % (bp_veh.id, vel_veh_00, vel_veh_01))

            if not list_equal_tol([loc_veh_00, loc_veh_01], 0.5):
                self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in veh_ids])
                self.fail("%s: Locations are not equal after simulation. [%.3f, %.3f]"
                  % (bp_veh.id, loc_veh_00, loc_veh_01))

            self.client.apply_batch_sync([carla.command.DestroyActor(x) for x in veh_ids])

class TestStickyControl(SyncSmokeTest):
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()

    def run_scenario(self, bp_veh, veh_control, continous = False, reset_after_first = False, sticky = None):
        ref_pos = -1
        veh_transf = carla.Transform(carla.Location(235, ref_pos, 0.2), carla.Rotation(yaw=90))
        veh_forward = veh_transf.rotation.get_forward_vector()

        if sticky is not None:
            bp_veh.set_attribute("sticky_control", sticky)

        batch = [SpawnActor(bp_veh, veh_transf)
            .then(ApplyTargetVelocity(FutureActor, carla.Vector3D(0, 0, 0)))]

        responses = self.client.apply_batch_sync(batch)

        if len(responses) != 1 or responses[0].error:
            self.fail("%s: The test car could not be correctly spawned" % (bp_veh.id))

        vehicle_id = responses[0].actor_id
        vehicle_00 = self.world.get_actor(vehicle_id)

        for _i in range(0, 10):
            self.world.tick()

        self.client.apply_batch_sync([ApplyVehicleControl(vehicle_00, veh_control)])
        self.world.tick()

        for _i in range(0, 150):
            if continous:
                self.client.apply_batch_sync([ApplyVehicleControl(vehicle_00, veh_control)])
            if reset_after_first:
                self.client.apply_batch_sync([ApplyVehicleControl(vehicle_00, carla.VehicleControl())])
            self.world.tick()

        loc_veh_00 = vehicle_00.get_location().y
        vel_veh_00 = vehicle_00.get_velocity().y
        dist_veh_00 = loc_veh_00 - ref_pos

        self.client.apply_batch([carla.command.DestroyActor(vehicle_id)])
        self.world.tick()

        return dist_veh_00, vel_veh_00

    def test_default(self):
        print("TestStickyControl.test_default")

        inp_control = carla.VehicleControl(throttle=1.0)
        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")

        bp_veh = bp_vehicles[0]
        d0, v0 = self.run_scenario(bp_veh, inp_control)
        d1, v1 = self.run_scenario(bp_veh, inp_control, continous=True)
        d2, v2 = self.run_scenario(bp_veh, inp_control, continous=True, sticky="False")

        if not equal_tol(d0, d1, 1e-3) or not equal_tol(v0, v1, 1e-3):
            self.fail("%s: The default input is not sticky: Default: [%f, %f] ContinousThrottle: [%f, %f]"
                % (bp_veh.id, d0, v0, d1, v1))

        if not equal_tol(d0, d2, 1e-3) or not equal_tol(v0, v2, 1e-3):
            self.fail("%s: The default input is not sticky: Default: [%f, %f] ContinousThrottle: [%f, %f]"
                % (bp_veh.id, d0, v0, d2, v2))

    def test_true(self):
        print("TestStickyControl.test_true")

        inp_control = carla.VehicleControl(throttle=1.0)
        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")

        bp_veh = bp_vehicles[0]
        d0, v0 = self.run_scenario(bp_veh, inp_control, sticky="True")
        d1, v1 = self.run_scenario(bp_veh, inp_control, continous=True)
        d2, v2 = self.run_scenario(bp_veh, inp_control, continous=True, sticky="False")

        if not equal_tol(d0, d1, 1e-3) or not equal_tol(v0, v1, 1e-3):
            self.fail("%s: The input is not sticky: StickyTrue: [%f, %f] ContinousThrottle: [%f, %f]"
                % (bp_veh.id, d0, v0, d1, v1))

        if not equal_tol(d0, d2, 1e-3) or not equal_tol(v0, v2, 1e-3):
            self.fail("%s: The input is not sticky: StickyTrue: [%f, %f] ContinousThrottle: [%f, %f]"
                % (bp_veh.id, d0, v0, d2, v2))

    def test_false(self):
        print("TestStickyControl.test_false")

        inp_control = carla.VehicleControl(throttle=1.0)
        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")

        bp_veh = bp_vehicles[0]
        d0, v0 = self.run_scenario(bp_veh, inp_control, sticky="False")
        d1, v1 = self.run_scenario(bp_veh, inp_control, reset_after_first=True, sticky="True")
        d2, v2 = self.run_scenario(bp_veh, inp_control, reset_after_first=True, sticky="False")

        if not equal_tol(d0, d1, 1e-5) or not equal_tol(v0, v1, 1e-5):
            self.fail("%s: The input is sticky: StickyFalse: [%f, %f] Reset: [%f, %f]"
                % (bp_veh.id, d0, v0, d1, v1))

        if not equal_tol(d0, d2, 1e-5) or not equal_tol(v0, v2, 1e-5):
            self.fail("%s: The input is sticky: StickyFalse: [%f, %f] Reset: [%f, %f]"
                % (bp_veh.id, d0, v0, d2, v2))
