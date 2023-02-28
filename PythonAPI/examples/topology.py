import math
import numpy as np
import networkx as nx
import matplotlib.pyplot as plt
import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass

#from tools.misc import draw_waypoints

import carla

print(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))+ '/carla')
try:
    sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))+ '/carla')
except IndexError:
    pass


from agents.navigation.global_route_planner_dao import GlobalRoutePlannerDAO
from agents.navigation.global_route_planner import GlobalRoutePlanner
from agents.navigation.local_planner import RoadOption
from agents.tools.misc import vector
from agents.tools.misc import draw_waypoints


def main():
	#type here
	client = carla.Client('localhost', 2000)
	wld = client.get_world()
	dao = GlobalRoutePlannerDAO(wld.get_map(), sampling_resolution=sampling_resolution)
	grp = GlobalRoutePlanner(dao)
	grp.setup()
	graph = grp

	topology = dao.get_topology()
	map = wld.get_map()
	waypoint_list = map.generate_waypoints(4.5)

	print(waypoint_list[0].transform.rotation.pitch)
	waypoint_list[0].transform.rotation.pitch=180
	print(waypoint_list[0].transform.rotation.pitch)

	#print("size= ",size(topology))
	#print("topology= \n")
	#print(topology[1]['entry'])
	waypoint_new_world=[]
	while True:

		wld.tick()
		for i in range(0, len(waypoint_list)):
			#print(i)
			#waypoint_new_world.append([waypoint_list[i]])
			#print(waypoint_list[i])#Tansformation'])
			draw_waypoints(wld, [waypoint_list[i]], 2)
			#print(i)
#Waypoint(Transform(Location(x=-132.513382, y=-60.670753, z=4.460000), Rotation(pitch=360.370636, yaw=163.647781, roll=0.000000)))
	#for x in range(len(topology)): 
	#	print("----start----",topology[x], "------end------" ,"\n")


if __name__ == '__main__':

	topology=None
	graph=None
	sampling_resolution = 2

	main()