import carla
import math
import itertools
import collections
import pandas as pd
import numpy as np
import random


def check_road_change(ego_vehicle_location, road_lane_ids, front, world_map):
    """
    Determine if the ego vehicle is about to change to a different road (in next/previous 60m).

    Parameters:
        ego_vehicle (carla.Vehicle): The ego vehicle for which we want to check the road change.
        road_lane_ids (list): A list of all road-lane identifiers of the map, where each identifier is a string in the format "roadId_laneId". 
            Format: ["1_2", "2_1", "3_2"].
        front (bool): If True, check the road change in the front direction of the ego vehicle,
                      otherwise check in the rear direction.
        world_map (carla.Map): The map representing the environment.

    Returns:
        tuple: A tuple containing two elements:
            - next_road_id (str): The ID of the next/previous road if the ego vehicle is about to change
                                 to a different road, otherwise None.
            - next_lanes (list of str): A list of lane IDs of the next/previous road if the ego vehicle is
                                        about to change to a different road, otherwise None.
    """
    ego_vehicle_waypoint = world_map.get_waypoint(ego_vehicle_location)
    
    # get first waypoint infront of / behind ego vehicle that is not on the same road id
    if front: # look in front of ego vehicle
        for i in range(1, 60, 5):
            next_waypoint = ego_vehicle_waypoint.next(i)[0]
            if next_waypoint.road_id != ego_vehicle_waypoint.road_id:
                break
    else: # look behind ego vehicle
        for i in range(1, 60, 5):
            next_waypoint = ego_vehicle_waypoint.previous(i)[0]
            if next_waypoint.road_id != ego_vehicle_waypoint.road_id:
                break

    # get road_id and lanes of road in front of / behind ego vehicle
    next_lanes = None
    next_road_id = None
    if next_waypoint.road_id != ego_vehicle_waypoint.road_id:
        next_road_id = str(next_waypoint.road_id)
        next_lanes = [
            id.split("_")[1]
            for id in road_lane_ids
            if next_road_id == id.split("_")[0]
        ]
        
    # get lanes of ego vehicle's road
    our_lanes = [
        id.split("_")[1]
        for id in road_lane_ids
        if str(ego_vehicle_waypoint.road_id) == id.split("_")[0]
    ]
    
    # return next_road_id and next_lanes if they exist, otherwise return None
    if next_lanes:
        next_lanes.sort()
        our_lanes.sort()
    if next_lanes == our_lanes:
        return (next_road_id, next_lanes)
    else:
        return (None, None)


def check_ego_on_highway(ego_vehicle_location, road_lane_ids, world_map):
    """
    Check if the ego vehicle is on a highway based on its location. The function considers the ego vehicle to be on a highway if:
        - it's on a road that has at least six lanes 
        - or if it's on a road with a set of lanes with consecutive lane identifiers (e.g., 1, 2, 3) and contains at least three lanes 
          because a city road with more than three lanes is missing lane_id=0 (e.g., -2, -1, 1, 2)

    Args:
        ego_vehicle_location (carla.Location): The current location of the ego vehicle.
        road_lane_ids (list): A list of all road-lane identifiers of the map, where each identifier is a string
            in the format "roadId_laneId". For example, ["1_2", "2_1", "3_2"].
        world_map (carla.Map): A carla object representing the map of the world.

    Returns:
        bool: True if the ego vehicle is on a highway, False otherwise.
    """
    # get waypoints of ego and its left and right lanes
    waypoints = []
    ego_waypoint = world_map.get_waypoint(ego_vehicle_location)
    waypoints.append(ego_waypoint)
    if ego_waypoint.get_left_lane() is not None:
        waypoints.append(ego_waypoint.get_left_lane())
    if ego_waypoint.get_right_lane() is not None:
        waypoints.append(ego_waypoint.get_right_lane())
    
    # check for all waypoints if they are on a highway, in case they have different road_id's
    for wp in waypoints:
        ego_vehilce_road_id = wp.road_id
        # get all lanes of the respective road
        lanes = []
        for id in road_lane_ids:
            if str(ego_vehilce_road_id) == id.split("_")[0]:
                lanes.append(id.split("_")[1])
        # cast lane_id's to int and check for highway condition
        lanes = [int(lane) for lane in lanes]
        if len(lanes) >= 6 or (
            sorted(lanes) == list(range(min(lanes), max(lanes) + 1)) and len(lanes) >= 3
        ):
            return True

    return False


def get_all_road_lane_ids(world_map):
    """
    Retrieve a set of unique road and lane identifiers in the format "roadId_laneId" from the given world map.

    Args:
        world_map (carla.Map): The map of the world from which road and lane identifiers are obtained.

    Returns:
        set: A set containing unique road and lane identifiers in the format "roadId_laneId".
    """
    road_lane_ids = set()

    # iterate through all waypoints in the world map
    for waypoint in world_map.generate_waypoints(1.0):
        # extract lane and road id's 
        lane_id = waypoint.lane_id
        road_id = waypoint.road_id
        # add road and lane identifiers to set
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


