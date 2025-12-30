#!/usr/bin/env python

# Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""
Example script to generate realistic traffic with the InvertedAI API
"""

import os
import time
import carla
import argparse
import logging
import math
import random
import invertedai as iai
import numpy as np
from tqdm import tqdm
from invertedai.common import AgentProperties, AgentState, TrafficLightState, Point
from carla import command, Location

#---------
# CARLA Utils
#---------

# Argument parser
def argument_parser():

    argparser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to')
    argparser.add_argument(
        '-n', '--number-of-vehicles',
        metavar='N',
        default=50,
        type=int,
        help='Number of vehicles spawned by InvertedAI')
    argparser.add_argument(
        '-w', '--number-of-walkers',
        metavar='W',
        default=0,
        type=int,
        help='Number of walkers')
    argparser.add_argument(
        '--safe',
        type=bool,
        default=True,
        help='Avoid spawning vehicles prone to accidents')
    argparser.add_argument(
        '--filterv',
        metavar='PATTERN',
        default='vehicle.*',
        help='Filter vehicle model')
    argparser.add_argument(
        '--generationv',
        metavar='G',
        default='All',
        help='restrict to certain vehicle generation')
    argparser.add_argument(
        '--filterw',
        metavar='PATTERN',
        default='walker.pedestrian.*',
        help='Filter pedestrian type')
    argparser.add_argument(
        '--generationw',
        metavar='G',
        default='All',
        help='restrict to certain pedestrian generation')
    argparser.add_argument(
        '-s', '--seed',
        metavar='S',
        type=int,
        help='Set random seed')
    argparser.add_argument(
        '--hero',
        action='store_true',
        default=False,
        help='Set one of the vehicles as hero')
    argparser.add_argument(
        '--iai-key',
        type=str,
        help="InvertedAI API key.")
    argparser.add_argument(
        '--record',
        action='store_true',
        help="Record the simulation using the CARLA recorder",
        default=False)
    argparser.add_argument(
        '--sim-length',
        type=int,
        default=60,
        help="Length of the simulation in seconds")
    argparser.add_argument(
        '--location',
        type=str,
        help=f"IAI formatted map on which to create simulate (default: carla:Town10HD, only tested there)",
        default='carla:Town10HD')
    argparser.add_argument(
        '--capacity',
        type=int,
        help=f"The capacity parameter of a quadtree leaf before splitting",
        default=100)
    argparser.add_argument(
        '--width',
        type=int,
        help=f"Full width of the area to initialize",
        default=250)
    argparser.add_argument(
        '--height',
        type=int,
        help=f"Full height of the area to initialize",
        default=250)
    argparser.add_argument(
        '--map-center',
        type=int,
        nargs='+',
        help=f"Center of the area to initialize",
        default=tuple([-50,20]))
    argparser.add_argument(
        '--iai-async',
        type=bool,
        help=f"Whether to call drive asynchronously",
        default=True)
    argparser.add_argument(
        '--api-model',
        type=str,
        help=f"IAI API model version",
        default="bI5p")
    argparser.add_argument(
        '--iai-log',
        action="store_true",
        help=f"Export a log file for the InvertedAI cosimulation, which can be replayed afterwards")
    argparser.add_argument(
        '--iai-waypoint-distance',
        type=int,
        default=15,
        help=f"Distance to the next waypoint for IAI agents"
    )
    argparser.add_argument(
        '--iai-waypoint-detection-threshold',
        type=int,
        default=2,
        help=f"Distance to which an agent is deemed as having reached its waypoint"
    )
    argparser.add_argument(
        '--iai-max-distance-away',
        type=int,
        default=20,
        help=f"Maximum distance away before a new waypoint is set for an agent"
    )

    args = argparser.parse_args()

    return args

# Setup CARLA client and world
def setup_carla_environment(host, port, location):
    map_name = location.split(":")[-1]

    step_length = 0.1 # 0.1 is the only step length that is supported by invertedai so far

    client = carla.Client(host, port)
    client.set_timeout(200.0)

    # Configure the simulation environment
    world = client.load_world(map_name)
    world_settings = carla.WorldSettings(
        synchronous_mode=True,
        fixed_delta_seconds=step_length,
    )
    world.apply_settings(world_settings)

    return client, world

# Set spectator view on a hero vehicle
def set_spectator(world, hero_v):

    spectator_offset_x = -6.
    spectator_offset_z = 6.
    spectator_offset_pitch = 20

    hero_t = hero_v.get_transform()

    yaw = hero_t.rotation.yaw
    spectator_l = hero_t.location + carla.Location(
        spectator_offset_x * math.cos(math.radians(yaw)),
        spectator_offset_x * math.sin(math.radians(yaw)),
        spectator_offset_z,
    )
    spectator_t = carla.Transform(spectator_l, hero_t.rotation)
    spectator_t.rotation.pitch -= spectator_offset_pitch
    world.get_spectator().set_transform(spectator_t)

#---------
# Initialize actors
#---------

# Initialize IAI agents from CARLA actors
def initialize_iai_agent(actor, agent_type):

    transf = actor.get_transform()
    vel = actor.get_velocity()
    speed = math.sqrt(vel.x**2. + vel.y**2. +vel.z**2.)

    agent_state = AgentState.fromlist([
                                        transf.location.x,
                                        transf.location.y,
                                        math.radians(transf.rotation.yaw),
                                        speed
                                    ])

    bb = actor.bounding_box
    length, width = bb.extent.x*2, bb.extent.y*2

    agent_properties = AgentProperties(length=length, width=width, agent_type=agent_type)
    if agent_type=="car":
        agent_properties.rear_axis_offset = length*0.38 # Empirical value fitted from InvertedAI initialization

    return agent_state, agent_properties

# Initialize IAI pedestrians from CARLA actors
def initialize_pedestrians(pedestrians):

    iai_pedestrians_states, iai_pedestrians_properties = [], []
    for actor in pedestrians:
        iai_ped_state, iai_ped_properties = initialize_iai_agent(actor,agent_type="pedestrian")
        iai_pedestrians_states.append(iai_ped_state)
        iai_pedestrians_properties.append(iai_ped_properties)

    return iai_pedestrians_states, iai_pedestrians_properties

# Spawn pedestrians in the simulation, which are driven by CARLA controllers (not by invertedai)
def spawn_pedestrians(client, world, num_pedestrians, bps):

    batch = []

    # Get spawn points for pedestrians
    spawn_points = []
    for i in range(num_pedestrians):
        
        loc = world.get_random_location_from_navigation()
        if (loc is not None):
            spawn_point = carla.Transform(location=loc)
            #Apply Offset in vertical to avoid collision spawning
            spawn_point.location.z += 1
            spawn_points.append(spawn_point)

    pedestrians = []
    walkers_list = []

    # Spawn pedestrians
    for i in range(len(spawn_points)):
        walker_bp = random.choice(bps)
        if walker_bp.has_attribute('is_invincible'):
            walker_bp.set_attribute('is_invincible', 'false')
        spawn_point = spawn_points[i]
        batch.append(command.SpawnActor(walker_bp, spawn_point))

    results = client.apply_batch_sync(batch, True)
    pedestrians = world.get_actors().filter('walker.*')
    for i in range(len(results)):
        if results[i].error:
            logging.error(results[i].error)
        else:
            walkers_list.append({"id": results[i].actor_id})

    # Spawn CARLA IA controllers for pedestrians
    batch = []
    walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
    for i in range(len(walkers_list)):
        batch.append(command.SpawnActor(walker_controller_bp, carla.Transform(), walkers_list[i]["id"]))
    results = client.apply_batch_sync(batch, True)

    world.tick()

    for controller in world.get_actors().filter('controller.ai.walker'):
        controller.start()
        dest = world.get_random_location_from_navigation()
        controller.go_to_location(dest)
        controller.set_max_speed(0.5 + random.random())

    return pedestrians

# Get blueprints according to the given filters
def get_actor_blueprints(world, filter, generation):
    bps = world.get_blueprint_library().filter(filter)

    if generation.lower() == "all":
        return bps

    # If the filter returns only one bp, we assume that this one needed
    # and therefore, we ignore the generation
    if len(bps) == 1:
        return bps

    try:
        int_generation = int(generation)
        # Check if generation is in available generations
        if int_generation in [1, 2, 3, 4]:
            bps = [x for x in bps if int(x.get_attribute('generation')) == int_generation]
            return bps
        else:
            print("   Warning! Actor Generation is not valid. No actor will be spawned.")
            return []
    except:
        print("   Warning! Actor Generation is not valid. No actor will be spawned.")
        return []

#---------
# InvertedAI - CARLA synchronization routines
#---------

# Get CARLA transform from IAI transform
def transform_iai_to_carla(agent_state):
    agent_transform = carla.Transform(
        carla.Location(
            agent_state.center.x,
            agent_state.center.y,
            0.1
        ),
        carla.Rotation(
            yaw=math.degrees(agent_state.orientation)
        )
    )

    return agent_transform

# Update transforms of CARLA agents driven by IAI and tick the world
def update_transforms(iai2carla,response):
    """
    Tick the carla simulation forward one time step
    Assume carla_actors is a list of carla actors controlled by IAI
    """
    for agent_id in iai2carla.keys():
        agentdict = iai2carla[agent_id]
        if agentdict["is_iai"]:            
            agent = response.agent_states[agent_id]
            agent_transform = transform_iai_to_carla(agent)
            try:     
                actor = agentdict["actor"]
                actor.set_transform(agent_transform)
            except:
                pass

# Assign existing IAI agents to CARLA vehicle blueprints and add these agents to the CARLA simulation
def assign_carla_blueprints_to_iai_agents(world,vehicle_blueprints,agent_properties,agent_states,recurrent_states,is_iai,noniai_actors):

    agent_properties_new = []
    agent_states_new = []
    recurrent_states_new = []
    iai2carla = {}

    for agent_id, state in enumerate(agent_states):

        if not is_iai[agent_id]:
            agent_properties_new.append(agent_properties[agent_id])
            agent_states_new.append(agent_states[agent_id])
            recurrent_states_new.append(recurrent_states[agent_id])
            actor = noniai_actors[agent_id]
            iai2carla[len(iai2carla)] = {"actor":actor, "is_iai":False, "type":agent_properties[agent_id].agent_type}
             
        else:

            blueprint = random.choice(vehicle_blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            agent_transform = transform_iai_to_carla(state)

            actor = world.try_spawn_actor(blueprint,agent_transform)
            
            if actor is not None:
                bb = actor.bounding_box.extent

                agent_attr = agent_properties[agent_id]

                agent_attr.length = 2*bb.x
                agent_attr.width = 2*bb.y
                agent_attr.rear_axis_offset = 2*bb.x/3

                agent_properties_new.append(agent_attr)
                agent_states_new.append(agent_states[agent_id])
                recurrent_states_new.append(recurrent_states[agent_id])

                actor.set_simulate_physics(False)

                iai2carla[len(iai2carla)] = {"actor":actor, "is_iai":True, "type":agent_properties[agent_id].agent_type}

    if len(agent_properties_new) == 0:
        raise Exception("No vehicles could be placed in Carla environment.")
    
    return agent_properties_new, agent_states_new, recurrent_states_new, iai2carla

# Initialize InvertedAI co-simulation
def initialize_simulation(args, world, agent_states=None, agent_properties=None):

    iai_seed = args.seed if args.seed is not None else random.randint(1,10000)
    traffic_lights_states, carla2iai_tl = initialize_tl_states(world)

    #################################################################################################
    # Initialize IAI Agents
    map_center = args.map_center
    print(f"Call location info.")
    location_info_response = iai.location_info(
        location = args.location,
        rendering_center = map_center
    )
    print(f"Begin initialization.") 
    # Acquire a grid of 100x100m regions in which to initialize vehicles to be controlled by IAI.
    regions = iai.get_regions_default(
        location = args.location,
        total_num_agents = args.number_of_vehicles,
        area_shape = (int(args.width/2),int(args.height/2)),
        map_center = map_center, 
    )
    # Place vehicles within the specified regions which will consider the relative states of nearby vehicles in neighbouring regions.
    response = iai.large_initialize(
        location = args.location,
        regions = regions,
        traffic_light_state_history = [traffic_lights_states],
        agent_states = agent_states,
        agent_properties = agent_properties,
        random_seed = iai_seed
    )

    return response, carla2iai_tl, location_info_response

#---------
# Synchronize InvertedAI and CARLA traffic lights
#---------

# Mapping between CARLA and IAI traffic lights IDs
def get_traffic_lights_mapping(world):
    tls = world.get_actors().filter('traffic.traffic_light*')
    tl_ids = sorted([tl.id for tl in list(tls)])
    carla2iai_tl = {}
    # ID for IAI traffic lights, only valid for Town10 for now (in both UE4 and UE5 versions of the map)
    iai_tl_id = 4364
    for carla_tl_id in tl_ids:
        carla2iai_tl[str(carla_tl_id)] = [str(iai_tl_id), str(iai_tl_id+1000)]
        iai_tl_id+=1

    return carla2iai_tl

# Returns IAI traffic light state based on CARLA traffic light state
def get_traffic_light_state_from_carla(carla_tl_state):

    if carla_tl_state == carla.TrafficLightState.Red:
        return TrafficLightState.red

    elif carla_tl_state == carla.TrafficLightState.Yellow:
        return TrafficLightState.yellow

    elif carla_tl_state == carla.TrafficLightState.Green:
        return TrafficLightState.green

    else:  # Unknown state, turn off traffic light
        return TrafficLightState.Off

# Assign IAI traffic lights based on the CARLA ones
def assign_iai_traffic_lights_from_carla(world, iai_tl, carla2iai_tl):

    traffic_lights = world.get_actors().filter('traffic.traffic_light*')
    
    carla_tl_dict = {}
    for tl in traffic_lights:
        carla_tl_dict[str(tl.id)]=tl.state

    for carla_tl_id, carla_state in carla_tl_dict.items():
        iai_tl_id_pair = carla2iai_tl[carla_tl_id]
        for iai_tl_id in iai_tl_id_pair:
            iai_tl[iai_tl_id] = get_traffic_light_state_from_carla(carla_state)

    return iai_tl

# Initialize traffic lights states
def initialize_tl_states(world):
    carla2iai_tl = get_traffic_lights_mapping(world)
    iai_tl_states = {}
    for tlpair in carla2iai_tl.values():
        for tl in tlpair:
            iai_tl_states[tl] = TrafficLightState.red # Initialize to given value

    iai_tl_states = assign_iai_traffic_lights_from_carla(world, iai_tl_states, carla2iai_tl)
    return iai_tl_states, carla2iai_tl


def get_distance(point1, point2):
    return np.sqrt((point2[0] - point1[0])**2 + (point2[1] - point1[1])**2)


def new_waypoint_needed(waypoint, agent_state, detection_threshold, max_distance_away):
    distance_from_waypoint = get_distance(waypoint, [agent_state.center.x, agent_state.center.y])
    return distance_from_waypoint <= detection_threshold or distance_from_waypoint >= max_distance_away


def get_next_waypoints(carla_map, agent_states, waypoints, args):
    indices_of_interest = []
    interested_agent_states = []
    if len(waypoints) == 0:
        indices_of_interest = [i for i in range(len(agent_states))]
    else:
        indices_of_interest = [i for i in range(len(agent_states)) if new_waypoint_needed(waypoints[i], agent_states[i], args.iai_waypoint_detection_threshold, args.iai_max_distance_away)]
    vehicle_locations = [Location(x=agent_states[i].center.x, y=agent_states[i].center.y, z=0.1) for i in indices_of_interest]
    closest_waypoints = [carla_map.get_waypoint(vehicle_location, project_to_road=True, lane_type=carla.LaneType.Driving) for vehicle_location in vehicle_locations]
    next_waypoints = [np.random.choice(wp.next(args.iai_waypoint_distance)) for wp in closest_waypoints]
    results = []
    if len(waypoints) == 0:
        results = [[waypoint.transform.location.x, waypoint.transform.location.y] for waypoint in next_waypoints]
    else:
        results = waypoints
        for ind in range(len(next_waypoints)):
            results[indices_of_interest[ind]] = [next_waypoints[ind].transform.location.x, next_waypoints[ind].transform.location.y]
    return results

#---------
# Main
#---------
def main():

    args = argument_parser()

    # Setup CARLA client and world
    client, world = setup_carla_environment(args.host, args.port, args.location)

    # Specify the IAI API key
    try:
        iai.add_apikey(args.iai_key)
    except:
        print("\n\tYou need to indicate the InvertedAI API key with the argument --iai-key. To obtain one, please go to https://www.inverted.ai \n")

    num_pedestrians = args.number_of_walkers

    FPS = int(1./world.get_settings().fixed_delta_seconds)

    if args.record:
        logfolder = os.getcwd()+"/logs/"
        if not os.path.exists(logfolder):
            os.system("mkdir "+logfolder)
        logfile = logfolder+"carla_record.log"
        client.start_recorder(logfile)
        print("Recording on file: %s" % logfile)

    seed = args.seed

    if seed:
        random.seed(seed)
    
    vehicle_blueprints = get_actor_blueprints(world, args.filterv, args.generationv)
    if args.safe:
        vehicle_blueprints = [x for x in vehicle_blueprints if x.get_attribute('base_type') == 'car']   

    agent_states, agent_properties = [], []
    is_iai = []
    noniai_actors = []
        
    # Add pedestrians (not driven by IAI)
    if num_pedestrians>0:
        if seed:
            world.set_pedestrians_seed(seed)
        blueprintsWalkers = get_actor_blueprints(world, args.filterw, args.generationw)
        if not blueprintsWalkers:
            raise ValueError("Couldn't find any walkers with the specified filters")
        pedestrians = spawn_pedestrians(client, world, num_pedestrians, blueprintsWalkers)
        iai_pedestrians_states, iai_pedestrians_properties = initialize_pedestrians(pedestrians)
        agent_states.extend(iai_pedestrians_states)
        agent_properties.extend(iai_pedestrians_properties)
        is_iai.extend( [False]*len(iai_pedestrians_states) )
        noniai_actors.extend(pedestrians)
    
    else:
        pedestrians = []
    
    num_noniai = len(agent_properties)
    # Initialize InvertedAI co-simulation
    response, carla2iai_tl, location_info_response = initialize_simulation(args, world, agent_states=agent_states, agent_properties=agent_properties)
    agent_properties = response.agent_properties
    is_iai.extend( [True]*(len(agent_properties)-num_noniai) )
    
    # Map IAI agents to CARLA actors and update response properties and states
    print(f"Number of agents initialized: {len(response.agent_states)}")
    agent_properties, agent_states_new, recurrent_states_new, iai2carla = assign_carla_blueprints_to_iai_agents(world,vehicle_blueprints,agent_properties,response.agent_states,response.recurrent_states,is_iai,noniai_actors)
    traffic_lights_states = assign_iai_traffic_lights_from_carla(world,response.traffic_lights_states, carla2iai_tl)
    response.agent_states = agent_states_new
    response.agent_properties = agent_properties
    response.recurrent_states = recurrent_states_new
    response.traffic_lights_states = traffic_lights_states

    # Write InvertedAI log file, which can be opened afterwards to visualize a gif and further analysis
    # See an example of usage here: https://github.com/inverted-ai/invertedai/blob/master/examples/scenario_log_example.py

    if args.iai_log:
        log_writer = iai.LogWriter()
        log_writer.initialize(
            location=args.location,
            location_info_response=location_info_response,
            init_response=response
        )
    iailog_path = os.path.join(os.getcwd(),f"iai_log.json")

    # Perform first CARLA simulation tick
    world.tick()
    
    iai_agent_indices = [i for i in range(len(agent_properties)) if iai2carla[i]["is_iai"]]
    try:

        vehicles = world.get_actors().filter('vehicle.*')
        print("Total number of agents:",len(agent_properties),"Vehicles",len(vehicles), "Pedestrians:",len(pedestrians))
        for index in iai_agent_indices:
            agent_properties[index].max_speed = 10.0
        # Get hero vehicle
        hero_v = None
        if args.hero:
            hero_v = vehicles[0]

        carla_map = world.get_map()
        waypoints = []
        for frame in tqdm(range(args.sim_length * FPS)):
            response.traffic_lights_states = assign_iai_traffic_lights_from_carla(world, response.traffic_lights_states, carla2iai_tl)
            iai_agent_states = [response.agent_states[i] for i in iai_agent_indices]
            waypoints = get_next_waypoints(carla_map, iai_agent_states, waypoints, args)
            for i in range(len(waypoints)):
                agent_properties[iai_agent_indices[i]].waypoint = Point(x=waypoints[i][0], y=waypoints[i][1])

            # IAI update step
            response = iai.large_drive(
                location = args.location,
                agent_states = response.agent_states,
                agent_properties = agent_properties,
                recurrent_states = response.recurrent_states,
                traffic_lights_states = response.traffic_lights_states,
                single_call_agent_limit = args.capacity,
                async_api_calls = args.iai_async,
                api_model_version = args.api_model,
                random_seed = seed
            )

            if args.iai_log:
                log_writer.drive(drive_response=response)

            # Update CARLA actors with new transforms from IAI agents
            update_transforms(iai2carla,response)

            # Tick CARLA simulation
            world.tick()

            # Update agents not driven by IAI in IAI cosimulation, like pedestrians
            for agent_id in iai2carla.keys():
                agentdict = iai2carla[agent_id]

                if not agentdict["is_iai"] and agentdict["type"] == "pedestrian":
                    actor = agentdict["actor"]
                    state, properties = initialize_iai_agent(actor, agentdict["type"])
                    response.agent_states[agent_id] = state
                    agent_properties[agent_id] = properties

            # Update spectator view if there is hero vehicle
            if hero_v is not None:
                set_spectator(world, hero_v)

            

    finally:
        time.sleep(0.5)

        if args.record:
            client.stop_recorder()

        if args.iai_log:
            log_writer.export_to_file(log_path=iailog_path)

if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
