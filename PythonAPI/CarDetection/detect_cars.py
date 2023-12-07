import pygame
import carla
import random
import time
import math
import collections
from collections import Counter
import pandas as pd
import itertools
from copy import deepcopy
import warnings
import numpy as np
import random
warnings.filterwarnings("ignore")
from utils import *
import sys
import os

if __name__ == "__main__":
    # config:
    distance_to_junc = 20
    direction_angle = 20
    radius = 100

    # First of all, we need to create the client that will send the requests
    # to the simulator. Here we'll assume the simulator is accepting
    # requests in the localhost at port 2000.
    client = carla.Client("localhost", 2000)
    print("client found")
    # Get traffic manager
    traffic_manager = client.get_trafficmanager()

    # Once we have a client we can retrieve the world that is currently running.
    world = client.get_world()

    # Set synchronous mode settings
    new_settings = world.get_settings()
    new_settings.synchronous_mode = True
    new_settings.fixed_delta_seconds = 0.3
    world.apply_settings(new_settings)

    world_map = world.get_map()

    # The world contains the list blueprints that we can use for adding new actors into the simulation.
    blueprint_library = world.get_blueprint_library()

    # Now let's filter all the blueprints of type 'vehicle' and choose one at random.
    bp = random.choice(blueprint_library.filter("vehicle"))

    # Now we need to give an initial transform to the vehicle. We choose a
    # random transform from the list of recommended spawn points of the map.
    transform = world.get_map().get_spawn_points()[0]

    # So let's tell the world to spawn the vehicle.
    ego_vehicle = world.spawn_actor(bp, transform)

    vehicle_type = ego_vehicle.type_id


    # Let's put the vehicle to drive around.
    ego_vehicle.set_autopilot(True)  # ToDo Use Carla KI

    # Make him a speeder :)
    # traffic_manager.vehicle_percentage_speed_difference(ego_vehicle, -30)

    # Define the radius to search for other vehicles
    radius = 200

    # Initialize speed of ego_vehicle to use as global variable
    speed = 0
    world.tick()
    junction = None
    same_junction = False
    on_junction = False
    junctions_detected = False
    junction_old = None
    wps_old = None
    highway_shape_old = None
    highway_shape = None
    wrong_shape = None
    junction_shape = None
    matrix = None
    junction_id = None
    street_type = None 
    road_lane_ids = get_all_road_lane_ids(world_map = world.get_map())
    on_entry = False
    junction_id_skip = None
    ego_on_bad_highway_street = False
    highway_junction = False
    highway_forward_vector = None
    df = initialize_dataframe()
    df_errors = initialize_error_dataframe()
    t_end = time.time() + 10
    while time.time() < t_end:
        print(time.time())
        try:
            follow_car(ego_vehicle, world)
            ego_location = ego_vehicle.get_location()
            ego_waypoint = world_map.get_waypoint(ego_location)
            ego_on_highway = check_ego_on_highway(ego_location, road_lane_ids, world_map)
            if not ego_on_highway:
                left_location = get_wp_x_units_to_left(ego_vehicle, 6)
                ego_on_highway = check_ego_on_highway(left_location, road_lane_ids, world_map)
                if ego_on_highway:
                    ego_on_bad_highway_street = True
            else:
                ego_on_bad_highway_street = False
            if ego_on_highway:
                distance_to_junc = 80
            else:
                distance_to_junc = 20
            
            if junctions_detected and not ego_waypoint.is_junction:
                junctions_detected = False 
                same_junction = False
                continue
            elif junctions_detected:
                pass
            elif ego_waypoint.is_junction and on_junction == False:
                on_junction = True
            elif on_junction:
                junction_old = junction
                wps_old = wps
                highway_shape_old = highway_shape
                on_junction = False
                junctions_detected = True
            elif is_junction_ahead(ego_waypoint, distance_to_junc) and not on_junction:
                junction = get_junction_ahead(ego_waypoint, distance_to_junc)
                junction_id = junction.id
                highway_junction = is_highway_junction(ego_vehicle, ego_waypoint, junction, road_lane_ids, direction_angle)

            current_lanes = []
            for id in road_lane_ids:
                if str(ego_waypoint.road_id) == id.split("_")[0]:
                    current_lanes.append(int(id.split("_")[1]))

            # CASES
            # 1. On city junction
            if (ego_waypoint.is_junction and not highway_junction and get_junction_ahead(ego_waypoint, distance_to_junc) and (get_junction_ahead(ego_waypoint, distance_to_junc).id not in [459, 870] or ego_waypoint.get_junction().id in [483, 870, 1249]))or (junction and junction.id == 1368 and ego_waypoint.is_junction):
                street_type = "On junction"

                if junction_shape:
                    matrix = detect_surrounding_cars_outside_junction(
                        deepcopy(junction_shape),
                        junction_roads_junction,
                        lanes_all_junction,
                        ego_vehicle,
                        world,
                        distance_to_junc,
                        junction
                    )
                    matrix = detect_cars_inside_junction(
                        matrix, ego_vehicle, wps, yaw, world
                    )
            
            # 2. City junction ahead
            elif (is_junction_ahead(ego_waypoint, distance_to_junc) and not highway_junction and get_junction_ahead(ego_waypoint, distance_to_junc) and get_junction_ahead(ego_waypoint, distance_to_junc).id not in [459, 870]) or (ego_waypoint.road_id in [2, 593, 3, 12, 13, 879, 880, 886, 467, 477, 468] and get_junction_ahead(ego_waypoint.next(20)[0], distance_to_junc) and get_junction_ahead(ego_waypoint.next(20)[0], distance_to_junc).id not in [459, 870] and not highway_junction) or (junction and junction.id == 1368 and is_junction_ahead(ego_waypoint, distance_to_junc)):
                street_type = "Junction ahead"
                if get_junction_ahead(ego_waypoint, distance_to_junc) and get_junction_ahead(ego_waypoint, distance_to_junc).id in [459, 870]:
                    junction = get_junction_ahead(ego_waypoint.next(20)[0], distance_to_junc)
                else:
                    junction = get_junction_ahead(ego_waypoint, distance_to_junc)
                wps = junction.get_waypoints(carla.LaneType().Driving)
                if junction.id == 1368: # traffic light junction logic
                    if ego_on_highway: # already on highway traffic light junction logic
                        street_type = "Highway traffic light"
                        starting_wp = get_closest_starting_waypoint(wps, ego_location)
                        wps = remove_wps(starting_wp, wps)
                        intial_wp = ego_waypoint
                    else: # entrying highway traffic light jucntion
                        starting_wp, middle_location = get_waypoint_on_highway_junction(junction, ego_waypoint, ego_location, wps, world_map)
                        wps = remove_wps(starting_wp, wps)
                        intial_wp = starting_wp.previous(5)[0]
                else: # Every other junction
                    street_type = "Junction ahead"
                    intial_wp = ego_waypoint

                junction_shape, lanes_all_junction, junction_roads_junction, yaw, = get_Junction_Shape(
                    ego_vehicle, intial_wp, wps, road_lane_ids, direction_angle, world_map
                )

                if junction_shape != "Error":
                    #print("Junction ahead")
                    
                    matrix = detect_ego_before_junction(
                        deepcopy(junction_shape),
                        junction_roads_junction,
                        lanes_all_junction,
                        ego_waypoint.lane_id,
                        ego_waypoint,
                        distance_to_junc,
                        junction
                    )
                    matrix = detect_surrounding_cars_outside_junction(
                        matrix,
                        junction_roads_junction,
                        lanes_all_junction,
                        ego_vehicle,
                        world,
                        distance_to_junc,
                        junction
                    )
                    matrix = detect_cars_inside_junction(
                        matrix, ego_vehicle, wps, yaw, world
                    )

            # 3. Highway entry/exit in front or behind
            elif (is_junction_ahead(ego_waypoint, 80) or (is_junction_behind(ego_waypoint, 40) and street_type == "Highway with entry/exit")) and not on_entry and highway_junction and ego_on_highway and junction_id_skip != junction.id and not check_ego_exit_highway(ego_vehicle, ego_waypoint, highway_forward_vector, highway_shape, current_lanes):
                street_type = "Highway with entry/exit"
                if distance(ego_location, junction.bounding_box.location) < 30 and not forward_vector_set:
                    highway_forward_vector = ego_vehicle.get_transform().get_forward_vector()
                    highway_forward_location = ego_vehicle.get_location()
                    forward_vector_set = True
                    exit_over = False
                if not same_junction and is_junction_ahead(ego_waypoint, 80):
                    forward_vector_set = False
                    same_junction = True
                    junction = get_junction_ahead(ego_waypoint, 80)

                    wps = junction.get_waypoints(carla.LaneType().Driving)
                    closest_waypoint = get_closest_starting_waypoint(wps, ego_location)
                    wps = prepare_waypoints(closest_waypoint, wps, ego_waypoint)
                    highway_shape = get_highway_shape(wps, junction, ego_waypoint)

                if (highway_shape is None) and (not junction_old is None):
                    junction = junction_old
                    wps = wps_old
                    highway_shape = highway_shape_old
                    wrong_shape = True
                if ego_on_bad_highway_street:
                    matrix = create_city_matrix(left_location, road_lane_ids, world_map, True, ego_on_bad_highway_street)
                else:
                    matrix = create_city_matrix(ego_location, road_lane_ids, world_map, False, ego_on_bad_highway_street)                
                if matrix:
                    matrix, cars_on_entryExit = detect_surronding_cars(
                        ego_location, ego_vehicle, matrix, road_lane_ids, world, radius, ego_on_highway, highway_shape
                    )
                    if not highway_shape is None and (highway_shape[0] != "normal_highway" or wrong_shape):
                        matrix = update_matrix(world_map, ego_vehicle, ego_location, highway_shape, wps, matrix, ego_waypoint, junction, cars_on_entryExit, direction_angle) 
                        if not junction_old is None and junction_old.id != junction.id:
                            matrix = update_matrix(world_map, ego_vehicle, ego_location, highway_shape_old, wps_old, matrix, ego_waypoint, junction_old, cars_on_entryExit, direction_angle, is_2nd_update=True) 
                        

            # 4. ego exiting Highway
            elif check_ego_exit_highway(ego_vehicle, ego_waypoint, highway_forward_vector, highway_shape, current_lanes) and not exit_over:
                #print("On highway exit")
                street_type = "On highway exit"
                if highway_forward_vector and distance(highway_forward_location, ego_location) > 60:
                    highway_forward_vector = None
                    highway_forward_location = None
                
                junction = ego_waypoint.get_junction()
                if not junction:
                    exit_over = True
                    continue
                junction_id_skip = junction.id

                wps = junction.get_waypoints(carla.LaneType().Driving)
                closest_waypoint, middle_location = get_waypoint_on_highway_junction(junction, ego_waypoint, ego_location, wps, world_map)
                wps = prepare_waypoints(closest_waypoint, wps, ego_waypoint)
                highway_shape = get_highway_shape(wps, junction, ego_waypoint)

                on_entry = True
                if (highway_shape is None) and (not junction_old is None):
                    same_junction = False
                    junction = junction_old
                    wps = wps_old
                    highway_shape = highway_shape_old


                matrix = create_city_matrix(middle_location, road_lane_ids, world_map, ghost=True)
                if matrix:
                    matrix, cars_on_entryExit = detect_surronding_cars(
                        middle_location, ego_vehicle, matrix, road_lane_ids, world, radius, True, highway_shape, ghost=True
                    )

                    if not highway_shape is None:
                        matrix  = update_matrix(world_map, ego_vehicle, middle_location, highway_shape, wps, matrix, closest_waypoint, junction, cars_on_entryExit, direction_angle, True, False) 


            # 5. Highway entry
            elif (is_junction_ahead(ego_waypoint, 20) or (len(current_lanes) <= 1 and ego_waypoint.is_junction)) and highway_junction and street_type in  ["Non highway street", "On junction", "On highway entry", "On highway exit"]:
                street_type = "On highway entry"
                on_entry = True
                
                junction = get_junction_ahead(ego_waypoint, 20)
                if not junction:
                    junction = ego_waypoint.get_junction()
                junction_id_skip = junction.id

                wps = junction.get_waypoints(carla.LaneType().Driving)
                closest_waypoint, middle_location = get_waypoint_on_highway_junction(junction, ego_waypoint, ego_location, wps, world_map)
                wps = prepare_waypoints(closest_waypoint, wps, ego_waypoint)
                highway_shape = get_highway_shape(wps, junction, ego_waypoint)

                on_entry = True
                if (highway_shape is None) and (not junction_old is None):
                    same_junction = False
                    junction = junction_old
                    wps = wps_old
                    highway_shape = highway_shape_old


                matrix = create_city_matrix(middle_location, road_lane_ids, world_map, ghost=True)
                if matrix:
                    matrix, cars_on_entryExit = detect_surronding_cars(
                        middle_location, ego_vehicle, matrix, road_lane_ids, world, radius, True, highway_shape, ghost=True
                    )

                    if not highway_shape is None:
                        matrix = update_matrix(world_map, ego_vehicle, middle_location, highway_shape, wps, matrix, closest_waypoint, junction, cars_on_entryExit, direction_angle, ghost=True, on_entry=True) 


            # 6. Normal Road
            else:
                highway_forward_vector = None
                highway_forward_location = None
                on_junction = False
                junction_old = None
                on_entry = False
                same_junction = False
                if ego_on_highway:
                    street_type = "On highway"
                else:
                    street_type = "Non highway street"
                matrix = create_city_matrix(ego_location, road_lane_ids, world_map)

                # special road (gas station)
                if ego_waypoint.road_id in  [467, 477]:
                    matrix = collections.OrderedDict([("468_-1", [0, 0, 0, 0, 0, 0, 0, 0]) if k == 'No_opposing_direction' else (k, v) for k, v in matrix.items()])
                elif ego_waypoint.road_id == 468:
                    matrix = collections.OrderedDict([("467_1", [0, 0, 0, 0, 0, 0, 0, 0]) if k == 'No_opposing_direction' else (k, v) for k, v in matrix.items()])        

                if matrix:
                    matrix, _ = detect_surronding_cars(
                        ego_location, ego_vehicle, matrix, road_lane_ids, world, radius, ego_on_highway, highway_shape
                    )

            df = safe_data(ego_vehicle, matrix, street_type, df)

            # clock.tick_busy_loop(60)
            time.sleep(0.1)
            world.tick()
        
        except Exception as e:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
            new_row = {"location": ego_location, "error": e, "fname": fname, "line": exc_tb.tb_lineno, "exc_type": exc_type}
            df_errors = df_errors._append(new_row, ignore_index=True)
            continue

    df_errors.to_csv("errors.csv")
    df.to_csv("test.csv")
    vehicles_list = [ego_vehicle]
    print("\ndestroying %d vehicles" % len(vehicles_list))
    client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

    time.sleep(0.5)

