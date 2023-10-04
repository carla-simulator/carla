import carla
import math
import itertools
import collections
import pandas as pd
import numpy as np
import random


def check_road_change(ego_vehicle_location, road_lane_ids, front, world_map):
    """
    Determine if the ego vehicle is about to change to a different road.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle for which we want to check the road change.
        road_lane_ids (list of str): List of road and lane IDs in the world map.
        front (bool): If True, check the road change in the front direction of the ego vehicle,
                      otherwise check in the rear direction.
        world_map (WorldMap): The map representing the environment.

    Returns:
        tuple: A tuple containing two elements:
            - next_road_id (str): The ID of the next road if the ego vehicle is about to change
                                 to a different road, otherwise None.
            - next_lanes (list of str): A list of lane IDs of the next road if the ego vehicle is
                                        about to change to a different road, otherwise None.
    """

    ego_vehicle_waypoint = world_map.get_waypoint(ego_vehicle_location)
    if front:
        next_waypoints = ego_vehicle_waypoint.next(60)
    else:
        next_waypoints = ego_vehicle_waypoint.previous(60)
    next_waypoint = next_waypoints[0]
    ego_vehicle_road_id = str(ego_vehicle_waypoint.road_id)
    next_waypoint_road_id = str(next_waypoint.road_id)

    next_lanes = None
    next_road_id = None
    if next_waypoint_road_id != ego_vehicle_road_id and len(next_waypoints) == 1:
        next_road_id = next_waypoint_road_id
        next_lanes = [
            id.split("_")[1]
            for id in road_lane_ids
            if next_waypoint_road_id == id.split("_")[0]
        ]
    return (next_road_id, next_lanes)


def check_ego_on_highway(ego_vehicle_location, road_lane_ids, world_map):
    waypoints = []
    ego_waypoint = world_map.get_waypoint(ego_vehicle_location)
    waypoints.append(ego_waypoint)
    if ego_waypoint.get_left_lane() is not None:
        waypoints.append(ego_waypoint.get_left_lane())
    if ego_waypoint.get_right_lane() is not None:
        waypoints.append(ego_waypoint.get_right_lane())
    for wp in waypoints:
        ego_vehilce_road_id = wp.road_id
        lanes = []
        for id in road_lane_ids:
            if str(ego_vehilce_road_id) == id.split("_")[0]:
                lanes.append(id.split("_")[1])
        lanes = [int(lane) for lane in lanes]
        if len(lanes) >= 6 or (
            sorted(lanes) == list(range(min(lanes), max(lanes) + 1)) and len(lanes) >= 3
        ):
            return True

    return False


def get_all_road_lane_ids(world_map):
    road_lane_ids = set()

    for waypoint in world_map.generate_waypoints(1.0):
        lane_id = waypoint.lane_id
        road_id = waypoint.road_id
        road_lane_ids.add(f"{road_id}_{lane_id}")

    return road_lane_ids


# Define a function to calculate the distance between two points
def distance(p1, p2):
    return math.sqrt((p1.x - p2.x) ** 2 + (p1.y - p2.y) ** 2 + (p1.z - p2.z) ** 2)


def follow_car(ego_vehicle, world):
    """
    Set the spectator's view to follow the ego vehicle.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle that the spectator will follow.
        world (World): The game world where the spectator view is set.

    Description:
        This function calculates the desired spectator transform by positioning the spectator
        10 meters behind the ego vehicle and 5 meters above it. The spectator's view will follow
        the ego vehicle from this transformed position.

    Note:
        To face the vehicle from behind, uncomment the line 'spectator_transform.rotation.yaw += 180'.

    Returns:
        None: The function does not return any value.
    """
    # Calculate the desired spectator transform
    vehicle_transform = ego_vehicle.get_transform()
    spectator_transform = vehicle_transform
    spectator_transform.location -= (
        vehicle_transform.get_forward_vector() * 10
    )  # Move 10 meters behind the vehicle
    spectator_transform.location += (
        vehicle_transform.get_up_vector() * 5
    )  # Move 5 meters above the vehicle
    # spectator_transform.rotation.yaw += 180 # Face the vehicle from behind

    # Set the spectator's transform in the world
    world.get_spectator().set_transform(spectator_transform)


def get_ego_direction_lanes(ego_vehicle, road_lane_ids, world_map):
    """
    Determine the lanes in the same and opposite direction as the ego vehicle.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle for which to determine the lane directions.
        road_lane_ids (list of str): List of road and lane IDs in the world map.
        world_map (WorldMap): The map representing the environment.

    Returns:
        tuple: A tuple containing two lists:
            - ego_direction (list of int): A list of lane IDs that are in the same direction as
                                           the ego vehicle's current lane.
            - other_direction (list of int): A list of lane IDs that are in the opposite direction
                                             to the ego vehicle's current lane.
    """
    ego_vehicle_location = ego_vehicle.get_location()
    ego_vehilce_waypoint = world_map.get_waypoint(ego_vehicle_location)
    ego_vehilce_lane_id = str(ego_vehilce_waypoint.lane_id)
    ego_vehilce_road_id = str(ego_vehilce_waypoint.road_id)

    lanes = []
    for id in road_lane_ids:
        if ego_vehilce_road_id == id.split("_")[0]:
            lanes.append(id.split("_")[1])
    lanes.sort()
    lanes = [int(id) for id in lanes]
    lanes_splitted = []
    z = 0
    for i in range(1, len(lanes)):
        if lanes[i] == lanes[i - 1] - 1 or lanes[i] == lanes[i - 1] + 1:
            continue
        else:
            lanes_splitted.append(lanes[z:i])
            z = i
    lanes_splitted.append(lanes[z:])
    lanes_splitted = [
        sorted(direction, key=abs, reverse=True) for direction in lanes_splitted
    ]

    # Initialize matrix and key_value_pairs
    other_direction = []
    ego_direction = []
    for direction in lanes_splitted:
        if int(ego_vehilce_lane_id) in direction:
            ego_direction = direction
        else:
            other_direction = direction

    return ego_direction, other_direction


