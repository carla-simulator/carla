# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import contextlib
import glob
import os
import sys

with contextlib.suppress(IndexError):
    sys.path.append(
        glob.glob(
            '../../carla/dist/carla-*%d.%d-%s.egg'
            % (sys.version_info.major, sys.version_info.minor, 'win-amd64' if os.name == 'nt' else 'linux-x86_64')
        )[0]
    )
