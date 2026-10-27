# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Exercise native OpenDRIVE exports without a running simulator."""

import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

import carla


OPEN_DRIVE = '''<OpenDRIVE><header revMajor="1" revMinor="4"><geoReference>
+proj=tmerc +lat_0=0 +lon_0=0 +k=1 +x_0=0 +y_0=0 +datum=WGS84 +units=m
</geoReference></header><road id="1" length="100" junction="-1"><planView>
<geometry s="0" x="0" y="0" hdg="0" length="100"><line/></geometry></planView>
<lanes><laneSection s="0"><center><lane id="0" type="none"/></center><right>
<lane id="-1" type="driving"><width sOffset="0" a="3.5" b="0" c="0" d="0"/>
</lane></right></laneSection></lanes></road></OpenDRIVE>'''


class TestMapSaveIO(unittest.TestCase):
    """Use the real Map constructor, parser and native save_to_disk binding."""

    def setUp(self):
        self.native_map = carla.Map('map-save-output', OPEN_DRIVE)

    def test_existing_directory_target_reports_open_failure(self):
        """An output directory must not look like a successfully saved file."""
        with tempfile.TemporaryDirectory(prefix='carla-map-save-') as directory:
            target = Path(directory) / 'blocked.xodr'
            target.mkdir()
            with self.assertRaises(RuntimeError):
                self.native_map.save_to_disk(str(target))
            self.assertTrue(target.is_dir())
            self.assertEqual(list(target.iterdir()), [])

    def test_nested_parent_and_extension_control(self):
        """Retain parent creation, extension replacement and successful contents."""
        with tempfile.TemporaryDirectory(prefix='carla-map-save-') as directory:
            requested = Path(directory) / 'nested' / 'map.txt'
            self.assertIsNone(self.native_map.save_to_disk(str(requested)))
            target = requested.with_suffix('.xodr')
            self.assertEqual(target.read_text(encoding='utf-8'), OPEN_DRIVE + '\n')
            self.assertFalse(requested.exists())

    def test_existing_file_overwrite_control(self):
        """A healthy export still truncates an existing output file."""
        with tempfile.TemporaryDirectory(prefix='carla-map-save-') as directory:
            target = Path(directory) / 'overwrite.xodr'
            target.write_text('old data' * 1000, encoding='utf-8')
            self.assertIsNone(self.native_map.save_to_disk(str(target)))
            self.assertEqual(target.read_text(encoding='utf-8'), OPEN_DRIVE + '\n')

    @unittest.skipUnless(os.name == 'posix', 'Requires POSIX file-size limits')
    def test_failed_flush_reports_write_failure(self):
        """Apply a real file-size limit only inside an isolated child process."""
        origin = Path(carla.__file__).resolve()
        module_root = origin.parent.parent if hasattr(carla, '__path__') else origin.parent
        program = '''
import resource
import signal
import sys
sys.path.insert(0, sys.argv[1])
import carla
native_map = carla.Map('limited-map-save', sys.argv[2])
previous_limit = resource.getrlimit(resource.RLIMIT_FSIZE)
previous_signal = signal.signal(signal.SIGXFSZ, signal.SIG_IGN)
failed = False
try:
    hard = previous_limit[1]
    limit = 128 if hard == resource.RLIM_INFINITY else min(128, hard)
    resource.setrlimit(resource.RLIMIT_FSIZE, (limit, hard))
    try:
        native_map.save_to_disk(sys.argv[3])
    except RuntimeError:
        failed = True
finally:
    resource.setrlimit(resource.RLIMIT_FSIZE, previous_limit)
    signal.signal(signal.SIGXFSZ, previous_signal)
assert failed, 'An actual failed OpenDRIVE write did not raise'
print('map-write-failure-reported')
'''
        with tempfile.TemporaryDirectory(prefix='carla-map-save-') as directory:
            result = subprocess.run(
                [sys.executable, '-I', '-B', '-c', program, str(module_root), OPEN_DRIVE,
                 str(Path(directory) / 'limited.xodr')], capture_output=True, text=True,
                encoding='utf-8', errors='replace', timeout=30, check=False)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn('map-write-failure-reported', result.stdout)


if __name__ == '__main__':
    unittest.main()
