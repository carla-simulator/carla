from .carla import *


# Allow from carla.command import ...
import sys

sys.modules["carla.command"] = command
del sys