def create_basic_matrix(ego_vehicle_location, road_lane_ids, world_map, ghost=False, ego_on_bad_highway_street = False):
    """
    Create a matrix representing the lanes around the ego vehicle.

    Parameters:
        ego_vehicle_location (carla.Location): The location object of ego vehicle for which to create the city matrix.
        road_lane_ids (list): A list of all road-lane identifiers of the map, where each identifier is a string in the format "roadId_laneId". 
            Format: ["1_2", "2_1", "3_2"].
        world_map (carla.Map): The map representing the environment.
        ghost (bool): Ghost mode when ego is exiting/entrying a highway - fix a location of an imaginary vehicle on highway to correctly build matrix from this ghost perspective.
        ego_on_bad_highway_street (bool): Indicates that ego is on the right lane of a highway that is an exit/entry and accounts as another road_id

    Returns:
        collections.OrderedDict: An ordered dictionary representing the city matrix. The keys for existing lanes are the lane IDs in the format "road_id_lane_id". 
            For non-existing lanes different placeholder exist, e.g.  left_outer_lane, left_inner_lane, No_4th_lane, No_opposing_direction
            The values indicate whether a vehicle is present: 0 - No vehicle, 1 - Ego vehicle, 3 - No road.
            Format example: {
                "left_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "left_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "1_2": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_1": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_-1": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_-2": [0, 0, 0, 0, 0, 0, 0, 0],
                "right_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "right_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
            }
    """
    # Get lane & road id for ego_vehicle
    ego_vehilce_waypoint = world_map.get_waypoint(ego_vehicle_location)
    ego_vehilce_lane_id = str(ego_vehilce_waypoint.lane_id)
    ego_vehilce_road_id = str(ego_vehilce_waypoint.road_id)

    # get all lanes of ego's road
    lanes = []
    for id in road_lane_ids:
        if ego_vehilce_road_id == id.split("_")[0]:
            lanes.append(id.split("_")[1])
    lanes.sort()
    lanes = [int(id) for id in lanes]
    
    # split lanes into directions & sort, e.g. [-2,-1,1,2] -> [[-2,-1],[2,1]]
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
    # Identify list of lanes of ego's direction and opposite direction
    for direction in lanes_splitted:
        if int(ego_vehilce_lane_id) in direction:
            ego_direction = direction
        else:
            other_direction = direction
    
    if len(ego_direction) >= 4: 
        # ego is on a highway with 4 or more lanes
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
        # ego is on highway with 3 lanes, i.e. the 4th lane is most likely an exit/entry
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
        # ego is on normal city road with 2 lanes in each direction
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
        # ego is on road with 2 lanes in one direction and no lanes in the other direction
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
        # ego is on normal road with 1 lane in each direction
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
        # ego is on road with 1 lane in one direction and no lanes in the other direction
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

    if key_value_pairs:
        matrix = collections.OrderedDict(key_value_pairs)
    
    # Insert ego in matrix, in case ego is not entrying/exiting a highway    
    if matrix and not ghost:
        try:
            matrix[str(ego_vehilce_road_id) + "_" + str(ego_vehilce_lane_id)][3] = 1
        except KeyError:
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
        ego_location (carla.Location): The location object of the ego vehicle.
        ego_vehicle (carla.Vehicle): The ego vehicle for which to detect surrounding cars.
        matrix (collections.OrderedDict): An ordered dictionary representing the city matrix. The keys for existing lanes are the lane IDs in the format "road_id_lane_id". 
            For non-existing lanes different placeholder exist, e.g.  left_outer_lane, left_inner_lane, No_4th_lane, No_opposing_direction.
            The values indicate whether a vehicle is present: 0 - No vehicle, 1 - Ego vehicle, 3 - No road.
            Format example: {
                "left_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "left_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "1_2": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_1": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_-1": [0, 0, 0, 1, 0, 0, 0, 0],
                "1_-2": [0, 0, 0, 0, 0, 0, 0, 0],
                "right_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "right_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
        road_lane_ids (list): A list of all road-lane identifiers of the map, where each identifier is a string in the format "roadId_laneId". 
            Format: ["1_2", "2_1", "3_2"].
        world (carla.World): The game world where the simulation is running.
        radius (int): The radius within which to detect surrounding cars.
        on_highway (bool): True if the ego vehicle is on a highway, False otherwise.
        highway_shape (tuple): Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([wp,..], [wp,..]), exit_wps: ([wp,..], [wp,..]))
        ghost (bool): Ghost mode when ego is exiting/entrying a highway - fix a location of an imaginary vehicle on highway to correctly build matrix from this ghost perspective.

    Returns:
        collections.OrderedDict: The updated city matrix with detected surrounding cars.
            Format example: {
                "left_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "left_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "1_2": [0, 0, 2, 0, 0, 0, 0, 0],
                "1_1": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_-1": [0, 2, 0, 1, 0, 2, 0, 0],
                "1_-2": [0, 0, 0, 0, 0, 0, 0, 0],
                "right_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "right_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],

    """
    world_map = world.get_map()
    ego_vehicle_waypoint = world_map.get_waypoint(ego_location)
    ego_vehicle_road_id = ego_vehicle_waypoint.road_id
    
    # Get all surronding cars in specified radius
    surrounding_cars = []
    for actor in world.get_actors():
        if "vehicle" in actor.type_id and (actor.id != ego_vehicle.id or ghost):
            distance_to_actor = distance(actor.get_location(), ego_location)
            if distance_to_actor <= radius:
                surrounding_cars.append(actor)

    # check if road id changes before/behind ego vehicle, if yes get list of lane_id's of next/previous road
    _, next_lanes = check_road_change(ego_location, road_lane_ids, True, world_map)
    _, prev_lanes = check_road_change(ego_location, road_lane_ids, False, world_map)
    if next_lanes:
        lanes_exist_futher = True
    if prev_lanes:
        lanes_existed_before = True

    # in the following, ignore cars that are on highway exit/entry lanes bc. they are captured in update_matrix()
    if not highway_shape is None:
        entry_wps = highway_shape[2] # Tuple with start and end waypoint of the entry: ([start_wp, start_wp..], [end_wp, end_wp..])
        exit_wps = highway_shape[3] # Tuple with start and end waypoint of the exit: ([start_wp, start_wp..], [end_wp, end_wp..])

        # get all road id's of entry and exit and previous/next road
        entry_road_id = []
        exit_road_id = []
        entry_city_road = [] # road before an entry in city
        exit_city_road = [] # road after an exit in city
        entry_highway_road = [] # road after an entry on highway
        exit_highway_road = [] # road before an exit on highway
        if entry_wps:
            for entry_wp in entry_wps[0]: # entry_wps[0] contains all start waypoints of entry
                entry_city_road.append(entry_wp.previous(3)[0].road_id)
                entry_road_id.append(entry_wp.road_id)
            for entry_wp in entry_wps[1]: # entry_wps[1] contains all end waypoints of entry
                entry_highway_road.append(entry_wp.next(3)[0].road_id)
            # TODO: Check if all cars on highway entry are captured: expecially on road after entry on highway
            if entry_wp.next(3)[0] and entry_wp.next(3)[0].get_left_lane() and entry_wp.next(3)[0].road_id == entry_wp.next(3)[0].get_left_lane().road_id:
                entry_highway_road = []
        if exit_wps:
            for exit_wp in exit_wps[1]: # exit_wps[1] contains all end waypoints of exit
                exit_city_road.append(exit_wp.next(3)[0].road_id)
                exit_road_id.append(exit_wp.road_id)
            for exit_wp in exit_wps[0]: # exit_wps[0] contains all start waypoints of exit
                exit_highway_road.append(exit_wp.previous(3)[0].road_id)
            # TODO: Check if all cars on highway exit are captured: expecially on road before exit on highway
            if exit_wp.next(3)[0] and exit_wp.next(3)[0].get_left_lane() and exit_wp.next(3)[0].road_id == exit_wp.next(3)[0].get_left_lane().road_id:
                exit_highway_road = []
       
    # check if ego is on road id of entry/exit or on road id right before/after an exit/entry road
    surrounding_cars_on_highway_entryExit = []
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
    
    # Update matrix based on the lane and position/distance to ego vehicle of other car
    for car in surrounding_cars:
        # Get road and lane_id of other car
        other_car_waypoint = world_map.get_waypoint(car.get_location())
        other_car_lane_id = other_car_waypoint.lane_id
        other_car_road_id = other_car_waypoint.road_id
        other_car_road_lane_id = str(other_car_road_id) + "_" + str(other_car_lane_id)

        # ignore car on highway entry / Exit bc. considered in update_matrix()
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
        
        # get column in matrix of other car 
        col = calculate_position_in_matrix(
            ego_location,
            ego_vehicle,
            car,
            matrix,
            world_map,
            ghost,
        )
        
        if col is None:
            continue
        
        # insert car in matrix
        if matrix:
            # if road id & lane id of other exist already in matrix (normal case, w/o other car on different road_id in front/behind)
            if other_car_road_lane_id in matrix.keys():
                if car.id == ego_vehicle.id:
                    matrix[other_car_road_lane_id][col] = 1
                else:
                    matrix[other_car_road_lane_id][col] = 2
                continue
            
            # elif road id changes in front / behind then place other car based on lane id 
            elif (lanes_exist_futher or lanes_existed_before) and str(
                other_car_lane_id
            ) in [str(road_lane.split("_")[1]) for road_lane in matrix.keys()]:
                if car.id == ego_vehicle.id:
                    matrix[str(ego_vehicle_road_id) + "_" + str(other_car_lane_id)][
                        col
                    ] = 1
                else:
                    try:
                        matrix[str(ego_vehicle_road_id) + "_" + str(other_car_lane_id)][
                            col
                        ] = 2
                    except:
                        pass

    return matrix, surrounding_cars_on_highway_entryExit


def get_forward_vector_distance(ego_vehicle_location, other_car, world_map):
    """
    Calculate the distance between point B (other vehicle) and point C (parallel right/left of ego) in a right-angled triangle.

    Parameters:
        ego_vehicle_location (carla.Location): The location of the ego vehicle in 3D space.
        other_car_location (carla.Location): The location of the other car in 3D space.
        world_map (carla.WorldMap): The map representing the environment.


    Returns:
        float: The distance between perpendicular_wp (waypoint left/right in parallel to ego) and other car in triangle calculation.
    """

    # Get the location of the other car
    other_car_location = other_car.get_location()

    # Calculate straight line distance between ego and other car
    distance_ego_other = ego_vehicle_location.distance(other_car_location)

    # Get waypoints
    ego_waypoint = world_map.get_waypoint(ego_vehicle_location)
    other_waypoint = world_map.get_waypoint(other_car_location)

    other_lane_id = other_waypoint.lane_id

    
    # get perpendicular waypoint (either ego_waypoint or waypoint on left/right lane of ego_waypoint), which is on same lane as other car
    left_lane_wp, right_lane_wp = ego_waypoint, ego_waypoint
    old_left_lane_wps, old_right_lane_wps = [], []
    while True:
        # termination condition: perpendicular waypoint not found, return straight distance
        if (not left_lane_wp or left_lane_wp.id in old_left_lane_wps) and (
            not right_lane_wp or right_lane_wp.id in old_right_lane_wps
        ):
            return distance_ego_other
        
        # if ego is on same lane as other then we can use straight distance calculation
        if ego_waypoint.lane_id == other_lane_id:
            return distance_ego_other
        
        # check if one waypoint to the left is on same lane as other car
        if left_lane_wp:
            old_left_lane_wps.append(left_lane_wp.id)
            left_lane_wp = left_lane_wp.get_left_lane()
            if left_lane_wp:
                if left_lane_wp.lane_id == other_lane_id:
                    perpendicular_wp = left_lane_wp
                    break
        
        # check if one waypoint to the right is on same lane as other car        
        if right_lane_wp:
            old_right_lane_wps.append(right_lane_wp.id)
            right_lane_wp = right_lane_wp.get_right_lane()
            if right_lane_wp:
                if right_lane_wp.lane_id == other_lane_id:
                    perpendicular_wp = right_lane_wp
                    break


    # calculate distance between ego and perpendicular waypoint (i.e. distance we go left/right in parallel to street)
    distance_opposite = ego_vehicle_location.distance(
        perpendicular_wp.transform.location
    )

    # return distance between perpendicular_wp and other car in right-angled triangle
    return math.sqrt(abs(distance_ego_other**2 - distance_opposite**2))


def check_car_in_front_or_behind(ego_location, other_location, rotation):
    """
    Chek if other car is in front or behind ego vehicle.

    Args:
        ego_location (carla.Location): The location object of the ego vehicle.
        other_location (carla.Location): The location object of the other car.
        rotation (carla.Rotation): The rotation object of the ego vehicle.

    Returns:
        float: The dot_product between forward vectors (similarity between the vectors): dot_product > 0 ==> in front, dot_product < 0 ==> behind
    """
    # Get ego to other vector location
    ego_to_other_vector = other_location - ego_location

    # Calculate forward vector of ego
    ego_forward_vector = carla.Vector3D(
        math.cos(math.radians(rotation.yaw)),
        math.sin(math.radians(rotation.yaw)),
        0,
    )

    # Calculate dot_product (similarity between the vectors): dot_product > 0 ==> in front, dot_product < 0 ==> behind
    dot_product = (
        ego_forward_vector.x * ego_to_other_vector.x
        + ego_forward_vector.y * ego_to_other_vector.y
    )
    return dot_product


def calculate_position_in_matrix(
    ego_location, ego_vehicle, other_car, matrix, world_map, ghost=False
):
    """
    Calculate the position of the other car in the city matrix based on its relative location and distance from the ego vehicle.
    Only determines the column, not the row since that is based on the lane_id of the other car.

    Parameters:
        ego_location (carla.Location): The location object of the ego vehicle.
        ego_vehicle (carla.Vehicle): The ego vehicle for reference.
        other_car (carla.Vehicle): The other car whose position is to be determined.
        matrix (collections.OrderedDict): An ordered dictionary representing the city matrix. The keys for existing lanes are the lane IDs in the format "road_id_lane_id". 
            For non-existing lanes different placeholder exist, e.g.  left_outer_lane, left_inner_lane, No_4th_lane, No_opposing_direction.
            The values indicate whether a vehicle is present: 0 - No vehicle, 1 - Ego vehicle, 3 - No road.
            Format example: {
                "left_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "left_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "1_2": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_1": [0, 0, 0, 0, 0, 0, 0, 0],
                "1_-1": [0, 0, 0, 1, 0, 0, 0, 0],
                "1_-2": [0, 0, 0, 0, 0, 0, 0, 0],
                "right_inner_lane": [3, 3, 3, 3, 3, 3, 3, 3],
                "right_outer_lane": [3, 3, 3, 3, 3, 3, 3, 3],
        world_map (carla.WorldMap): The map representing the environment.
        ghost (bool): Ghost mode when ego is exiting/entrying a highway - fix a location of an imaginary vehicle on highway to correctly build matrix from this ghost perspective.

    Returns:
        int or None: The column index in the city matrix representing the column in the city matrix of the other car,
                    or None if the other car is not within the specified distance range.

    Note:
        The city matrix should be pre-generated using the 'create_basic_matrix' function. Other cars are detected using the detect_surronding cars func.
    """

    # Get ego vehicle rotation and location
    if ghost:
        rotation = other_car.get_transform().rotation # for simplicity use other actors rotation when in ghost mode
    else:
        rotation = ego_vehicle.get_transform().rotation

    # Get other car vehicle location
    other_location = other_car.get_transform().location

    # Calculate new distance: ego to other car
    new_distance = get_forward_vector_distance(ego_location, other_car, world_map)

    # check if car is behind or in front of ego vehicle: dot_product > 0 ==> in front, dot_product < 0 ==> behind
    dot_product = check_car_in_front_or_behind(ego_location, other_location, rotation)

    # Get road_lane_id of other vehicle
    other_car_waypoint = world_map.get_waypoint(other_location)
    other_car_road_lane_id = str(other_car_waypoint.road_id) + "_" + str(other_car_waypoint.lane_id)

    road_lane_ids = get_all_road_lane_ids(world_map)
    
    # if ego is on highway use different speed factor --> we look further ahead/behind on highway
    if check_ego_on_highway(ego_location, road_lane_ids, world_map):
        speed_factor = 2.0
    else:
        speed_factor = 1

    col = None
    # Other car is in parallel of ego_vehicle
    if (
        abs(dot_product) < 4
        and other_car_road_lane_id in list(matrix.keys())
        and matrix[other_car_road_lane_id][3] != 1
    ):
        col = 3
    # Other car is in front of ego_vehicle
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

def initialize_error_dataframe():
    col_names = []

    col_names.append("location")
    col_names.append("error")
    col_names.append("fname")
    col_names.append("line")
    col_names.append("exc_type")

    return pd.DataFrame(columns=col_names)


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
    Check if a junction is ahead of the ego vehicle within a specified distance, starting from the ego vehicle's current waypoint and incrementing by 1m till specified distance.

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
    if ego_waypoint.is_junction:
        return True
    return False


def is_junction_behind(ego_waypoint, distance):
    """
    Check if a junction is behind the ego vehicle within a specified distance, starting from the ego vehicle's current waypoint and incrementing by 1m till specified distance.

    Parameters:
        ego_waypoint (carla.Waypoint): The current waypoint of the ego vehicle.
        distance (int): The maximum distance (in meters) to search for a junction behind.

    Returns:
        bool: True if a junction is found behind within the specified distance, False otherwise.
    """
    # return True if junction is ahead of ego in <= distance meter, start checking at 1m ahead and increment by 1 every loop
    for x in list(range(1, distance + 1)):
        if ego_waypoint.previous(x)[0].is_junction:
            return True
    if ego_waypoint.is_junction:
        return True
    return False


def get_junction_ahead(ego_waypoint, distance):
    """
    Get the junction ahead of the ego vehicle within a specified distance, starting from the ego vehicle's current waypoint and incrementing by 1m till specified distance.

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
    if ego_waypoint.is_junction:
        return ego_waypoint.get_junction()
    return None


def get_junction_behind(ego_waypoint, distance):
    """
    Get the junction behind the ego vehicle within a specified distance, starting from the ego vehicle's current waypoint and incrementing by 1m till specified distance.

    Parameters:
        ego_waypoint (carla.Waypoint): The current waypoint of the ego vehicle.
        distance (int): The maximum distance (in meters) to search for a junction behind.

    Returns:
        carla.Junction or None: The carla.Junction object representing the junction behind if found within
                                 the specified distance. Returns None if no junction is found.
    """
    for x in list(range(1, distance + 1)):
        if ego_waypoint.previous(x)[0].is_junction:
            return ego_waypoint.previous(x)[0].get_junction()
    if ego_waypoint.is_junction:
        return ego_waypoint.get_junction()
    return None


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
    key_value_pairs, junction_roads, lanes_all, ego_wp, distance_to_junc
):
    """
    Update the city matrix to detect the position of the ego vehicle relative to a junction.

    Parameters:
        key_value_pairs (list): A list of key-value pairs representing the city matrix.
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
        junction_roads (list): For all roads going into the junction object one sublist containing the following information: 
            Format: [road_id of road after junction, direction from ego perspective, end_wp of junction road, lane_id of road after junction end wp].
        lanes_all (dict): A dictionary containing lane information for different directions.
            Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.        
        ego_wp (carla.Waypoint): The waypoint of the ego vehicle.
        distance_to_junc (int): The distance in meters, when the junction ahead is recognized, from the ego vehicle to the junction.

    Returns:
        key_value_pairs (list): A list of key-value pairs representing the city matrix, including junction shape + ego vehicle
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].    
    """
    lane_id_ego = ego_wp.lane_id

    # cast lane id's to int in lanes_all
    for road in junction_roads:
        lanes_all[road[1]] = [int(id) for id in lanes_all[road[1]]] # road[1] = direction

    # check on which lane ego is: i=0 means most out
    if lane_id_ego > 0:
        lanes_all["ego"].sort(reverse=True)
    else:
        lanes_all["ego"].sort()
    for i in range(len(lanes_all["ego"])):
        if lanes_all["ego"][i] == lane_id_ego:
            break  # i=0: lane most out
    
    # check how many lanes the roads right and left of ego have: columns = number of cells until inner junction 
    if len(lanes_all["left"]) == 0: # T-junction only right and straight
        columns = int((8 - len(lanes_all["right"])) / 2)
    elif len(lanes_all["right"]) == 0: # T-junction only left and straight
        columns = int((8 - len(lanes_all["left"])) / 2)
    else: # X-junction: consider road right or left with less lanes
        columns = int(
            (8 - min([len(lanes_all["left"]), len(lanes_all["right"])])) / 2
        )  # min number of lanes of right/left
    if columns == 0:
        columns = 2

    # determine column of ego in matrix = c
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
    if ego_wp.road_id == 23:
        return key_value_pairs

    # set ego cell to 1
    for r in range(8):
        if key_value_pairs[-1 - r][1][c] != 3:
            key_value_pairs[-1 - r - i][1][c] = 1
            break

    return key_value_pairs

