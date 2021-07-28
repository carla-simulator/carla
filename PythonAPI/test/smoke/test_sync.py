# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import carla
import time

try:
    # python 3
    from queue import Queue as Queue
    from queue import Empty
except ImportError:
    # python 2
    from Queue import Queue as Queue
    from Queue import Empty


class TestSynchronousMode(SyncSmokeTest):
    def test_reloading_map(self):
        print("TestSynchronousMode.test_reloading_map")
        settings = carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=0.05)
        for _ in range(0, 4):
            self.world = self.client.reload_world()
            self.world.apply_settings(settings)
            # workaround: give time to UE4 to clean memory after loading (old assets)
            time.sleep(5)

    def _test_camera_on_synchronous_mode(self):
        print("TestSynchronousMode.test_camera_on_synchronous_mode")

        cam_bp = self.world.get_blueprint_library().find('sensor.camera.rgb')
        t = carla.Transform(carla.Location(z=10))
        camera = self.world.spawn_actor(cam_bp, t)
        try:

            image_queue = Queue()
            camera.listen(image_queue.put)

            frame = None

            for _ in range(0, 100):
                self.world.tick()
                ts = self.world.get_snapshot().timestamp

                if frame is not None:
                    self.assertEqual(ts.frame, frame + 1)

                frame = ts.frame

                image = image_queue.get()
                self.assertEqual(image.frame, ts.frame)
                self.assertEqual(image.timestamp, ts.elapsed_seconds)

        finally:
            camera.destroy()

    def test_sensor_transform_on_synchronous_mode(self):
        print("TestSynchronousMode.test_sensor_transform_on_synchronous_mode")
        bp_lib = self.world.get_blueprint_library()

        spawn_points = self.world.get_map().get_spawn_points()
        self.assertNotEqual(len(spawn_points), 0)

        car_bp = bp_lib.find('vehicle.ford.mustang')
        car = self.world.spawn_actor(car_bp, spawn_points[0])
        # List of sensors that are not events, these are retrieved every frame
        sensor_ids = [
            "sensor.lidar.ray_cast",
            "sensor.lidar.ray_cast_semantic",
            "sensor.other.gnss",
            "sensor.other.radar",
            "sensor.other.imu",
#            "sensor.camera.rgb",
#            "sensor.camera.depth",
#            "sensor.camera.semantic_segmentation"
            ]
        sensor_bps = [bp_lib.find(n) for n in sensor_ids]
        trans = carla.Transform(carla.Location(x=1.6, z=1.7))
        sensors = [self.world.spawn_actor(sensor, trans, car) for sensor in sensor_bps]
        queues = [Queue()] * len(sensor_bps)
        car.apply_control(carla.VehicleControl(0.75))
        # car.set_autopilot(True)

        def sensor_callback(data, name, queue):
            queue.put((data, name))

        try:
            for i in range(len(sensors)):
                sensors[i].listen(lambda data, i=i: sensor_callback(
                    data, sensor_ids[i], queues[i]))
            local_frame = 0
            for _ in range(0, 100):
                self.world.tick()
                snapshot_frame = self.world.get_snapshot().frame
                sensors_data = []

                try:
                    # Get the data once it's received
                    for queue in queues:
                        sensors_data.append(queue.get(True, 1.0))
                except Empty:
                    print("[Warning] Some sensor data has been missed")

                for i in range(len(queues)):
                    self.assertEqual(
                        queues[i].qsize(), 0, "\nQueue " + str(sensor_ids[i]) + "oversized")

                # Just in case some sensors do not have the correct transform the same frame
                # they are spawned, like the IMU.
                if local_frame < 1:
                    continue

                # All the data has been correclty retrieved
                self.assertEqual(len(sensors_data), len(sensor_bps))

                # All the sensor frame number are the same
                for sensor_data in sensors_data:
                    self.assertEqual(sensor_data[0].frame, snapshot_frame)
                # All the sensor transforms match in the snapshot and the callback
                for i in range(len(sensors_data)):
                    self.assertEqual(
                        sensors_data[i][0].transform,
                        sensors[i].get_transform(),
                        "\n\nThe sensor and sensor_data transforms from '" +
                        str(sensors_data[i][1]) + "' does not match in the same frame! (" +
                        str(local_frame) + ")\nSensor Data:\n  " +
                        str(sensors_data[i][0].transform) + "\nSensor Transform:\n  " +
                        str(sensors[i].get_transform()))
                local_frame += 1

        finally:
            for sensor in sensors:
                if sensor is not None:
                    sensor.stop()
                    sensor.destroy()
            if car is not None:
                car.destroy()

    def batch_scenario(self, batch_tick, after_tick):
        bp_veh = self.world.get_blueprint_library().filter("vehicle.*")[0]
        veh_transf = self.world.get_map().get_spawn_points()[0]

        frame_init = self.world.get_snapshot().frame

        batch = [carla.command.SpawnActor(bp_veh, veh_transf)]

        responses = self.client.apply_batch_sync(batch, batch_tick)
        if after_tick:
            self.world.tick()

        if len(responses) != 1 or responses[0].error:
            self.fail("%s: The test car could not be correctly spawned" % (bp_veh.id))

        vehicle_id = responses[0].actor_id

        frame_after = self.world.get_snapshot().frame

        self.client.apply_batch_sync([carla.command.DestroyActor(vehicle_id)])

        return frame_init, frame_after

    def test_apply_batch_sync(self):
        print("TestSynchronousMode.test_apply_batch_sync")

        a_t0, a_t1 = self.batch_scenario(False, False)
        self.assertEqual(a_t0, a_t1, "Something has failed with the apply_batch_sync. These frames should be equal: %d %d" % (a_t0, a_t1))

        a_t0, a_t1 = self.batch_scenario(True, False)
        self.assertEqual(a_t0+1, a_t1, "Something has failed with the apply_batch_sync. These frames should be consecutive: %d %d" % (a_t0, a_t1))

        a_t0, a_t1 = self.batch_scenario(False, True)
        self.assertEqual(a_t0+1, a_t1, "Something has failed with the apply_batch_sync. These frames should be consecutive: %d %d" % (a_t0, a_t1))
