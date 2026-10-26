# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Encode actual images to MP4 and count/decode the resulting FFmpeg frames."""
import importlib.util
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

AVAILABLE = all(importlib.util.find_spec(name) is not None
                for name in ('moviepy', 'natsort', 'PIL', 'imageio_ffmpeg', 'numpy'))
ROOT = Path(os.environ.get('CARLA_FRAME_VIDEO_TEST_ROOT', Path(__file__).resolve().parents[3]))
SCRIPT = ROOT / 'PythonAPI/util/tools/generate_video_from_frames.py'


def decode_frames(path):
    """Read actual encoded RGB frames through the bundled FFmpeg decoder."""
    import imageio_ffmpeg  # pylint: disable=import-outside-toplevel
    import numpy as np  # pylint: disable=import-outside-toplevel
    command = [imageio_ffmpeg.get_ffmpeg_exe(), '-v', 'error', '-i', str(path),
               '-f', 'rawvideo', '-pix_fmt', 'rgb24', '-']
    result = subprocess.run(command, capture_output=True, check=False, timeout=15)
    if result.returncode:
        raise AssertionError(result.stderr.decode('utf-8', errors='replace'))
    return list(np.frombuffer(result.stdout, dtype=np.uint8).reshape((-1, 48, 64, 3)))


@unittest.skipUnless(AVAILABLE, 'Optional MoviePy/imageio/natsort/Pillow media dependencies are required')
class TestFrameVideoInputs(unittest.TestCase):
    """Exercise the production CLI, image readers, FFmpeg writer and decoder."""

    def encode(self, count, metadata=False, skip_last=False):
        """Create real frames, invoke the CLI and read back its actual MP4."""
        from PIL import Image  # pylint: disable=import-outside-toplevel
        with tempfile.TemporaryDirectory() as directory:
            folder = Path(directory) / 'frames'
            folder.mkdir()
            colors = [(240, 10, 10), (10, 240, 10), (10, 10, 240)]
            for index, name in enumerate(('1.png', '2.png', '10.png')[:count]):
                Image.new('RGB', (64, 48), colors[index]).save(folder / name)
                if metadata:
                    (folder / (Path(name).stem + '.json')).write_text(json.dumps({'frame': index}))
            if metadata:
                (folder / 'archive').mkdir()
            command = [sys.executable, str(SCRIPT), '--frames', str(folder),
                       '--video', str(Path(directory) / 'result'), '--fps', '5']
            if skip_last:
                command.append('--skip-last-frame')
            result = subprocess.run(command, capture_output=True, text=True, check=False, timeout=30)
            self.assertEqual(result.returncode, 0, result.stderr)
            return decode_frames(Path(directory) / 'result.mp4'), colors

    def test_single_valid_frame_is_encoded(self):
        """A valid one-frame sequence must not become an empty list."""
        frames, _ = self.encode(1)
        self.assertEqual(len(frames), 1)

    def test_all_valid_frames_and_natural_order_are_preserved(self):
        """The final image and numeric ordering both survive actual encoding."""
        frames, colors = self.encode(3)
        self.assertEqual(len(frames), 3)
        for frame, color in zip(frames, colors):
            self.assertLess(max(abs(float(frame[:, :, channel].mean()) - color[channel])
                                for channel in range(3)), 8)

    def test_metadata_sidecars_and_subdirectories_are_not_frames(self):
        """Dataset JSON labels alongside valid frames must not reach the decoder."""
        frames, _ = self.encode(3, metadata=True)
        self.assertEqual(len(frames), 3)

    def test_skip_last_frame_remains_an_explicit_workaround(self):
        """Keep the older bad-final-render workaround available on request."""
        frames, _ = self.encode(3, skip_last=True)
        self.assertEqual(len(frames), 2)

    def test_existing_cli_options_remain_available_control(self):
        """The existing documented frames/video/fps interface remains intact."""
        result = subprocess.run([sys.executable, str(SCRIPT), '--help'],
                                capture_output=True, text=True, check=False, timeout=15)
        self.assertEqual(result.returncode, 0, result.stderr)
        for option in ('--frames', '--video', '--fps'):
            self.assertIn(option, result.stdout)


if __name__ == '__main__':
    unittest.main()