def add_actor_as_surrounding_car(surrounding_cars, junction_roads, actor, actor_location, actor_waypoint, junction_id, distance_to_junc):
    """
    check on which road actor is and update respective sublist in surrounding_cars: append tuple (actor, different_road_distance).
    different_road_distance: None if actor is already on road_id of road going into junction & otherwise the distance till they reach the road going into the junction.

    Parameters:
        surrounding_cars (dict): Dictionary where the keys are directions from ego perspective and the values are lists of tuples
            Format: {'ego': [(actor, different_road_distance), ..], 'left': [(actor, different_road_distance), ..]..}
        junction_roads (list): For all roads going into the junction object one sublist containing the following information: 
            Format: [road_id of road after junction, direction from ego perspective, end_wp of junction road, lane_id of road after junction end wp].
        actor (carla.Actor): The actor (car) object that needs to be checked.
        actor_location (carla.Location): The location of the actor.
        actor_waypoint (carla.Waypoint): The waypoint object representing the current location of the actor.
        junction_id (int): The unique identifier for the junction.
        distance_to_junc (int): The distance in meters, when the junction ahead is recognized, from the ego vehicle to the junction.

    Returns:
        dict: Updated surrounding_cars dictionary where the keys are directions from ego perspective and the values are lists of tuples
            Format: {'ego': [(actor, different_road_distance), ..], 'left': [(actor, different_road_distance), ..]..}
    """
    # for each road going into junction check if actor is on that road    
    for road in junction_roads:
        # ignore actors that are too far away (add extra 20% because distance_to_actor is calculated with road[2] waypoint, which can be on other lane)
        # or that are already on the junction
        distance_to_actor = distance(actor_location, road[2].transform.location) # road[2] = end_wp of junction road
        if (distance_to_actor > distance_to_junc * 1.2) or (actor_waypoint.is_junction and actor_waypoint.get_junction().id == junction_id): 
             continue
        
        
        different_road_distance = None

        # check if actor is on road going into junction and determine different_road_distance: 
        # None if actor is already on road_id of road going into junction & otherwise the distance till they reach the road going into the junction
        included = True # only include actors that are on one road that goes into the junction
        if str(actor_waypoint.road_id) == road[0]: # actor is already on road_id going into junction
            different_road_distance = None
        elif (
            str(actor_waypoint.next(int(distance_to_junc / 2))[0].road_id)
            == road[0]
        ): # actor is in (distance_to_junc / 2) meters on road_id going into junction
            different_road_distance = int(distance_to_junc / 2)
        elif (
            str(actor_waypoint.previous(int(distance_to_junc / 2))[0].road_id)
            == road[0]
        ) and (not actor_waypoint.is_junction): # actor was before (distance_to_junc / 2) meters on road_id going out of junction
            different_road_distance = (
                int(distance_to_junc / 2) * -1
            )  # negative bc. look back from actor
        elif (
            str(actor_waypoint.next(int(distance_to_junc / 3))[0].road_id)
            == road[0]
        ): # actor is in (distance_to_junc / 3) meters on road_id going into junction
            different_road_distance = int(distance_to_junc / 3)
        elif (
            str(
                actor_waypoint.previous(int(distance_to_junc / 3))[
                    0
                ].road_id
            )
            == road[0]
        ): # actor was before (distance_to_junc / 3) meters on road_id going out of junction
            different_road_distance = (
                int(distance_to_junc / 3) * -1
            )  # negative bc. look back from actor
        else:
            # ignore actors that are not on one road that goes into the junction
            included = False
        
        # add actor to surrounding_cars
        if included:
            surrounding_cars[road[1]].append(
                (actor, different_road_distance)
            )
    
    return surrounding_cars

def get_number_of_cells_until_junction(direction, lanes_all):
    """
    Determine the number of cells until the inner junction of the respective road.
    
    Parameters:
        direction (string): Direction of the road of the actor from perspective of the ego vehicle.
        lanes_all (dict): A dictionary containing lane information for different directions.
            Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.   
    
    Returns:
        int: Number of cells until inner junction of respective road.

    """
    # determine number of cells in matrix outside the junction object of respective road = columns
    if (direction == "ego") or (direction == "straight"):
        if len(lanes_all["left"]) == 0: # T-junction only right and straight
            columns = int((8 - len(lanes_all["right"])) / 2)
        elif len(lanes_all["right"]) == 0: # T-junction only left and straight
            columns = int((8 - len(lanes_all["left"])) / 2)
        else: # X-junction
            columns = int(
                (8 - max([len(lanes_all["left"]), len(lanes_all["right"])])) / 2
            )  # max number of lanes of right/left
    elif (direction == "left") or (direction == "right"):
        if len(lanes_all["ego"]) == 0: # case should not exist 
            columns = int((8 - len(lanes_all["straight"])) / 2)
        elif len(lanes_all["straight"]) == 0: # T-junction only right and left
            columns = int((8 - len(lanes_all["ego"])) / 2)
        else:
            columns = int(
                (8 - max([len(lanes_all["ego"]), len(lanes_all["straight"])]))
                / 2
            )  # max number of lanes of ego/straight
    return columns

def get_cell_of_actor_outside_junction(actor_waypoint, different_road_distance, lanes_all, direction, end_wp, lane_id_outside_juntion, distance_to_junc, columns):
    """
    Determine the cell of an actor outside a junction. Return actor_lane and actor_column:
    - The lane of the actor, actor_lane=0: lane most out of lanes incoming the junction.
    - The column of the actor, actor_column=0: cell farsest away from junction inner part.
    
    Parameters:
        actor_waypoint (carla.Waypoint): Waypoint of the actor to be inserted in the matrix.
        different_road_distance (int): None if actor is already on road_id of road going into junction & otherwise the distance till they reach the road going into the junction.
        lanes_all (dict): A dictionary containing lane information for different directions.
            Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.   
        direction (string): Direction of the road of the actor from perspective of the ego vehicle.
        end_wp (carla.Waypoint): Waypoint of the end of a junction road: just before the respective road outside junction.
        lane_id_outside_juntion (int): Lane id of the lane on the road outside the junction.
        distance_to_junc (int): The distance in meters, when the junction ahead is recognized, from the ego vehicle to the junction.
        columns (int): Number of cells until inner junction of respective road.

    Returns:
        int: The lane of the actor, actor_lane=0: lane most out of lanes incoming the junction.
        int: The column of the actor, actor_column=0: cell farsest away from junction inner part.
    """
    
    # get lane id of actor when we look forward/backward until the road_id of the road going into the junction
    if not (different_road_distance is None): #  actor is not already on road_id of road going into junction
        if different_road_distance > 0:
            actor_waypoint_lane_id = actor_waypoint.next(
                different_road_distance
            )[0].lane_id
        elif different_road_distance < 0:
            actor_waypoint_lane_id = actor_waypoint.previous(
                (different_road_distance * -1)
            )[0].lane_id
    else: # actor is already on road_id of road going into junction
        actor_waypoint_lane_id = actor_waypoint.lane_id

    # determine lane of actor in matrix, actor_lane=0: lane most out of lanes incoming the junction
    for actor_lane in range(len(lanes_all[direction])): # road[1] = direction
        if lanes_all[direction][actor_lane] == actor_waypoint_lane_id:
            break

    # determine cell of actor in matrix
    if ((int(lane_id_outside_juntion) > 0) and (actor_waypoint_lane_id > 0)) or (
        (int(lane_id_outside_juntion) < 0) and (actor_waypoint_lane_id < 0)
    ): # actor on outgoing lane
        # calculate distance between actor and end_wp of junction road
        actor_distance_to_junction = actor_waypoint.transform.location.distance(
            end_wp.transform.location
        )
        # determine the column of the actor, c=0: cell farsest away from junction inner part.
        if actor_distance_to_junction < (
            distance_to_junc / columns
        ):  # doesn't matter how many columns: write "2" in the cell clostest to junction inner part
            c = columns - 1
        elif (columns == 2) or (
            actor_distance_to_junction < (distance_to_junc / columns) * 2
        ):  # if 2 columns then it must be the cell farthest away && if 3 columns check if junction is closer than 2/3 of DISTANCE_TO_JUNCTION, then middle cell
            c = columns - 2
        else:  # if 3 columns and further away, then cell farthest away
            c = columns - 3
    else: # incoming actor
        # determine the column of the actor, c=0: cell farsest away from junction inner part.
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
    
    return actor_lane, c

def update_matrix_with_actor_outside_junction(key_value_pairs, actor_lane, actor_column, columns, cell_val, direction, ego_vehicle, actor):
    """
    Insert actor in matrix, outside junction.

    Parameters:
        key_value_pairs (list): A list of key-value pairs representing the city matrix, including junction shape
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
        actor_lane (int): The lane of the actor, actor_lane=0: lane most out of lanes incoming the junction.
        actor_column (int): The column of the actor, actor_column=0: cell farsest away from junction inner part.
        columns (int): Number of cells until inner junction of respective road.
        cell_val (int): Value to write in matrix cell: 1=ego, 2=other
        direction (string): Direction of the road of the actor from perspective of the ego vehicle.
        ego_vehicle (carla.Vehicle): The ego vehicle.
        actor (carla.Actor): The actor (car) object that needs to be written in matrix.

    Returns:
        list: Updated list of key-value pairs representing the city matrix, including newly added actor
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
    """
    if direction == "ego":
        # ego road: also check if cell is already occupied by ego (for other direction not needed bc. hard cut of matrix to normal road when ego leaves inner junction)
        for j in range(8):
            
            if key_value_pairs[-1 - j][1][actor_column] != 3:
                # in case ego vehicle is already in that matrix cell
                if key_value_pairs[-1 - j - actor_lane][1][actor_column] == 1:
                    # Calculate dot_product (similarity between the vectors): dot_product > 0 ==> in front, dot_product < 0 ==> behind
                    dot_product = check_car_in_front_or_behind(
                        ego_vehicle.get_location(),
                        actor.get_location(),
                        ego_vehicle.get_transform().rotation,
                    )
                    if dot_product > 0:  # other car infront
                        if (columns != 1) and (
                            actor_column == columns - 1
                        ):  # already in front cell
                            key_value_pairs[-1 - j - actor_lane][1][actor_column - 1] = 1
                            key_value_pairs[-1 - j - actor_lane][1][actor_column] = cell_val
                        elif (
                            columns != 1
                        ):  # not in front cell but more than 1 cells available
                            key_value_pairs[-1 - j - actor_lane][1][actor_column + 1] = 2
                    else:  # other car behind
                        if (columns != 1) and (actor_column == 0):  # already in last cell
                            key_value_pairs[-1 - j - actor_lane][1][actor_column + 1] = 1
                            key_value_pairs[-1 - j - actor_lane][1][actor_column] = cell_val
                        elif (
                            columns != 1
                        ):  # not in last cell but more than 1 cells available
                            key_value_pairs[-1 - j - actor_lane][1][actor_column - 1] = cell_val
                else: # if ego not in that cell
                    key_value_pairs[-1 - j - actor_lane][1][actor_column] = cell_val
                break
    elif direction == "left":
        for j in range(8):
            # left road
            if key_value_pairs[actor_column][1][j] != 3:
                key_value_pairs[actor_column][1][j + actor_lane] = cell_val
                break
    elif direction == "straight":
        for j in range(8):
            # straight road
            if key_value_pairs[j][1][-1 - actor_column] != 3:
                key_value_pairs[j + actor_lane][1][-1 - actor_column] = cell_val
                break
    elif direction == "right":
        for j in range(8):
            # right road
            if key_value_pairs[-1 - actor_column][1][-1 - j] != 3:
                key_value_pairs[-1 - actor_column][1][-1 - j - actor_lane] = cell_val
                break
            
    return key_value_pairs

