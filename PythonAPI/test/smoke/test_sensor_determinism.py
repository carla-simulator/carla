# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SmokeTest

import carla
import time
import numpy as np
import filecmp
import shutil
import os

try:
    # python 3
    from queue import Queue as Queue
    from queue import Empty
except ImportError:
    # python 2
    from Queue import Queue as Queue
    from Queue import Empty

class DeterminismError(Exception):
    pass

class Scenario(object):
    def __init__(self, client, world, save_snapshots_mode=False):
        self.world = world
        self.client = client
        self.actor_list = []
        self.init_timestamp = []
        self.active = False
        self.prefix = ""
        self.save_snapshots_mode = save_snapshots_mode
        self.snapshots = []
        self.sensor_list = []
        self.sensor_queue = Queue()

    def init_scene(self, prefix, settings = None, spectator_tr = None):
        self.prefix = prefix
        self.actor_list = []
        self.active = True
        self.snapshots = []
        self.sensor_list = []
        self.sensor_queue = Queue()

        self.reload_world(settings, spectator_tr)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

        # Init timestamp
        snapshot = self.world.get_snapshot()
        self.init_timestamp = {'frame0' : snapshot.frame, 'time0' : snapshot.timestamp.elapsed_seconds}

    def add_actor(self, actor, actor_name="Actor"):
        actor_idx = len(self.actor_list)

        name = str(actor_idx) + "_" + actor_name

        self.actor_list.append((name, actor))

        if self.save_snapshots_mode:
            self.snapshots.append(np.empty((0,11), float))

    def wait(self, frames=100):
        for _i in range(0, frames):
            self.world.tick()
            if self.active:
                for _s in self.sensor_list:
                    self.sensor_queue.get(True, 15.0)

    def clear_scene(self):
        for sensor in self.sensor_list:
            sensor[1].destroy()

        for actor in self.actor_list:
            actor[1].destroy()

        self.active = False

    def reload_world(self, settings = None, spectator_tr = None):
        if settings is not None:
            self.world.apply_settings(settings)
        if spectator_tr is not None:
            self.reset_spectator(spectator_tr)

        self.client.reload_world(False)
        # workaround: give time to UE4 to clean memory after loading (old assets)
        time.sleep(5)

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

        for _i in range(0, tics):
            self.world.tick()
            self.sensor_syncronization()
            self.save_snapshots()

        self.world.apply_settings(original_settings)
        self.save_snapshots_to_disk()
        self.clear_scene()

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
        np.savetxt(self.get_filename(name, frame), points)
        self.sensor_queue.put((lidar_data.frame, name))

    def add_semlidar_snapshot(self, lidar_data, name="SemLiDAR"):
        if not self.active:
            return

        data = np.frombuffer(lidar_data.raw_data, dtype=np.dtype([
            ('x', np.float32), ('y', np.float32), ('z', np.float32),
            ('CosAngle', np.float32), ('ObjIdx', np.uint32), ('ObjTag', np.uint32)]))
        points = np.array([data['x'], data['y'], data['z'], data['CosAngle'], data['ObjTag']]).T

        frame = lidar_data.frame - self.init_timestamp['frame0']
        np.savetxt(self.get_filename(name, frame), points)
        self.sensor_queue.put((lidar_data.frame, name))

    def add_radar_snapshot(self, radar_data, name="Radar"):
        if not self.active:
            return

        points = np.frombuffer(radar_data.raw_data, dtype=np.dtype('f4'))
        points = np.reshape(points, (int(points.shape[0] / 4), 4))

        frame = radar_data.frame - self.init_timestamp['frame0']
        np.savetxt(self.get_filename(name, frame), points)
        self.sensor_queue.put((radar_data.frame, name))

    def sensor_syncronization(self):
        # Sensor Syncronization
        w_frame = self.world.get_snapshot().frame
        for sensor in self.sensor_list:
            s_frame = self.sensor_queue.get(True, 15.0)[0]

            while s_frame < w_frame:
                s_frame = self.sensor_queue.get(True, 15.0)[0]

            if w_frame != s_frame:
                raise DeterminismError("FrameSyncError: Frames are not equal for sensor %s: %d %d"
                                       % (sensor[0], w_frame, s_frame))

