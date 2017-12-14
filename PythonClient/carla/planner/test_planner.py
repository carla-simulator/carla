from carla import sensor
from carla.sensor import Camera
from carla.settings import CarlaSettings
from carla.tcp import TCPConnectionError
from carla.util import print_over_same_line
from carla.planner.planner  import Planner

from carla.carla_server_pb2 import Control




planner= Planner('Town01')




resolution = planner._city_track._map.get_graph_resolution()

for i in range(resolution[0]):
    for j in range(resolution[1]):
        
        world_source = planner._city_track._map.make_world_node((i,j))
        source_ori = planner._city_track._map.get_lane_orientation(world_source)
        print ' Making Route from (',i, ',',j,') o (',source_ori[0],',',source_ori[1],')'

        for k in range(resolution[0]):
            for l in range(resolution[1]):
                world_target = planner._city_track._map.make_world_node((k,l))

                target_ori = planner._city_track._map.get_lane_orientation(world_target)
                print  '    To (',k, ',',l,') o (',target_ori[0],',',target_ori[1],')'

                path_distance=planner.get_shortest_path_distance(
                    world_source,source_ori,world_target,target_ori)

                print 'Distance is ',path_distance
                command=planner.get_next_command(
                    world_source,source_ori,world_target,target_ori)
                print 'Command is ',command
                print 'Latest Route ',planner._city_track._route