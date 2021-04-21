# Copyright (c) 2021 Computer Vision Center (CVC) at the Universitat Autonoma de
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

    if isinstance(obj_a, carla.libcarla.Vector3D):
        diff = abs(obj_a - obj_b)
        return diff.x < tol and diff.y < tol and diff.z < tol

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

def change_physics_control(vehicle, tire_friction = None, drag = None, wheel_sweep = None):
    # Change Vehicle Physics Control parameters of the vehicle
    physics_control = vehicle.get_physics_control()

    if drag is not None:
        physics_control.drag_coefficient = drag

    if wheel_sweep is not None:
        physics_control.use_sweep_wheel_collision = wheel_sweep
    
    front_left_wheel = physics_control.wheels[0]
    rear_left_wheel = physics_control.wheels[2]
    front_right_wheel = physics_control.wheels[1]
    rear_right_wheel = physics_control.wheels[3]

    if tire_friction is not None:
        front_left_wheel.tire_friction = tire_friction
        front_right_wheel.tire_friction = tire_friction
        rear_left_wheel.tire_friction = tire_friction
        rear_right_wheel.tire_friction = tire_friction

    wheels = [front_left_wheel, front_right_wheel, rear_left_wheel, rear_right_wheel]
    physics_control.wheels = wheels

    return physics_control


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
        pc_a = change_physics_control(vehicle, tire_friction=5)
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
            pc_a.append(change_physics_control(vehicles[i], tire_friction=friction))
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
        for bp_veh in bp_vehicles:
            self.check_single_physics_control(bp_veh)

    def test_multiple_physics_control(self):
        print("TestApplyVehiclePhysics.test_multiple_physics_control")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        for idx in range(0, len(bp_vehicles)):
            self.check_multiple_physics_control(bp_vehicles, idx)

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        self.check_multiple_physics_control(bp_vehicles)


