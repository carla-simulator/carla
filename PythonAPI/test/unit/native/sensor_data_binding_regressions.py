# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Actual sensor-binding regressions using native producer fixtures, without RPC."""

import gc
import unittest
import weakref

import carla
import numpy as np

# These private factories belong only to the separately built test extension.
# pylint: disable=protected-access,no-member



class TestRawBufferLifetime(unittest.TestCase):
    """Observe native weak ownership before any post-deletion payload read."""

    @staticmethod
    def factories():
        """Return actual registry-deserialized radar and camera producer fixtures."""
        return (lambda: carla._validation_radar(10.), carla._validation_image)

    def test_native_packet_storage_survives_original_python_wrapper(self):
        """A readonly raw view retains native storage without retaining its wrapper."""
        for factory in self.factories():
            with self.subTest(factory=factory):
                packet = factory()
                view = packet.raw_data
                expected = bytes(view)
                del packet
                gc.collect()
                self.assertTrue(carla._validation_native_alive())
                self.assertEqual(bytes(view), expected)
                view.release()
                self.assertFalse(carla._validation_native_alive())

    def test_chained_slices_and_casts_keep_native_storage_alive(self):
        """Slice chains share the native owner after the root view is released."""
        for factory in self.factories():
            with self.subTest(factory=factory):
                packet = factory()
                root = packet.raw_data
                expected = bytes(root)[1:-1]
                derived = memoryview(root)[1:-1].cast('B')
                del packet
                root.release()
                gc.collect()
                self.assertTrue(carla._validation_native_alive())
                self.assertEqual(bytes(derived), expected)
                derived.release()
                self.assertFalse(carla._validation_native_alive())

    def test_numpy_view_keeps_native_storage_until_the_array_is_deleted(self):
        """NumPy buffer consumers retain the same backing native allocation."""
        for factory in self.factories():
            with self.subTest(factory=factory):
                packet = factory()
                expected = bytes(packet.raw_data)
                array = np.frombuffer(packet.raw_data, dtype=np.uint8)
                del packet
                gc.collect()
                self.assertTrue(carla._validation_native_alive())
                self.assertEqual(array.tobytes(), expected)
                self.assertFalse(array.flags.writeable)
                del array
                gc.collect()
                self.assertFalse(carla._validation_native_alive())

    def test_multiple_exports_release_native_storage_only_after_the_last_view(self):
        """Independent exports and child views each keep storage alive."""
        for factory in self.factories():
            with self.subTest(factory=factory):
                packet = factory()
                first, second = packet.raw_data, packet.raw_data
                first_child, second_child = first[1:], second[1:]
                del packet
                first.release()
                second.release()
                self.assertTrue(carla._validation_native_alive())
                first_child.release()
                self.assertTrue(carla._validation_native_alive())
                second_child.release()
                self.assertFalse(carla._validation_native_alive())

    def test_readonly_format_and_contiguity_controls(self):
        """Preserve the public contiguous readonly byte-buffer contract."""
        for factory in (*self.factories(), carla._validation_flow):
            packet = factory()
            view = packet.raw_data
            self.assertTrue(view.readonly)
            self.assertEqual(view.format, 'B')
            self.assertEqual(view.ndim, 1)
            self.assertEqual(view.itemsize, 1)
            self.assertTrue(view.c_contiguous)
            with self.assertRaises(TypeError):
                view[0] = 0
            view.release()

    def test_image_updates_remain_visible_without_copying(self):
        """Image writes remain visible through the existing exported view."""
        packet = carla._validation_image()
        view = packet.raw_data
        packet[0] = carla.Color(8, 9, 10, 255)
        self.assertEqual(bytes(view[:4]), bytes((10, 9, 8, 255)))
        view.release()

    def test_empty_native_radar_view_control(self):
        """Empty packets expose a valid readonly zero-length view."""
        packet = carla._validation_radar(0., 0)
        view = packet.raw_data
        self.assertEqual(len(view), 0)
        self.assertEqual(bytes(view), b'')
        self.assertTrue(view.readonly)
        view.release()

    def test_native_python_wrapper_cycle_does_not_leak(self):
        """A wrapper dictionary can own a view without creating a reference cycle."""
        packet = carla._validation_image()
        observer = weakref.ref(packet)
        packet.retained_view = packet.raw_data
        del packet
        gc.collect()
        self.assertIsNone(observer())
        self.assertFalse(carla._validation_native_alive())

    def test_fake_image_derived_view_keeps_its_value_owner(self):
        """Color-coded flow retains its own storage independently of its wrapper."""
        fake = carla._validation_flow().get_color_coded_flow()
        native_tracking = carla._validation_track_fake(fake)
        observer = weakref.ref(fake)
        root = fake.raw_data
        expected = bytes(root)
        child = root[:].cast('B')
        del fake
        root.release()
        gc.collect()
        self.assertTrue(carla._validation_native_alive() if native_tracking else observer() is not None)
        self.assertEqual(bytes(child), expected)
        child.release()
        gc.collect()
        self.assertFalse(carla._validation_native_alive() if native_tracking else observer() is not None)

    def test_fake_image_owner_cycle_is_collected(self):
        """Color-coded flow view ownership cannot leak its Python wrapper."""
        fake = carla._validation_flow().get_color_coded_flow()
        native_tracking = carla._validation_track_fake(fake)
        observer = weakref.ref(fake)
        fake.retained_view = fake.raw_data
        del fake
        gc.collect()
        self.assertIsNone(observer())
        if native_tracking:
            self.assertFalse(carla._validation_native_alive())

    def test_fake_image_readonly_layout_control(self):
        """Keep color-coded flow dimensions and byte layout unchanged."""
        fake = carla._validation_flow().get_color_coded_flow()
        view = fake.raw_data
        self.assertEqual(len(view), fake.width * fake.height * 4)
        self.assertTrue(view.readonly)
        self.assertTrue(view.c_contiguous)
        view.release()


if __name__ == '__main__':
    unittest.main()
