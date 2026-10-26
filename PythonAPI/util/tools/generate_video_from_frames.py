# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
# de Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Generate a video from naturally ordered image frames in a folder."""

import argparse
import glob
import mimetypes
import os

from moviepy.video.io import ImageSequenceClip
from natsort import natsorted


def argument_parser():
    """Read the existing output options and optional final-frame workaround."""
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument('--frames', required=True, help='Folder where image frames are stored')
    argparser.add_argument('--video', default='video', help='Name of the output video')
    argparser.add_argument('--fps', default=20, type=int, help='Frames per second (FPS)')
    argparser.add_argument('--skip-last-frame', action='store_true',
                           help='Exclude the final image if its render is known to be incomplete')
    return argparser.parse_args()


def image_frames(folder):
    """Exclude dataset metadata, directories and videos from the image sequence."""
    images = []
    for filename in glob.glob(os.path.join(folder, '*')):
        kind, encoding = mimetypes.guess_type(filename)
        if os.path.isfile(filename) and kind and kind.startswith('image/') and encoding is None:
            images.append(filename)
    return natsorted(images)


def main():
    """Encode every valid input frame unless the caller requests the workaround."""
    args = argument_parser()
    image_files = image_frames(args.frames)
    if args.skip_last_frame:
        image_files = image_files[:-1]
    if not image_files:
        raise ValueError('No image frames remain to encode')
    with ImageSequenceClip.ImageSequenceClip(image_files, fps=args.fps) as clip:
        clip.write_videofile(args.video + '.mp4')


if __name__ == '__main__':
    main()