class TestVehicleFriction(SyncSmokeTest):
    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()

    def _test_vehicle_zero_friction(self):
        print("TestVehicleFriction.test_vehicle_zero_friction")

        self.client.load_world("Town05_Opt")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        for bp_veh in bp_vehicles:
            veh_transf = carla.Transform(carla.Location(35, -200, 0.1), carla.Rotation(yaw=90))

            vehicle_00 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_00.set_target_velocity(carla.Vector3D(0, 0, 0))
            vehicle_00.set_enable_gravity(True)

            veh_transf.location.x -= 4
            vehicle_01 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_01.set_target_velocity(carla.Vector3D(0, 0, 0))
            vehicle_01.set_enable_gravity(True)

            veh_transf.location.x -= 4
            veh_transf.location.z += 0.5
            vehicle_02 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_02.set_target_velocity(carla.Vector3D(0, 0, 0))
            vehicle_02.set_enable_gravity(False)

            self.wait(10)

            vel_ref = 100.0 / 3.6
            vehicle_00.apply_physics_control(change_physics_control(vehicle_00, tire_friction=0.0, drag=0.0))
            vehicle_01.apply_physics_control(change_physics_control(vehicle_01, tire_friction=0.0, drag=0.0))
            vehicle_02.apply_physics_control(change_physics_control(vehicle_02, tire_friction=0.0, drag=0.0))
            self.wait(1)
            vehicle_00.set_target_velocity(carla.Vector3D(0, vel_ref, 0))
            vehicle_01.set_target_velocity(carla.Vector3D(0, vel_ref, 0))
            vehicle_02.set_target_velocity(carla.Vector3D(0, vel_ref, 0))

            self.wait(1)

            vel_veh_00 = vehicle_00.get_velocity().y
            vel_veh_01 = vehicle_01.get_velocity().y
            vel_veh_02 = vehicle_02.get_velocity().y

            if not list_equal_tol([vel_ref, vel_veh_00, vel_veh_01, vel_veh_02], 1e-3):
                vehicle_00.destroy()
                vehicle_01.destroy()
                vehicle_02.destroy()
                self.fail("%s: Velocities are not equal after initialization. Ref: %.3f -> [%.3f, %.3f, %.3f]"
                  % (bp_veh.id, vel_ref, vel_veh_00, vel_veh_01, vel_veh_02))

            self.wait(200)

            vel_veh_00 = vehicle_00.get_velocity().y
            vel_veh_01 = vehicle_01.get_velocity().y
            vel_veh_02 = vehicle_02.get_velocity().y
            if not list_equal_tol([vel_ref, vel_veh_00, vel_veh_01, vel_veh_02], 1e-3):
                vehicle_00.destroy()
                vehicle_01.destroy()
                vehicle_02.destroy()
                self.fail("%s: Velocities are not equal after simulation. Ref: %.3f -> [%.3f, %.3f, %.3f]"
                  % (bp_veh.id, vel_ref, vel_veh_00, vel_veh_01, vel_veh_02))

            loc_veh_00 = vehicle_00.get_location().y
            loc_veh_01 = vehicle_01.get_location().y
            loc_veh_02 = vehicle_02.get_location().y
            if not list_equal_tol([loc_veh_00, loc_veh_01, loc_veh_02], 1e-3):
                vehicle_00.destroy()
                vehicle_01.destroy()
                vehicle_02.destroy()
                self.fail("%s: Locations are not equal after simulation" % bp_veh.id)

            vehicle_00.destroy()
            vehicle_01.destroy()
            vehicle_02.destroy()


    def test_vehicle_wheel_collision(self):
        print("TestVehicleFriction.test_vehicle_wheel_collision")

        self.client.load_world("Town05_Opt")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = [x for x in bp_vehicles if int(x.get_attribute('number_of_wheels')) == 4]

        for bp_veh in bp_vehicles:
            veh_transf = carla.Transform(carla.Location(35, -200, 1), carla.Rotation(yaw=90))

            vehicle_00 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_00.set_target_velocity(carla.Vector3D(0, 0, 0))

            veh_transf.location.x -= 4
            vehicle_01 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_01.set_target_velocity(carla.Vector3D(0, 0, 0))

            self.wait(10)

            vel_ref = 100.0 / 3.6
            vehicle_00.apply_physics_control(change_physics_control(vehicle_00, tire_friction=10.0, wheel_sweep = False))
            vehicle_01.apply_physics_control(change_physics_control(vehicle_01, tire_friction=10.0, wheel_sweep = False))
            self.wait(1)

            vehicle_00.set_target_velocity(carla.Vector3D(0, vel_ref, 0))
            vehicle_01.set_target_velocity(carla.Vector3D(0, vel_ref, 0))
            self.wait(1)

            self.wait(200)

            vel_veh_00 = vehicle_00.get_velocity().y
            vel_veh_01 = vehicle_01.get_velocity().y

            if not list_equal_tol([vel_veh_00, vel_veh_01], 0.1):
                vehicle_00.destroy()
                vehicle_01.destroy()
                self.fail("%s: Velocities are not equal after simulation. [%.3f, %.3f]"
                  % (bp_veh.id, vel_veh_00, vel_veh_01))

            loc_veh_00 = vehicle_00.get_location().y
            loc_veh_01 = vehicle_01.get_location().y

            if not list_equal_tol([loc_veh_00, loc_veh_01], 1):
                vehicle_00.destroy()
                vehicle_01.destroy()
                self.fail("%s: Locations are not equal after simulation. [%.3f, %.3f]"
                  % (bp_veh.id, loc_veh_00, loc_veh_01))

            vehicle_00.destroy()
            vehicle_01.destroy()

    def _test_vehicle_volume_trigger(self):
        print("TestVehicleFriction.test_vehicle_volume_trigger")

        self.client.load_world("Town05_Opt")

        bp_vehicles = self.world.get_blueprint_library().filter("vehicle.*")
        bp_vehicles = [x for x in bp_vehicles if int(x.get_attribute('number_of_wheels')) == 4]

        value_vol_friction = 3.5
        friction_bp = self.world.get_blueprint_library().find('static.trigger.friction')
        friction_bp.set_attribute('friction', str(value_vol_friction))
        extent = carla.Location(300.0, 2000.0, 700.0)
        friction_bp.set_attribute('extent_x', str(extent.x))
        friction_bp.set_attribute('extent_y', str(extent.y))
        friction_bp.set_attribute('extent_z', str(extent.z))

        vol_transf = carla.Transform(carla.Location(27, -200, 1))
        vol_transf.location.y = -120

        self.world.debug.draw_box(box=carla.BoundingBox(vol_transf.location, extent * 1e-2), rotation=vol_transf.rotation, life_time=1000, thickness=0.5, color=carla.Color(r=0,g=255,b=0))
        friction_trigger = self.world.spawn_actor(friction_bp, vol_transf)

        for bp_veh in bp_vehicles:
            veh_transf = carla.Transform(carla.Location(36, -200, 1), carla.Rotation(yaw=90))

            vehicle_00 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_00.set_target_velocity(carla.Vector3D(0, 0, 0))
            vehicle_00.set_enable_gravity(True)

            veh_transf.location.x -= 8
            vehicle_01 = self.world.spawn_actor(bp_veh, veh_transf)
            vehicle_01.set_target_velocity(carla.Vector3D(0, 0, 0))
            vehicle_01.set_enable_gravity(True)
            
            spectator_transform = carla.Transform(veh_transf.location, veh_transf.rotation)
            spectator_transform.location.z += 3
            self.world.get_spectator().set_transform(spectator_transform)

            self.wait(100)

            vel_ref = 100.0 / 3.6
            friction_ref = 0.0
            vehicle_00.apply_physics_control(change_physics_control(vehicle_00, tire_friction=friction_ref, drag=0.0))
            vehicle_01.apply_physics_control(change_physics_control(vehicle_01, tire_friction=friction_ref, drag=0.0))
            self.wait(1)
 
            vehicle_00.set_target_velocity(carla.Vector3D(0, vel_ref, 0))
            vehicle_01.set_target_velocity(carla.Vector3D(0, vel_ref, 0))
            self.wait(10)

            # Before trigger
            bef_vel_veh_00 = vehicle_00.get_velocity().y
            bef_vel_veh_01 = vehicle_01.get_velocity().y
            bef_tire_fr_00 = vehicle_00.get_physics_control().wheels[0].tire_friction
            bef_tire_fr_01 = vehicle_01.get_physics_control().wheels[0].tire_friction

            if not equal_tol(bef_vel_veh_00, vel_ref, 1e-3) or not equal_tol(bef_tire_fr_00, friction_ref, 1e-3):
                self.fail("%s: Reference vehicle has changed before trigger. Vel: %.3f [%.3f]. Fric: %.3f [%.3f]"
                  % (bp_veh.id, bef_vel_veh_00, vel_ref, bef_tire_fr_00, friction_ref))
            if not equal_tol(bef_vel_veh_01, vel_ref, 1e-3) or not equal_tol(bef_tire_fr_01, friction_ref, 1e-3):
                self.fail("%s: Check vehicle has changed before trigger. Vel: %.3f [%.3f]. Fric: %.3f [%.3f]"
                  % (bp_veh.id, bef_vel_veh_01, vel_ref, bef_tire_fr_01, friction_ref))


            self.wait(120)


            # Inside trigger
            ins_vel_veh_00 = vehicle_00.get_velocity().y
            ins_vel_veh_01 = vehicle_01.get_velocity().y
            ins_tire_fr_00 = vehicle_00.get_physics_control().wheels[0].tire_friction
            ins_tire_fr_01 = vehicle_01.get_physics_control().wheels[0].tire_friction

            if not equal_tol(ins_vel_veh_00, vel_ref, 1e-3) or not equal_tol(ins_tire_fr_00, friction_ref, 1e-3):
                self.fail("%s: Reference vehicle has changed inside trigger. Vel: %.3f [%.3f]. Fric: %.3f [%.3f]"
                  % (bp_veh.id, ins_vel_veh_00, vel_ref, ins_tire_fr_00, friction_ref))
            if ins_vel_veh_01 > vel_ref or not equal_tol(ins_tire_fr_01, value_vol_friction, 1e-3):
                self.fail("%s: Check vehicle is not correct inside trigger. Vel: %.3f [%.3f]. Fric: %.3f [%.3f]"
                  % (bp_veh.id, ins_vel_veh_01, vel_ref, ins_tire_fr_01, value_vol_friction))


            self.wait(200)


            # Outside trigger
            out_vel_veh_00 = vehicle_00.get_velocity().y
            out_vel_veh_01 = vehicle_01.get_velocity().y
            out_tire_fr_00 = vehicle_00.get_physics_control().wheels[0].tire_friction
            out_tire_fr_01 = vehicle_01.get_physics_control().wheels[0].tire_friction

            if not equal_tol(out_vel_veh_00, vel_ref, 1e-3) or not equal_tol(out_tire_fr_00, friction_ref, 1e-3):
                self.fail("%s: Reference vehicle has changed after trigger. Vel: %.3f [%.3f]. Fric: %.3f [%.3f]"
                  % (bp_veh.id, ins_vel_veh_00, vel_ref, out_tire_fr_00, friction_ref))
            if out_vel_veh_01 > vel_ref or not equal_tol(out_tire_fr_01, friction_ref, 1e-3):
                self.fail("%s: Check vehicle is not correct after trigger. Vel: %.3f [%.3f]. Fric: %.3f [%.3f]"
                  % (bp_veh.id, out_vel_veh_01, vel_ref, out_tire_fr_01, friction_ref))

            vehicle_00.destroy()
            vehicle_01.destroy()
            friction_trigger.destroy()


