import socket
import threading
import unittest

import numpy as np

from alpamayo_protocol import ProtocolError, recv_message, send_message


class ProtocolTest(unittest.TestCase):
    def test_round_trip_metadata_and_arrays(self):
        sender, receiver = socket.socketpair()
        array = np.arange(24, dtype=np.float32).reshape(2, 3, 4)
        thread = threading.Thread(
            target=send_message,
            args=(sender, "prediction", {"cot": "clear road"}, {"path": array}),
        )
        thread.start()
        message = recv_message(receiver)
        thread.join(timeout=2)
        sender.close()
        receiver.close()
        self.assertEqual(message.kind, "prediction")
        self.assertEqual(message.metadata, {"cot": "clear road"})
        np.testing.assert_array_equal(message.arrays["path"], array)

    def test_rejects_invalid_magic(self):
        sender, receiver = socket.socketpair()
        sender.sendall(b"NOPE\x01\x00\x00\x00\x00")
        with self.assertRaises(ProtocolError):
            recv_message(receiver)
        sender.close()
        receiver.close()


if __name__ == "__main__":
    unittest.main()
