# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

from . import SyncSmokeTest

import array
import carla
import time


class TestV2X(SyncSmokeTest):
    # The packaged build only ships Town10HD_Opt; the base tearDown loads
    # Town03 (absent) and would fail, so override it here.
    def tearDown(self):
        self.world.apply_settings(self.settings)
        self.world.tick()
        self.settings = None
        self.client.load_world("Town10HD_Opt")
        time.sleep(5)
        self.world = None
        self.client = None

    def wait(self, frames=20):
        for _ in range(0, frames):
            self.world.tick()

    def spawn_pair(self, gap=12.0):
        # Spawn two vehicles on the same lane, the second a safe distance ahead
        # of the first (along its forward vector) so they never overlap, and
        # close enough (~gap m, well within the V2X filter distance) to talk.
        bp = self.world.get_blueprint_library().filter("vehicle.*")[0]
        spawn_points = self.world.get_map().get_spawn_points()
        self.assertGreater(len(spawn_points), 0)
        base = spawn_points[0]
        first = self.world.spawn_actor(bp, base)
        fwd = base.get_forward_vector()
        second = None
        for distance in (gap, gap + 5.0, gap + 10.0, gap + 15.0):
            tr = carla.Transform(
                base.location + carla.Location(x=fwd.x * distance, y=fwd.y * distance, z=0.3),
                base.rotation)
            second = self.world.try_spawn_actor(bp, tr)
            if second is not None:
                break
        self.assertIsNotNone(second, "could not spawn the second vehicle without collision")
        return first, second

    def test_custom_v2x_send_and_receive(self):
        print("TestV2X.test_custom_v2x_send_and_receive")

        sender_vehicle, receiver_vehicle = self.spawn_pair()

        bp_custom = self.world.get_blueprint_library().find('sensor.other.v2x_custom')
        sender = self.world.spawn_actor(bp_custom, carla.Transform(), attach_to=sender_vehicle)
        receiver = self.world.spawn_actor(bp_custom, carla.Transform(), attach_to=receiver_vehicle)

        received = []
        receiver.listen(lambda data: received.append(data))

        self.wait(5)
        for _ in range(0, 20):
            message = carla.CustomV2XBytes()
            message.set_string("hello v2x")
            sender.send(message)
            self.world.tick()

        self.wait(5)

        sender.destroy()
        receiver.destroy()
        sender_vehicle.destroy()
        receiver_vehicle.destroy()

        self.assertGreater(len(received), 0, "custom V2X receiver did not get any message")
        first = received[0]
        self.assertGreater(first.get_message_count(), 0)
        # CustomV2XData.get() returns a nested dict; the binary payload lives at
        # ["Message"]["Message"] (DataSize / MaxDataSize / Bytes).
        payload = first[0].get()["Message"]["Message"]
        self.assertEqual(payload["DataSize"], len("hello v2x"))
        self.assertEqual(payload["Bytes"], b"hello v2x")

    def test_custom_v2x_set_bytes_uses_buffer_byte_length(self):
        print("TestV2X.test_custom_v2x_set_bytes_uses_buffer_byte_length")

        sender_vehicle, receiver_vehicle = self.spawn_pair()

        bp_custom = self.world.get_blueprint_library().find('sensor.other.v2x_custom')
        sender = self.world.spawn_actor(bp_custom, carla.Transform(), attach_to=sender_vehicle)
        receiver = self.world.spawn_actor(bp_custom, carla.Transform(), attach_to=receiver_vehicle)

        received = []
        receiver.listen(lambda data: received.append(data))

        # array.array('i', ...) has itemsize 4, so the byte length (8) differs
        # from the element count (2). set_bytes must size the copy from the byte
        # length, not the element count.
        payload = array.array('i', [0x04030201, 0x08070605])
        raw = payload.tobytes()

        self.wait(5)
        for _ in range(0, 20):
            message = carla.CustomV2XBytes()
            message.set_bytes(payload)
            sender.send(message)
            self.world.tick()

        self.wait(5)

        sender.destroy()
        receiver.destroy()
        sender_vehicle.destroy()
        receiver_vehicle.destroy()

        self.assertGreater(len(received), 0, "custom V2X receiver did not get any message")
        payload_dict = received[0][0].get()["Message"]["Message"]
        self.assertEqual(payload_dict["DataSize"], len(raw))
        self.assertEqual(payload_dict["Bytes"], raw)

    def test_v2x_cam_generation(self):
        print("TestV2X.test_v2x_cam_generation")

        first_vehicle, second_vehicle = self.spawn_pair()

        bp_cam = self.world.get_blueprint_library().find('sensor.other.v2x')
        # Force a high CAM rate so a message is generated within the tick budget.
        bp_cam.set_attribute('fixed_rate', 'true')
        bp_cam.set_attribute('gen_cam_max', '0.1')

        sender = self.world.spawn_actor(bp_cam, carla.Transform(), attach_to=first_vehicle)
        receiver = self.world.spawn_actor(bp_cam, carla.Transform(), attach_to=second_vehicle)

        received = []
        receiver.listen(lambda data: received.append(data))

        self.wait(40)

        sender.destroy()
        receiver.destroy()
        first_vehicle.destroy()
        second_vehicle.destroy()

        self.assertGreater(len(received), 0, "V2X CAM receiver did not get any message")
        self.assertGreater(received[0].get_message_count(), 0)

    def test_v2i_infrastructure_sensor_without_owner(self):
        print("TestV2X.test_v2i_infrastructure_sensor_without_owner")

        spawn_points = self.world.get_map().get_spawn_points()
        infrastructure_tr = spawn_points[0]

        bp_custom = self.world.get_blueprint_library().find('sensor.other.v2x_custom')
        # V2I: spawned standalone in the infrastructure, no vehicle parent.
        infrastructure = self.world.spawn_actor(bp_custom, infrastructure_tr)

        self.wait(5)
        for _ in range(0, 10):
            message = carla.CustomV2XBytes()
            message.set_string("infrastructure")
            infrastructure.send(message)
            self.world.tick()

        # The sensor must survive ticking and sending without an owner.
        self.assertTrue(infrastructure.is_alive)
        infrastructure.destroy()
