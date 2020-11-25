#!/usr/bin/env python

# Copyright (c) 2020 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Check raycast sensors determinism for CARLA
This script spawn all the raycast sensors in a simple scenario and check if their
output are deterministic.
"""

import glob
import os
import sys
import argparse
import time
import filecmp
import shutil
from queue import Queue
from queue import Empty

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
    def __init__(self, client, world):
        self.world = world
        self.client = client
        self.actor_list = []
        self.sensor_list = []
        self.init_timestamp = []
        self.sensor_queue = Queue()
        self.active = False
        self.prefix = ""

    def init_scene(self, prefix):
        self.prefix = prefix
        self.actor_list = []
        self.sensor_list = []
        self.sensor_queue = Queue()
        self.active = True

        snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : snapshot.frame, 'time0' :
                snapshot.timestamp.elapsed_seconds}

    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()
            if self.active:
                for s in self.sensor_list:
                    self.sensor_queue.get(True, 1.0)

    def add_sensor(self, sensor, sensor_type):

        sen_idx = len(self.sensor_list)
        if sensor_type == "LiDAR":
            name = str(sen_idx) + "_LiDAR"
            sensor.listen(lambda data : self.add_lidar_snapshot(data, name))
        elif sensor_type == "SemLiDAR":
            name = str(sen_idx) + "_SemLiDAR"
            sensor.listen(lambda data : self.add_semlidar_snapshot(data, name))
        elif sensor_type == "Radar":
            name = str(sen_idx) + "_Radar"
            sensor.listen(lambda data : self.add_radar_snapshot(data, name))

        self.sensor_list.append((name, sensor))


    def add_lidar_snapshot(self, lidar_data, name="LiDAR"):
        if not self.active:
            return

        points = np.frombuffer(lidar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))

        frame = lidar_data.frame - self.init_timestamp['frame0']
        time_f = lidar_data.timestamp - self.init_timestamp['time0']

        np.savetxt(self.get_filename(frame, name), points)

        self.sensor_queue.put((lidar_data.frame, name))

    def add_semlidar_snapshot(self, lidar_data, name="SemLiDAR"):
        if not self.active:
            return

        data = np.frombuffer(lidar_data.raw_data, dtype=np.dtype([
            ('x', np.float32), ('y', np.float32), ('z', np.float32),
            ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))

        points = np.array([data['x'], data['y'], data['z'], data['CosAngle'], data['ObjTag']]).T

        frame = lidar_data.frame - self.init_timestamp['frame0']
        time_f = lidar_data.timestamp - self.init_timestamp['time0']

        np.savetxt(self.get_filename(frame, name), points)

        self.sensor_queue.put((lidar_data.frame, name))

    def add_radar_snapshot(self, radar_data, name="Radar"):
        if not self.active:
            return

        points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))

        frame = radar_data.frame - self.init_timestamp['frame0']
        time_f = radar_data.timestamp - self.init_timestamp['time0']

        np.savetxt(self.get_filename(frame, name), points)

        self.sensor_queue.put((radar_data.frame, name))

    def clear_scene(self):
        for sensor in self.sensor_list:
            sensor[1].destroy()

        for actor in self.actor_list:
            actor.destroy()

        self.active = False

    def get_filename_with_prefix(self, prefix, f, id=""):
        add_id = "" if id=="" else id + "_"
        return prefix + add_id + ("%04d") % f + ".out"

    def get_filename(self, f, id=""):
        add_id = "" if id=="" else id + "_"
        return self.prefix + add_id + ("%04d") % f + ".out"

    def run_simulation(self, prefix, tics = 200):
        self.init_scene(prefix)

        for _i in range(0, tics):
            self.world.tick()
            w_frame = self.world.get_snapshot().frame

            for s in self.sensor_list:
                s_frame = self.sensor_queue.get(True, 1.0)[0]

                if w_frame != s_frame:
                    print("Error!!! frames are not equal for %d: %d %d" % (s[0], w_frame, s_frame))

        self.clear_scene()


class SpawnLidarNoDropff(Scenario):
    def init_scene(self, prefix):
        super().init_scene(prefix)

        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        lidar_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar_bp.set_attribute('channels', '64')
        lidar_bp.set_attribute('points_per_second', '200000')
        lidar_bp.set_attribute('dropoff_general_rate', '0.0')
        lidar_bp.set_attribute('noise_seed', '43233')

        lidar_tr = carla.Transform(carla.Location(z=2))
        lidar = world.spawn_actor(lidar_bp, lidar_tr, attach_to=vehicle00)

        self.add_sensor(lidar, "LiDAR")
        self.actor_list.append(vehicle00)

        self.wait(1)

class SpawnSemanticLidar(Scenario):
    def init_scene(self, prefix):
        super().init_scene(prefix)

        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        lidar_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
        lidar_bp.set_attribute('channels', '64')
        lidar_bp.set_attribute('points_per_second', '200000')

        lidar_tr = carla.Transform(carla.Location(z=2))
        lidar = world.spawn_actor(lidar_bp, lidar_tr, attach_to=vehicle00)

        self.add_sensor(lidar, "SemLiDAR")
        self.actor_list.append(vehicle00)

        self.wait(1)

class SpawnRadar(Scenario):

    def init_scene(self, prefix):
        super().init_scene(prefix)

        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        radar_bp = world.get_blueprint_library().find('sensor.other.radar')
        radar_bp.set_attribute('noise_seed', '54283')

        radar_tr = carla.Transform(carla.Location(z=2))
        radar = world.spawn_actor(radar_bp, radar_tr, attach_to=vehicle00)

        self.add_sensor(radar, "Radar")
        self.actor_list.append(vehicle00)

        self.wait(1)

class SpawnLidarWithDropff(Scenario):
    def init_scene(self, prefix):
        super().init_scene(prefix)

        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)

        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        lidar_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar_bp.set_attribute('channels', '64')
        lidar_bp.set_attribute('points_per_second', '200000')
        lidar_bp.set_attribute('noise_seed', '249013')

        lidar_tr = carla.Transform(carla.Location(z=2))
        lidar = world.spawn_actor(lidar_bp, lidar_tr, attach_to=vehicle00)

        self.add_sensor(lidar, "LiDAR")
        self.actor_list.append(vehicle00)

        self.wait(1)

class SpawnAllRaycastSensors(Scenario):

    def init_scene(self, prefix):
        super().init_scene(prefix)

        world = self.world

        blueprint_library = world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)
        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        vehicle01_tr = carla.Transform(carla.Location(50, -200, 0.1), carla.Rotation(yaw=-178.5))
        vehicle01 = world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        vehicle01.set_target_velocity(carla.Vector3D(+25, 0, 0))

        radar_bp = world.get_blueprint_library().find('sensor.other.radar')
        radar_bp.set_attribute('noise_seed', '54283')
        radar_tr = carla.Transform(carla.Location(z=2))
        radar = world.spawn_actor(radar_bp, radar_tr, attach_to=vehicle00)

        lidar01_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar01_bp.set_attribute('noise_seed', '12134')
        lidar01_tr = carla.Transform(carla.Location(x=1, z=2))
        lidar01 = world.spawn_actor(lidar01_bp, lidar01_tr, attach_to=vehicle00)

        lidar02_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
        lidar02_tr = carla.Transform(carla.Location(x=1, z=2))
        lidar02 = world.spawn_actor(lidar02_bp, lidar02_tr, attach_to=vehicle01)

        lidar03_bp = world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar03_bp.set_attribute('noise_seed', '23135')
        lidar03_tr = carla.Transform(carla.Location(z=2))
        lidar03 = world.spawn_actor(lidar03_bp, lidar03_tr, attach_to=vehicle01)

        self.add_sensor(radar, "Radar")
        self.add_sensor(lidar01, "LiDAR")
        self.add_sensor(lidar02, "SemLiDAR")
        self.add_sensor(lidar03, "LiDAR")
        self.actor_list.append(vehicle00)
        self.actor_list.append(vehicle01)

        self.wait(1)

class TestScenario():
    def __init__(self, scene):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__

    def compare_files(self, file_i, file_j):

        # First, we check if the files are exactly equal,
        # if they are the simulations are equivalent
        check_ij = filecmp.cmp(file_i, file_j)
        if check_ij:
            return True

        # If not, if we have different number of points
        # the simulations are not equivalent
        data_i = np.loadtxt(file_i)
        data_j = np.loadtxt(file_j)
        if data_i.shape != data_j.shape:
            return False

        # If they have the same number of points but there is
        # a small diference, the simulations could be equivalent
        # differing only in floaring-point arithmetic errors
        max_error = np.amax(np.abs(data_i-data_j))

        return max_error < 0.01

    def check_simulations(self, rep_prefixes, sim_tics):
        repetitions = len(rep_prefixes)
        mat_check = np.zeros((repetitions, repetitions), int)

        for i in range(0, repetitions):
            mat_check[i][i] = 1
            for j in range(0, i):
                sim_check = True
                for f_idx in range(1, sim_tics):
                    for sensor in self.scene.sensor_list:
                        file_i = self.scene.get_filename_with_prefix(rep_prefixes[i], f_idx, sensor[0])
                        file_j = self.scene.get_filename_with_prefix(rep_prefixes[j], f_idx, sensor[0])

                        check_ij = self.compare_files(file_i, file_j)
                        sim_check = sim_check and check_ij

                mat_check[i][j] = int(sim_check)
                mat_check[j][i] = int(sim_check)

        determinism = np.sum(mat_check,axis=1)

        determinism_set = list(set(determinism))
        determinism_set.sort(reverse=True)

        return determinism_set

    def test_scenario(self, repetitions = 1, sim_tics = 100):
        print("Testing Determinism in %s -> " % \
                (self.scenario_name),  end='')

        settings = self.world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        self.world.apply_settings(settings)

        path = os.path.dirname(os.path.realpath(__file__))
        path = os.path.join(path, "_sensors") + os.path.sep
        if not os.path.exists(path):
            os.mkdir(path)

        prefix = path + self.scenario_name

        t_start = time.perf_counter()
        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "_rep_" + ("%03d" % i) + "_"
            self.scene.run_simulation(prefix_rep, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

        t_end = time.perf_counter()

        determ_repet = self.check_simulations(sim_prefixes, sim_tics)
        print("Deterministic Repetitions: %r / %2d" % (determ_repet, repetitions), end="")
        print("  -> Comp. FPS: %.0f" % ((repetitions*sim_tics)/(t_end-t_start)))
        return


def main(arg):
    """Main function of the script"""
    client = carla.Client(arg.host, arg.port)
    client.set_timeout(2.0)
    world = client.get_world()
    pre_settings = world.get_settings()
    world.apply_settings(pre_settings)

    spectator_transform = carla.Transform(carla.Location(160, -205, 5), carla.Rotation(yaw=180))
    spectator_transform.location.z += 5
    spectator = world.get_spectator()
    spectator.set_transform(spectator_transform)

    try:
        repetitions = 10
        test_all = TestScenario(SpawnAllRaycastSensors(client, world))
        test_all.test_scenario(repetitions)

        test_lidar_1 = TestScenario(SpawnLidarNoDropff(client, world))
        test_lidar_1.test_scenario(repetitions)

        test_lidar_2 = TestScenario(SpawnLidarWithDropff(client, world))
        test_lidar_2.test_scenario(repetitions)

        test_semlidar = TestScenario(SpawnSemanticLidar(client, world))
        test_semlidar.test_scenario(repetitions)

        test_radar = TestScenario(SpawnRadar(client, world))
        test_radar.test_scenario(repetitions)

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
    args = argparser.parse_args()

    try:
        main(args)
    except KeyboardInterrupt:
        print(' - Exited by user.')