def detect_surrounding_cars_outside_junction(
    key_value_pairs,
    junction_roads,
    lanes_all,
    ego_vehicle,
    world,
    world_map,
    distance_to_junc,
    junction,):
    """
    Detects and records surrounding cars outside the junction object in the city matrix.

    Parameters:
        key_value_pairs (list): A list of key-value pairs representing the city matrix, including junction shape + ego vehicle
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
        junction_roads (list): For all roads going into the junction object one sublist containing the following information: 
            Format: [road_id of road after junction, direction from ego perspective, end_wp of junction road, lane_id of road after junction end wp].
        lanes_all (dict): A dictionary containing lane information for different directions.
            Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.   
        ego_vehicle (carla.Vehicle): The ego vehicle.
        world (carla.World): The world object including the map and actors.
        world_map (carla.Map): The map of the world in which the waypoints are located.
        distance_to_junc (int): The distance in meters, when the junction ahead is recognized, from the ego vehicle to the junction.
        junction (carla.Junction): The junction object.

    Returns:
        list: Updated list of key-value pairs representing the city matrix, including: junction shape + ego vehicle & other cars outside junction
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
    """
    ego_waypoint = world_map.get_waypoint(ego_vehicle.get_location())
    junction_id = junction.id

    surrounding_cars = {"ego": [], "left": [], "straight": [], "right": []}
    actors = world.get_actors()
    ego_actor = actors.find(ego_vehicle.id)
    actors = [ego_actor] + [actor for actor in actors if actor.id != ego_vehicle.id] # ego vehicle first in list, otherwise we might overwrite other car that is already in matrix
    
    # check if ego already in matrix
    ego_already_in_matrix = any(1 in val for key, val in key_value_pairs)
    
    # detect actors that are outside junction and determine how far away they are
    for actor in actors:
        if "vehicle" in actor.type_id:
            # if ego already in matrix, skip ego actor
            if actor.id == ego_vehicle.id and ego_already_in_matrix:
                continue
            
            actor_location = actor.get_location()
            actor_waypoint = world_map.get_waypoint(actor_location)
            
            # catch special case traffic light junction: ignore cars that are outside junction object and on lanes in other direction than ego
            if "Town04" in world_map.name and junction_id == 1368 and np.sign(ego_waypoint.lane_id) != np.sign(actor_waypoint.lane_id) and actor_waypoint.road_id in [40, 41]:
                continue

            # check on which road actor is and update respective sublist in surrounding_cars: append tuple (actor, different_road_distance)
            # different_road_distance: None if actor is already on road_id of road going into junction & otherwise the distance till they reach the road going into the junction
            surrounding_cars = add_actor_as_surrounding_car(surrounding_cars, junction_roads, actor, actor_location, actor_waypoint, junction_id, distance_to_junc)

    # update matrix with surrounding_cars
    for road in junction_roads:
        # sort list of lane ID's
        # catch special case traffic light junction: sort lanes in reverse order 
        if (int(road[3]) < 0 and not ("Town04" in world_map.name and junction.id == 1368)) or (int(road[3]) > 0 and "Town04" in world_map.name and junction.id == 1368):
            lanes_all[road[1]].sort(reverse=True)
        else:
            lanes_all[road[1]].sort()
        
        columns = get_number_of_cells_until_junction(road[1], lanes_all) # road[1] = direction 
        
        for actor, different_road_distance in surrounding_cars[road[1]]: # road[1] = direction
            actor_location = actor.get_location()
            actor_waypoint = world_map.get_waypoint(actor_location)

            actor_lane, actor_column = get_cell_of_actor_outside_junction(actor_waypoint, different_road_distance, lanes_all, road[1], road[2], road[3], distance_to_junc, columns)

            # write in identified cell: 1=ego, 2=other car
            if actor.id == ego_vehicle.id:
                cell_val = 1
            else:
                cell_val = 2
            
            # update matrix: write actor in corresponding cell
            key_value_pairs = update_matrix_with_actor_outside_junction(key_value_pairs, actor_lane, actor_column, columns, cell_val, road[1], ego_vehicle, actor)

    # catch special case traffic light junction: switch 2 lanes of ego and straight direction
    if junction.id == 1368:
        key_value_pairs[2][1][-3:], key_value_pairs[5][1][-3:] = key_value_pairs[5][1][-3:], key_value_pairs[2][1][-3:]
        key_value_pairs[3][1][-3:], key_value_pairs[4][1][-3:] = key_value_pairs[4][1][-3:], key_value_pairs[3][1][-3:]

    return key_value_pairs


def get_junction_shape(ego_vehicle, intial_wp, wps, road_lane_ids, direction_angle, world_map):
    """
    Determines the shape of the junction and builds the respective matrix and returns relevant information. 
    Also builds traffic light junction on highway on Town04.

    Parameters:
        ego_vehicle (carla.Vehicle): The ego vehicle.
        intial_wp (carla.Waypoint): Waypoint of the ego vehicle (or of the ghost in traffic light highway entry/exit scenario) infront of junction.
        junction (carla.Junction): The junction object.
        road_lane_ids (list): A list of all road-lane identifiers of the map, where each identifier is a string
            in the format "roadId_laneId". For example, ["1_2", "2_1", "3_2"].
        direction_angle (float): Angle to determine the direction of other roads going in to the junction from ego perspective.
        world_map (carla.Map): The map of the world in which the waypoints are located.

    Returns:
        key_value_pairs (list): A list of key-value pairs representing the city matrix.
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
        lanes_all (dict): A dictionary containing lane information for different directions.
            Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.
        junction_roads (list): For all roads going into the junction object one sublist containing the following information: 
            Format: [road_id of road after junction, direction from ego perspective, end_wp of junction road, lane_id of road after junction end wp].
        yaw (float): The yaw angle of the ego vehicle's rotation.
    """
    # get information about the junction and the roads going into it
    lanes_all, junction_roads = get_all_lanes(
        ego_vehicle, intial_wp, wps, road_lane_ids, direction_angle, world_map
    )

    # initialize matrix
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
    
    # get number of non-driving-lanes in matrix (w value 3) for each direction from ego perspective: But take only half bc. we do it for all 4 directions
    ego_lanes = int((8 - len(lanes_all["ego"])) / 2)
    ego_complete = False
    left_lanes = int((8 - len(lanes_all["left"])) / 2)
    left_complete = False
    straight_lanes = int((8 - len(lanes_all["straight"])) / 2)
    straight_complete = False
    right_lanes = int((8 - len(lanes_all["right"])) / 2)
    right_complete = False
    
    # build junction iteratively
    i = 0
    while any(
        [not ego_complete, not left_complete, not straight_complete, not right_complete]
    ):
        # 1. ego (left side of matrix)
        if not left_complete:
            for j in range(ego_lanes):
                key_value_pairs[j][1][i] = 3

        # 2. left (top of matrix)
        if not straight_complete:
            for j in range(left_lanes):
                key_value_pairs[i][1][-1 - j] = 3

        # 3. straight (right side of matrix)
        if not right_complete:
            for j in range(straight_lanes):
                key_value_pairs[-1 - j][1][-1 - i] = 3

        # 4. right (bottom of matrix)
        if not ego_complete:
            for j in range(right_lanes):
                key_value_pairs[-1 - i][1][j] = 3

        # check if sides of matrix are already complete
        i = i + 1
        if i == 8: # end: matrix is 8x8
            break        
        if i == left_lanes:
            left_complete = True 
        if i == straight_lanes:
            straight_complete = True
        if i == right_lanes:
            right_complete = True
        if i == ego_lanes:
            ego_complete = True
            
    # get yaw value of ego: needed for correct detection of cars inside matrix
    # catch special case entrying traffic light junction on highway: use yaw of ghost not ego
    if "Town04" in world_map.name and world_map.get_waypoint(ego_vehicle.get_location()).road_id == 23: 
        if world_map.get_waypoint(ego_vehicle.get_location()).lane_id == -3: # yaw, in case we entry to the right
            yaw = 179
        else: # yaw, in case we entry to the left
            yaw = 1
    # normal junction
    else:
        ego_transform = ego_vehicle.get_transform()
        ego_rotation = ego_transform.rotation
        yaw = ego_rotation.yaw

    return key_value_pairs, lanes_all, junction_roads, yaw


# Inner junction:
def get_closest_starting_waypoint(junction_waypoints, ego_location):
    """
    From a list of junction waypoint tuples (start_wp, end_wp), find the closest starting waypoint to the ego vehicle.

    Parameters:
        junction_waypoints (list): List of tuples containing waypoint and lane ID pairs.
                                   Format: [(start_wp, end_wp), ...]
        ego_vehicle (carla.Location): The ego location.

    Returns:
        carla.Waypoint: The closest starting waypoint to the ego vehicle.
    """
    closest_start_wp = junction_waypoints[0][0]
    # get closest start waypoint to ego
    for start_wp, _ in junction_waypoints:
        if distance(start_wp.transform.location, ego_location) < distance(
            closest_start_wp.transform.location, ego_location
        ):
            closest_start_wp = start_wp

    return closest_start_wp


def get_all_lanes(ego_vehicle, ego_wp, junction_waypoints, road_lane_ids, direction_angle, world_map):
    """
    Get all lanes related to the junction and the corresponding directions from the ego vehicle's perspective.

    Parameters:
        ego_vehicle (Vehicle): The ego vehicle.
        ego_wp (Waypoint): The waypoint associated with the ego vehicle.
        junction (Junction): The junction of interest.
        road_lane_ids (list): List of road and lane IDs in the world map.
        direction_angle (float): The angle used to determine directions from the ego vehicle.
        world_map (carla.Map): The map of the world in which the waypoints are located.

    Returns:
        dict: A dictionary containing a list for each direction from ego perspective (key) of lane ids of the roads going into the junction object. 
                Format: {"ego": [lane_ids], "left": [lane_ids], "straight": [lane_ids], "right": [lane_ids]}.
        list: For all roads going into the junction object one sublist containing the following information: 
                Format: [road_id of road after junction, direction from ego perspective, end_wp of junction road, lane_id of road after junction end wp].
    """
    
    ego_location = ego_wp.transform.location
    road_id_ego = str(ego_wp.road_id)
    closest_start_wp = get_closest_starting_waypoint(junction_waypoints, ego_location)

    # pre processing of junction_waypoints to get necessary information
    end_wps = [[], [], [], []]
    for i, (_, end_wp) in enumerate(junction_waypoints):
        # get road id and lane id of the road after the end_wp
        # catch special case of highway traffic light junction: adjust end_wp
        if "Town04" in world_map.name and junction_waypoints[0][0].get_junction().id == 1368 and ((i < 2) or (i>=len(junction_waypoints)-2)):  
            road_id_end_wp = str(end_wp.previous(2)[0].road_id)
            lane_id_end_wp = str(end_wp.previous(2)[0].lane_id)
        else:
            # end
            road_id_end_wp = str(end_wp.next(1)[0].road_id)
            lane_id_end_wp = str(end_wp.next(1)[0].lane_id)

        end_wps[0].append(end_wp) # end_wp of junction road 
        end_wps[1].append(road_id_end_wp) # road id of road after junction end wp

        # get direction from ego perspective
        # catch special case of gas station junction objects
        if (road_id_end_wp != road_id_ego and str(end_wp.next(10)[0].road_id) != road_id_ego) \
            and (not ((int(road_id_end_wp) in [2, 3] and int(road_id_ego) in [467, 468, 477]) or int(road_id_ego)  in [12, 13, 879, 880, 886]) \
                or not ((int(road_id_end_wp) in [12, 13] and int(road_id_ego) in [12, 13, 879, 880, 886]) or int(road_id_ego)  in [467, 468, 477])):
            end_wps[2].append(
                get_waypoint_direction(
                    ego_vehicle, closest_start_wp, end_wp, direction_angle #  TODO: comments
                )
            )

        else:
            end_wps[2].append("ego")

        # get lane id of road after junction end wp
        end_wps[3].append(lane_id_end_wp)
    

    # get distinct road id (of road after junction) + corresponding direction from ego + end_wp
    junction_roads = []
    for i in range(len(end_wps[0])):
        if end_wps[1][i] not in [x[0] for x in junction_roads]:
            junction_roads.append(
                [end_wps[1][i], end_wps[2][i], end_wps[0][i], end_wps[3][i]]
            )

    # get for all directions the lane ids of the roads that go into the junction
    lanes_all = {
        "ego": [],
        "left": [],
        "straight": [],
        "right": [],
    }  # direction from ego perpective
    for lane_id in road_lane_ids:  # iterate through all lanes of map
        for road in junction_roads:  # for each road that goes into the junction
            if road[0] == lane_id.split("_")[0]:
                # catch special case of highway traffic light junction
                if "Town04" in world_map.name and (junction_waypoints[0][0].get_junction().id == 1368) and int(lane_id.split("_")[1])*np.sign(ego_wp.lane_id) < 0 and int(road[0]) != 23:
                    continue
                else:
                    lanes_all[road[1]].append(lane_id.split("_")[1]) # append lane id
    
    return lanes_all, junction_roads