def create_city_matrix(ego_vehicle_location, road_lane_ids, world_map, ghost=False):
    """
    Create a matrix representing the lanes around the ego vehicle.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle for which to create the city matrix.
        road_lane_ids (list of str): List of road and lane IDs in the world map.
        world_map (WorldMap): The map representing the environment.

    Returns:
        collections.OrderedDict: An ordered dictionary representing the city matrix.
                                 The keys are lane IDs in the format "road_id_lane_id".
                                 The values are lists of integers indicating lane directions:
                                     0 - Lane in the same direction as ego vehicle's lane.
                                     1 - Ego vehicle's lane.
                                     3 - Empty lane (no traffic).
    """
    # Get lane id for ego_vehicle
    ego_vehilce_waypoint = world_map.get_waypoint(ego_vehicle_location)
    ego_vehilce_lane_id = str(ego_vehilce_waypoint.lane_id)
    ego_vehilce_road_id = str(ego_vehilce_waypoint.road_id)

    lanes = []
    for id in road_lane_ids:
        if ego_vehilce_road_id == id.split("_")[0]:
            lanes.append(id.split("_")[1])
    lanes.sort()
    lanes = [int(id) for id in lanes]
    lanes_splitted = []
    z = 0
    for i in range(1, len(lanes)):
        if lanes[i] == lanes[i - 1] - 1 or lanes[i] == lanes[i - 1] + 1:
            continue
        else:
            lanes_splitted.append(lanes[z:i])
            z = i
    lanes_splitted.append(lanes[z:])
    lanes_splitted = [
        sorted(direction, key=abs, reverse=True) for direction in lanes_splitted
    ]

    # Initialize matrix and key_value_pairs
    matrix = None
    key_value_pairs = None
    other_direction = []
    ego_direction = []
    for direction in lanes_splitted:
        if int(ego_vehilce_lane_id) in direction:
            ego_direction = direction
        else:
            other_direction = direction
    if len(ego_direction) >= 4:
        key_value_pairs = [
            ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[3]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[2]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[1]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            ("right_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
        ]
    elif len(ego_direction) == 3:
        key_value_pairs = [
            ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[2]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[1]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            ("No_4th_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
        ]

    elif len(ego_direction) == 2 and len(other_direction) == 2:
        key_value_pairs = [
            ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            (
                ego_vehilce_road_id + "_" + str(other_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(other_direction[1]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[1]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            ("right_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
        ]
    elif len(ego_direction) == 2 and len(other_direction) == 0:
        key_value_pairs = [
            ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("No_opposing_direction", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("No_opposing_direction", [3, 3, 3, 3, 3, 3, 3, 3]),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[1]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            ("right_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
        ]
    elif len(ego_direction) == 1 and len(other_direction) == 1:
        key_value_pairs = [
            ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("No_opposing_direction", [3, 3, 3, 3, 3, 3, 3, 3]),
            (
                ego_vehilce_road_id + "_" + str(other_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            ("No_own_right_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
        ]
    elif len(ego_direction) == 1 and len(other_direction) == 0:
        key_value_pairs = [
            ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("No_other_right_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("No_opposing_direction", [3, 3, 3, 3, 3, 3, 3, 3]),
            (
                ego_vehilce_road_id + "_" + str(ego_direction[0]),
                [0, 0, 0, 0, 0, 0, 0, 0],
            ),
            ("No_own_right_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
            ("right_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
        ]

    # Update matrix
    if key_value_pairs:
        matrix = collections.OrderedDict(key_value_pairs)

    if matrix and not ghost:
        matrix[str(ego_vehilce_road_id) + "_" + str(ego_vehilce_lane_id)][3] = 1

    return matrix


def detect_surronding_cars(
    ego_location,
    ego_vehicle,
    matrix,
    road_lane_ids,
    world,
    radius,
    on_highway,
    highway_shape,
    ghost=False,
):
    """
    Detect surrounding cars and update the city matrix accordingly.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle for which to detect surrounding cars.
        matrix (collections.OrderedDict): The city matrix representing the lanes around the ego vehicle.
        road_lanes_ids (list of str): List of road and lane IDs in the world map.
        world (carla.World): The game world where the simulation is running.
        radius (float): The radius within which to detect surrounding cars.

    Returns:
        collections.OrderedDict: The updated city matrix with detected surrounding cars.

    Description:
        This function detects surrounding cars within a specified radius from the ego vehicle and
        updates the city matrix accordingly. The city matrix is represented as an ordered dictionary
        with lane IDs as keys and lists of integers as values. Each list corresponds to the direction
        of the lane, where:
            - 0 indicates a lane in the same direction as the ego vehicle's current lane.
            - 1 indicates the ego vehicle's current lane.
            - 2 indicates a lane with a detected surrounding car.
            - 3 indicates an empty lane with no traffic.

        The function calculates the position of the detected cars in the city matrix and updates the
        corresponding cells in the matrix to indicate the presence of surrounding cars. The matrix is
        updated based on the lanes and positions/distance of the detected cars relative to the ego vehicle.

    Note:
        The city matrix should be pre-generated using the 'create_city_matrix' function.
        The returned city matrix may contain lane directions that are not within the ego vehicle's vicinity,
        as determined by the input road_lane_ids and world.
    """
    world_map = world.get_map()
    # Get all surronding cars in specified radius
    surrounding_cars = []
    surrounding_cars_on_highway_entryExit = []
    ego_vehicle_waypoint = world_map.get_waypoint(ego_location)
    ego_vehicle_road_id = ego_vehicle_waypoint.road_id

    for actor in world.get_actors():
        if "vehicle" in actor.type_id and (actor.id != ego_vehicle.id or ghost):
            distance_to_actor = distance(actor.get_location(), ego_location)
            if distance_to_actor <= radius:
                surrounding_cars.append(actor)

    _, next_lanes = check_road_change(ego_location, road_lane_ids, True, world_map)
    _, prev_lanes = check_road_change(ego_location, road_lane_ids, False, world_map)
    lanes_exist_futher = False
    lanes = []
    for id in road_lane_ids:
        if str(ego_vehicle_road_id) == id.split("_")[0]:
            lanes.append(id.split("_")[1])
    try:
        if next_lanes and matrix:
            # lanes = [road_lane.split("_")[1] for road_lane in matrix.keys()]
            lanes_exist_futher = all(lane in next_lanes for lane in lanes) or all(
                lane in lanes for lane in next_lanes
            )
    except IndexError:
        pass

    lanes_existed_before = False
    try:
        if prev_lanes and matrix:
            # lanes = [road_lane.split("_")[1] for road_lane in matrix.keys()]
            lanes_existed_before = all(lane in prev_lanes for lane in lanes) or all(
                lane in lanes for lane in prev_lanes
            )
    except IndexError:
        pass

    # in the following, ignore cars that are on highway exit/entry lanes
    if not highway_shape is None:
        entry_wps = highway_shape[2]
        exit_wps = highway_shape[3]

        entry_road_id = []
        exit_road_id = []
        entry_city_road = []
        exit_city_road = []
        entry_highway_road = []
        exit_highway_road = []
        if entry_wps:
            for entry_wp in entry_wps[0]:
                entry_city_road.append(entry_wp.previous(3)[0].road_id)
                entry_road_id.append(entry_wp.road_id)
            for entry_wp in entry_wps[1]:
                entry_highway_road.append(entry_wp.next(3)[0].road_id)
            if entry_wp.next(3)[0] and entry_wp.next(3)[0].get_left_lane() and entry_wp.next(3)[0].road_id == entry_wp.next(3)[0].get_left_lane().road_id:
                entry_highway_road = []
        if exit_wps:
            for exit_wp in exit_wps[1]:
                exit_city_road.append(exit_wp.next(3)[0].road_id)
                exit_road_id.append(exit_wp.road_id)
            for exit_wp in exit_wps[0]:
                exit_highway_road.append(exit_wp.previous(3)[0].road_id)
            if exit_wp.next(3)[0] and exit_wp.next(3)[0].get_left_lane() and exit_wp.next(3)[0].road_id == exit_wp.next(3)[0].get_left_lane().road_id:
                exit_highway_road = []
       
        
    # Update matrix based on the lane and position/distance to ego vehicle of other car
    if (
        on_highway
        and (not highway_shape is None)
        and (
            ego_vehicle_road_id
            in entry_road_id
            + entry_city_road
            + exit_road_id
            + exit_city_road
            + exit_highway_road
            + entry_highway_road
        )
    ):
        surrounding_cars_on_highway_entryExit.append(ego_vehicle)

    for car in surrounding_cars:
        # Get road and lane_id of other car
        other_car_waypoint = world_map.get_waypoint(car.get_location())
        other_car_lane_id = other_car_waypoint.lane_id
        other_car_road_id = other_car_waypoint.road_id
        other_car_road_lane_id = str(other_car_road_id) + "_" + str(other_car_lane_id)

        # ignore car on highway entry / Exit
        if (
            on_highway
            and (not highway_shape is None)
            and (
                other_car_road_id
                in entry_road_id
                + entry_city_road
                + exit_road_id
                + exit_city_road
                + exit_highway_road
                + entry_highway_road
            )
        ):
            surrounding_cars_on_highway_entryExit.append(car)
            continue
        col = calculate_position_in_matrix(
            ego_location,
            ego_vehicle,
            car,
            matrix,
            world_map,
            ego_vehicle.get_velocity(),
            ghost,
        )
        if col is None:
            continue
        if matrix:
            if other_car_road_lane_id in matrix.keys():
                if car.id == ego_vehicle.id:
                    matrix[other_car_road_lane_id][col] = 1
                else:
                    matrix[other_car_road_lane_id][col] = 2
                continue
            elif (lanes_exist_futher or lanes_existed_before) and str(
                other_car_lane_id
            ) in [str(road_lane.split("_")[1]) for road_lane in matrix.keys()]:
                if car.id == ego_vehicle.id:
                    matrix[str(ego_vehicle_road_id) + "_" + str(other_car_lane_id)][
                        col
                    ] = 1
                else:
                    matrix[str(ego_vehicle_road_id) + "_" + str(other_car_lane_id)][
                        col
                    ] = 2

    return matrix, surrounding_cars_on_highway_entryExit


def get_forward_vector_distance(ego_vehicle_location, other_car, world_map):
    """
    Calculate the distance between point B (other vehicle) and point C (parallel right/left of ego) in a right-angled triangle.

    Parameters:
        distance_ego_other (float): The distance between points A (ego) and B (other) => the hypotenuse.
        other_car_location (carla.Location): The location of the other car in 3D space.
        ego_vehicle_location (carla.Location): The location of the ego vehicle in 3D space.

    Returns:
        float: The distance between point B (other) and point C (parallel right/left of ego).
    """

    # Get the locations of the ego vehcile and the other car
    other_car_location = other_car.get_location()

    # Calculate straight line distance between ego and other car
    distance_ego_other = ego_vehicle_location.distance(other_car_location)

    # Get waypoints
    ego_waypoint = world_map.get_waypoint(ego_vehicle_location)
    other_waypoint = world_map.get_waypoint(other_car_location)

    other_lane_id = other_waypoint.lane_id

    left_lane_wp, right_lane_wp = ego_waypoint, ego_waypoint
    old_left_lane_wps, old_right_lane_wps = [], []
    while True:
        if (not left_lane_wp or left_lane_wp.id in old_left_lane_wps) and (
            not right_lane_wp or right_lane_wp.id in old_right_lane_wps
        ):
            return distance_ego_other
        if ego_waypoint.lane_id == other_lane_id:
            return distance_ego_other
        if left_lane_wp:
            old_left_lane_wps.append(left_lane_wp.id)
            left_lane_wp = left_lane_wp.get_left_lane()
            if left_lane_wp:
                if left_lane_wp.lane_id == other_lane_id:
                    perpendicular_wp = left_lane_wp
                    break
        if right_lane_wp:
            old_right_lane_wps.append(right_lane_wp.id)
            right_lane_wp = right_lane_wp.get_right_lane()
            if right_lane_wp:
                if right_lane_wp.lane_id == other_lane_id:
                    perpendicular_wp = right_lane_wp
                    break

    distance_opposite = ego_vehicle_location.distance(
        perpendicular_wp.transform.location
    )

    return math.sqrt(abs(distance_ego_other**2 - distance_opposite**2))


def check_car_in_front_or_behind(ego_location, other_location, rotation):
    # Get ego to other vector
    ego_to_other_vector = other_location - ego_location

    # Calculate forward vector
    ego_forward_vector = carla.Vector3D(
        math.cos(math.radians(rotation.yaw)),
        math.sin(math.radians(rotation.yaw)),
        0,
    )

    # Calculate dot_product (similarity between the vectors)
    dot_product = (
        ego_forward_vector.x * ego_to_other_vector.x
        + ego_forward_vector.y * ego_to_other_vector.y
    )
    return dot_product


def calculate_position_in_matrix(
    ego_location, ego_vehicle, other_car, matrix, world_map, velocity, ghost=False
):
    """
    Calculate the position of the other car in the city matrix based on its relative location and distance from the ego vehicle.
    Only determines the column, not the row since that is based on the lane_id of the other car.

    Parameters:
        ego_vehicle (carla.Vehicle): The ego vehicle for reference.
        other_car (carla.Vehicle): The other car whose position is to be determined.
        matrix (collections.OrderedDict): The city matrix representing the lanes around the ego vehicle.
        world_map (carla.WorldMap): The map representing the environment.

    Returns:
        int or None: The column index in the city matrix representing the column in the city matrix of the other car,
                    or None if the other car is not within the specified distance range.

    Description:
        This function calculates the position of the other car in the city matrix based on its relative location and
        distance from the ego vehicle. The city matrix is represented as an ordered dictionary with lane IDs as keys and
        lists of integers as values. Each list corresponds to the direction of the lane, where:
            - 0 indicates a lane behind the ego vehicle's current lane.
            - 1 indicates the ego vehicle's current lane.
            - 2 indicates a lane in front of the ego vehicle's current lane with a detected surrounding car.
            - 3 indicates an empty lane (no traffic).

        The function first calculates the Euclidean distance between the ego vehicle and the other car using the
        `triangle_calculation` function. It then calculates the dot product between the ego vehicle's forward vector
        and the vector from the ego vehicle to the other car to determine whether the other car is in front of or behind
        the ego vehicle.

        Based on the distance and position of the other car, the function determines the appropriate column index in the
        city matrix and returns it. If the other car is not within the specified distance range or not on the same road as
        the ego vehicle, the function returns None.

    Note:
        The city matrix should be pre-generated using the 'create_city_matrix' function. Other cars are detected using the detect_surronding cars func.
    """

    # Get ego vehicle rotation and location
    if ghost:
        rotation = other_car.get_transform().rotation
    else:
        rotation = ego_vehicle.get_transform().rotation

    # Get other car vehicle location
    other_location = other_car.get_transform().location

    # Calculate new distance
    new_distance = get_forward_vector_distance(ego_location, other_car, world_map)

    # Get distance between ego_vehicle and other car
    distance_to_actor = distance(other_location, ego_location)

    # check car behind or in front of ego vehicle
    dot_product = check_car_in_front_or_behind(ego_location, other_location, rotation)

    # Get road_lane_id of other vehicle
    other_car_waypoint = world_map.get_waypoint(other_location)
    other_car_lane_id = other_car_waypoint.lane_id
    other_car_road_id = other_car_waypoint.road_id
    other_car_road_lane_id = str(other_car_road_id) + "_" + str(other_car_lane_id)

    # Get road_lane_id of ego vehicle
    ego_car_waypoint = world_map.get_waypoint(ego_location)
    ego_car_lane_id = ego_car_waypoint.lane_id
    ego_car_road_id = ego_car_waypoint.road_id
    ego_car_road_lane_id = str(ego_car_road_id) + "_" + str(ego_car_lane_id)

    # velocity = ego_vehicle.get_velocity()
    ego_speed = (
        3.6 * (velocity.x**2 + velocity.y**2 + velocity.z**2) ** 0.5
    )  # Convert m/s to km/h
    road_lane_ids = get_all_road_lane_ids(world_map)
    if check_ego_on_highway(ego_location, road_lane_ids, world_map):
        speed_factor = 2.0
    else:
        speed_factor = 1

    col = None

    # Other car is in front of ego_vehicle
    if (
        abs(dot_product) < 4
        and other_car_road_lane_id in list(matrix.keys())
        and matrix[other_car_road_lane_id][3] != 1
    ):
        col = 3
    elif dot_product > 0:
        if new_distance < 10 * speed_factor:
            col = 4
        elif new_distance < 20 * speed_factor:
            col = 5
        elif new_distance < 30 * speed_factor:
            col = 6
        elif new_distance < 40 * speed_factor:
            col = 7

    # Other car is behind ego_vehicle
    else:
        if new_distance < 10 * speed_factor:
            col = 2
        elif new_distance < 20 * speed_factor:
            col = 1
        elif new_distance < 30 * speed_factor:
            col = 0
    return col


def initialize_dataframe():
    """
    Initialize an empty DataFrame with predefined column names.

    Returns:
        pandas.DataFrame: An empty DataFrame with columns representing a grid of cells around the ego vehicle,
                          as well as additional columns for speed, steering angle, and in_junction flag.
    """
    # Initialize datafram
    column_names = [f"{i}_{j}" for i in range(8) for j in range(8)]

    column_names.append("speed")
    column_names.append("steering_angle")
    column_names.append("in_junction")
    # Initialize the DataFrame with empty values
    return pd.DataFrame(columns=column_names)


def get_row(matrix):
    """
    Convert the city matrix into a row of data in the DataFrame.

    Parameters:
        matrix (collections.OrderedDict): The city matrix representing the lanes around the ego vehicle.

    Returns:
        dict: A dictionary representing a row of data to be added to the DataFrame.
    """
    row_data = {}

    keys = [0, 1, 2, 3, 4, 5, 6, 7]
    counter = 0
    # Iterate over the data dictionary
    for key, values in matrix.items():
        # Get the column names based on the key
        column_names = [f"{keys[counter]}_{i}" for i in range(0, 8)]
        # Create a dictionary with column names as keys and values from data as values
        row_data.update(dict(zip(column_names, values)))
        counter += 1

    return row_data


def get_speed(ego_vehicle):
    velocity = ego_vehicle.get_velocity()
    ego_speed = (
        3.6 * (velocity.x**2 + velocity.y**2 + velocity.z**2) ** 0.5
    )  # Convert m/s to km/h

    return ego_speed


def get_steering_angle(ego_vehicle):
    control = ego_vehicle.get_control()

    # Get the steering angle in radians
    steering_angle = math.radians(control.steer)
    return steering_angle


#########################################
# help functions for junctions:
#########################################


def is_junction_ahead(ego_waypoint, distance):
    """
    Check if a junction is ahead of the ego vehicle within a specified distance.

    Parameters:
        ego_waypoint (carla.Waypoint): The current waypoint of the ego vehicle.
        distance (int): The maximum distance (in meters) to search for a junction ahead.

    Returns:
        bool: True if a junction is found ahead within the specified distance, False otherwise.
    """
    # return True if junction is ahead of ego in <= distance meter, start checking at 1m ahead and increment by 1 every loop
    for x in list(range(1, distance + 1)):
        if ego_waypoint.next(x)[0].is_junction:
            return True
    return False


def get_junction_ahead(ego_waypoint, distance):
    """
    Get the junction ahead of the ego vehicle within a specified distance.

    Parameters:
        ego_waypoint (carla.Waypoint): The current waypoint of the ego vehicle.
        distance (int): The maximum distance (in meters) to search for a junction ahead.

    Returns:
        carla.Junction or None: The carla.Junction object representing the junction ahead if found within
                                 the specified distance. Returns None if no junction is found.
    """
    for x in list(range(1, distance + 1)):
        if ego_waypoint.next(x)[0].is_junction:
            return ego_waypoint.next(x)[0].get_junction()
    return None


def get_junction_behind(ego_waypoint, distance):
    """
    Get the junction ahead of the ego vehicle within a specified distance.

    Parameters:
        ego_waypoint (carla.Waypoint): The current waypoint of the ego vehicle.
        distance (int): The maximum distance (in meters) to search for a junction ahead.

    Returns:
        carla.Junction or None: The carla.Junction object representing the junction ahead if found within
                                 the specified distance. Returns None if no junction is found.
    """
    for x in list(range(1, distance + 1)):
        if ego_waypoint.previous(x)[0].is_junction:
            return ego_waypoint.previous(x)[0].get_junction()
    return None


def is_junction_behind(ego_waypoint, distance):
    """
    Check if a junction is behind the ego vehicle within a specified distance.

    Parameters:
        ego_waypoint (carla.Waypoint): The current waypoint of the ego vehicle.
        distance (int): The maximum distance (in meters) to search for a junction in the back.

    Returns:
        bool: True if a junction is found behind within the specified distance, False otherwise.
    """
    # return True if junction is ahead of ego in <= distance meter, start checking at 1m ahead and increment by 1 every loop
    for x in list(range(1, distance + 1)):
        if ego_waypoint.previous(x)[0].is_junction:
            return True
    return False


def get_waypoint_direction(
    ego_vehicle, closest_start_wp, junction_waypoint, direction_angle
):
    """
    Get the direction of a waypoint relative to the ego vehicle and a junction.

    Parameters:
        ego_vehicle (carla.Vehicle): The ego vehicle for which to determine the direction.
        closest_start_wp (carla.Waypoint): The closest waypoint to the junction where the ego vehicle enters.
        junction_waypoint (carla.Waypoint): The waypoint representing the junction.
        direction_angle (float): The angle threshold in degrees to classify the direction.

    Returns:
        str: The direction of the waypoint relative to the ego vehicle and the junction.
             Possible return values: "straight", "left", or "right".
    """

    # Get the location of the ego vehicle when entering junction
    ego_location = closest_start_wp.transform.location

    point1_x = closest_start_wp.transform.location.x
    point1_y = closest_start_wp.transform.location.y
    next_waypoint = closest_start_wp.next(5)[0]
    point2_x = next_waypoint.transform.location.x
    point2_y = next_waypoint.transform.location.y

    # Calculate the vector components
    vector_x = point2_x - point1_x
    vector_y = point2_y - point1_y

    # Calculate the yaw angle
    yaw = math.atan2(vector_y, vector_x)
    yaw_degrees = math.degrees(yaw)
    ego_rotation = yaw_degrees
    # Get the orientation of the ego vehicle
    ego_rotation_1 = ego_vehicle.get_transform().rotation.yaw
    # ego_rotation = ego_rotation_1
    # Get the location of the junction waypoint
    junction_location = junction_waypoint.transform.location

    # Calculate the angle between the ego vehicle and the junction waypoint
    angle = math.atan2(
        junction_location.y - ego_location.y, junction_location.x - ego_location.x
    )
    angle = math.degrees(angle)

    if abs(ego_rotation) > 100:
        if angle < 0:
            angle = 360 - abs(angle)
        if ego_rotation < 0:
            ego_rotation = 360 - abs(ego_rotation)

    # Calculate the difference between the angles
    angle_diff = angle - ego_rotation

    # Determine the direction of the junction waypoint
    if abs(angle_diff) < direction_angle:
        return "straight"
    elif angle_diff <= -direction_angle:
        return "left"
    else:
        return "right"


def detect_ego_before_junction(
    key_value_pairs, junction_roads, lanes_all, lane_id_ego, ego_wp, distance_to_junc
):
    """
    Update the city matrix to detect the position of the ego vehicle relative to a junction.

    Parameters:
        key_value_pairs (list of tuples): A list of tuples representing the city matrix.
        junction_roads (list of tuples): A list of tuples containing road IDs and lane IDs of the junction.
        lanes_all (dict): A dictionary containing lane IDs of the roads around the junction.
        lane_id_ego (int): The lane ID of the ego vehicle.
        ego_wp (carla.Waypoint): The waypoint of the ego vehicle.
        distance_to_junc (int): The distance in meters from the ego vehicle to the junction.

    Returns:
        list of tuples: The updated city matrix with the position of the ego vehicle detected.
    """
    # detect ego before junction

    # cast lane id's to int
    for road in junction_roads:
        lanes_all[road[1]] = [int(id) for id in lanes_all[road[1]]]

    # check on which lane ego is: i=0 means most out
    if lane_id_ego > 0:
        lanes_all["ego"].sort(reverse=True)
    else:
        lanes_all["ego"].sort()
    for i in range(int(len(lanes_all["ego"]) / 2)):
        if lanes_all["ego"][i] == lane_id_ego:
            break  # i=0: lane most out

    # check how many lanes the roads right and left of ego have
    if len(lanes_all["left"]) == 0:
        columns = int((8 - len(lanes_all["right"])) / 2)
    elif len(lanes_all["right"]) == 0:
        columns = int((8 - len(lanes_all["left"])) / 2)
    else:
        columns = int(
            (8 - min([len(lanes_all["left"]), len(lanes_all["right"])])) / 2
        )  # min number of lanes of right/left

    # determine cell of ego in matrix
    if ego_wp.next(int(distance_to_junc / columns))[
        0
    ].is_junction:  # doesn't matter how many columns: write "1" in the cell clostest to junction inner part
        c = columns - 1
    elif columns == 2:  # if 2 columns that it must be the cell farsest away
        c = columns - 2
    elif ego_wp.next(int(distance_to_junc / columns) * 2)[
        0
    ].is_junction:  # if 3 columns check if junction is closer than 10m, then middle cell
        c = columns - 2
    else:  # if 3 columns and further away than 10m, then cell farsest away
        c = columns - 3
    for j in range(8):
        if key_value_pairs[-1 - j][1][c] != 3:
            key_value_pairs[-1 - j - i][1][c] = 1
            break

    return key_value_pairs


def detect_surrounding_cars_outside_junction(
    key_value_pairs,
    junction_roads,
    lanes_all,
    ego_vehicle,
    world,
    distance_to_junc,
    junction,
):
    """
    Detects and records surrounding cars outside the junction in the city matrix.

    Parameters:
        key_value_pairs (list): A list of key-value pairs representing the city matrix.
        junction_roads (list): List of junction roads and lane information.
                               Format: [road_id, direction, outgoing_lane_wp, lane_id].
        lanes_all (dict): A dictionary containing lane information for different directions.
                          Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.
        ego_vehicle (Vehicle): The ego vehicle.
        world (CarlaWorld): The world representation.
        distance_to_junc (float): Distance from ego vehicle to the junction in meters.
        junction (Junction): The junction object.

    Returns:
        list: Updated key-value pairs representing the city matrix with information about surrounding cars.
    """
    world_map = world.get_map()
    junction_id = junction.id
    # junction_roads =
    #   [road_id of incoming road, direction from ego-perspective, wp of outgoing lane of incoming road, corresponding lane_id ],
    #       ... for all 4 directions ego, left, right, straight
    #   ]
    surrounding_cars = {"ego": [], "left": [], "straight": [], "right": []}
    for actor in world.get_actors():
        if "vehicle" in actor.type_id and actor.id != ego_vehicle.id:
            for road in junction_roads:
                actor_location = actor.get_location()
                actor_waypoint = world_map.get_waypoint(actor_location)
                distance_to_actor = distance(actor_location, road[2].transform.location)
                different_road_distance = None

                if (actor_waypoint.is_junction) and (
                    actor_waypoint.get_junction().id == junction_id
                ):
                    actor_outside_junction = False
                else:
                    actor_outside_junction = True

                if (
                    distance_to_actor <= distance_to_junc * 1.2
                ) and actor_outside_junction:  # add extra 20% because distance_to_actor is calculated with road[2] waypoint, which can be on other lane
                    included = True
                    if str(actor_waypoint.road_id) == road[0]:
                        different_road_distance = None
                    elif (
                        str(actor_waypoint.next(int(distance_to_junc / 2))[0].road_id)
                        == road[0]
                    ):
                        different_road_distance = int(distance_to_junc / 2)
                    elif (
                        str(
                            actor_waypoint.previous(int(distance_to_junc / 2))[
                                0
                            ].road_id
                        )
                        == road[0]
                    ) and (not actor_waypoint.is_junction):
                        different_road_distance = (
                            int(distance_to_junc / 2) * -1
                        )  # negative bc. look back from actor
                    elif (
                        str(actor_waypoint.next(int(distance_to_junc / 3))[0].road_id)
                        == road[0]
                    ):
                        different_road_distance = int(distance_to_junc / 3)
                    elif (
                        str(
                            actor_waypoint.previous(int(distance_to_junc / 3))[
                                0
                            ].road_id
                        )
                        == road[0]
                    ):
                        different_road_distance = (
                            int(distance_to_junc / 3) * -1
                        )  # negative bc. look back from actor
                    else:
                        included = False
                    if included:
                        surrounding_cars[road[1]].append(
                            (actor, different_road_distance)
                        )

    for road in junction_roads:
        for actor, different_road_distance in surrounding_cars[road[1]]:
            actor_location = actor.get_location()
            actor_waypoint = world_map.get_waypoint(actor_location)

            if not (different_road_distance is None):
                if different_road_distance > 0:
                    actor_waypoint_lane_id = actor_waypoint.next(
                        different_road_distance
                    )[0].lane_id
                elif different_road_distance < 0:
                    actor_waypoint_lane_id = actor_waypoint.previous(
                        (different_road_distance * -1)
                    )[0].lane_id
            else:
                actor_waypoint_lane_id = actor_waypoint.lane_id

            # sort list of lane ID's
            if int(road[3]) < 0:
                lanes_all[road[1]].sort(reverse=True)
            else:
                lanes_all[road[1]].sort()

            # determine lane of actor, i=0: lane most out of lanes incoming the junction
            for i in range(int(len(lanes_all[road[1]]))):
                if lanes_all[road[1]][i] == actor_waypoint_lane_id:
                    break

            # determine number of cells/columns in matrix outside the inner junction
            if (road[1] == "ego") or (road[1] == "straight"):
                if len(lanes_all["left"]) == 0:
                    columns = int((8 - len(lanes_all["right"])) / 2)
                elif len(lanes_all["right"]) == 0:
                    columns = int((8 - len(lanes_all["left"])) / 2)
                else:
                    columns = int(
                        (8 - max([len(lanes_all["left"]), len(lanes_all["right"])])) / 2
                    )  # max number of lanes of right/left
            elif (road[1] == "left") or (road[1] == "right"):
                if len(lanes_all["ego"]) == 0:
                    columns = int((8 - len(lanes_all["straight"])) / 2)
                elif len(lanes_all["straight"]) == 0:
                    columns = int((8 - len(lanes_all["ego"])) / 2)
                else:
                    columns = int(
                        (8 - max([len(lanes_all["ego"]), len(lanes_all["straight"])]))
                        / 2
                    )  # max number of lanes of ego/straight

            # determine cell of actor in matrix
            if ((int(road[3]) > 0) and (actor_waypoint_lane_id > 0)) or (
                (int(road[3]) < 0) and (actor_waypoint_lane_id < 0)
            ):  # actor on outgoing lane
                # location.distance(waypoints[i].transform.location
                actor_distance_junction = actor_waypoint.transform.location.distance(
                    road[2].transform.location
                )
                if actor_distance_junction < (
                    distance_to_junc / columns
                ):  # doesn't matter how many columns: write "2" in the cell clostest to junction inner part
                    c = columns - 1
                elif (columns == 2) or (
                    actor_distance_junction < (distance_to_junc / columns) * 2
                ):  # if 2 columns then it must be the cell farthest away && if 3 columns check if junction is closer than 2/3 of DISTANCE_TO_JUNCTION, then middle cell
                    c = columns - 2
                else:  # if 3 columns and further away, then cell farthest away
                    c = columns - 3
            else:  # incoming actor
                if actor_waypoint.next(int(distance_to_junc / columns))[
                    0
                ].is_junction:  # doesn't matter how many columns: write "2" in the cell clostest to junction inner part
                    c = columns - 1
                elif (columns == 2) or (
                    actor_waypoint.next(int(distance_to_junc / columns) * 2)[
                        0
                    ].is_junction
                ):  # if 2 columns that it must be the cell farsest away && if 3 columns check if junction is closer than 2/3 of DISTANCE_TO_JUNCTION, then middle cell
                    c = columns - 2
                else:  # if 3 columns and further away than 10m, then cell farsest away
                    c = columns - 3

            # write "2" in identified cell
            if road[1] == "ego":
                for j in range(8):
                    # ego road
                    if key_value_pairs[-1 - j][1][c] != 3:
                        # in case ego vehicle is already in that matrix cell
                        if key_value_pairs[-1 - j - i][1][c] == 1:
                            # Calculate dot_product (similarity between the vectors)
                            dot_product = check_car_in_front_or_behind(
                                ego_vehicle.get_location(),
                                actor.get_location(),
                                ego_vehicle.get_transform().rotation,
                            )
                            if dot_product > 0:  # other car infront
                                if (columns != 1) and (
                                    c == columns - 1
                                ):  # already in front cell
                                    key_value_pairs[-1 - j - i][1][c - 1] = 1
                                elif (
                                    columns != 1
                                ):  # not in front cell but more than 1 cells available
                                    key_value_pairs[-1 - j - i][1][c + 1] = 2
                            else:  # other car behind
                                if (columns != 1) and (c == 0):  # already in last cell
                                    key_value_pairs[-1 - j - i][1][c + 1] = 1
                                elif (
                                    columns != 1
                                ):  # not in last cell but more than 1 cells available
                                    key_value_pairs[-1 - j - i][1][c - 1] = 2
                        else:
                            key_value_pairs[-1 - j - i][1][c] = 2
                        break
            elif road[1] == "left":
                for j in range(8):
                    # left road
                    if key_value_pairs[c][1][j] != 3:
                        key_value_pairs[c][1][j + i] = 2
                        break
            elif road[1] == "straight":
                for j in range(8):
                    # straight road
                    if key_value_pairs[j][1][-1 - c] != 3:
                        key_value_pairs[j + i][1][-1 - c] = 2
                        break
            elif road[1] == "right":
                for j in range(8):
                    # right road
                    if key_value_pairs[-1 - c][1][-1 - j] != 3:
                        key_value_pairs[-1 - c][1][-1 - j - i] = 2
                        break

    return key_value_pairs


def getJunctionShape(ego_vehicle, ego_wp, wps, road_lane_ids, direction_angle):
    """
    Determines the shape of the junction in the city matrix and returns relevant information.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle.
        ego_wp (Waypoint): Waypoint of the ego vehicle.
        junction (Junction): The junction object.
        road_lane_ids (dict): A dictionary containing lane IDs for different directions.
                             Format: {"ego": lane_id, "left": lane_id, "straight": lane_id, "right": lane_id}.
        direction_angle (float): Angle to determine direction at junction.

    Returns:
        tuple: A tuple containing the following information:
            key_value_pairs (list): A list of key-value pairs representing the city matrix.
            lanes_all (dict): A dictionary containing lane information for different directions.
                              Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.
            junction_roads (list): List of junction roads and lane information.
                                   Format: [road_id, direction, outgoing_lane_wp, lane_id].
            yaw (float): The yaw angle of the ego vehicle's rotation.
    """
    lanes_all, junction_roads = get_all_lanes(
        ego_vehicle, ego_wp, wps, road_lane_ids, direction_angle
    )

    key_value_pairs = [
        ("1", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("2", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("3", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("4", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("5", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("6", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("7", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("8", [0, 0, 0, 0, 0, 0, 0, 0]),
    ]
    left_lanes = int((8 - len(lanes_all["ego"])) / 2)
    left_complete = False
    top_lanes = int((8 - len(lanes_all["left"])) / 2)
    top_complete = False
    right_lanes = int((8 - len(lanes_all["straight"])) / 2)
    right_complete = False
    bottom_lanes = int((8 - len(lanes_all["right"])) / 2)
    bottom_complete = False
    i = 0

    while any(
        [not left_complete, not top_complete, not right_complete, not bottom_complete]
    ):

        # 1. ego/left
        if not top_complete:
            for j in range(left_lanes):
                key_value_pairs[j][1][i] = 3

        # 2. top
        if not right_complete:
            for j in range(top_lanes):
                key_value_pairs[i][1][-1 - j] = 3

        # 3. right
        if not bottom_complete:
            for j in range(right_lanes):
                key_value_pairs[-1 - j][1][-1 - i] = 3

        # 4. bottom
        if not left_complete:
            for j in range(bottom_lanes):
                key_value_pairs[-1 - i][1][j] = 3

        # check if sides of matrix are already complete
        i = i + 1
        if i == 8:
            break
        if i == bottom_lanes:
            bottom_complete = True
        if i == right_lanes:
            right_complete = True
        if i == top_lanes:
            top_complete = True
        if i == left_lanes:
            left_complete = True

    ego_transform = ego_vehicle.get_transform()
    ego_rotation = ego_transform.rotation
    yaw = ego_rotation.yaw
    return key_value_pairs, lanes_all, junction_roads, yaw


# Inner junction:
def get_clostest_starting_waypoint(junction_waypoints, ego_location):
    """
    Find the closest starting waypoint to the ego vehicle from a list of junction waypoints.

    Parameters:
        junction_waypoints (list): List of tuples containing waypoint and lane ID pairs.
                                   Format: [(Waypoint, lane_id), ...]
        ego_vehicle (Vehicle): The ego vehicle.

    Returns:
        Waypoint: The closest starting waypoint to the ego vehicle.
    """
    closest_start_wp = junction_waypoints[0][0]
    # get closest start waypoint to ego
    for start_wp, _ in junction_waypoints:
        if distance(start_wp.transform.location, ego_location) < distance(
            closest_start_wp.transform.location, ego_location
        ):
            closest_start_wp = start_wp

    return closest_start_wp


def get_all_lanes(ego_vehicle, ego_wp, junction_waypoints, road_lane_ids, direction_angle):
    """
    Get all lanes related to the junction and the corresponding directions from the ego vehicle's perspective.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle.
        ego_wp (Waypoint): The waypoint associated with the ego vehicle.
        junction (Junction): The junction of interest.
        road_lane_ids (list): List of road and lane IDs in the world map.
        direction_angle (float): The angle used to determine directions from the ego vehicle.

    Returns:
        dict, list: A dictionary containing lists of lanes for each direction
                    and a list of junction roads with their respective directions and end waypoints.
    """
    ego_location = ego_vehicle.get_location()
    road_id_ego = str(ego_wp.road_id)
    lane_id_ego = ego_wp.lane_id
    start_wps = [[], [], []]
    end_wps = [[], [], [], []]
    closest_start_wp = get_clostest_starting_waypoint(junction_waypoints, ego_location)

    for start_wp, end_wp in junction_waypoints:
        # end
        road_id_end_wp = str(end_wp.next(2)[0].road_id)
        lane_id_end_wp = str(end_wp.next(2)[0].lane_id)
        end_wps[0].append(end_wp)
        end_wps[1].append(road_id_end_wp)
        if road_id_end_wp != road_id_ego:
            end_wps[2].append(
                get_waypoint_direction(
                    ego_vehicle, closest_start_wp, end_wp, direction_angle
                )
            )
        elif road_id_end_wp == road_id_ego:
            end_wps[2].append("ego")
        end_wps[3].append(lane_id_end_wp)

    # get distinct road id + corresponding direction from ego + end_wp
    junction_roads = []
    for i in range(len(end_wps[0])):
        if end_wps[1][i] not in [x[0] for x in junction_roads]:
            junction_roads.append(
                [end_wps[1][i], end_wps[2][i], end_wps[0][i], end_wps[3][i]]
            )

    lanes_all = {
        "ego": [],
        "left": [],
        "straight": [],
        "right": [],
    }  # direction from ego perpective
    for lane_id in road_lane_ids:  # iterate through all lanes of map
        for road in junction_roads:  # for each road of the junction
            if road[0] == lane_id.split("_")[0]:
                lanes_all[road[1]].append(lane_id.split("_")[1])

    return lanes_all, junction_roads


def remove_elements(lst):
    result = []
    for i in range(len(lst)):
        should_remove = False
        for j in range(i + 1, len(lst)):
            if abs(lst[i] - lst[j]) <= 1.5:
                should_remove = True
                break
        if not should_remove:
            result.append(lst[i])
    return result


def find_center_points(values):
    """
    Find center points and extents of a given list of values.

    Parameters:
        values (list): A list of numeric values.

    Returns:
        list: A list of tuples, where each tuple contains the center point and extent of adjacent values.
    """
    values = list(dict.fromkeys(values))
    values.sort()
    values = remove_elements(values)
    value_borders = []
    if len(values) == 6:
        for i in range(len(values) - 1):
            if i == 0:
                value_borders.append(values[i])
            elif i == len(values) - 2:
                value_borders.append(values[i + 1])
            else:
                border = (values[i] + values[i + 1]) / 2
                value_borders.append(border)
    elif len(values) == 5:
        if abs(values[0] - values[1]) > abs(values[-1] - values[-2]):
            reverse = False
        else:
            values.sort(reverse=True)
            reverse = True
        for i in range(len(values) - 1):
            if i == 0:
                value_borders.append(values[i])
            elif i == len(values) - 2:
                border = (values[i] + values[i + 1]) / 2
                value_borders.append(border)
                if values[-1] > values[-2]:
                    value_borders.append(values[i + 1] + 6)
                else:
                    value_borders.append(values[i + 1] - 6)
            else:
                border = (values[i] + values[i + 1]) / 2
                value_borders.append(border)
        if reverse:
            value_borders.sort()

    elif len(values) == 4:
        middle = (values[1] + values[2]) / 2
        # middle_left = (values[0] + middle) / 2
        # middle_right = (values[2] + middle) / 2
        value_borders.append(values[0])
        # value_borders.append(middle_left)
        value_borders.append(middle)
        # value_borders.append(middle_right)
        value_borders.append(values[-1])

    elif len(values) == 3:
        if abs(values[0] - values[1]) > abs(values[-1] - values[-2]):
            reverse = False
        else:
            values.sort(reverse=True)
            reverse = True

        middle = (values[1] + values[2]) / 2
        middle_left = (values[0] + middle) / 2
        middle_right = (values[2] + middle) / 2

        value_borders.append(values[0])
        # value_borders.append(middle_left)
        value_borders.append(middle)
        # value_borders.append(middle_right)
        if values[-1] > values[-2]:
            value_borders.append(values[-1] + 6)
        else:
            value_borders.append(values[-1] - 6)

        if reverse:
            value_borders.sort()

    center_values = []
    for i in range(len(value_borders) - 1):
        center_value = (value_borders[i] + value_borders[i + 1]) / 2
        extent = abs(value_borders[i + 1] - center_value)
        center_values.append((center_value, extent))

    return center_values


def group_waypoints(waypoints):
    """
    Group waypoints that are within a distance of 6 meters from each other.

    Parameters:
        waypoints (list): A list of carla.Waypoint objects.

    Returns:
        list: A list of lists, where each inner list contains waypoints that are close to each other.
    """
    groups = []
    groups.append([waypoints[0]])
    for i in range(1, len(waypoints)):
        group_identified = False
        for group in groups:
            if (group_identified == False) and (
                group[-1].transform.location.distance(waypoints[i].transform.location)
                <= 6
            ):
                group.append(waypoints[i])
                group_identified = True
        if group_identified == False:
            groups.append([waypoints[i]])
    return groups


def divide_bounding_box_into_boxes(junction):
    """
    Divide the bounding box of the given junction into smaller boxes.

    Parameters:
        junction (carla.Junction): A carla.Junction object.

    Returns:
        list: A list of boxes, where each box is represented as a list containing a carla.BoundingBox object and an integer value initialized to 0.
    """
    junction_waypoints = junction.get_waypoints(carla.LaneType().Driving)
    junction_waypoints = list(sum(junction_waypoints, ()))
    grouped_waypoints = group_waypoints(junction_waypoints)
    locations = []
    for group in grouped_waypoints:
        temp = []
        for wp in group:
            temp.append((wp.transform.location.x, wp.transform.location.y))
            temp = list(dict.fromkeys(temp))
        locations.append(temp)
    locations = [item for row in locations for item in row]

    x_values = []
    y_values = []
    for point in locations:
        x_values.append(point[0])
        y_values.append(point[1])

    center_values_x = find_center_points(x_values)
    center_values_y = find_center_points(y_values)

    center_values_y.sort(key=lambda x: x[0], reverse=True)
    points = list(itertools.product(center_values_x, center_values_y))

    boxes = []
    for point in points:
        box = [
            carla.BoundingBox(
                carla.Location(point[0][0], point[1][0], 0),
                carla.Vector3D(point[0][1], point[1][1], 100),
            ),
            0,
        ]
        boxes.append(box)

    return boxes


def fill_boxes(boxes, ego_vehicle, world):
    """
    Fill the boxes with information about surrounding cars.

    Parameters:
        boxes (list): A list of boxes, where each box is represented as a list containing a carla.BoundingBox object and an integer value initialized to 0.
        ego_vehicle (carla.Vehicle): The ego vehicle, a carla.Vehicle object.
        world (carla.World): The carla.World object.

    Returns:
        list: A list of boxes with updated integer values representing surrounding cars:
              0 - No car inside the box
              1 - The ego vehicle is inside the box
              2 - Another surrounding car is inside the box
    """
    surrounding_cars = []
    for actor in world.get_actors():
        if "vehicle" in actor.type_id:
            distance_to_actor = distance(
                actor.get_location(), ego_vehicle.get_location()
            )
            if distance_to_actor <= 100:
                surrounding_cars.append(actor)
    for car in surrounding_cars:
        car_location = car.get_location()
        
        for box in boxes:
            if car.id == ego_vehicle.id:
                if box[0].contains(car_location, carla.Transform()):
                    box[1] = 1
            else:
                if box[0].contains(car_location, carla.Transform()):
                    box[1] = 2

    return boxes


def build_grid(boxes):
    """
    Build a grid from the list of boxes.

    Parameters:
        boxes (list): A list of boxes, where each box is represented as a list containing a carla.BoundingBox object and an integer value.

    Returns:
        list: A 2D grid representation of the boxes.
    """
    x = boxes[0][0].location.x
    grid = []
    row = [boxes[0]]
    i = 1
    for i in range(1, len(boxes)):
        if boxes[i][0].location.x == x:
            row.append(boxes[i])
        else:
            grid.append(row)
            row = []

            row.append(boxes[i])
            x = boxes[i][0].location.x
    grid.append(row)
    return grid


def transpose_2d_array(array):
    transposed_array = [list(row) for row in zip(*array[::-1])]
    return transposed_array


def rotate_grid(grid, yaw):
    """
    Rotate the grid based on the yaw angle.

    Parameters:
        grid (list): A 2D grid representation of the boxes.
        yaw (float): The yaw angle in degrees.

    Returns:
        list: The rotated grid.
    """
    if 45 <= yaw <= 135:
        return transpose_2d_array(transpose_2d_array(grid))
    elif (135 <= yaw <= 180) or (-180 <= yaw <= -135):
        # Rotate 90 degrees to the left
        return transpose_2d_array(grid)
    elif -135 <= yaw <= -45:
        # Rotate 180 degrees
        return grid
    elif -45 <= yaw <= 45:
        # Rotate 90 degrees to the right
        return transpose_2d_array(transpose_2d_array(transpose_2d_array(grid)))


def check_flipping_rows_and_columns(ego_vehicle):
    ego_transform = ego_vehicle.get_transform()
    ego_rotation = ego_transform.rotation
    yaw = ego_rotation.yaw

    if (yaw >= -45 and yaw <= 45) or (
        (yaw >= -180 and yaw < -135) or (yaw >= 135 and yaw <= 180)
    ):
        return True


def get_grid_corners(junction_shape):
    """
    Find the corner coordinates of the junction in the given grid representation.

    Parameters:
        junction_shape (list): A 2D grid representation of the junction where each element
                               represents the road state at that grid cell. The elements
                               can have values 0, 1, 2, or 3. A value of 3 indicates no road,
                               while 0, 1, and 2 indicate different types of roads.

    Returns:
        list: A list containing the coordinates of the four corners of the junction.
              Each coordinate is represented as [row, column] in the grid format.

    Notes:
        - The grid representation must be a 2D list with 8 rows and 8 columns.
        - The function handles the possibility of a 90-degree rotation in the grid,
          ensuring correct corner identification regardless of the junction's orientation.
    """
    y_1 = None
    for i in range(8):
        if junction_shape[i][1][0] != 3:
            y_1 = i
            break

    for j in range(7, -1, -1):
        if junction_shape[j][1][0] != 3:
            y_2 = j
            break

    # left is no street
    if sum(junction_shape[0][1]) == 8 * 3:
        for k in range(8):
            if junction_shape[7][1][k] != 3:
                x_1 = k
                break

        for l in range(7, -1, -1):
            if junction_shape[7][1][l] != 3:
                x_2 = l
                break
    else:
        for k in range(8):
            if junction_shape[0][1][k] != 3:
                x_1 = k
                break

        for l in range(7, -1, -1):
            if junction_shape[0][1][l] != 3:
                x_2 = l
                break

    return [[y_1, x_1], [y_1, x_2], [y_2, x_1], [y_2, x_2]]


def detect_cars_inside_junction(key_value_pairs, ego_vehicle, junction, yaw, world):
    """
    Detect and mark cars inside the junction in the grid representation.

    Parameters:
        key_value_pairs (list of tuples): A list of tuples containing lane IDs and their
                                         respective grid representations. The grid
                                         representations are 2D lists where each element
                                         represents the state of the cell.
        ego_vehicle (carla.Vehicle): The ego vehicle for reference.
        junction (carla.Junction): The carla.Junction object representing the junction.
        yaw (float): The yaw angle (rotation) of the ego vehicle in degrees.
        world (carla.World): The carla.World object representing the simulation world.

    Returns:
        dict: A dictionary with updated lane IDs and their corresponding grid representations
              after marking cars inside the junction.

    Notes:
        - The function uses grid representations to identify cars inside the junction.
        - It calculates the corners of the junction to determine the junction boundaries.
        - The `rotate_grid` function is used to handle the rotation of the junction if needed.
        - The function checks whether rows and columns should be flipped based on the ego vehicle's orientation.
        - The `divide_bounding_box_into_boxes`, `fill_boxes`, and `build_grid` functions are used to construct the grid.
    """
    grid_corners = get_grid_corners(key_value_pairs)

    rows_min = min([value[0] for value in grid_corners])
    rows_max = max([value[0] for value in grid_corners])
    cols_min = min([value[1] for value in grid_corners])
    cols_max = max([value[1] for value in grid_corners])

    rows = rows_max - rows_min + 1
    columns = cols_max - cols_min + 1

    if check_flipping_rows_and_columns(ego_vehicle):
        rows, columns = columns, rows
    boxes = divide_bounding_box_into_boxes(junction)

    boxes = fill_boxes(boxes, ego_vehicle, world)

    grid = build_grid(boxes)

    grid = rotate_grid(grid, yaw)

    i = rows_min
    for row in grid:
        j = cols_min
        for cell in row:
            key_value_pairs[i][1][j] = cell[1]
            j = j + 1
        i = i + 1

    return dict(key_value_pairs)


def is_highway_junction(ego_vehicle, ego_wp, junction, road_lane_ids, direction_angle):
    lanes_all, junction_roads = get_all_lanes(
        ego_vehicle, ego_wp, junction.get_waypoints(carla.LaneType().Driving), road_lane_ids, direction_angle
    )

    highway_junction = False
    for _, lanes in lanes_all.items():
        if len(lanes) >= 8:
            highway_junction = True
            break
    return highway_junction


# Inner junction:
class Point:
    def __init__(self, x, y, z):
        self.x = x
        self.y = y
        self.z = z

    def __str__(self):
        return f"{(self.x, self.y)}"


# cluster wps of highway junctions
def remove_wps(closest_waypoint, wps):
    wps_on_ego_lane = []
    for group in wps:
        enough_distance = (
            distance(group[0].transform.location, group[1].transform.location) > 5
        )

        if (
            group[0].road_id == closest_waypoint.road_id
            or group[0].previous(10)[0].road_id
            == closest_waypoint.previous(10)[0].road_id
        ) and enough_distance:
            wps_on_ego_lane.append(group)
    exit_and_entry_wp = []
    for group in wps:
        enough_distance = (
            distance(group[0].transform.location, group[1].transform.location) > 3
        )
        for wp in wps_on_ego_lane:
            if (
                distance(wp[0].transform.location, group[0].transform.location) < 7
                and enough_distance
                and group not in wps_on_ego_lane
            ):
                exit_and_entry_wp.append(group)
                break
            elif (
                distance(wp[1].transform.location, group[1].transform.location) < 7
                and enough_distance
                and group not in wps_on_ego_lane
            ):
                exit_and_entry_wp.append(group)

    result = wps_on_ego_lane + exit_and_entry_wp
    return list(dict.fromkeys(result))


def clustering_algorithm(points, threshold):
    R = []  # Resulting list of clusters
    while points:
        # Pick a random point from the remaining points
        current_point = random.choice(points)
        C = [current_point]  # Create a new cluster with the current point
        points.remove(current_point)  # Remove the current point from the list

        point_added = True
        while point_added:
            point_added = False
            for i in range(len(points)):
                if points[i][0]:
                    point = points[i]
                    for cluster_point in C:
                        if (
                            distance(
                                point[0].transform.location,
                                cluster_point[0].transform.location,
                            )
                            < threshold
                        ):
                            C.append(point)  # Add the point to the cluster
                            points.remove(point)  # Remove the point from the list
                            point_added = True
                            break
                if point_added:
                    break

        R.append(C)  # Add the cluster to the resulting list

    return R


def mark_start_end(tuples_list):
    modified_list = [
        ((first, "start"), (second, "end")) for first, second in tuples_list
    ]
    return modified_list


def remove_unnecessary_waypoints_in_group(wps_grouped):
    for group in wps_grouped:
        unique_waypoints = []
        if len(group) > 4:
            road_ids = [str(wp[0].road_id) for wp in group]
            for wp in group:
                if road_ids.count(str(wp[0].road_id)) == 1:
                    unique_waypoints.append(wp)
            for unique_wp in unique_waypoints:
                group.remove(unique_wp)

    return wps_grouped


def prepare_waypoints(closest_waypoint, wps, ego_waypoint):
    wps = remove_wps(closest_waypoint, wps)
    wps = mark_start_end(wps)
    wps = list(sum(wps, ()))
    wps = [wp for wp in wps if wp[0]]
    wps_grouped = clustering_algorithm(wps, 6)
    wps_grouped = remove_unnecessary_waypoints_in_group(wps_grouped)
    return wps_grouped


# highway entry/exit
def get_exit_or_entry(wps, exit_or_entry, junction):
    junction_wps = junction.get_waypoints(carla.LaneType().Driving)

    if exit_or_entry == "exit":
        order = ["end", "start"]
    elif exit_or_entry == "entry":
        order = ["start", "end"]
    first = []
    second = []
    for group in wps:
        if len(group) != 4:
            for wp in group:
                if wp[1] == order[0]:
                    first.append(wp[0])

    for first_wp in first:
        for wp_pair in junction_wps:
            if wp_pair[0].id == first_wp.id:
                second.append(wp_pair[1])
                break
            elif wp_pair[1].id == first_wp.id:
                second.append(wp_pair[0])
                break

    if not first or not second:
        print("Error in get_exit()")
        return None
    if exit_or_entry == "exit":
        return (second, first)  # first = end_wp, second = start_wp
    elif exit_or_entry == "entry":
        return (first, second)  # first = start_wp, second = end_wp


def get_highway_shape(wps_grouped, junction):      
    # Check if entry or exit
    wps_grouped.sort(key=len)
    if len(wps_grouped[0]) == 2:
        if wps_grouped[0][0][1] != wps_grouped[0][1][1]:
            highway_type = "entry_and_exit"
        elif wps_grouped[0][0][1] == "start":
            highway_type = "dual_entry"
        elif wps_grouped[0][0][1] == "end":
            highway_type = "dual_exit"

    elif len(wps_grouped[0]) == 1:
        if wps_grouped[0][0][1] == "start":
            highway_type = "single_entry"
        else:
            highway_type = "single_exit"
    else:
        print("Junction object w/o shape")
        return ("normal_highway", 4, None, None)

    # Check number of lanes
    road_ids = [wp[0].road_id for wp in wps_grouped[1]]
    if len(set(road_ids)) == 1:
        straight_lanes = 4
    else:
        straight_lanes = 3

    entry_wps = None
    exit_wps = None
    # get start & end waypoint of entry / exit as tuple
    if highway_type in ["entry_and_exit", "single_entry", "dual_entry"]:
        entry_wps = get_exit_or_entry(wps_grouped, "entry", junction)
    if highway_type in ["entry_and_exit", "single_exit", "dual_exit"]:
        exit_wps = get_exit_or_entry(wps_grouped, "exit", junction)

    return (highway_type, straight_lanes, entry_wps, exit_wps)


def get_distance_junction_start(wp):
    x = 1
    while wp.previous(x)[0].is_junction:
        x = x + 1
    return x


def get_distance_junction_end(wp):
    x = 1
    while wp.next(x)[0].is_junction:
        x = x + 1
    return x


def search_entry_or_exit(
    world_map,
    ego_vehicle_location,
    direction,
    exit_entry_found,
    matrix,
    highway_shape,
    wps,
    entry_wps,
    exit_wps,
    degree,
    junction_id,
    right_lane_end_wp,
    right_lane_start_wp,
):
    ego_waypoint = world_map.get_waypoint(ego_vehicle_location)

    # for dual entry/exit we compare yaw values based on most right highway lane
    if highway_shape[0] in ["dual_exit", "dual_entry"]:
        initial_wp = ego_waypoint
        if direction == "before":
            while True:
                tmp_right_lane = initial_wp.get_right_lane()
                if (tmp_right_lane is None) or (
                    tmp_right_lane.lane_type != carla.LaneType().Driving
                ):
                    break
                initial_wp = tmp_right_lane
        # for looking behind, we consider left line first bc. otherwise wp.previous() go on entry
        elif direction == "behind":
            while True:
                tmp_left_lane = initial_wp.get_left_lane()
                if (tmp_left_lane is None) or (
                    tmp_left_lane.lane_type != carla.LaneType().Driving
                ):
                    break
                initial_wp = tmp_left_lane

    # for other entry/exit we compare yaw on entry/exit
    else:
        initial_wp = ego_waypoint

    col_entryExit = None
    # Look in front
    if direction == "before":
        z = 3  # column help in matrix
        col_distances = [6, 15, 35, 55, 75]
    # Look behind
    elif direction == "behind":
        z = (
            -1
        )  # column help in matrix; -1 because in list is first value only a threshold to see when junction too far behind
        if highway_shape[0] in [
            "entry_and_exit"
        ]:  # special because entry & exit are very close compared to normal column distance in matrix
            col_distances = [55, 35, 10, 6]
        elif highway_shape[0] in ["dual_exit"]:  # special because for dual exit
            col_distances = [55, 35, 20, 6]
        else:
            col_distances = [55, 35, 15, 6]
    for i, col_distance in enumerate(
        col_distances
    ):  # index 0 equals distance closest (10)
        if exit_entry_found:
            break
        for j in [
            -5,
            0,
            5,
        ]:  #  check 3 values in each column, bc. otherwise sometimes jumps are bigger than junction
            if exit_entry_found:
                break
            # Look in front
            if direction == "before":
                # if (col_distance == 4) and (j == 5):
                #     j = 0
                wp = initial_wp.next(col_distance + j)[0]
            # Look behind
            elif direction == "behind":
                # j = 0
                if highway_shape[0] == ["dual_entry", "dual_exit"]:
                    wp = initial_wp.previous(col_distance + j)[0]
                    while True:
                        tmp_right_lane = wp.get_right_lane()
                        if (tmp_right_lane is None) or (
                            tmp_right_lane.lane_type != carla.LaneType().Driving
                        ):
                            break
                        wp = tmp_right_lane
                else:
                    wp = initial_wp.previous(col_distance + j)[0]

            if (wp.is_junction) and (wp.get_junction().id == junction_id):
                # now we have to check if parallel or curve away, depending on exit / entry as first search
                if highway_shape[0] in [
                    "entry_and_exit",
                    "single_exit",
                ]:  # search for exit first
                    distance_junction_start = get_distance_junction_start(wp)
                    yaw_difference = abs(exit_wps[0][0].transform.rotation.yaw) - abs(
                        exit_wps[0][0]
                        .next(distance_junction_start)[0]
                        .transform.rotation.yaw
                    )
                    already_in_curve = abs(yaw_difference) > degree
                elif highway_shape[0] in ["single_entry"]:
                    distance_junction_end = get_distance_junction_end(wp)
                    yaw_difference = abs(entry_wps[1][0].transform.rotation.yaw) - abs(
                        entry_wps[1][0]
                        .previous(distance_junction_end)[0]
                        .transform.rotation.yaw
                    )
                    already_in_curve = abs(yaw_difference) < degree
                elif highway_shape[0] in ["dual_entry"]:
                    yaw_difference = abs(
                        right_lane_end_wp.transform.rotation.yaw
                    ) - abs(wp.transform.rotation.yaw)
                    already_in_curve = abs(yaw_difference) < degree
                elif highway_shape[0] in ["dual_exit"]:
                    yaw_difference = abs(
                        right_lane_end_wp.transform.rotation.yaw
                    ) - abs(wp.transform.rotation.yaw)
                    already_in_curve = abs(yaw_difference) > degree
                else:
                    already_in_curve = False

                # if ego already too far away, dont show in matrix
                if (
                    (direction == "behind")
                    and already_in_curve
                    and (col_distance == 55)
                ):
                    exit_entry_found = True
                    break

                # if wp already in curve, update matrix
                if already_in_curve:
                    matrix[6][1][z + i] = 0
                    matrix[7][1][z + i] = 0
                    col_entryExit = z + i
                    if (
                        z + i + 1 < 8
                    ):  # case: curve is 70m infront (last column in matrix)
                        if highway_shape[0] in ["entry_and_exit"]:
                            if highway_shape[1] == 3:
                                matrix[5][1][z + i + 1] = 3
                            # add entry, if closer than 70m
                            if z + i + 2 < 8:
                                matrix[6][1][z + i + 2] = 0
                                matrix[7][1][z + i + 2] = 0
                        elif highway_shape[0] in ["dual_entry"]:
                            matrix[6][1][z + i + 1] = 0
                            matrix[7][1][z + i + 1] = 0
                        elif highway_shape[0] in ["dual_exit"]:
                            matrix[6][1][z + i + 1] = 0
                            matrix[7][1][z + i + 1] = 0
                    exit_entry_found = True
                    break

    return exit_entry_found, col_entryExit, matrix

def insert_in_matrix(matrix, car, ego_vehicle, col, row):
    if car.id == ego_vehicle.id:
        matrix[row][1][col] = 1
    else:
        if matrix[row][1][col] == 1:
            dot_product = check_car_in_front_or_behind(
                ego_vehicle.get_location(),
                car.get_location(),
                ego_vehicle.get_transform().rotation,
            )
            if dot_product > 0:
                if row == 6 or row == 7:
                    matrix[row - 1][1][col] = 2
                elif row == 5 and col < 7:
                    matrix[row][1][col + 1] = 2
            else:
                if row == 6:
                    matrix[row + 1][1][col] = 2
                elif row == 5 and col > 0:
                    matrix[row][1][col - 1] = 2
        else:
            matrix[row][1][col] = 2

def get_right_lane_wp(wps):
    right_lane_wp = wps[0][0]  # first wp of group
    for wp in wps:
        if abs(wp[0].lane_id) > abs(right_lane_wp.lane_id):
            right_lane_wp = wp[0]
    return right_lane_wp

def update_matrix(
    world_map,
    ego_vehicle,
    ego_location,
    highway_shape,
    wps,
    matrix,
    ego_waypoint,
    junction,
    cars_on_entryExit,
    direction_angle,
    ghost=False,
    on_entry = False,
):
    if highway_shape[1] == 3:
        matrix[list(matrix.keys())[5]] = [0, 0, 0, 0, 0, 0, 0, 0]

    matrix = list(matrix.items())
    exit_entry_found = False
    entry_wps, exit_wps = highway_shape[2], highway_shape[3]

    # TODO: put in a config file
    # set the angle threshold depending on curve type
    if highway_shape[0] == "entry_and_exit":
        degree = 30
    elif highway_shape[0] in ["single_entry", "single_exit"]:
        degree = 20
    elif highway_shape[0] in ["dual_entry"] or highway_shape[0] in ["dual_exit"]:
        degree = 18  # TODO: test and fine tuning of degree
    else:
        # just in case
        degree = 20

    # get right_lane_end_wp & right_lane_start_wp for dual entry & exit surrounding cars angle calculation
    for wps_group in wps:
        if (len(wps_group) == 4) and (wps_group[0][1] == "end"):
            right_lane_end_wp = get_right_lane_wp(wps_group)
        elif (len(wps_group) == 4) and (wps_group[0][1] == "start"):
            right_lane_start_wp = get_right_lane_wp(wps_group)

    # First: Look behind ego
    exit_entry_found, col_entryExit, matrix = search_entry_or_exit(
        world_map,
        ego_location,
        "behind",
        exit_entry_found,
        matrix,
        highway_shape,
        wps,
        entry_wps,
        exit_wps,
        degree,
        junction.id,
        right_lane_end_wp,
        right_lane_start_wp,
    )
    # if not yet found, look in front
    if exit_entry_found == False:
        _, col_entryExit, matrix = search_entry_or_exit(
            world_map,
            ego_location,
            "before",
            exit_entry_found,
            matrix,
            highway_shape,
            wps,
            entry_wps,
            exit_wps,
            degree,
            junction.id,
            right_lane_end_wp,
            right_lane_start_wp,
        )

    # get road ids of: entry/exit, city road before and highway road after junction object
    entry_road_id = []
    exit_road_id = []
    entry_city_road = []
    exit_city_road = []
    entry_city_lane_id = []    
    exit_city_lane_id = []
    entry_highway_road = []
    exit_highway_road = []
    entry_highway_lane_id = []
    exit_highway_lane_id = []

    if entry_wps:
        for entry_wp in entry_wps[0]:
            entry_city_road.append(entry_wp.previous(3)[0].road_id)
            entry_city_lane_id.append(abs(entry_wp.previous(3)[0].lane_id))
            entry_road_id.append(entry_wp.road_id)
        for entry_wp in entry_wps[1]:
            entry_highway_road.append(entry_wp.next(3)[0].road_id)
            entry_highway_lane_id.append(abs(entry_wp.next(3)[0].lane_id))
    if exit_wps:
        for exit_wp in exit_wps[1]:
            exit_city_road.append(exit_wp.next(3)[0].road_id)
            exit_city_lane_id.append(abs(exit_wp.next(3)[0].road_id))
            exit_road_id.append(exit_wp.road_id)
        for exit_wp in exit_wps[0]:
            exit_highway_road.append(exit_wp.previous(3)[0].road_id)
            exit_highway_lane_id.append(abs(exit_wp.previous(3)[0].lane_id))


    # detect surrounding cars on exit/entry
    for car in cars_on_entryExit:
        col = calculate_position_in_matrix(
            ego_location,
            ego_vehicle,
            car,
            dict(matrix),
            world_map,
            ego_vehicle.get_velocity(),
            ghost,
        )
        if col is None:
            continue
        # matrix = list(matrix)
        if matrix:
            ego_already_in_matrix = False
            for key, row in matrix:
                if 1 in row:
                    ego_already_in_matrix = True

            if (ego_already_in_matrix and car.id == ego_vehicle.id):
                continue
            other_car_waypoint = world_map.get_waypoint(car.get_location())
            other_car_road_id = other_car_waypoint.road_id
            other_car_lane_id = other_car_waypoint.lane_id
            
            if other_car_road_id in entry_city_road + exit_city_road and not on_entry:
                continue
            distance_clostest_starting_waypoint = distance(get_clostest_starting_waypoint(junction.get_waypoints(carla.LaneType().Driving), car.get_location()).transform.location, car.get_location())
            if other_car_road_id in entry_city_road + exit_city_road:
                if distance_clostest_starting_waypoint > 30:
                    continue
                elif distance_clostest_starting_waypoint < 15:
                    row = 6
                else:
                    row = 7
                if highway_shape[0] in ["entry_and_exit"]:
                    col_entry = col_entryExit + 2
                    col_exit = col_entryExit
                elif highway_shape[0] in ["dual_entry"]:
                    entry_city_lane_id.sort()
                    # dual entry
                    if abs(other_car_lane_id) == entry_city_lane_id[0]:
                        col_entry = col_entryExit
                    elif abs(other_car_lane_id) == entry_city_lane_id[1]:
                        col_entry = col_entryExit + 1


                elif highway_shape[0] in ["dual_exit"]:
                    entry_city_lane_id.sort()
                    # dual exit
                    if abs(other_car_lane_id) == entry_city_lane_id[0]:
                        col_entry = col_entryExit +1
                    elif abs(other_car_lane_id) == entry_city_lane_id[1]:
                        col_entry = col_entryExit
                else:
                    col_entry = col_entryExit
                    col_exit = col_entryExit
            


                if entry_city_road == exit_city_road:
                    if is_junction_behind(other_car_waypoint, 40):
                        check_junction = get_junction_behind(other_car_waypoint, 40)
                        road_lane_ids = get_all_road_lane_ids(world_map)
                        if is_highway_junction(car, other_car_waypoint, check_junction, road_lane_ids, direction_angle):
                            on_exit_street = True
                        else:
                            on_exit_street = False
                else:
                    on_exit_street = False
                if (other_car_road_id in entry_city_road) and (col_entry < 8) and not on_exit_street:
                    insert_in_matrix(matrix, car, ego_vehicle, col_entry, row)
                elif (other_car_road_id in exit_city_road) and (col_exit < 8) and on_exit_street:
                    insert_in_matrix(matrix, car, ego_vehicle, col_exit, row)
                continue
                
            distance_junction_end = get_distance_junction_end(
                other_car_waypoint
            )
            distance_junction_start = get_distance_junction_start(
                other_car_waypoint
            )
            if other_car_road_id in entry_road_id + entry_highway_road:
                if highway_shape[0] == "dual_entry": 
                    yaw_difference = abs(
                        right_lane_end_wp.transform.rotation.yaw
                    ) - abs(
                        right_lane_end_wp.previous(distance_junction_end)[
                            0
                        ].transform.rotation.yaw
                    )
                else:
                    yaw_difference = abs(entry_wps[1][0].transform.rotation.yaw) - abs(
                        entry_wps[1][0]
                        .previous(distance_junction_end)[0]
                        .transform.rotation.yaw
                    )
            elif other_car_road_id in exit_road_id + exit_highway_road:
                if highway_shape[0] == "dual_exit":
                    yaw_difference = abs(
                        right_lane_start_wp.transform.rotation.yaw
                    ) - abs(
                        right_lane_start_wp.next(distance_junction_start)[
                            0
                        ].transform.rotation.yaw
                    )
                else:

                    yaw_difference = abs(exit_wps[0][0].transform.rotation.yaw) - abs(
                        exit_wps[0][0]
                        .next(distance_junction_start)[0]
                        .transform.rotation.yaw
                    )
             
            already_in_curve = abs(yaw_difference) > degree
            already_after_curve = abs(yaw_difference) > degree * 2
            if not on_entry:
                if already_after_curve:
                    row = 7
                elif already_in_curve:

                    row = 6
                else:
                    row = 5
            else:
                row = 5
                """
                distance_bounding_box = distance(junction.bounding_box.location, car.get_location())
                print(distance_bounding_box)
                if distance_bounding_box < 30:
                    row = 5
                if distance_bounding_box < 40:
                    row = 6
                else:
                    row = 7
                """
            if already_in_curve:
                if highway_shape[0] in ["entry_and_exit"]:
                    col_entry = col_entryExit + 2
                    col_exit = col_entryExit
                elif highway_shape[0] in ["dual_entry"]:
                    # dual entry
                    entry_road_id = [
                        x for _, x in sorted(zip(entry_highway_lane_id, entry_road_id))
                    ]
                    entry_highway_lane_id.sort()
                    if other_car_road_id == entry_road_id[0]:
                        col_entry = col_entryExit
                    elif other_car_road_id == entry_road_id[1]:
                        col_entry = col_entryExit + 1
                elif highway_shape[0] in ["dual_exit"]:
                    # dual exit
                    exit_road_id = [
                        x for _, x in sorted(zip(exit_highway_lane_id, exit_road_id))
                    ]
                    exit_highway_lane_id.sort()
                    if other_car_road_id == exit_road_id[0]:
                        col_exit = col_entryExit + 1
                    elif other_car_road_id == exit_road_id[1]:
                        col_exit = col_entryExit  # + 1
                else:
                    col_entry = col_entryExit
                    col_exit = col_entryExit
            else:
                if (highway_shape[1] == 3) and (col <= (col_entryExit + 1)):
                    col_entry = col_entryExit + 2
                    col_exit = col_entryExit
                else:
                    if highway_shape[0] in ["dual_entry"]:
                        # dual entry
                        entry_road_id = [
                            x
                            for _, x in sorted(
                                zip(entry_highway_lane_id, entry_road_id)
                            )
                        ]
                        entry_highway_lane_id.sort()
                        if other_car_road_id == entry_road_id[0]:
                            row = row - 1
                    elif highway_shape[0] in ["dual_exit"]:
                        # dual exit
                        exit_road_id = [
                            x
                            for _, x in sorted(zip(exit_highway_lane_id, exit_road_id))
                        ]
                        exit_highway_lane_id.sort()
                        if other_car_road_id == exit_road_id[0]:
                            row = row - 1
                    col_entry = col
                col_exit = col

            entry_streets = entry_road_id + entry_highway_road
            exit_streets = exit_road_id + exit_highway_road
            if (other_car_road_id in entry_streets) and (col_entry < 8):
                insert_in_matrix(matrix, car, ego_vehicle, col_entry, row)

            elif (other_car_road_id in exit_streets) and (col_exit < 8):
                insert_in_matrix(matrix, car, ego_vehicle, col_exit, row)

    return dict(matrix)


def get_waypoint_on_highway_junction(
    ego_waypoint, ego_vehicle_location, wps, world_map
):
    bbx_location = get_junction_ahead(ego_waypoint, 40).bounding_box.location

    # Calculate the direction vector from bounding box to ego vehicle
    direction = ego_vehicle_location - bbx_location
    distance = math.sqrt(
        direction.x**2 + direction.y**2 + direction.z**2
    )  # Calculate the distance between the two points

    # Normalize the direction vector if the distance is not zero
    if distance > 0:
        direction /= distance

    # Move the bounding box location closer to the ego vehicle by a certain distance (e.g., 5 meters)
    distance_to_move = 5.0
    new_bbx_location = bbx_location + direction * distance_to_move

    highway_waypoint = world_map.get_waypoint(new_bbx_location).previous(40)[0]

    closest_waypoint = get_clostest_starting_waypoint(
        wps, highway_waypoint.transform.location
    )

    return closest_waypoint, new_bbx_location


#####################
# Functions not in use
#####################


def get_road(lane_start, lane_end):
    waypoints = []
    waypoints.append(lane_start)
    next_wp = lane_start.next(2)[0]
    while next_wp.road_id == waypoints[-1].road_id:
        waypoints.append(next_wp)
        next_wp = next_wp.next(2)[0]
    return waypoints


def waypoints_on_straight_line(waypoints, tolerance=0.1):
    """Returns True if the given waypoints lie on a straight line, False otherwise.
    The tolerance parameter controls the maximum deviation from a straight line that is allowed.
    """
    if len(waypoints) < 2:
        return False  # If there are fewer than two waypoints, they can't lie on a line

    # Calculate the slope between the first and second waypoints
    x1, y1 = waypoints[0].transform.location.x, waypoints[0].transform.location.y
    x2, y2 = waypoints[1].transform.location.x, waypoints[1].transform.location.y
    if x2 - x1 != 0:  # Check for vertical line
        slope = (y2 - y1) / (x2 - x1)
    else:
        slope = math.inf

    # Check if the remaining waypoints lie on the same line within the given tolerance
    for i in range(2, len(waypoints)):
        x, y = waypoints[i].transform.location.x, waypoints[i].transform.location.y
        if x - x1 != 0:  # Check for vertical line
            new_slope = (y - y1) / (x - x1)
        else:
            new_slope = math.inf
        if abs(new_slope - slope) > tolerance:
            return False
    return True


def getJunctionShapeForHighway(lanes_all):
    key_value_pairs = [
        ("1", [3, 3, 3, 3, 3, 3, 3, 3]),
        ("2", [3, 3, 3, 3, 3, 3, 3, 3]),
        ("3", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("4", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("5", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("6", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("7", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("8", [3, 3, 3, 3, 3, 3, 3, 3]),
    ]
    # get lanes on highway
    if len(lanes_all["ego"]) == 3:
        for i in range(8):
            key_value_pairs[6][1][i] = 3
        highway_exit_at = 5
    else:
        highway_exit_at = 6

    # check if exit, entry or both
    # exit and entry
    if len(lanes_all["right"]) == 2:
        (
            key_value_pairs[highway_exit_at][1][3],
            key_value_pairs[highway_exit_at][1][4],
        ) = (3, 3)
    elif np.sign(int(lanes_all["right"][0])) == np.sign(int(lanes_all["ego"][0])):
        for i in range(3, 8):
            key_value_pairs[highway_exit_at][1][i] = 3
    else:
        for i in range(5):
            key_value_pairs[highway_exit_at][1][i] = 3

    return key_value_pairs


### OLD VERSIONS ###


def get_center_values(extent, num_values, initial_number):
    interval = extent / num_values
    a = [initial_number + interval]
    current_number = initial_number + interval
    for i in range(num_values - 1):
        current_number = current_number + interval * 2
        a.append(current_number)
    return a


def divide_bounding_box_into_boxes_old(bounding_box, rows=2, cols=2):
    # Get the location and extent of the bounding box
    location = bounding_box.location
    extent = bounding_box.extent

    center_x_values = get_center_values(extent.x, cols, location.x - extent.x)
    center_y_values = get_center_values(extent.y, rows, location.y - extent.y)
    center_y_values.sort(reverse=True)

    new_x_extent = extent.x / cols
    new_y_extent = extent.y / rows

    points = list(itertools.product(center_x_values, center_y_values))

    boxes = []
    for point in points:
        box = [
            carla.BoundingBox(
                carla.Location(point[0], point[1], 0),
                carla.Vector3D(new_x_extent, new_y_extent, 100),
            ),
            0,
        ]
        boxes.append(box)
    return boxes


def getJunctionShape_old(ego_vehicle, ego_wp, junction, road_lane_ids, direction_angle):
    junction_waypoints = junction.get_waypoints(carla.LaneType().Driving)

    # group waypoints
    grouped_start_wp = group_waypoints([wp[0] for wp in junction_waypoints])
    grouped_end_wp = group_waypoints([wp[1] for wp in junction_waypoints])

    # get location of start waypoint closest to ego (maybe do in get_direction function)
    ego_location = ego_vehicle.get_location()
    closest_start_wp = grouped_start_wp[0][0]
    start_index = 0
    for i, group in enumerate(grouped_start_wp):
        for wp in group:
            if distance(wp.transform.location, ego_location) < distance(
                closest_start_wp.transform.location, ego_location
            ):
                closest_start_wp = wp
                start_index = i
    end_index = 0
    closest_end_wp = grouped_end_wp[0][0]
    for i, group in enumerate(grouped_end_wp):
        for wp in group:
            if distance(wp.transform.location, ego_location) < distance(
                closest_end_wp.transform.location, ego_location
            ):
                closest_end_wp = wp
                end_index = i

    # consider only groups of waypoints that are not closest to ego and check if all waypoints in that groups are left/right/straight
    ### we can derive shape in general of junction from ego-perspective
    incoming_directions = []
    for group in grouped_start_wp[:start_index] + grouped_start_wp[start_index + 1 :]:
        for wp in group:
            incoming_directions.append(
                get_waypoint_direction(
                    ego_vehicle, closest_start_wp, wp, direction_angle
                )
            )
    outgoing_directions = []
    for group in grouped_end_wp[:end_index] + grouped_end_wp[end_index + 1 :]:
        for wp in group:
            outgoing_directions.append(
                get_waypoint_direction(
                    ego_vehicle, closest_start_wp, wp, direction_angle
                )
            )
    all_directions = incoming_directions + outgoing_directions
    # X-Junction
    if all(
        [
            "left" in all_directions,
            "right" in all_directions,
            "straight" in all_directions,
        ]
    ):
        junction_shape = "X"
    # T-junction, no left
    elif all(["right" in all_directions, "straight" in all_directions]):
        junction_shape = "T, no left"
    # T-junction, no straight
    elif all(["left" in all_directions, "right" in all_directions]):
        junction_shape = "T, no straight"
    # T-junction, no right
    elif all(["left" in all_directions, "straight" in all_directions]):
        junction_shape = "T, no right"
    else:
        junction_shape = "Error"

    # check how many lanes the in/out going roads have
    key_value_pairs = [
        ("1", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("2", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("3", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("4", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("5", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("6", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("7", [0, 0, 0, 0, 0, 0, 0, 0]),
        ("8", [0, 0, 0, 0, 0, 0, 0, 0]),
    ]
    #### meine road wie viele Lanes
    road_ids = [
        str(ego_wp.road_id),
        "",
        "",
        "",
    ]  # 1.ego/left, 2. top, 3. right, 4. bottom of matrix
    for group in grouped_end_wp[:end_index] + grouped_end_wp[end_index + 1 :]:
        direction = get_waypoint_direction(
            ego_vehicle, closest_start_wp, group[0], direction_angle
        )
        if direction == "left":
            road_ids[1] = str(group[0].next(5)[0].road_id)
        elif direction == "straight":
            road_ids[2] = str(group[0].next(5)[0].road_id)
        elif direction == "right":
            road_ids[3] = str(group[0].next(5)[0].road_id)

    lanes_all = [[], [], [], []]  # 1.ego/left, 2. top, 3. right, 4. bottom of matrix
    for lane_id in road_lane_ids:  # iterate through all lanes of map
        for i, road_id in enumerate(road_ids):  # for each road of the junction
            if road_id == lane_id.split("_")[0]:
                lanes_all[i].append(lane_id.split("_")[1])
    # 1. ego/left
    for i in range(
        int((8 - len(lanes_all[0])) / 2)
    ):  # 8 cells in matrix; lanes[0] => ego/left
        key_value_pairs[i][1][0] = 3
        key_value_pairs[-1 - i][1][0] = 3
    # 2. top
    for i in range(
        int((8 - len(lanes_all[1])) / 2)
    ):  # 8 cells in matrix; lanes[0] => ego/left
        key_value_pairs[0][1][i] = 3
        key_value_pairs[0][1][-1 - i] = 3
    # 3. right
    for i in range(
        int((8 - len(lanes_all[2])) / 2)
    ):  # 8 cells in matrix; lanes[0] => ego/left
        key_value_pairs[i][1][-1] = 3
        key_value_pairs[-1 - i][1][-1] = 3
    # 4. botom
    for i in range(
        int((8 - len(lanes_all[3])) / 2)
    ):  # 8 cells in matrix; lanes[0] => ego/left
        key_value_pairs[-1][1][i] = 3
        key_value_pairs[-1][1][-1 - i] = 3

    #### alle end Routen wie viele Lanes
    #### Matrix bauen
    # finalize the junction structure

    # build the grid from inner part of junction: number of cells depends on in/out going roads and their lanes -> cut grid respectively
    # junction_bbox = junction.bounding_box  # Get the bounding box of the junction
    # junction_shape = (junction_bbox.extent.x, junction_bbox.extent.y)  # Get the shape of the junction as (width, length)

    # check if vehicles in junction, if yes check in which cell of grid their are currently; keep in mid persepctive of ego and rotation of grid

    # check if other vehicles are outside of junction but close by

    # calculate yaw value

    return dict(key_value_pairs)
