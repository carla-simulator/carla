# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

import glob
import os
import platform
import sys

try:
    if platform.system() == 'Darwin':
        hosttype = 'macosx-*-x86_64'
    elif os.name == 'nt':
        hosttype = 'win-amd64'
    else:
        hosttype = 'linux-x86_64'
    sys.path.append(glob.glob('../../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        hosttype))[0])
except IndexError:
    pass
