# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# pylint: disable=W0401,import-self
from .libcarla import *

# Allow from carla.command import ...
import sys
sys.modules["carla.command"] = command
del sys