def remove_elements(lst):
    """
    Filters out elements from the list that have another element in the list within a distance of 1.5.

    Args:
    lst (list): A list of numeric values (ints or floats) to be filtered.

    Returns:
    list: A list containing the filtered elements of 'lst'.
    """
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
    In a list of all x- or y-values of a junction, find the points that are end/start points of a target bounding box.

    Parameters:
        values (list): A list of numeric values, either all x-values or y-values of waypoints on a junction.

    Returns:
        list: A list of tuples, where each tuple contains the center point and extent of one bounding box.
    """
    # remove duplicates, sort values and remove elements that are too close (<=1.5m) to each other
    values = list(dict.fromkeys(values)) 
    values.sort()
    values = remove_elements(values)
    
    # get border points of targeted bounding boxes along x axis if "values" contains x coordinates, otherwise along y axis
    value_borders = []
    if len(values) in [4, 6]:
    # Junction cases (directions from ego perspective):
    # -> 6 x values: junction with ego road & straight road (any number of lanes but not 0) and left or/and right road (with at least one of right/left road has 2 lanes in each direction)
    # -> 6 y values: junction with ego road & optional straight road (with at least one of ego/straight road has 2 lanes in each direction) and left and right road (both any number of lanes but not 0)
    # -> 4 x values: junction with ego road & straight road (any number of lanes but not 0) and left or/and right road (with max 1 lane in each direction per road)
    # -> 4 y values: junction with ego road & optional straight road (with max 1 lane in each direction) and left and right road (both any number of lanes but not 0)
    # ==> Take first and last value of list (outside bound), as well as the middle of remaining points
        for i in range(len(values) - 1):
            if i == 0: # first value in values
                value_borders.append(values[i])
            elif i == len(values) - 2: # last value in values
                value_borders.append(values[i + 1])
            else:
                border = (values[i] + values[i + 1]) / 2
                value_borders.append(border)
    
    elif len(values) in [3, 5]:
    # Junction cases (directions from ego perspective):
    # -> 5 x values: junction with ego road (any number of lanes but not 0) & no straight road and left or/and right road (with at least one of right/left road has 2 lanes in each direction)
    # -> 5 y values: junction with ego road & optional straight road (with at least one of ego/straight road has 2 lanes in each direction) and left or right (not both) road (any number of lanes but not 0)
    # -> 3 x values: junction with ego road (any number of lanes but not 0) & no straight road and left or/and right road (with max 1 lane in each direction)
    # -> 3 y values: junction with ego road & optional straight road (with max 1 lane in each direction) and left or right (not both) road (any number of lanes but not 0)
    # ==> Take first value of list (one outside bound), as well as the middle of remaining points and the last value +/-6m of list (other outside bound)
        
        # for x values: sort such that first value in values is the x value of ego road
        # for y values: sort such that first value in values is the y value of the one existing road of left or right road
        if abs(values[0] - values[1]) > abs(values[-1] - values[-2]):
            reverse = False
        else:
            values.sort(reverse=True)
            reverse = True
            
        for i in range(len(values) - 1):
            if i == 0: # first value 
                value_borders.append(values[i])
            elif i == len(values) - 2: #  last 2 values
                border = (values[i] + values[i + 1]) / 2
                value_borders.append(border)
                # add/substract 6m to/from last value bc. this coordinate is the most out lane on side of junction where no road goes into junction
                if values[-1] > values[-2]:
                    value_borders.append(values[i + 1] + 6)
                else:
                    value_borders.append(values[i + 1] - 6)
            else:
                border = (values[i] + values[i + 1]) / 2
                value_borders.append(border)
        if reverse:
            value_borders.sort()

    # determine center values and extents of bounding boxes
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
    groups.append([waypoints[0]]) # initial group with intial wp
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


def divide_bounding_box_into_boxes(junction_waypoints):
    """
    Divide the bounding box of the given junction into smaller boxes, with respect to number and width of lanes.

    Parameters:
        junction_waypoints (list): A list of relevant waypoint tuples from the junction object [(start_wp, end_wp), ..]

    Returns:
        list: A list of boxes, where each box is represented as a list containing a carla.BoundingBox object and an integer value initialized to 0.
    """
    
    # get grouped junction waypoints of junction by distance of 6m
    junction_waypoints = list(sum(junction_waypoints, ())) #  concatenate tuples of junction_waypoints to get single list with all start & end waypoints
    grouped_waypoints = group_waypoints(junction_waypoints)
    
    # put distinct location tuples (x,y) of all groups together
    locations = []
    for group in grouped_waypoints:
        # get distinct location tuples (x,y) of respective group of junction waypoints
        temp = []
        for wp in group:
            temp.append((wp.transform.location.x, wp.transform.location.y))
            temp = list(dict.fromkeys(temp)) #  delete duplicates
            
        # append list to locations
        locations.append(temp)
        
    locations = [item for row in locations for item in row] # flatten list of lists

    # get all x values and y values of all junction waypoints without duplicates or too close x or y values
    x_values = []
    y_values = []
    for point in locations: # point = (x, y)
        x_values.append(point[0])
        y_values.append(point[1])

    # get list with center points and extents of bounding boxes for both axis
    center_values_x = find_center_points(x_values)
    center_values_y = find_center_points(y_values)

    center_values_y.sort(key=lambda x: x[0], reverse=True)
    points = list(itertools.product(center_values_x, center_values_y)) # Cartesian product of center values of x and y = detemines number of bounding boxes

    # create new bounding boxes
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

def fill_boxes(boxes, ego_vehicle, world, junction_waypoints, traffic_light_entry_right):
    """
    Fill the boxes with information about surrounding cars.

    Parameters:
        boxes (list): A list of bounding boxes, where each box is represented as a list containing a carla.BoundingBox object and an integer value initialized to 0.
        ego_vehicle (carla.Vehicle): The ego vehicle, a carla.Vehicle object.
        world (carla.World): The carla.World object.
        junction_waypoints (list): A list of relevant waypoint tuples from the junction object [(start_wp, end_wp), ..]
        traffic_light_entry_right (bool): True if the ego vehicle is entering the traffic light junction Town04, turning to the right side.

    Returns:
        list: A list of boxes with updated integer values representing surrounding cars:
            Format: [[carla.BoundingBox, int], [carla.BoundingBox, int], ...]. 0: no car, 1: ego, 2: other car.
        list: A list of tuples, where each tuple contains the row to be changed with the respective the cell value in case of the special case traffic light junction on Town04.
            Format: [(row, cell value), (row, cell value), ...].
    """   
    world_map = world.get_map()
    # get actors in surrounding of ego vehicle up to 100m
    surrounding_cars = []
    for actor in world.get_actors():
        if "vehicle" in actor.type_id:
            distance_to_actor = distance(
                actor.get_location(), ego_vehicle.get_location()
            )
            if distance_to_actor <= 100:
                surrounding_cars.append(actor)
    
    special_junction_rows = []
    for car in surrounding_cars:
        car_location = car.get_location()
        
        # catch special case traffic light junction, bc. when we entry to the left then we cross the first half of the junction, which is not covered by boundingboxes & not by detect_cars_outside_junction
        if "Town04" in world_map.name and junction_waypoints[-1][0].get_junction().id == 1368 and distance(junction_waypoints[-1][0].transform.location, car_location) < 30 and world_map.get_waypoint(car_location).is_junction and not traffic_light_entry_right:
            if car.id == ego_vehicle.id:
                special_junction_rows.append((1, 1)) # (row, cell value)
            elif world.get_map().get_waypoint(car_location).road_id != 1450:
                special_junction_rows.append((1, 2)) # (row, cell value)
            continue
        
        # check if car is inside bounding box
        for box in boxes:
            if car.id == ego_vehicle.id:
                if box[0].contains(car_location, carla.Transform()):
                    box[1] = 1
            else:
                if box[0].contains(car_location, carla.Transform()):
                    if box[1] == 1:
                    # another actor is in the same box as the ego vehicle
                        # TODO: Other car gets lost if in the same box as the ego box
                        # here: check the relative pos. of the other car and safe it into list[(other_car, rel_pos)]
                        # later when matrix is build, add these other cars to the matrix based on the rel_pos
                        continue
                    else:
                        box[1] = 2

    return boxes, special_junction_rows

def build_grid(boxes):
    """
    Build a grid from the list of boxes. All boxes with the same x value are grouped together in a row.

    Parameters:
        boxes (list): A list of boxes, where each box is represented as a list containing a carla.BoundingBox object and an integer value.

    Returns:
        list: A 2D grid representation of the boxes.
            Format: [[carla.BoundingBox, carla.BoundingBox, carla.BoundingBox, ...], [carla.BoundingBox, carla.BoundingBox, carla.BoundingBox, ...], ...]
    """
    x = boxes[0][0].location.x
    grid = []
    row = [boxes[0]]
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
    Rotate the grid based on the yaw angle of ego.

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
    """
    Check ego vehicle direction: if north or south, return True.

    Args:
        ego_vehicle (carla.Vehicle): Ego vehicle object to get ego direction.

    Returns:
        bool: Return True if ego vehicle is driving north or south.
    """
    ego_transform = ego_vehicle.get_transform()
    ego_rotation = ego_transform.rotation
    yaw = ego_rotation.yaw

    # check in which direction of world ego is driving: if yaw is between -45 and 45 (=north) or between -180 and -135 or between 135 and 180 (south)
    if (yaw >= -45 and yaw <= 45) or (
        (yaw >= -180 and yaw < -135) or (yaw >= 135 and yaw <= 180)
    ):
        return True
    else:
        return False


def get_grid_corners(junction_shape):
    """
    Find the corner coordinates of the inner junction in the given grid representation.

    Parameters:
        key_value_pairs (list): A list of key-value pairs representing the city matrix.
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].

    Returns:
        list: A list containing the coordinates of the four corners of the inner junction.
            Format: [[row, column], [row, column], [row, column], [row, column]] in the grid format.

    Notes:
        - The grid representation must be a 2D list with 8 rows and 8 columns.
        - The function handles the possibility of a 90-degree rotation in the grid,
          ensuring correct corner identification regardless of the junction's orientation.
    """
    # get top y coordinate: y1
    y_1 = None
    for i in range(8):
        if junction_shape[i][1][0] != 3:
            y_1 = i
            break

    # get bottom y coordinate: y2
    for j in range(7, -1, -1):
        if junction_shape[j][1][0] != 3:
            y_2 = j
            break

    # if left from ego perspective is no street: check in last row
    if sum(junction_shape[0][1]) == 8 * 3: # all 8 value of value 3
        row = 7
    # else: left from ego perspective is a street: ckeck in first row
    else:
        row = 0        
    # get left x coordinate: x1
    for k in range(8):
        if junction_shape[row][1][k] != 3:
            x_1 = k
            break
    # get right x coordinate: x2
    for l in range(7, -1, -1):
        if junction_shape[row][1][l] != 3:
            x_2 = l
            break

    return [[y_1, x_1], [y_1, x_2], [y_2, x_1], [y_2, x_2]]


def detect_cars_inside_junction(key_value_pairs, ego_vehicle, junction_waypoints, yaw, world):
    """
    Detect and mark cars inside the junction in the grid representation.

    Parameters:
        key_value_pairs (list): A list of key-value pairs representing the city matrix.
            Format: [("1", [0, 0, 0, 0, 0, 0, 0, 0]), ("2", [0, 0, 0, 0, 0, 0, 0, 0]), ...].
        ego_vehicle (carla.Vehicle): The ego vehicle for reference.
        junction_waypoints (list): A list of relevant waypoint tuples from the junction object [(start_wp, end_wp), ..]
        yaw (float): The yaw angle of the ego vehicle's rotation.
        world (carla.World): The carla.World object representing the simulation world.

    Returns:
        dict: A dictionary with updated lane IDs and their corresponding grid representations after marking cars inside the junction.
            Format: {"1": [0, 0, 0, 0, 0, 0, 0, 0], "2": [0, 0, 0, 0, 0, 0, 0, 0], ...}.

    Notes:
        - The function uses bounding boxes to identify cars inside the junction.
        - It calculates the corners of the inner junction in matrix to determine the junction boundaries.
        - The `rotate_grid` function is used to handle the rotation of the junction if needed.
        - The function checks whether rows and columns should be flipped based on the ego vehicle's orientation.
        - The `divide_bounding_box_into_boxes`, `fill_boxes`, and `build_grid` functions are used to construct the grid.
    """
    # catch special case traffic light junction: check if we are on entrying to the right or left
    if "Town04" in world.get_map().name and sum(key_value_pairs[0][1]) == 3 * 8: # all values of first row == 3 --> no special row needed 
        traffic_light_entry_right = True
    else:
        traffic_light_entry_right = False
    
    # divide junction bounding boxes and check if cars are inside
    boxes = divide_bounding_box_into_boxes(junction_waypoints)
    boxes, special_junction_rows = fill_boxes(boxes, ego_vehicle, world, junction_waypoints, traffic_light_entry_right)
    grid = build_grid(boxes)

    # Check ego vehicle direction: if north or south => rotate 90 degrees back
    grid = rotate_grid(grid, yaw)

    # transfer grid of inner junction to matrix values
    grid_corners = get_grid_corners(key_value_pairs)
    i = min([value[0] for value in grid_corners]) # minimum row index of inner junction
    for row in grid:
        j = min([value[1] for value in grid_corners]) # minimum col index of inner junction
        for cell in row:
            key_value_pairs[i][1][j] = cell[1]
            j = j + 1
        i = i + 1
        
    # catch special case traffic light junction: add special rows to matrix bc. when we entry to the left then we cross the first half of the junction, which is not covered by boundingboxes & not by detect_cars_outside_junction
    for special_junction_row in special_junction_rows:
        if key_value_pairs[special_junction_row[0]][1][4] != 1:
            key_value_pairs[special_junction_row[0]][1][4] = special_junction_row[1] # TODO: if we turn left then we currently ignore cars turning right that are already on junction
        else:
            key_value_pairs[special_junction_row[0]+1][1][4] = special_junction_row[1] # TODO: if we turn left then we currently ignore cars turning right that are already on junction
    
    return dict(key_value_pairs)


def is_highway_junction(ego_vehicle, ego_wp, junction, road_lane_ids, direction_angle, world_map):
    lanes_all, junction_roads = get_all_lanes(
        ego_vehicle, ego_wp, junction.get_waypoints(carla.LaneType().Driving), road_lane_ids, direction_angle, world_map
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
def remove_wps(closest_waypoint, wps, world_map):
    """
    Remove waypoints from the given list that are not relevant for the current scenario. In the list of waypoint tuples [(start_wp, end_wp), ..] of highway junctions are often waypoints
    that are on the lane of the opposite direction, which should be ignored.

    Args:
        closest_waypoint (carla.Waypoint): the closest starting waypoint to the ego vehicle from the list of junction waypoint tuples [(start_wp, end_wp), ..].
        wps (list): A list of relevant waypoint tuples from the junction object [(start_wp, end_wp), ..]
        world_map (carla.Map): The map of the world in which the waypoints are located.

    Returns:
        list: A filtered list of relevant waypoints for the current scenario.
    """
    # initialize bool for special case traffc light junction
    is_traffic_light_junction = False
    
    # get waypoint tuples that start on the same lane as the ego vehicle is and where start & end waypoint are not too close to each other
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
    
    #get waypoint tuples of entries/exits
    exit_and_entry_wp = []
    for group in wps:
        enough_distance = (
            distance(group[0].transform.location, group[1].transform.location) > 3
        )
        for wp in wps_on_ego_lane:
            # add waypoint tuple to exit_and_entry_wp, if waypoint tuple starts close enough (<7m) to one of the starting waypoints of tuples starting from ego lane
            # and where start & end waypoint are not too close to each other
            # and waypoint tuple does not start on ego lane  
            if (
                (distance(wp[0].transform.location, group[0].transform.location) < 7
                and enough_distance
                and group not in wps_on_ego_lane)
                or ("Town04" in world_map.name and group[0].road_id in [1450, 1412, 1421, 1432, 1442])
            ):
                # catch special case for traffic light junction on highway:
                if "Town04" in world_map.name and group[0].road_id in [1450, 1412, 1421, 1432, 1442]:
                    is_traffic_light_junction = True
                    exit_and_entry_wp.append((group[0], group[0])) # add tuple (start_wp, start_wp) to exit_and_entry_wp bc. some end_wp are on the opposite lane (irrelevant) but tuple is needed for junction shape
                # else, all normal cases:
                else:    
                    exit_and_entry_wp.append(group)
                break
            # add waypoint tuple to exit_and_entry_wp, if waypoint tuple ends close enough (<7m) to one of the end waypoints of tuples starting from ego lane
            # and where start & end waypoint are not too close to each other
            # and waypoint tuple does not start on ego lane  
            elif (
                (distance(wp[1].transform.location, group[1].transform.location) < 7
                and enough_distance
                and group not in wps_on_ego_lane)
            ):
                exit_and_entry_wp.append(group)

    # catch special case for traffic light junction on highway:
    if is_traffic_light_junction:
        # change start & end waypoint of the first 2 tuples in wps_on_ego_lane to simulate that on the 4 highway lanes there are 2 lanes in each direction, needed for junction shape
        wps_on_ego_lane[0] = (wps_on_ego_lane[0][1], wps_on_ego_lane[0][0])
        wps_on_ego_lane[1] = (wps_on_ego_lane[1][1], wps_on_ego_lane[1][0])
    if is_traffic_light_junction: #and closest_waypoint.previous(2)[0].lane_id > 0:
        exit_and_entry_wp = exit_and_entry_wp[0:2]

    result = wps_on_ego_lane + exit_and_entry_wp
    return list(dict.fromkeys(result))



def clustering_algorithm(points, threshold):
    """
    Threshold-based clustering of a list of waypoints.

    Args:
        points (list): List with a tuple for each junction waypoint with its corresponding tag: start or end. 
            Format: [(start_wp, "start"), (end_wp, "end"), ..])]
        threshold (int): Integer to determine the clustering distance threshold. 

    Returns:
        list: List with a list for each cluster of waypoints.
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]
    """
    R = []  # Resulting list of clusters
    while points:
        # Pick a random point from the remaining points
        current_point = random.choice(points)
        C = [current_point]  # Create a new cluster with the current point
        points.remove(current_point)  # Remove the current point from the list

        # check if remaining points are within the threshold distance of points already in new cluster
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
    """
    Add a string to each tuple in the given list to mark if it is a start or end waypoint.

    Args:
        list: A list of tuples, where each tuple contains a start and end waypoint of one junction road.
            Format: [(start_wp, end_wp), ..]

    Returns:
        list: Updated list of tuples, where each tuple contains a start tuple and end tuple with the waypoint and a tag whether it is a start or end.
            Format: [((start_wp, "start"), (end_wp, "end)), ..]
    """
    modified_list = [
        ((first, "start"), (second, "end")) for first, second in tuples_list
    ]
    return modified_list


def remove_unnecessary_waypoints_in_group(wps_grouped):
    """
    Remove waypoints with a unique road id, in case there are more than 4 waypoints in a group bc. then an entry/exit road is on one lane of the normal highway leading in false highway shapes.

    Args:
        wps_grouped (list): List with a list for each cluster of waypoints.
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]

    Returns:
        list: List with an updated list for each cluster of waypoints.
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]
    """
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


def prepare_waypoints(closest_waypoint, wps, world_map):
    """
    Prepare and cluster the raw junction waypoints for using them in highway scenarios.

    Args:
        closest_waypoint (carla.Waypoint): the closest starting waypoint to the ego vehicle from the list of junction waypoint tuples [(start_wp, end_wp), ..].
        wps (list): A list of raw waypoint tuples from the junction object [(start_wp, end_wp), ..]
        world_map (carla.Map): Carla object of the world.

    Returns:
        list: List with a list for each cluster of waypoints -> to detect shape of highway junction.
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]
    """
    wps = remove_wps(closest_waypoint, wps, world_map)
    wps = mark_start_end(wps)
    wps = list(sum(wps, ())) # concatenate tuples of wps to get single list with all start & end waypoints,  i.e. [(start_wp, "start"), (end_wp, "end"), ..])]
    wps = [wp for wp in wps if wp[0]] # filter out None values
    wps_grouped = clustering_algorithm(wps, 6)
    wps_grouped = remove_unnecessary_waypoints_in_group(wps_grouped)
    return wps_grouped


# highway entry/exit
def get_exit_or_entry(wps, exit_or_entry, junction, world_map):
    """
    Searches for the tuple of start and end waypoint of the entry/exit of the highway junction ahead.

    Args:
        wps (list): List with a list for each cluster of waypoints -> to detect shape of highway junction.
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]
        exit_or_entry (String): String determining if entry or exit waypoints should be returned: "entry" or "exit".
        junction (carla.Junction): Junction object of the highway junction ahead.
        world_map (carla.Map): Carla object of the world.

    Returns:
        tuple: Tuple with lists of start and end waypoints of the entry/exit.
            Format: ([start_wp, start_wp..], [end_wp, end_wp..])
    """
    junction_wps = junction.get_waypoints(carla.LaneType().Driving) # get raw junction waypoint tuples [(start_wp, end_wp), ..]
    first = []
    second = []

    # special case for junction 1368: traffic light junction on highway
    if "Town04" in world_map.name and junction.id == 1368:
        entry_road_ids = [1450, 1412, 1421, 1432, 1442] # road ids of entry roads: 1450 turns right; remaining turn left
        for start_wp, end_wp in junction_wps:
            if start_wp.road_id in entry_road_ids:
                first.append(start_wp)
                second.append(end_wp)
        return (first, second)  # first = start_wp, second = end_wp

    if exit_or_entry == "exit":
        order = ["end", "start"]
    elif exit_or_entry == "entry":
        order = ["start", "end"]

    # with respect to entry/exit, get first and second waypoint of entry/exit
    for group in wps:
        if len(group) != 4: 
            for wp in group:
                if wp[1] == order[0]: # check if wp tag ("start"/"end") is equal to order[0] ("start"/"end")
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
        return (second, first)  # first = [end_wp, end_wp..], second = [start_wp, start_wp..]
    elif exit_or_entry == "entry":
        return (first, second)  # first = [start_wp, start_wp..], second = [end_wp, end_wp..]


def get_highway_shape(wps_grouped, junction, ego_waypoint, world_map):
    """
    Determine the shape of the highway junction ahead. 

    Args:
        wps_grouped (list): List with a list for each cluster of waypoints -> to detect shape of highway junction.
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]
        junction (carla.Junction): Carla object of the junction ahead. 
        ego_waypoint (carla.Waypoint): The waypoint of the ego vehicle.

    Returns:
        tuple: Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([start_wp, start_wp..], [end_wp, end_wp..]), exit_wps: ([end_wp, end_wp..], [start_wp, start_wp..]))
    """
    # Check if entry or exit
    wps_grouped.sort(key=len) # sort waypoint clusters based on number of waypoints per cluster
    
    # special case for junction 1368: traffic light junction on highway => set highway_type & straight_lanes
    if "Town04" in world_map.name and junction.id == 1368:
        if ego_waypoint.road_id == 41:
            highway_type = "traffic_light_junction_entry_left"
        elif ego_waypoint.road_id == 40:
            highway_type = "traffic_light_junction_entry_right"
        else:
            print("Error in get_highway_shape: Junction 1368 (traffic light junction on highway on Map04), ego vehicle not on highway road 40 or 41")
        straight_lanes = 4
    # normal case: determine highway_type and straight_lanes
    else:
        if len(wps_grouped[0]) == 2: # smallest cluster has 2 waypoints
            if wps_grouped[0][0][1] != wps_grouped[0][1][1]:
                highway_type = "entry_and_exit"
            elif wps_grouped[0][0][1] == "start":
                highway_type = "dual_entry"
            elif wps_grouped[0][0][1] == "end":
                highway_type = "dual_exit"

        elif len(wps_grouped[0]) == 1: # smallest cluster has 1 waypoint
            if wps_grouped[0][0][1] == "start":
                highway_type = "single_entry"
            else:
                highway_type = "single_exit"
        else: # smallest cluster has >2 waypoints --> junction object w/o shape ahead, e.g. if entry/exit is on the other lane direction of highway but junction object spans over boths sides
            print("Junction object w/o shape")
            return ("normal_highway", 4, None, None)

        # Check number of lanes going straight on highway
        road_ids = [wp[0].road_id for wp in wps_grouped[1]]
        if len(set(road_ids)) == 1:
            straight_lanes = 4
        else:
            straight_lanes = 3
    
    entry_wps = None
    exit_wps = None
    # get start & end waypoints of entry / exit as tuple
    if highway_type in ["entry_and_exit", "single_entry", "dual_entry", "traffic_light_junction_entry_left", "traffic_light_junction_entry_right"]:
        entry_wps = get_exit_or_entry(wps_grouped, "entry", junction, world_map)
    if highway_type in ["entry_and_exit", "single_exit", "dual_exit"]:
        exit_wps = get_exit_or_entry(wps_grouped, "exit", junction, world_map)

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

def static_entry_exit_matrix(matrix, highway_type, on_entry):
    """
    When ego is entrying or exiting a highway, the matrix is static. The entry/exit is placed in the matrix based on the highway type.

    Args:
        matrix (list): Matrix with a tuple (key, value) for each lane with the key indicating the road id and lane id of the lane "roadID_laneID".
            Example format: [('1_-2', [0,0,0,1,0,0,0,0]), ...]
        highway_type (String): Indicatiting type of highway shape, e.g. single entry/exit, double entry/exit or entry&exit
        on_entry (bool): Indicating if ego is on an entry or exit lane

    Returns:
        matrix (list): Matrix with a tuple (key, value) for each lane with the key indicating the road id and lane id of the lane "roadID_laneID".
            Format example: [
                ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
                ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
                ("1_1", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("1_2", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("1_3", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("1_4", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("right_inner_lane", [3, 3, 3, 0, 3, 3, 3, 3]),
                ("right_outer_lane", [3, 3, 3, 0, 3, 3, 3, 3]),
            ]
        int: Column index in the matrix of entry/exit of interest.
    """
    if highway_type == "entry_and_exit": 
        if on_entry:
            # entrying highway: entry should be in center of matrix
            col_entryExit = 1
            # exit lane 
            matrix[6][1][1] = 0
            matrix[7][1][1] = 0
            # entry lane
            matrix[6][1][3] = 0
            matrix[7][1][3] = 0
        else:
            # exiting highway: exit should be in center of matrix
            col_entryExit = 3
            # exit lane 
            matrix[6][1][3] = 0
            matrix[7][1][3] = 0
            # entry lane
            matrix[6][1][5] = 0
            matrix[7][1][5] = 0
    elif highway_type == "dual_entry": 
        col_entryExit = 3
        # 1st entry lane 
        matrix[6][1][3] = 0
        matrix[7][1][3] = 0
        # 2nd entry lane
        matrix[6][1][4] = 0
        matrix[7][1][4] = 0
    else:
        col_entryExit = 3
        matrix[6][1][3] = 0
        matrix[7][1][3] = 0
    return matrix, col_entryExit

def get_initial_wp_for_entry_or_exit_detection(highway_shape, ego_waypoint, direction):
    """
    Determine the initial waypoint for the search for entry/exit: from this point we look infront/behind to find the entry/exit

    Args:
        highway_shape (tuple): Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([wp,..], [wp,..]), exit_wps: ([wp,..], [wp,..]))      
        ego_waypoint (carla.Waypoint): The waypoint object of ego vehicle.
        direction (string): String indicating the direction of the search for entry/exit: "before" or "behind".
    
    Returns:
        carla.Waypoint: initial waypoint for the search for entry/exit: from this point we look infront/behind to find the entry/exit
    """
    # for dual entry/exit Always take left lane wp bc if ego on right lane it can lead to complications when getting wp.previous() or wp.next() returns wp from a entry/exit road/lane
    if highway_shape[0] in ["dual_exit", "dual_entry"]:
        # for dual exit/entry we compare yaw values based on highway lane (bc. exit/entry lanes are straight w/o curve) -> Make sure we stay on highway with wp.next() & wp.previous() by using initial_wp from most left lane to go back and then move to right lane for check of already in parallel to entry/exit
        initial_wp = ego_waypoint
        while True:
            tmp_left_lane = initial_wp.get_left_lane()
            if (tmp_left_lane is None) or (
                tmp_left_lane.lane_type != carla.LaneType().Driving
            ):
                break
            initial_wp = tmp_left_lane
    # for other entry/exit scenarios we compare yaw on entry/exit lanes (bc. highway lanes are straight and entry/exit is curved)
    else:
        initial_wp = ego_waypoint
    return initial_wp

def get_next_wp_to_check_if_already_entry_or_exit(direction, initial_wp, distance, j, highway_shape):
    """
    Get the next waypoint in distance+j meter infront/back of ego to check if it is parallel to entry/exit. If yes, then we found the entry/exit.

    Args:
        direction (string): String indicating the direction of the search for entry/exit: "before" or "behind".
        initial_wp (carla.Waypoint): Initial waypoint for the search for entry/exit: from this point we look infront/behind to find the entry/exit
        distance (int): Distance threshold for a specific column in matrix, e.g. if a matrix column represents cars between 40-60m infront of ego, then distance is ca. 50 (value depends on entry/exit type).
        j (int): Extra value to add on distance, bc. otherwise sometimes jumps from one column distance to the next are bigger than junction.
        highway_shape (tuple): Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([wp,..], [wp,..]), exit_wps: ([wp,..], [wp,..]))

    Returns:
        carla.Waypoint: Next waypoint infront/behind ego to be checked if it is parallel to entry/exit.
    """
    # Look in front
    if direction == "before": 
        # if (distance == 4) and (j == 5):
        #     j = 0
        wp = initial_wp.next(distance + j)[0]
    # Look behind
    elif direction == "behind":  
        # for dual exit/entry we compare yaw values based on highway lane (bc. exit/entry lanes are straight w/o curve) -> Make sure we stay on highway with wp.next() & wp.previous() in following functions by using initial_wp from most left lane to go back/forward and then move to right lane for check of already in parallel to entry/exit
        if highway_shape[0] == ["dual_entry", "dual_exit"]:
            wp = initial_wp.previous(distance + j)[0]
            while True:
                tmp_right_lane = wp.get_right_lane()
                if (tmp_right_lane is None) or (
                    tmp_right_lane.lane_type != carla.LaneType().Driving
                ):
                    break
                wp = tmp_right_lane
        else:
            wp = initial_wp.previous(distance + j)[0]
    return wp


def check_wp_parallel_to_entry_exit(wp, entry_wps, exit_wps, right_lane_end_wp, highway_shape, degree):
    """
    Check whether or not the given waypoint is parallel to entry/exit. If yes, then we found the entry/exit.

    Args:
        wp (carla.Waypoint): Next waypoint infront/behind ego to be checked if it is parallel to entry/exit.
        entry_wps (tuple): Tuple with lists of start and end waypoints of the entry.
            Format: ([start_wp, start_wp..], [end_wp, end_wp..])
        exit_wps (tuple): Tuple with lists of start and end waypoints of the exit.
            Format: ([start_wp, start_wp..], [end_wp, end_wp..])
        right_lane_end_wp (carla.Waypoint): End waypoint with highest absolute lande_id -> right (most out) lane
        highway_shape (tuple): Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([wp,..], [wp,..]), exit_wps: ([wp,..], [wp,..]))   
        degree (int): Angle threshold depending on curve type to determine how far car is on entry/exit curve
        
    Returns:
        bool: Boolean indicating if the given waypoint is parallel to entry/exit, if True then we found the entry/exit.
    """
    # we have to check if wp is in parallel to entry/exit, depending on exit / entry type
    if highway_shape[0] in [
        "entry_and_exit",
        "single_exit",
    ]:
        # search for exit: compare yaw values of start_wp of exit road with yaw value of wp on exit lane (which is parallel to given wp to be checked)
        distance_junction_start = get_distance_junction_start(wp)
        yaw_difference = abs(exit_wps[0][0].transform.rotation.yaw) - abs(
            exit_wps[0][0]
            .next(distance_junction_start)[0]
            .transform.rotation.yaw
        )
        already_in_curve = abs(yaw_difference) > degree
    elif highway_shape[0] in ["single_entry"]:
        # search for entry: compare yaw values of end_wp of entry road with yaw value of wp on entry lane (which is parallel to given wp to be checked)
        distance_junction_end = get_distance_junction_end(wp)
        yaw_difference = abs(entry_wps[1][0].transform.rotation.yaw) - abs(
            entry_wps[1][0]
            .previous(distance_junction_end)[0]
            .transform.rotation.yaw
        )
        already_in_curve = abs(yaw_difference) < degree
    elif highway_shape[0] in ["dual_entry"]:
        # different logic for dual entry/exit bc. here the highway lanes are curved and the entry/exit lanes are straight
        # search for dual entry: compare yaw values of right_lane_end_wp on highway with yaw value of given wp (which is on same lane)
        yaw_difference = abs(
            right_lane_end_wp.transform.rotation.yaw
        ) - abs(wp.transform.rotation.yaw)
        already_in_curve = abs(yaw_difference) < degree
    elif highway_shape[0] in ["dual_exit"]:
        # different logic for dual entry/exit bc. here the highway lanes are curved and the entry/exit lanes are straight
        # search for dual exit: compare yaw values of right_lane_end_wp on highway with yaw value of given wp (which is on same lane)
        yaw_difference = abs(
            right_lane_end_wp.transform.rotation.yaw
        ) - abs(wp.transform.rotation.yaw)
        already_in_curve = abs(yaw_difference) > degree
    else:
        already_in_curve = False
    return already_in_curve

def search_entry_or_exit(
    world_map,
    ego_vehicle_location,
    direction,
    exit_entry_found,
    matrix,
    highway_shape,
    entry_wps,
    exit_wps,
    degree,
    junction_id,
    right_lane_end_wp,
    ghost=False,
    on_entry=False):
    """This function searches for the entry or exit in front on highway. If ego is exiting/entrying a fixed static matrix is used.
    Otherwise, when ego is on highway, the function looks behind & in front to find entry / exit. The idea is to go a certain distance to the front/back
    and then check if right lane is already curved.

    Args:
        world_map (carla.Map): The map representing the environment.
        ego_vehicle_location (carla.Location): The location object of the ego vehicle.
        direction (string): String indicating the direction of the search for entry/exit: "before" or "behind".
        exit_entry_found (bool): Boolean indicating if the entry/exit was found already.
        matrix (list): Matrix with a tuple (key, value) for each lane with the key indicating the road id and lane id of the lane "roadID_laneID".
            Example format: [('1_-2', [0,0,0,1,0,0,0,0]), ...]
        highway_shape (tuple): Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([wp,..], [wp,..]), exit_wps: ([wp,..], [wp,..]))                
        entry_wps (tuple): Tuple with lists of start and end waypoints of the entry.
            Format: ([start_wp, start_wp..], [end_wp, end_wp..])
        exit_wps (tuple): Tuple with lists of start and end waypoints of the exit.
            Format: ([start_wp, start_wp..], [end_wp, end_wp..])
        degree (int): Angle threshold depending on curve type to determine how far car is on entry/exit curve
        junction_id (int): The identifier of the current junction object.
        right_lane_end_wp (carla.Waypoint): End waypoint with highest absolute lande_id -> right (most out) lane
        ghost (bool): Ghost mode when ego is exiting/entrying a highway - fix a location of an imaginary vehicle on highway to correctly build matrix from this ghost perspective.
        on_entry (bool): Indicating if ego is on an entry or exit lane

    Returns:
        bool: Indicating if the entry/exit was found already.
        int: Column index in the matrix of entry/exit of interest (for dual entry/exit and entry+exit the first column is returned)
        matrix (list): Updated matrix with a tuple (key, value) for each lane with the key indicating the road id and lane id of the lane "roadID_laneID".
            Format example: [
                ("left_outer_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
                ("left_inner_lane", [3, 3, 3, 3, 3, 3, 3, 3]),
                ("1_1", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("1_2", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("1_3", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("1_4", [0, 0, 0, 0, 0, 0, 0, 0]),
                ("right_inner_lane", [3, 3, 3, 0, 3, 3, 3, 3]),
                ("right_outer_lane", [3, 3, 3, 0, 3, 3, 3, 3]),
            ]
    """
    ego_waypoint = world_map.get_waypoint(ego_vehicle_location)
    
    # if entrying/exiting highway, static matrix based on highway type
    if ghost:
        matrix, col_entryExit = static_entry_exit_matrix(matrix, highway_shape[0], on_entry)
        return True, col_entryExit, matrix
    
    # get the initial waypoint for the search: from this point we look infront/behind for entry/exit
    initial_wp = get_initial_wp_for_entry_or_exit_detection(highway_shape, ego_waypoint, direction)

    # define distance thresholds for each column in matrix
    if direction == "before": # Look in front
        z = 3  # column help in matrix
        col_distances = [6, 15, 35, 55, 75]
    elif direction == "behind": # Look behind
        z = (
            -1
        )  # column help in matrix; -1 because in col_distances list, when looking behind, is first value only a threshold to see when junction too far behind
        if highway_shape[0] in [
            "entry_and_exit"
        ]:  # special because entry & exit are very close compared to normal column distance in matrix
            col_distances = [55, 20, 10, 6]
        elif highway_shape[0] in ["dual_exit"]:  # special because for dual exit
            col_distances = [55, 35, 20, 6]
        else:
            col_distances = [55, 35, 15, 6]
    
    # iterate through column distances find entry/exit
    col_entryExit = None
    for i, col_distance in enumerate(
        col_distances
    ):  # index 0 equals distance closest (10)
        if exit_entry_found:
            break
        for j in [
            -5,
            0,
            5,
        ]:  # check 3 values for each column, bc. otherwise sometimes jumps are bigger than junction
            
            if exit_entry_found:
                break
            
            # don't look only 1m behind ego, bc. then sometimes entry/exit is to fast in column behind ego column
            if direction == "behind" and col_distance == 6 and j == -5:
                # j = 0
                continue
            
            # get next waypoint infront/back to be checked if it is parallel to entry/exit 
            wp = get_next_wp_to_check_if_already_entry_or_exit(direction, initial_wp, col_distance, j, highway_shape)
            
            # waypoint can only be parallel to entry/exit if it is on the junction object            
            if (wp.is_junction) and (wp.get_junction().id == junction_id):
                wp_is_parallel_to_entry_exit = check_wp_parallel_to_entry_exit(wp, entry_wps, exit_wps, right_lane_end_wp, highway_shape, degree)
                
                # if ego already too far away, dont show in matrix
                if (
                    (direction == "behind")
                    and wp_is_parallel_to_entry_exit
                    and (col_distance == 55)
                ):
                    exit_entry_found = True
                    break

                # if wp is parallel to entry/exit, update matrix
                if wp_is_parallel_to_entry_exit:
                    if highway_shape[0] in ["traffic_light_junction_entry_left"]:
                        matrix[0][1][z + i] = 0
                        matrix[1][1][z + i] = 0
                    else:
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
                    if (z + i - 1 >= 0):
                        if highway_shape[0] in ["traffic_light_junction_entry_right"]:
                            matrix[6][1][z + i - 1] = 0
                            matrix[7][1][z + i - 1] = 0
                    exit_entry_found = True
                    break
    return exit_entry_found, col_entryExit, matrix


def insert_in_matrix(matrix, car, ego_vehicle, col, row):
    if car.id == ego_vehicle.id:
        matrix[row][1][col] = 1
    else:
        if matrix[row][1][col] == 1:
            # check if car is behind or in front of ego vehicle: dot_product > 0 ==> in front, dot_product < 0 ==> behind
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
    """
    Iterates over grouped highway junction waypoints and returns the waypoint of the right (most out) lane.

    Args:
        wps (list): List of one waypoint cluster of grouped highway junction waypoints.
            Format: [(start_wp, "start"), (start_wp, "start"), ..]

    Returns:
        carla.Waypoint: Waypoint with highest absolute lande_id -> right (most out) lane
    """
    right_lane_wp = wps[0][0]  # initialize = first wp of group
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
    junction,
    cars_on_entryExit,
    direction_angle,
    ghost=False,
    on_entry = False,
):
    """_summary_

    Args:
        world_map (carla.Map): The map representing the environment.
        ego_vehicle (carla.Vehicle): The vehicle object of the ego vehicle.
        ego_vehicle_location (carla.Location): The location object of the ego vehicle.
        highway_shape (tuple): Tuple containing highway_type, number of straight highway lanes, entry waypoint tuple and/ exit waypoint tuple.
            Format: (highway_type: string, straight_lanes: int, entry_wps: ([wp,..], [wp,..]), exit_wps: ([wp,..], [wp,..]))
        wps (list): List with a list for each cluster of waypoints
            Format: [[(start_wp, "start"), (end_wp, "end"), ..], [(start_wp, "start"), (end_wp, "end"), ..], ..]
        matrix (collections.OrderedDict): Matrix with keys for existing lanes are the lane IDs in the format "road_id_lane_id". 
            For non-existing lanes different placeholder exist, e.g.  left_outer_lane, left_inner_lane, No_4th_lane, No_opposing_direction
            The values indicate whether a vehicle is present: 0 - No vehicle, 1 - Ego vehicle, 3 - No road.
        junction (carla.Junction): Junction object of the current junction.
        cars_on_entryExit (list): List encompassing all actors on the entry/exit lanes of the junction.
        direction_angle (float): The angle used to determine directions from the ego vehicle.
        ghost (bool): Ghost mode when ego is exiting/entrying a highway - fix a location of an imaginary vehicle on highway to correctly build matrix from this ghost perspective.
        on_entry (bool): Indicating if ego is on an entry or exit lane

    Returns:
        _type_: _description_
    """
    # If main highway road consists of 3 straight lanes, then we have to add 1 row representing the entry/exit lane
    if highway_shape[1] == 3 and sum (matrix[list(matrix.keys())[5]]) == 24:
        matrix[list(matrix.keys())[5]] = [0, 0, 0, 0, 0, 0, 0, 0]

    matrix = list(matrix.items())
    exit_entry_found = False
    entry_wps = highway_shape[2] # Tuple with lists of start and end waypoint of the entry: ([start_wp, start_wp..], [end_wp, end_wp..])
    exit_wps = highway_shape[3] # Tuple with lists of start and end waypoint of the exit: ([start_wp, start_wp..], [end_wp, end_wp..])
    
    # TODO: put in a config file
    # set the angle threshold depending on curve type: determine how far car is on entry/exit curve
    if highway_shape[0] == "entry_and_exit":
        degree = 30
    elif highway_shape[0] in ["single_entry", "single_exit"]:
        degree = 20
    elif highway_shape[0] in ["dual_entry", "dual_exit"]:
        degree = 18  # TODO: test and fine tuning of degree
    elif highway_shape[0] in [ "traffic_light_junction_entry_left", "traffic_light_junction_entry_right"]: # special case Town04
        degree = 30
    else:
        # just in case
        degree = 20

    # get right_lane_end_wp & right_lane_start_wp for dual entry & exit surrounding cars angle calculation
    if highway_shape[0] in ["dual_entry", "dual_exit"]:
        for wps_group in wps:
            if (len(wps_group) == 4) and (wps_group[0][1] == "end"): # This group usually contains 4 end waypoints on highway
                # get end waypoint on right lane
                right_lane_end_wp = get_right_lane_wp(wps_group)
            elif (len(wps_group) == 4) and (wps_group[0][1] == "start"): # This group usually contains 4 start waypoints on highway
                # get start waypoint on right lane
                right_lane_start_wp = get_right_lane_wp(wps_group)
    else:
        right_lane_end_wp = None
        right_lane_start_wp = None


    # First: Look behind ego and search for entry/exit
    exit_entry_found, col_entryExit, matrix = search_entry_or_exit(
        world_map,
        ego_location,
        "behind",
        exit_entry_found,
        matrix,
        highway_shape,
        entry_wps,
        exit_wps,
        degree,
        junction.id,
        right_lane_end_wp,
        ghost,
        on_entry,
    )
    # if not yet found, look in front and search for entry/exit
    if exit_entry_found == False:
        _, col_entryExit, matrix = search_entry_or_exit(
            world_map,
            ego_location,
            "before",
            exit_entry_found,
            matrix,
            highway_shape,
            entry_wps,
            exit_wps,
            degree,
            junction.id,
            right_lane_end_wp,
            ghost,
            on_entry
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
            distance_clostest_starting_waypoint = distance(get_closest_starting_waypoint(junction.get_waypoints(carla.LaneType().Driving), car.get_location()).transform.location, car.get_location())
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
                        if is_highway_junction(car, other_car_waypoint, check_junction, road_lane_ids, direction_angle, world_map):
                            on_exit_street = True
                        else:
                            on_exit_street = False
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
            try:
                already_in_curve = abs(yaw_difference) > degree
                already_after_curve = abs(yaw_difference) > degree * 2
            except UnboundLocalError:
                already_after_curve = False
                already_in_curve = False
            if not on_entry:
                if already_after_curve:
                    row = 7
                elif already_in_curve:

                    row = 6
                else:
                    row = 5
            else:
                row = 5

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
            
            # if on exit lane, don't jump back! 
            if ghost and not on_entry:
                col_exit = 3

            entry_streets = entry_road_id + entry_highway_road
            exit_streets = exit_road_id + exit_highway_road
            if (other_car_road_id in entry_streets) and (col_entry < 8):
                insert_in_matrix(matrix, car, ego_vehicle, col_entry, row)

            elif (other_car_road_id in exit_streets) and (col_exit < 8):
                insert_in_matrix(matrix, car, ego_vehicle, col_exit, row)

    return dict(matrix)


def get_waypoint_on_highway_junction(
    junction, ego_waypoint, ego_vehicle_location, wps, world_map
):
    """
    When the ego vehicle is entering/exiting a highway, this function acts as if the ego vehicle is already/still on the highway
    and it is approaching the junction object from the highway. By this, the correct waypoints on the highway can be found for other
    functions to work properly.
    
    Parameters:
        junction (carla.Junction): The junction object for which to find the correct waypoints, as if ego is always on highway.
        ego_waypoint (carla.Waypoint): The current ego vehicle waypoint.
        ego_vehicle_location (carla.Location): The location of the ego vehicle.
        wps (list of carla.Waypoint tuples): List of tuples of start and end waypoints of the junction.
        world_map (carla.Map): The world map object.

    Returns:
        carla.Waypoint: The closest starting waypoint of the junction to the simulated ego vehicle position (i.e. form the perspective of the ghost vehicle).
        carla.Location: A location moved from the center of the bounding box representing the ghost location (i.e. this is the central column in our matrix).
    """
    
    bbx_location = junction.bounding_box.location

    # Calculate the direction vector from bounding box to ego vehicle
    direction = ego_vehicle_location - bbx_location
    distance = math.sqrt(
        direction.x**2 + direction.y**2 + direction.z**2
    )  # Calculate the distance between the two points

    # Normalize the direction vector if the distance is not zero
    if distance > 0:
        direction /= distance
   
    # Move from the center location of the bounding box to get a location that is defenitely on a lane in the correct direction (bc. some junction objects span over all lanes in both directions)
    if "Town04" in world_map.name and ego_waypoint.road_id == 23 and ego_waypoint.lane_id == -2: # before traffic light junction & left turn entry ahead
        distance_to_move = -15.0 # move bbox location away from ego towards highway lanes in the direction left
    else:
        distance_to_move = 5.0 # move bbox location closer to ego towards highway lanes in the direction right  
    new_bbx_location = bbx_location + direction * distance_to_move

    # get a waypoint on the highway infront of the junction object
    highway_waypoint = world_map.get_waypoint(new_bbx_location).previous(40)[0]

    # get closest waypoint from this imaginary waypoint
    closest_waypoint = get_closest_starting_waypoint(
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


def safe_data(ego_vehicle, matrix, street_type, df):
    steering_angle = get_steering_angle(ego_vehicle)
    speed = get_speed(ego_vehicle)
    print("============================================================")
    if matrix:
        row_data = get_row(matrix)
        row_data["speed"] = speed
        row_data["steering_angle"] = steering_angle
        row_data["street_type"] = street_type
        df = df._append(row_data, ignore_index=True)
        return df






# Highway exits

def get_location_x_units_to_left(ego_vehicle, x):
    """
    Returns location that is 'x' units to the left of the ego vehicle. Takes into account the ego vehicle's current location and
    orientation to determine the target location.

    Args:
        ego_vehicle (carla.Vehicle): The ego vehicle for which the target location is calculated.
        x (int): The number of units to the left of the ego vehicle for the target location.

    Returns:
        carla.Location: The calculated target location 'x' units to the left of the ego vehicle.
    """
    location = ego_vehicle.get_location()
    transform = ego_vehicle.get_transform()

    # create a vector to the left of the ego vehicle
    ego_forward_vector = transform.get_forward_vector()
    left_vector = carla.Vector3D(x=-ego_forward_vector.y, y=ego_forward_vector.x, z=ego_forward_vector.z)

    # get target location
    target_location = location - left_vector * x
    target_location = carla.Location(target_location.x, target_location.y, target_location.z)
    return target_location


def angle_between_vectors(a, b):
    dot = a.x * b.x + a.y * b.y
    mag_a = math.sqrt(a.x**2 + a.y**2)
    mag_b = math.sqrt(b.x**2 + b.y**2)
    cos_theta = dot / (mag_a * mag_b)
    theta = math.acos(cos_theta)
    return math.degrees(theta)


def check_ego_exit_highway(ego_vehicle, ego_waypoint, highway_forward_vector, highway_shape, current_lanes):
    if highway_forward_vector and "exit" in highway_shape[0] and highway_shape[0] != "dual_exit":
        if (ego_waypoint.lane_id == max(current_lanes, key=abs) or len(current_lanes) <=2) and angle_between_vectors(ego_vehicle.get_transform().get_forward_vector(), highway_forward_vector) >= 8:
            return True
        else:
            return False
    else: 
        return False




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
    """
    Divides a bounding box into smaller bounding boxes.

    Args:
        bounding_box (_type_): _description_
        rows (int, optional): _description_. Defaults to 2.
        cols (int, optional): _description_. Defaults to 2.

    Returns:
        _type_: _description_
    """
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
