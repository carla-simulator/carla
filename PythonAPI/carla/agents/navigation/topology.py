import math
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
import glob
import os
import sys

try:
    sys.path.append(glob.glob('../../dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

#from tools.misc import draw_waypoints

import carla

print(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
try:
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
except IndexError:
    pass


from navigation.global_route_planner_dao import GlobalRoutePlannerDAO
from navigation.global_route_planner2 import GlobalRoutePlanner
from navigation.local_planner import RoadOption
from tools.misc import vector
from tools.misc import draw_waypoints


def main():
	#type here
	client = carla.Client('localhost', 2000)
	wld = client.get_world()
	dao = GlobalRoutePlannerDAO(wld.get_map(), sampling_resolution=sampling_resolution)
	grp = GlobalRoutePlanner(dao)
	grp.setup()
	self._grp = grp

	topology = dao.get_topology()

	print(topology)


if __name__ == '__main__':

	topology=None
	graph=None
	sampling_resolution = 4.5

	main()