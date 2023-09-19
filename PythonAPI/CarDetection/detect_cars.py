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

warnings.filterwarnings("ignore")
from utils import *


if __name__ == "__main__":
    # config:
    distance_to_junc = 10
    direction_angle = 20

    # First of all, we need to create the client that will send the requests
    # to the simulator. Here we'll assume the simulator is accepting
    # requests in the localhost at port 2000.
    client = carla.Client("127.0.0.1", 2000)
    client.set_timeout(2.0)

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
    transform = random.choice(world.get_map().get_spawn_points())

    # So let's tell the world to spawn the vehicle.
    ego_vehicle = world.spawn_actor(bp, transform)

    vehicle_type = ego_vehicle.type_id

    # Print the type of the ego car
    print("Ego car type:", vehicle_type)

    # Let's put the vehicle to drive around.
    ego_vehicle.set_autopilot(True)  # ToDo Use Carla KI

    # Make him a speeder :)
    # traffic_manager.vehicle_percentage_speed_difference(ego_vehicle, -30)

    # Define the radius to search for other vehicles
    radius = 200

    # Initialize speed of ego_vehicle to use as global variable
    speed = 0

    # clock = pygame.time.Clock()
    df = initialize_dataframe()
    # Loop until the program is interrupted
    world.tick()
    try:
        road_lane_ids = get_all_road_lane_ids(world_map)
        junction_shape = "None"
        matrix = None
        same_junction = False
        while True:
            follow_car(ego_vehicle, world)
            ego_location = ego_vehicle.get_location()
            ego_waypoint = world_map.get_waypoint(ego_location)

            junction = None
            highway = False

            if ego_waypoint.is_junction:
                junction = ego_waypoint.get_junction()
            elif is_junction_ahead(ego_waypoint, distance_to_junc):
                junction = get_junction_ahead(ego_waypoint, distance_to_junc)
                if not same_junction:
                    same_junction = True
                    highway = is_highway_junction(
                        ego_vehicle,
                        ego_waypoint,
                        junction,
                        road_lane_ids,
                        direction_angle,
                    )

            # check if car on junction
            if ego_waypoint.is_junction and not highway and junction.id != 459:
                in_junction = True
                print("Junction:")
                if junction_shape != "None":
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
                        matrix, ego_vehicle, junction, yaw, world
                    )
                    for key, value in matrix.items():
                        print(key, value)

            # car is approaching junction
            elif (
                is_junction_ahead(ego_waypoint, distance_to_junc)
                and not highway
                and junction.id != 459
            ):
                in_junction = True
                junction = get_junction_ahead(ego_waypoint, distance_to_junc)
                (
                    junction_shape,
                    lanes_all_junction,
                    junction_roads_junction,
                    yaw,
                ) = getJunctionShape(
                    ego_vehicle, ego_waypoint, junction, road_lane_ids, direction_angle
                )

                if junction_shape != "Error":
                    print("Junction ahead:")
                    matrix = detect_ego_before_junction(
                        deepcopy(junction_shape),
                        junction_roads_junction,
                        lanes_all_junction,
                        ego_waypoint.lane_id,
                        ego_waypoint,
                        distance_to_junc,
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
                        matrix, ego_vehicle, junction, yaw, world
                    )

                    for key, value in dict(matrix).items():
                        print(key, value)

            # else normal road:
            else:
                same_junction = False
                in_junction = False
                matrix = create_city_matrix(ego_vehicle, road_lane_ids, world_map)

                if matrix:
                    matrix, _ = detect_surronding_cars(
                        ego_vehicle, matrix, road_lane_ids, world, radius
                    )
                    for key, value in matrix.items():
                        print(key, value)

            steering_angle = get_steering_angle(ego_vehicle)
            speed = get_speed(ego_vehicle)
            print("============================================================")
            if matrix:
                row_data = get_row(matrix)
                row_data["speed"] = speed
                row_data["steering_angle"] = steering_angle
                row_data["in_junction"] = in_junction
                df = df.append(row_data, ignore_index=True)

            # clock.tick_busy_loop(60)
            time.sleep(0.3)
            world.tick()

    except KeyboardInterrupt:
        pass

    finally:
        df.to_csv("x_hours.csv")
        vehicles_list = [ego_vehicle]
        print("\ndestroying %d vehicles" % len(vehicles_list))
        client.apply_batch([carla.command.DestroyActor(x) for x in vehicles_list])

        time.sleep(0.5)
