#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Test collisions example for CARLA
This script runs several scenarios involving collisions and check if they
are deterministic for different simulation parameters.
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


class Scenario():
    def __init__(self, client, world, save_snapshots_mode=False):
        self.world = world
        self.client = client
        self.actor_list = []
        self.init_timestamp = []
        self.active = False
        self.prefix = ""
        self.save_snapshots_mode = save_snapshots_mode
        self.snapshots = []

    def init_scene(self, prefix, settings = None, spectator_tr = None):
        self.prefix = prefix
        self.actor_list = []
        self.active = True
        self.snapshots = []

        self.reload_world(settings, spectator_tr)

        # Init timestamp
        world_snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : world_snapshot.frame, 'time0' : world_snapshot.timestamp.elapsed_seconds}

    def add_actor(self, actor, actor_name="Actor"):
        actor_idx = len(self.actor_list)

        name = str(actor_idx) + "_" + actor_name

        self.actor_list.append((name, actor))

        if self.save_snapshots_mode:
            self.snapshots.append(np.empty((0,11), float))

    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()

    def clear_scene(self):
        for actor in self.actor_list:
            actor[1].destroy()

        self.active = False

    def reload_world(self, settings = None, spectator_tr = None):
        self.client.reload_world()
        if settings is not None:
            self.world.apply_settings(settings)
        if spectator_tr is not None:
            self.reset_spectator(spectator_tr)

    def reset_spectator(self, spectator_tr):
        spectator = self.world.get_spectator()
        spectator.set_transform(spectator_tr)

    def save_snapshot(self, actor):
        snapshot = self.world.get_snapshot()

        actor_snapshot = np.array([
                float(snapshot.frame - self.init_timestamp['frame0']), \
                snapshot.timestamp.elapsed_seconds - self.init_timestamp['time0'], \
                actor.get_location().x, actor.get_location().y, actor.get_location().z, \
                actor.get_velocity().x, actor.get_velocity().y, actor.get_velocity().z, \
                actor.get_angular_velocity().x, actor.get_angular_velocity().y, actor.get_angular_velocity().z])
        return actor_snapshot

    def save_snapshots(self):
        if not self.save_snapshots_mode:
            return

        for i in range (0, len(self.actor_list)):
            self.snapshots[i] = np.vstack((self.snapshots[i], self.save_snapshot(self.actor_list[i][1])))

    def save_snapshots_to_disk(self):
        if not self.save_snapshots_mode:
            return

        for i, actor in enumerate(self.actor_list):
            np.savetxt(self.get_filename(actor[0]), self.snapshots[i])

    def get_filename_with_prefix(self, prefix, actor_id=None, frame=None):
        add_id = "" if actor_id is None else "_" + actor_id
        add_frame = "" if frame is None else ("_%04d") % frame
        return prefix + add_id + add_frame + ".out"

    def get_filename(self, actor_id=None, frame=None):
        return self.get_filename_with_prefix(self.prefix, actor_id, frame)

    def run_simulation(self, prefix, run_settings, spectator_tr, tics = 200):
        original_settings = self.world.get_settings()

        self.init_scene(prefix, run_settings, spectator_tr)

        t_start = time.perf_counter()
        for _i in range(0, tics):
            self.world.tick()
            self.save_snapshots()
        t_end = time.perf_counter()

        self.world.apply_settings(original_settings)
        self.save_snapshots_to_disk()
        self.clear_scene()

        return t_end - t_start


class TwoSpawnedCars(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(100, -257, 0.02), carla.Rotation(yaw=181.5))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(110, -253, 0.04), carla.Rotation(yaw=181.5))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(-25, 0, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)



class TwoCarsSlowSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(100, -256, 0.015), carla.Rotation(yaw=178))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-12, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+12, 0, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)


class TwoCarsHighSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -256, 0.015), carla.Rotation(yaw=180))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(40, -255, 0.04), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        self.wait(1)

        vehicle00.set_target_velocity( carla.Vector3D(-50, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+50, 0, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)


class ThreeCarsSlowSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("prius")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("a2")[0], vehicle01_tr)

        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
        vehicle02 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle02_tr)

        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-15, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+15, 0, 0))
        vehicle02.set_target_velocity(carla.Vector3D(0, -15, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")
        self.add_actor(vehicle02, "Car")

        self.wait(1)



class ThreeCarsHighSpeedCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(110, -255, 0.05), carla.Rotation(yaw=180))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("prius")[0], vehicle00_tr)

        vehicle01_tr = carla.Transform(carla.Location(53, -257, 0.00), carla.Rotation(yaw=0))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("a2")[0], vehicle01_tr)

        vehicle02_tr = carla.Transform(carla.Location(85, -230, 0.04), carla.Rotation(yaw=-90))
        vehicle02 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle02_tr)

        self.wait(1)

        vehicle00.set_target_velocity(carla.Vector3D(-30, 0, 0))
        vehicle01.set_target_velocity(carla.Vector3D(+30, 0, 0))
        vehicle02.set_target_velocity(carla.Vector3D(0, -30, 0))

        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")
        self.add_actor(vehicle02, "Car")

        self.wait(1)



class CarBikeCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = self.world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)

        bike_tr = carla.Transform(carla.Location(85, -245, 0.04), carla.Rotation(yaw=-90))
        bike = self.world.spawn_actor(blueprint_library.filter("*gazelle*")[0], bike_tr)
        self.wait(1)

        car.set_target_velocity(carla.Vector3D(+30, 0, 0))
        bike.set_target_velocity(carla.Vector3D(0, -12, 0))

        self.add_actor(car, "Car")
        self.add_actor(bike, "Bike")

        self.wait(1)



