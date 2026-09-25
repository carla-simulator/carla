# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Regression test for https://github.com/carla-simulator/carla/issues/9898:
sensor.camera.rt_lens's async readback enqueues before this tick's
CaptureScene(), so it reads back the PREVIOUS tick's pixels. Forces the
async path via the carla.RTLens.SyncModeBlockingReadback console variable
(set through the raw console_command RPC, no msgpack dependency needed),
then checks delivered frame numbers/transforms match the tick they claim.
"""

from . import SyncSmokeTest

import carla

import socket
import struct

try:
    from queue import Queue, Empty
except ImportError:
    from Queue import Queue, Empty


TESTING_ADDRESS = ('localhost', 3654)


def _pack_console_command(cmd, msgid=1):
    # msgpack-RPC request: [type=0, msgid, "console_command", [[False], cmd]]
    assert 0 <= msgid <= 0x7f
    cmd_bytes = cmd.encode('utf-8')
    method = b'console_command'
    out = bytearray()
    out += b'\x94'  # fixarray, 4 elements
    out += b'\x00'  # msgid type = request
    out += struct.pack('B', msgid)  # fixint call id
    out += bytes([0xa0 | len(method)]) + method  # fixstr
    out += b'\x92'  # fixarray, 2 elements (params)
    out += b'\x91' + b'\xc2'  # [False]
    if len(cmd_bytes) <= 0x1f:
        out += bytes([0xa0 | len(cmd_bytes)]) + cmd_bytes  # fixstr
    else:
        out += b'\xd9' + struct.pack('B', len(cmd_bytes)) + cmd_bytes  # str8
    return bytes(out)


def _unpack_console_command_reply(data, msgid=1):
    # msgpack-RPC response: [type=1, msgid, error, result: bool]
    if len(data) < 4 or data[0] != 0x94:
        raise AssertionError("console_command reply is not a 4-element msgpack array: %r" % (data,))
    if data[1] != 0x01:
        raise AssertionError("console_command reply is not a response (type byte %#x): %r" % (data[1], data))
    if data[2] != msgid:
        raise AssertionError("console_command reply msgid %d does not match request %d: %r" % (data[2], msgid, data))
    if data[3] != 0xc0:
        raise AssertionError("console_command reply carries an error (byte %#x): %r" % (data[3], data))
    if len(data) < 5 or data[4] not in (0xc2, 0xc3):
        raise AssertionError("console_command reply result is not a bool: %r" % (data,))
    return data[4] == 0xc3


def console_command(cmd, address=TESTING_ADDRESS, timeout=10.0):
    sk = socket.create_connection(address, timeout=timeout)
    try:
        sk.sendall(_pack_console_command(cmd))
        sk.settimeout(timeout)
        reply = sk.recv(1 << 16)
    finally:
        sk.close()
    ok = _unpack_console_command_reply(reply)
    if not ok:
        raise AssertionError("server rejected console_command(%r)" % (cmd,))


class TestRTLensCaptureSync(SyncSmokeTest):
    def test_rt_lens_async_frame_transform_coherence(self):
        print("TestRTLensCaptureSync.test_rt_lens_async_frame_transform_coherence")

        camera = None
        cvar_forced_async = False
        try:
            # Force the async readback path even in synchronous world mode.
            console_command("carla.RTLens.SyncModeBlockingReadback 0")
            cvar_forced_async = True

            bp_lib = self.world.get_blueprint_library()
            cam_bp = bp_lib.find('sensor.camera.rt_lens')
            cam_bp.set_attribute('image_size_x', '128')
            cam_bp.set_attribute('image_size_y', '128')
            if cam_bp.has_attribute('samples_per_pixel'):
                cam_bp.set_attribute('samples_per_pixel', '256')

            start = carla.Transform(carla.Location(x=0.0, z=10.0))
            camera = self.world.spawn_actor(cam_bp, start)

            image_queue = Queue()
            camera.listen(image_queue.put)

            transform_by_frame = {}
            num_ticks = 40
            for i in range(num_ticks):
                t = carla.Transform(carla.Location(x=float(i), z=10.0))
                camera.set_transform(t)
                self.world.tick()
                frame = self.world.get_snapshot().frame
                transform_by_frame[frame] = t
            last_ticked_frame = frame

            images = []
            while True:
                try:
                    images.append(image_queue.get(True, 6.0))
                except Empty:
                    break

            self.assertGreater(len(images), 0, "no rt_lens frames were delivered at all")

            max_delivered_frame = max(image.frame for image in images)
            # Post-fix, the last tick's readback is never enqueued within
            # this test, so no delivered frame should reach it.
            self.assertLess(
                max_delivered_frame, last_ticked_frame,
                "rt_lens delivered frame %d, the very last tick this test ran; its readback "
                "must still be one tick behind (frame/timestamp/transform pipeline is out of "
                "sync with the actual render)" % max_delivered_frame)

            for image in images:
                expected = transform_by_frame.get(image.frame)
                self.assertIsNotNone(
                    expected,
                    "rt_lens delivered frame %d, which was never ticked (known frames: %s)"
                    % (image.frame, sorted(transform_by_frame.keys())))
                self.assertAlmostEqual(
                    image.transform.location.x, expected.location.x, delta=1e-2,
                    msg="rt_lens frame %d: delivered transform x=%.3f does not match "
                        "that tick's actual x=%.3f (frame/timestamp/transform pipeline "
                        "is out of sync)" % (image.frame, image.transform.location.x,
                                              expected.location.x))

        finally:
            if camera is not None:
                camera.stop()
                camera.destroy()
            if cvar_forced_async:
                console_command("carla.RTLens.SyncModeBlockingReadback 1")
