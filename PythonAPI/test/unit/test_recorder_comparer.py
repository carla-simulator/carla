import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


SCRIPT = Path(__file__).resolve().parents[2] / 'util' / 'recorder_comparer.py'


class TestRecorderComparer(unittest.TestCase):
    def compare(self, left, right):
        with tempfile.TemporaryDirectory() as directory:
            files = [Path(directory) / name for name in ('left.txt', 'right.txt')]
            for path, rows in zip(files, (left, right)):
                path.write_text('\n'.join(['header'] * 3 + rows) + '\n')
            return subprocess.run([sys.executable, str(SCRIPT), '-f1', str(files[0]), '-f2',
                                   str(files[1])], input='\n' * 12, capture_output=True, text=True)

    def test_trailing_rows_in_either_file(self):
        rows = ['Frame 1 at 0'] + ['payload'] * 10
        for left, right in ((rows, rows + ['extra']), (rows + ['extra'], rows)):
            with self.subTest():
                self.assertIn('extra', self.compare(left, right).stdout)

    def test_mismatch_before_first_frame(self):
        result = self.compare(['left'] + ['payload'] * 10, ['right'] + ['payload'] * 10)
        self.assertIn('left', result.stdout)
        self.assertIn('right', result.stdout)

    def test_destroy_and_platform_time_require_matching_row_types(self):
        rows = ['Frame 1 at 0'] + ['payload'] * 10
        for event in ('Destroy 1', 'Current platform time: 0'):
            with self.subTest(event=event):
                self.assertIn('unexpected', self.compare(rows + [event], rows + ['unexpected']).stdout)

    def test_create_and_position_are_distinct(self):
        rows = ['Frame 1 at 0'] + ['payload'] * 10
        self.assertIn('Create', self.compare(rows + ['Create 1: data'], rows + ['Id: 1 data']).stdout)

    def test_horizontal_whitespace_in_frame_and_payload(self):
        rows = ['payload'] * 10
        self.assertIn('Both files are identical', self.compare(
            ['Frame  1 at  0', 'Id:   9 value'] + rows, ['Frame 1 at 0', 'Id: 10 value'] + rows).stdout)