class CarWalkerCollision(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super().init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        car_tr = carla.Transform(carla.Location(50, -255, 0.04), carla.Rotation(yaw=0))
        car = self.world.spawn_actor(blueprint_library.filter("*lincoln*")[0], car_tr)

        walker_tr = carla.Transform(carla.Location(85, -255, 1.00), carla.Rotation(yaw=-90))
        walker_bp = blueprint_library.filter("walker.pedestrian.0007")[0]
        if walker_bp.has_attribute('is_invincible'):
            walker_bp.set_attribute('is_invincible', 'false')
        walker = self.world.spawn_actor(walker_bp, walker_tr)

        self.wait(1)

        car.set_target_velocity(carla.Vector3D(+20, 0, 0))
        walker.set_simulate_physics(True)
        self.add_actor(car, "Car")
        self.add_actor(walker, "Walker")

        self.wait(1)



class CollisionScenarioTester():
    def __init__(self, scene, output_path):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__
        self.output_path = output_path

    def compare_files(self, file_i, file_j):
        check_ij = filecmp.cmp(file_i, file_j)

        if check_ij:
            return True

        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)

        max_error = np.amax(np.abs(data_i-data_j))

        return max_error < 0.01

    def check_simulations(self, rep_prefixes, gen_prefix):
        repetitions = len(rep_prefixes)
        mat_check = np.zeros((repetitions, repetitions), int)

        for i in range(0, repetitions):
            mat_check[i][i] = 1
            for j in range(0, i):
                sim_check = True
                for actor in self.scene.actor_list:
                    actor_id = actor[0]
                    file_i = self.scene.get_filename_with_prefix(rep_prefixes[i], actor_id)
                    file_j = self.scene.get_filename_with_prefix(rep_prefixes[j], actor_id)

                    check_ij = self.compare_files(file_i, file_j)
                    sim_check = sim_check and check_ij
                mat_check[i][j] = int(sim_check)
                mat_check[j][i] = int(sim_check)

        determinism = np.sum(mat_check,axis=1)
        #max_rep_equal = np.amax(determinism)
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
        for actor in self.scene.actor_list:
            actor_id = actor[0]
            reference_id = "reference_" + actor_id
            file_repetition = self.scene.get_filename_with_prefix(rep_prefixes[max_idx], actor_id)
            file_reference  = self.scene.get_filename_with_prefix(prefix, reference_id)

            shutil.copyfile(file_repetition, file_reference)

        if min_idx != max_idx:
            for actor in self.scene.actor_list:
                actor_id = actor[0]
                failed_id = "failed_" + actor_id
                file_repetition = self.scene.get_filename_with_prefix(rep_prefixes[min_idx], actor_id)
                file_failed     = self.scene.get_filename_with_prefix(prefix, failed_id)

                shutil.copyfile(file_repetition, file_failed)

        for r_prefix in rep_prefixes:
            for actor in self.scene.actor_list:
                actor_id = actor[0]
                file_repetition = self.scene.get_filename_with_prefix(r_prefix, actor_id)

                os.remove(file_repetition)

    def test_scenario(self, fps=20, fps_phys=100, repetitions = 1, sim_tics = 100):
        output_str = "Testing Determinism in %s for %3d render FPS and %3d physics FPS -> " % (self.scenario_name, fps, fps_phys)

        # Creating run features: prefix, settings and spectator options
        prefix = self.output_path + self.scenario_name + "_" + str(fps) + "_" + str(fps_phys)

        config_settings = self.world.get_settings()
        config_settings.synchronous_mode = True
        config_settings.fixed_delta_seconds = 1.0/fps
        config_settings.substepping = True
        config_settings.max_substep_delta_time = 1.0/fps_phys
        config_settings.max_substeps = 16

        spectator_tr = carla.Transform(carla.Location(120, -256, 10), carla.Rotation(yaw=180))

        t_comp = 0
        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "_rep" + str(i)
            t_comp += self.scene.run_simulation(prefix_rep, config_settings, spectator_tr, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

        determ_repet = self.check_simulations(sim_prefixes, prefix)
        output_str += "Deterministic Repetitions: %r / %2d" % (determ_repet, repetitions)
        output_str += "  -> Comp. Time per frame: %.0f" % (t_comp/repetitions*sim_tics)

        if determ_repet[0] != repetitions:
            print("Error!!! Scenario %s is not deterministic: %d / %d" % (self.scenario_name, determ_repet[0], repetitions))

        return output_str



def main(arg):
    """Main function of the script"""
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(30.0)
    world = client.get_world()
    pre_settings = world.get_settings()
    world = client.load_world("Town03")

    spectator_transform = carla.Transform(carla.Location(120, -256, 5), carla.Rotation(yaw=180))
    spectator_transform.location.z += 5
    spectator = world.get_spectator()
    spectator.set_transform(spectator_transform)

    try:
        # Setting output temporal folder
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_collisions") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)


        test_list = [
            CollisionScenarioTester(TwoSpawnedCars(client, world, True), output_path),
            CollisionScenarioTester(TwoCarsSlowSpeedCollision(client, world, True), output_path),
            CollisionScenarioTester(TwoCarsHighSpeedCollision(client, world, True), output_path),
            CollisionScenarioTester(CarBikeCollision(client, world, True), output_path),
            CollisionScenarioTester(CarWalkerCollision(client, world, True), output_path),
            CollisionScenarioTester(ThreeCarsSlowSpeedCollision(client, world, True), output_path),
            CollisionScenarioTester(ThreeCarsHighSpeedCollision(client, world, True), output_path),
        ]

        repetitions = 10
        for item in test_list:
            print("--------------------------------------------------------------")
            #item.test_scenario(20,  20, repetitions)
            #item.test_scenario(20,  40, repetitions)
            #item.test_scenario(20,  60, repetitions)
            #item.test_scenario(20,  80, repetitions)
            out = item.test_scenario(20, 100, repetitions)
            print(out)

        print("--------------------------------------------------------------")

        # Remove all the output files
        #shutil.rmtree(path)


    finally:
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
#    argparser.add_argument(
#        '-fps', '--fps',
#        metavar='FPS',
#        default=20,
#        type=int,
#        help='Frames per simulatation second (default: 20)')
#    argparser.add_argument(
#        '-phys_fps', '--phys_fps',
#        metavar='PHYSFPS',
#        default=100,
#        type=int,
#        help='Target physical frames per simulatation second, it will \
#            divide the dt in substeps if required to get more precision.  (default: 100)')
    args = argparser.parse_args()

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')
