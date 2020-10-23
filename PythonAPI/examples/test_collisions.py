#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Test collisions example for CARLA
This script runs several scenarios involving collisions and check if they
are deterministic.
"""

import glob
import os
import sys
import argparse
import time
import filecmp
import shutil

import numpy as np

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

import carla


def wait(world, frames=100):
    for i in range(0, frames):
        world.tick()

class Scenario():
    def __init__(self, client, world):
        self.world = world
        self.client = client
        self.vehicle_list = []
        self.snapshots = []
        self.init_timestamp = []

    def init_scene(self):
        self.snapshots = []
        for v in self.vehicle_list:
            self.snapshots.append(np.empty((0,11), float))

        snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : snapshot.frame, 'time0' : snapshot.timestamp.elapsed_seconds}

    def clear_scene(self):
        for v in self.vehicle_list:
            v.destroy()

    def save_snapshot(self, vehicle):
        snapshot = self.world.get_snapshot()

        vehicle_snapshot = np.array([
                float(snapshot.frame - self.init_timestamp['frame0']), \
                snapshot.timestamp.elapsed_seconds - self.init_timestamp['time0'], \
                vehicle.get_velocity().x, vehicle.get_velocity().y, vehicle.get_velocity().z, \
                vehicle.get_location().x, vehicle.get_location().y, vehicle.get_location().z, \
                vehicle.get_angular_velocity().x, vehicle.get_angular_velocity().y, vehicle.get_angular_velocity().z])
        return vehicle_snapshot

    def save_snapshots(self):
        for i in range (0, len(self.vehicle_list)):
            self.snapshots[i] = np.vstack((self.snapshots[i], self.save_snapshot(self.vehicle_list[i])))
    
    def get_filename(self, prefix, i):
        return prefix + "_v" + str(i) + ".out"

    def run_simulation(self, prefix, tics = 200):
        self.init_scene()

        for i in range(0, tics):
            self.world.tick()
            self.save_snapshots()

        for i in range(0, len(self.vehicle_list)):
            np.savetxt(self.get_filename(prefix, i), self.snapshots[i])

        self.clear_scene()

class SpawnCars01(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(100, -257, 0.02), carla.Rotation(yaw=181.5))
        vehicle00 = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(110, -253, 0.04), carla.Rotation(yaw=181.5))
        vehicle01 = world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        wait(world, 1)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(-25, 0, 0))

        self.vehicle_list = []
        self.vehicle_list.append(vehicle00)
        self.vehicle_list.append(vehicle01)

        wait(world, 1)

        super().init_scene()


class CarCollision01(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle_transform = carla.Transform(carla.Location(100, -256, 0.015), carla.Rotation(yaw=178))
        vehicle = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle_transform)

        opponent_transform = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))
        opponent = world.spawn_actor(blueprint_library.filter("lincoln")[0], opponent_transform)
        wait(world, 1)

        vehicle.set_target_velocity( carla.Vector3D(-12, 0, 0))
        opponent.set_target_velocity(carla.Vector3D(+12, 0, 0))

        self.vehicle_list = []
        self.vehicle_list.append(vehicle)
        self.vehicle_list.append(opponent)

        wait(world, 1)

        super().init_scene()


class CarCollision02(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle_transform = carla.Transform(carla.Location(140, -256, 0.015), carla.Rotation(yaw=180))
        vehicle = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle_transform)

        opponent_transform = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))
        opponent = world.spawn_actor(blueprint_library.filter("lincoln")[0], opponent_transform)
        wait(world, 1)

        vehicle.set_target_velocity( carla.Vector3D(-50, 0, 0))
        opponent.set_target_velocity(carla.Vector3D(+50, 0, 0))

        self.vehicle_list = []
        self.vehicle_list.append(vehicle)
        self.vehicle_list.append(opponent)

        wait(world, 1)

        super().init_scene()


class CarCollision03(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle00 = world.spawn_actor(blueprint_library.filter("prius")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle01 = world.spawn_actor(blueprint_library.filter("a2")[0], vehicle01_tr)

        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
        vehicle02 = world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle02_tr)

        wait(world, 1)

        vehicle00.set_target_velocity(carla.Vector3D(-30, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+30, 0, 0))
        vehicle02.set_target_velocity(carla.Vector3D(0, -30, 0))

        self.vehicle_list = []
        self.vehicle_list.append(vehicle00)
        self.vehicle_list.append(vehicle01)
        self.vehicle_list.append(vehicle02)

        wait(world, 1)

        super().init_scene()

class CarCollision04(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle00 = world.spawn_actor(blueprint_library.filter("prius")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle01 = world.spawn_actor(blueprint_library.filter("a2")[0], vehicle01_tr)

        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
        vehicle02 = world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle02_tr)

        wait(world, 1)

        vehicle00.set_target_velocity(carla.Vector3D(-15, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+15, 0, 0))
        vehicle02.set_target_velocity(carla.Vector3D(0, -15, 0))

        self.vehicle_list = []
        self.vehicle_list.append(vehicle00)
        self.vehicle_list.append(vehicle01)
        self.vehicle_list.append(vehicle02)

        wait(world, 1)

        super().init_scene()


class CarBikeCollis1(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)
        wait(world, 1)

        bike_tr = carla.Transform(carla.Location(85, -245, 0.04), carla.Rotation(yaw=-90))
        bike = world.spawn_actor(blueprint_library.filter("*gazelle*")[0], bike_tr)
        wait(world, 1)

        car.set_target_velocity(carla.Vector3D(+30, 0, 0))
        bike.set_target_velocity(carla.Vector3D(0, -12, 0))

        self.vehicle_list = []
        self.vehicle_list.append(car)
        self.vehicle_list.append(bike)

        wait(world, 1)

        super().init_scene()


class CarWalkCollis1(Scenario):
    def init_scene(self):
        world = self.world

        blueprint_library = world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)
        wait(world, 1)

        walker_tr = carla.Transform(carla.Location(85, -248, 1.00), carla.Rotation(yaw=-90))
        walker_tr = carla.Transform(carla.Location(85, -255, 1.00), carla.Rotation(yaw=-90))

        walker_bp = blueprint_library.filter("walker.pedestrian.0007")[0]
        if walker_bp.has_attribute('is_invincible'):
            walker_bp.set_attribute('is_invincible', 'false')

        walker = world.spawn_actor(walker_bp, walker_tr)
        wait(world, 1)

        car.set_target_velocity(carla.Vector3D(+20, 0, 0))

        walker_control = walker.get_control()
        walker_control.direction = carla.Vector3D(0, -1, 0)
        walker_control.speed = 5
        #walker.apply_control(walker_control)
        walker.set_simulate_physics(True)

        self.vehicle_list = []
        self.vehicle_list.append(car)
        self.vehicle_list.append(walker)

        wait(world, 1)

        super().init_scene()


class TestScenario():
    def __init__(self, scene):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__

    def compare_files(self, file_i, file_j):
        check_ij = filecmp.cmp(file_i, file_j)

        if check_ij:
            return True

        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)

        max_error = np.amax(np.abs(data_i-data_j))
        #print(max_error)

        return max_error < 0.2

    def check_simulations(self, rep_prefixes, gen_prefix):
        repetitions = len(rep_prefixes)
        mat_check = np.zeros((repetitions, repetitions), int)

        for i in range(0, repetitions):
            mat_check[i][i] = 1
            for j in range(0, i):
                sim_check = True
                for veh_idx in range(0, len(self.scene.vehicle_list)):
                    file_i = self.scene.get_filename(rep_prefixes[i], veh_idx)
                    file_j = self.scene.get_filename(rep_prefixes[j], veh_idx)

                    check_ij = self.compare_files(file_i, file_j)
                    sim_check = sim_check and check_ij
                mat_check[i][j] = int(sim_check)
                mat_check[j][i] = int(sim_check)

        determinism = np.sum(mat_check,axis=1)
        max_rep_equal = np.amax(determinism)
        max_rep_equal_idx = np.argmax(determinism)
        min_rep_equal_idx = np.argmin(determinism)

        determinism_set = list(set(determinism))
        determinism_set.sort(reverse=True)

        #print(determinism)
        #print(np.argmax(determinism))
        #print(np.argmin(determinism))

        self.save_simulations(rep_prefixes, gen_prefix, max_rep_equal_idx, min_rep_equal_idx)

        return determinism_set

    def save_simulations(self, rep_prefixes, prefix, max_idx, min_idx):
        for i in range(0, len(self.scene.vehicle_list)):
            file_repetition  = self.scene.get_filename(rep_prefixes[max_idx], i)
            file_reference   = self.scene.get_filename(prefix + "reference", i)

            shutil.copyfile(file_repetition, file_reference)
            
        if min_idx != max_idx:
            for i in range(0, len(self.scene.vehicle_list)):
                file_repetition  = self.scene.get_filename(rep_prefixes[min_idx], i)
                file_failed   = self.scene.get_filename(prefix + "failed", i)

                shutil.copyfile(file_repetition, file_failed)

        for r_prefix in rep_prefixes:
            for i in range(0, len(self.scene.vehicle_list)):
                file_repetition   = self.scene.get_filename(r_prefix, i)
                
                os.remove(file_repetition)

    def test_determ_one_config(self, fps, fps_phys, repetitions = 1, sim_tics = 100):
        print("Testing Determinism in %s for %3d render FPS and %3d physics FPS -> " % \
                (self.scenario_name, fps, fps_phys),  end='')

        old_settings = self.world.get_settings()

        settings = self.world.get_settings()
        delta = 1.0/fps
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = delta

        settings.substepping = True
        settings.max_substep_delta_time = 1.0/fps_phys
        settings.max_substeps = 16
        self.world.apply_settings(settings)

        path = os.path.dirname(os.path.realpath(__file__))
        path = os.path.join(path, "_out") + os.path.sep
        prefix = path + self.scenario_name + "_" + str(fps) + "_" + str(fps_phys) + "_"

        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "rep" + str(i) 
            self.scene.run_simulation(prefix_rep, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

        determ_repet = self.check_simulations(sim_prefixes, prefix)
        print("Deterministic Repetitions: %r / %2d" % (determ_repet, repetitions))

        return



def main(arg):
    """Main function of the script"""
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(2.0)
    world = client.get_world()
    pre_settings = world.get_settings()
    world.apply_settings(pre_settings)

    fps = arg.fps
    fps_phys = arg.phys_fps

    spectator_transform = carla.Transform(carla.Location(120, -256, 5), carla.Rotation(yaw=180))
    spectator_transform.location.z += 5
    spectator = world.get_spectator()
    spectator.set_transform(spectator_transform)

    try: 
        repetitions = 20

        test00 = TestScenario(SpawnCars01(client, world))
        #test00.test_determ_one_config(20, 20, repetitions)
        #test00.test_determ_one_config(20, 40, repetitions)
        #test00.test_determ_one_config(20, 60, repetitions)
        #test00.test_determ_one_config(20, 80, repetitions)
        #test00.test_determ_one_config(20, 100, repetitions)

        testW1 = TestScenario(CarWalkCollis1(client, world))
        testW1.test_determ_one_config(20,  20, repetitions)
        testW1.test_determ_one_config(20,  40, repetitions)
        testW1.test_determ_one_config(20,  60, repetitions)
        testW1.test_determ_one_config(20,  80, repetitions)
        testW1.test_determ_one_config(20, 100, repetitions)

        print("--------------------------------------------------------------")
        test01 = TestScenario(CarCollision01(client, world))
        test01.test_determ_one_config(20,  20, repetitions)
        test01.test_determ_one_config(20,  40, repetitions)
        test01.test_determ_one_config(20,  60, repetitions)
        test01.test_determ_one_config(20,  80, repetitions)
        test01.test_determ_one_config(20, 100, repetitions)

        print("--------------------------------------------------------------")
        test02 = TestScenario(CarCollision02(client, world))
        test02.test_determ_one_config(20,  20, repetitions)
        test02.test_determ_one_config(20,  40, repetitions)
        test02.test_determ_one_config(20,  60, repetitions)
        test02.test_determ_one_config(20,  80, repetitions)
        test02.test_determ_one_config(20, 100, repetitions)

        print("--------------------------------------------------------------")
        testBike01 = TestScenario(CarBikeCollis1(client, world))
        testBike01.test_determ_one_config(20,  20, repetitions)
        testBike01.test_determ_one_config(20,  40, repetitions)
        testBike01.test_determ_one_config(20,  60, repetitions)
        testBike01.test_determ_one_config(20,  80, repetitions)
        testBike01.test_determ_one_config(20, 100, repetitions)

        print("--------------------------------------------------------------")
        test03 = TestScenario(CarCollision03(client, world))
        test03.test_determ_one_config(20, 20,  repetitions)
        test03.test_determ_one_config(20, 40,  repetitions)
        test03.test_determ_one_config(20, 60,  repetitions)
        test03.test_determ_one_config(20, 80,  repetitions)
        test03.test_determ_one_config(20, 100, repetitions)

        print("--------------------------------------------------------------")
        test04 = TestScenario(CarCollision04(client, world))
        test04.test_determ_one_config(20, 20,  repetitions)
        test04.test_determ_one_config(20, 40,  repetitions)
        test04.test_determ_one_config(20, 60,  repetitions)
        test04.test_determ_one_config(20, 80,  repetitions)
        test04.test_determ_one_config(20, 100, repetitions)

        print("--------------------------------------------------------------")


    finally:
        settings = world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = 0.0
        world.apply_settings(pre_settings)



if __name__ == "__main__":

    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='localhost',
        help='IP of the host CARLA Simulator (default: localhost)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port of CARLA Simulator (default: 2000)')
    argparser.add_argument(
        '--filter',
        metavar='PATTERN',
        default='model3',
        help='actor filter (default: "vehicle.*")')
    argparser.add_argument(
        '-fps', '--fps',
        metavar='FPS',
        default=20,
        type=int,
        help='Frames per simulatation second (default: 20)')
    argparser.add_argument(
        '-phys_fps', '--phys_fps',
        metavar='PHYSFPS',
        default=100,
        type=int,
        help='Target physical frames per simulatation second, it will \
            divide the dt in substeps if required to get more precision.  (default: 100)')
    args = argparser.parse_args()

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')
