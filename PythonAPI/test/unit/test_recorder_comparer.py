# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB). This work is licensed under the MIT license.

import contextlib
import importlib.util
import io
from pathlib import Path
import sys
import tempfile
import unittest
from unittest.mock import patch


def load_comparer():
    source = Path(__file__).resolve().parents[2] / "util/recorder_comparer.py"
    spec = importlib.util.spec_from_file_location("recorder_comparer", source)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


class TestRecorderComparer(unittest.TestCase):
    # These are the headers, frame/event rows and footer emitted by
    # CarlaRecorderQuery::QueryInfo, not binary recorder input.
    @staticmethod
    def report(events, frames=1):
        return ("Version: 1\nMap: Town03\nDate: 01/01/26 12:00:00\n\n"
                "Frame 1 at 0.05 seconds\n" + "\n".join(events) +
                "\n\nFrames: {}\nDuration: 0.05 seconds\n".format(frames))

    def compare(self, first, second):
        module = load_comparer()
        with tempfile.TemporaryDirectory() as directory:
            paths = [Path(directory) / name for name in ('one.txt', 'two.txt')]
            for path, text in zip(paths, (first, second)):
                path.write_text(text)
            output = io.StringIO()
            argv = ['recorder_comparer.py', '-f1', str(paths[0]), '-f2', str(paths[1])]
            with patch.object(sys, 'argv', argv), patch('builtins.input', return_value=''), \
                    contextlib.redirect_stdout(output):
                module.main()
            return output.getvalue()

    def assert_different(self, first, second):
        output = self.compare(first, second)
        self.assertIn('Line being compared:', output)
        self.assertIn('No more lines to compare', output)
        self.assertNotIn('Both files are identical', output)
        self.assertNotIn('Detected an exception', output)
        return output

    def test_short_valid_report(self):
        report = self.report([' Destroy 12'])
        self.assertIn('Both files are identical', self.compare(report, report))

    def test_short_valid_report_difference(self):
        self.assert_different(self.report([' Destroy 12'], frames=1),
                              self.report([' Destroy 12'], frames=2))

    def test_invalid_server_response_is_rejected(self):
        with self.assertRaisesRegex(ValueError, '[Rr]ecorder'):
            self.compare('File not found on server', 'File not found on server')

    def test_unequal_valid_report_lengths_in_both_orders(self):
        common = [' Positions: 7'] + [' Id: 1 Location: (1, 2, 3)'] * 7
        first = self.report(common + ['Frame 2 at 0.05 seconds',
                                      ' Positions: 0', ' Current visual time: 0.05'], frames=2)
        second = self.report(common)
        for left, right in ((first, second), (second, first)):
            with self.subTest(first_longer=len(left) > len(right)):
                output = self.assert_different(left, right)
                self.assertIn('Duration:', output)

    def test_record_categories_cannot_be_hidden_by_id_masking(self):
        common = [' Id: 1 Location: (1, 2, 3)'] * 7
        pairs = [(' Destroy 1', ' Current platform time: 2'),
                 (' Current platform time: 1', ' Destroy 2')]
        for first, second in pairs:
            with self.subTest(first=first):
                self.assert_different(self.report(common + [first]),
                                      self.report(common + [second]))

    def test_same_category_variable_ids_and_platform_times_stay_ignored(self):
        first = [' Create 1: vehicle.test', ' Id: 1 Location: (1, 2, 3)',
                 ' Destroy 1', ' Current platform time: 123'] * 2
        second = [' Create 20: vehicle.test', ' Id: 20 Location: (1, 2, 3)',
                  ' Destroy 20', ' Current platform time: 456'] * 2
        self.assertIn('Both files are identical', self.compare(self.report(first), self.report(second)))


if __name__ == '__main__':
    unittest.main()