class SpawnAllRaycastSensors(Scenario):
    def init_scene(self, prefix, settings = None, spectator_tr = None):
        super(SpawnAllRaycastSensors, self).init_scene(prefix, settings, spectator_tr)

        blueprint_library = self.world.get_blueprint_library()

        vehicle00_tr = carla.Transform(carla.Location(140, -205, 0.1), carla.Rotation(yaw=181.5))
        vehicle00 = self.world.spawn_actor(blueprint_library.filter("tt")[0], vehicle00_tr)
        vehicle00.set_target_velocity(carla.Vector3D(-25, 0, 0))

        vehicle01_tr = carla.Transform(carla.Location(50, -200, 0.1), carla.Rotation(yaw=1.5))
        vehicle01 = self.world.spawn_actor(blueprint_library.filter("lincoln")[0], vehicle01_tr)
        vehicle01.set_target_velocity(carla.Vector3D(25, 0, 0))

        radar_bp = self.world.get_blueprint_library().find('sensor.other.radar')
        radar_bp.set_attribute('noise_seed', '54283')
        radar_tr = carla.Transform(carla.Location(z=2))
        radar = self.world.spawn_actor(radar_bp, radar_tr, attach_to=vehicle00)

        lidar01_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar01_bp.set_attribute('noise_seed', '12134')
        lidar01_tr = carla.Transform(carla.Location(x=1, z=2))
        lidar01 = self.world.spawn_actor(lidar01_bp, lidar01_tr, attach_to=vehicle00)

        lidar02_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast_semantic')
        lidar02_tr = carla.Transform(carla.Location(x=1, z=2))
        lidar02 = self.world.spawn_actor(lidar02_bp, lidar02_tr, attach_to=vehicle01)

        lidar03_bp = self.world.get_blueprint_library().find('sensor.lidar.ray_cast')
        lidar03_bp.set_attribute('noise_seed', '23135')
        lidar03_tr = carla.Transform(carla.Location(z=2))
        lidar03 = self.world.spawn_actor(lidar03_bp, lidar03_tr, attach_to=vehicle01)

        self.add_sensor(radar, "Radar")
        self.add_sensor(lidar01, "LiDAR")
        self.add_sensor(lidar02, "SemLiDAR")
        self.add_sensor(lidar03, "LiDAR")
        self.add_actor(vehicle00, "Car")
        self.add_actor(vehicle01, "Car")

        self.wait(1)

class SensorScenarioTester():
    def __init__(self, scene, output_path):
        self.scene = scene
        self.world = self.scene.world
        self.client = self.scene.client
        self.scenario_name = self.scene.__class__.__name__
        self.output_path = output_path

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
                        file_i = self.scene.get_filename_with_prefix(rep_prefixes[i], sensor[0], f_idx)
                        file_j = self.scene.get_filename_with_prefix(rep_prefixes[j], sensor[0], f_idx)

                        check_ij = self.compare_files(file_i, file_j)
                        sim_check = sim_check and check_ij

                mat_check[i][j] = int(sim_check)
                mat_check[j][i] = int(sim_check)

        determinism = np.sum(mat_check,axis=1)

        determinism_set = list(set(determinism))
        determinism_set.sort(reverse=True)

        return determinism_set

    def test_scenario(self, repetitions = 1, sim_tics = 100):
        prefix = self.output_path + self.scenario_name

        config_settings = self.world.get_settings()
        config_settings.synchronous_mode = True
        config_settings.fixed_delta_seconds = 0.05

        spectator_tr = carla.Transform(carla.Location(160, -205, 10), carla.Rotation(yaw=180))

        sim_prefixes = []
        for i in range(0, repetitions):
            prefix_rep = prefix + "_rep_" + ("%03d" % i)
            self.scene.run_simulation(prefix_rep, config_settings, spectator_tr, tics=sim_tics)
            sim_prefixes.append(prefix_rep)

        determ_repet = self.check_simulations(sim_prefixes, sim_tics)
    
        if determ_repet[0] != repetitions:
            raise DeterminismError("SensorOutputError: Scenario %s is not deterministic: %d / %d" % (self.scenario_name, determ_repet[0], repetitions))


class TestSensorDeterminism(SmokeTest):
    def test_all_sensors(self):
        print("TestSensorDeterminism.test_all_sensors")

        orig_settings = self.world.get_settings()

        # Setting output temporal folder
        output_path = os.path.dirname(os.path.realpath(__file__))
        output_path = os.path.join(output_path, "_sensors") + os.path.sep
        if not os.path.exists(output_path):
            os.mkdir(output_path)

        try:
            test_sensors = SensorScenarioTester(SpawnAllRaycastSensors(self.client, self.world), output_path)
            test_sensors.test_scenario(repetitions=5, sim_tics = 100)
        except DeterminismError as err:
            test_sensors.scene.clear_scene()
            # Remove all the output files
            shutil.rmtree(output_path)
            self.fail(err)

        self.world.apply_settings(orig_settings)

        # Remove all the output files
        shutil.rmtree(output_path)
