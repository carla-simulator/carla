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
import imageio

from tqdm import tqdm
from enum import Enum
from dataclasses import dataclass
from invertedai.common import AgentProperties, AgentState, TrafficLightState, Point, RecurrentState
from carla import command, Location
from typing import List, Tuple, Any, Optional

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
        default="nBu1")
    argparser.add_argument(
        '--iai-log',
        action="store_true",
        help=f"Export a log file for the InvertedAI cosimulation, which can be replayed afterwards")
    argparser.add_argument(
        '--capture-video',
        action="store_true",
        help=f"Capture video within Carla.")

    args = argparser.parse_args()

    return args

#---------
# Video Capture
#---------    

@dataclass
class VideoResolution:
    res_x: int
    res_y: int

class VideoResolutionEnum(Enum):
    ULTRAHD = VideoResolution(res_x=3840,res_y=2160)
    FULLHD = VideoResolution(res_x=1920,res_y=1080)
    HD = VideoResolution(res_x=1280,res_y=720)

class CameraType(Enum):
    RGB = "sensor.camera.rgb"
    SEGMENTATION = "sensor.camera.semantic_segmentation"
    DEPTH = "sensor.camera.depth"

class CameraAttachment(Enum):
    FPV = "fpv"
    REARPOLE = "rearpole"
    CCTV = "cctv"

@dataclass
class CameraAttachmentConfiguration:
    attachment: CameraAttachment
    transform: carla.Transform
    actor_to_attach: Optional[carla.Actor] = None

def get_default_cam_attachment(
    attachment: CameraAttachment,
    transform: Optional[carla.Transform] = None,
    actor_to_attach: Optional[carla.Actor] = None
) -> CameraAttachmentConfiguration:
    if attachment == CameraAttachment.REARPOLE:
        if actor_to_attach is None: 
            raise ValueError(f"Must include an actor to attach for this camera attachment configuration: {attachment}.")
        return CameraAttachmentConfiguration(
            attachment = attachment,
            actor_to_attach = actor_to_attach,
            transform = carla.Transform(
                carla.Location(x=-10.0, z=3.0),
                carla.Rotation(yaw=0.0,pitch=-10.0,roll=0.0)
            ) if transform is None else transform
        )
    if attachment == CameraAttachment.FPV:
        if actor_to_attach is None: 
            raise ValueError(f"Must include an actor to attach for camera attachment configuration: {attachment}.")
        return CameraAttachmentConfiguration(
            attachment = attachment,
            actor_to_attach = actor_to_attach,
            transform = carla.Transform(
                carla.Location(x=2.8, z=1.5)
            ) if transform is None else transform
        )
    if attachment == CameraAttachment.CCTV:
        if transform is None:
            raise ValueError(f"Must include a transform for camera attachment configuration: {attachment}.")
        return CameraAttachmentConfiguration(
            attachment = attachment,
            actor_to_attach = None,
            transform = transform
        )

@dataclass
class CameraSpecification:
    attachment_cfg: CameraAttachmentConfiguration
    type: CameraType
    fps: int
    fov: int = 110
    resolution: VideoResolutionEnum = VideoResolutionEnum.FULLHD
    name: str = str(int(time.time()))
    save_path: str = os.getcwd()
    
class CameraRecorder:
    def __init__(
        self,
        name,
        save_dir_path,
        sensor_type
    ):
        self.name = name

        self.full_dir_path = os.path.join(save_dir_path,name)
        os.mkdir(self.full_dir_path)

        self.data = None

        self.sensor_type = sensor_type

    def sensor_callback(self,data):
        if self.sensor_type == CameraType.SEGMENTATION:
            data.convert(carla.ColorConverter.CityScapesPalette)
        elif self.sensor_type == CameraType.DEPTH:
            data.convert(carla.ColorConverter.LogarithmicDepth)
            
        data.save_to_disk(path = os.path.join(self.full_dir_path,'%08d' % data.frame))

@dataclass
class CarlaSensorObject:
    cam_spec: CameraSpecification
    sensor: carla.Sensor
    recorder: CameraRecorder

class SensorManager:
    def __init__(
        self,
        camera_specs: List[CameraSpecification],
        world: carla.World,
        spectator_id: Optional[int] = None
    ):
        self.world = world
        self.spectator_id = spectator_id
        self.cameras = [self.add_camera(spec) for spec in camera_specs]

    def update_all_sensors(self):
        self.update_cameras()
        self.update_spectator()

    def update_cameras(self):
        for cam in self.cameras:
            attached_actor = cam.cam_spec.attachment_cfg.actor_to_attach
            transform = cam.cam_spec.attachment_cfg.transform
            if attached_actor is not None:
                x, y, z = transform.location.x, transform.location.y, transform.location.z
                sensor_transform = carla.Transform(
                    attached_actor.get_transform().transform(carla.Location(x, y, z)),
                    attached_actor.get_transform().rotation,
                )
            else:
                sensor_transform = transform

            cam.sensor.set_transform(sensor_transform)

    def update_spectator(self):
        if self.spectator_id is not None:
            self.world.get_spectator().set_transform(self.cameras[self.spectator_id].sensor.get_transform())

    def add_camera(
        self,
        cam_spec: CameraSpecification,
    ) -> CarlaSensorObject:
        if cam_spec.attachment_cfg.actor_to_attach is None:
            sensor_transform = cam_spec.attachment_cfg.transform
        else:
            x, y, z = (
                cam_spec.attachment_cfg.transform.location.x, 
                cam_spec.attachment_cfg.transform.location.y, 
                cam_spec.attachment_cfg.transform.location.z
            )
            sensor_transform = carla.Transform(
                cam_spec.attachment_cfg.actor_to_attach.get_transform().transform(carla.Location(x, y, z)),
                cam_spec.attachment_cfg.actor_to_attach.get_transform().rotation,
            )
        
        camera_bp = self.world.get_blueprint_library().find(cam_spec.type.value)
        camera_bp.set_attribute('image_size_x', str(cam_spec.resolution.value.res_x))
        camera_bp.set_attribute('image_size_y', str(cam_spec.resolution.value.res_y))
        camera_bp.set_attribute('sensor_tick', str(1/cam_spec.fps))
        camera_bp.set_attribute('fov', str(cam_spec.fov))
        sensor = self.world.spawn_actor(camera_bp, sensor_transform)

        recorder = CameraRecorder(
            name=cam_spec.name,
            save_dir_path=cam_spec.save_path,
            sensor_type=cam_spec.type.value,
        )
        sensor.listen(recorder.sensor_callback)

        return CarlaSensorObject(
            cam_spec = cam_spec,
            sensor = sensor,
            recorder = recorder
        )
    
    def write_videos(
        self,
        delete_images: bool = True
    ):
        for cam in self.cameras:
            img_list = []

            for img_path in sorted(os.listdir(cam.recorder.full_dir_path)):
                img = imageio.imread(os.path.join(cam.recorder.full_dir_path,img_path))
                img_list.append(img)

            full_video_dir = os.path.join(cam.recorder.full_dir_path,"video")
            os.mkdir(full_video_dir)
            full_video_path = os.path.join(full_video_dir,f"{cam.recorder.name}.mp4")
            imageio.mimsave(
                full_video_path, 
                img_list, 
                format='FFMPEG', 
                fps=cam.cam_spec.fps,
                macro_block_size = 1
            )

            if delete_images:
                for filename in os.listdir(cam.recorder.full_dir_path):
                    if filename.endswith('.png'):
                        os.remove(os.path.join(cam.recorder.full_dir_path, filename))


#---------
# Simulation Agent Data
#---------    

class AgentType(Enum):
    CARLA = 0
    IAI = 1
    EGO = 2

@dataclass
class AgentData:
    type: AgentType
    properties: AgentProperties
    state: AgentState
    recurrent_state: Optional[RecurrentState] = None
    other: Optional[Any] = None
    carla_actor: Optional[carla.Actor] = None

class SimulationData:
    def __init__(
        self,
        agent_data: List[AgentData]
    ):
        self.all_agent_data = agent_data

    def get_all_states(self) -> List[AgentState]:
        return [agent.state for agent in self.all_agent_data]

    def get_all_properties(self) -> List[AgentProperties]:
        return [agent.properties for agent in self.all_agent_data]

    def get_all_recurrent_states(self) -> List[Optional[RecurrentState]]:
        return [agent.recurrent_state for agent in self.all_agent_data]

    def get_all_carla_states(self ) -> List[Optional[carla.Actor]]:
        return [agent.carla_actor for agent in self.all_agent_data]

    def get_all_other_data_per_type(
        self,
        agent_type: AgentType
    ) -> List[Optional[Any]]:
        return [agent.other for agent in self.all_agent_data if agent.type == agent_type]
    
    def update_non_carla_iai_states(
        self,
        agent_states: List[AgentState],
        agent_properties: List[AgentProperties],
        agent_recurrent_states: List[RecurrentState]
    ):
        for agent_id in range(len(self.all_agent_data)):
            if not self.all_agent_data[agent_id].type == AgentType.CARLA:
                self.all_agent_data[agent_id].state = agent_states[agent_id]
                self.all_agent_data[agent_id].properties = agent_properties[agent_id]
                self.all_agent_data[agent_id].recurrent_state = agent_recurrent_states[agent_id]

    # Update transforms of CARLA agents driven by IAI and tick the world
    def update_carla_states_from_iai(self):
        """
        Tick the carla simulation forward one time step
        Assume carla_actors is a list of carla actors controlled by IAI
        """
        for agent in self.all_agent_data:
            if not agent.type == AgentType.CARLA:
                agent_transform = transform_iai_to_carla(agent.state)
                try:     
                    agent.carla_actor.set_transform(agent_transform)
                except:
                    pass

    def update_iai_states_from_carla(self):
        for agent_id, agent in enumerate(self.all_agent_data):
            if agent.type == AgentType.CARLA:
                state, properties = initialize_iai_agent(agent.carla_actor, agent.properties.agent_type)
                self.all_agent_data[agent_id].state = state
                self.all_agent_data[agent_id].properties = properties

    def get_type_indexes(
        self,
        agent_type: AgentType
    ) -> List[int]:
        return [ind for ind in range(len(self.all_agent_data)) if self.all_agent_data[ind].type == agent_type]
    
    def get_all_types(self) -> List[AgentType]:
        return [agent.type for agent in self.all_agent_data]

#---------
# Carla Environment Setup
#---------

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
def spawn_pedestrians(
    client, 
    world, 
    num_pedestrians, 
    bps
):

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
def get_actor_blueprints(
    world, 
    filter, 
    generation
):
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

# Assign existing IAI agents to CARLA vehicle blueprints and add these agents to the CARLA simulation
def assign_carla_blueprints_to_agents(
    world,
    vehicle_blueprints,
    agent_data
):
    agents_to_pop = []

    for agent_id, data in enumerate(agent_data):
        if not data.type == AgentType.CARLA:
            blueprint = random.choice(vehicle_blueprints)
            if blueprint.has_attribute('color'):
                color = random.choice(blueprint.get_attribute('color').recommended_values)
                blueprint.set_attribute('color', color)
            agent_transform = transform_iai_to_carla(data.state)

            actor = world.try_spawn_actor(blueprint,agent_transform)
            
            if actor is not None:
                bb = actor.bounding_box.extent
                actor.set_simulate_physics(False)

                agent_prop = data.properties

                agent_prop.length = 2*bb.x
                agent_prop.width = 2*bb.y
                agent_prop.rear_axis_offset = 2*bb.x/3

                agent_data[agent_id].properties = agent_prop
                agent_data[agent_id].carla_actor = actor

            else:
                agents_to_pop.append(agent_id)

    for agent_id in reversed(agents_to_pop):
        agent_data.pop(agent_id)
    
    if len(agent_data) == 0:
        raise Exception("No vehicles could be placed in Carla environment.")
    
    return agent_data

# Initialize InvertedAI co-simulation
def initialize_simulation(
    args, 
    world,
    seed,
    vehicle_blueprints,
    agent_data,
    existing_agent_states=None, 
    existing_agent_properties=None
):
    
    traffic_lights_states, carla2iai_tl = initialize_tl_states(world)

    #################################################################################################
    # Initialize IAI Agents
    map_center = args.map_center
    print(f"Call location info.")
    location_info_response = iai.location_info(
        location = args.location,
        include_map_source=True,
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
        agent_states = existing_agent_states,
        agent_properties = existing_agent_properties,
        random_seed = seed
    )

    num_sampled_agents = len(response.agent_properties)
    agent_data += [AgentData(
        type = AgentType.IAI,
        state = response.agent_states[ind],
        properties = response.agent_properties[ind],
        recurrent_state = response.recurrent_states[ind]
    ) for ind in range(num_sampled_agents)]
    
    agent_data = assign_carla_blueprints_to_agents(
        world = world,
        vehicle_blueprints = vehicle_blueprints,
        agent_data = agent_data
    )

    agent_states = []
    agent_properties = []
    agent_recurrent_states = []
    for data in agent_data:
        agent_states.append(data.state)
        agent_properties.append(data.properties)
        agent_recurrent_states.append(data.recurrent_state)
    response.agent_states = agent_states
    response.agent_properties = agent_properties
    response.recurrent_states = agent_recurrent_states

    return response, carla2iai_tl, location_info_response, agent_data

#---------
# Ego Simulator-in-the-Loop Integration
# EDIT THESE FUNCTIONS HERE TO INTEGRATE YOUR OWN EGO VEHICLE
# Every "pass" is where your own code should be added
#---------
def convert_ego_states_to_iai_format(
    ego_states: List
) -> List[AgentState]:
    agent_states = []

    pass

    return agent_states

def convert_ego_properties_to_iai_format(
    ego_properties: List
) -> List[AgentProperties]:
    agent_properties = []

    pass

    return agent_properties

def initialize_ego_vehicle() -> Tuple[List[AgentState],List[AgentProperties]]:
    ego_states = []
    ego_properties = []

    pass
    
    ego_agent_states = convert_ego_states_to_iai_format(ego_states)
    ego_agent_properties = convert_ego_properties_to_iai_format(ego_properties)

    return ego_agent_states, ego_agent_properties

def tick_ego_vehicle() -> Tuple[List[AgentState],List[AgentProperties]]:
    ego_states = []
    ego_properties = []

    pass
    
    updated_ego_agent_states = convert_ego_states_to_iai_format(ego_states)
    updated_ego_agent_properties= convert_ego_properties_to_iai_format(ego_properties)

    return updated_ego_agent_states, updated_ego_agent_properties

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
def assign_iai_traffic_lights_from_carla(
    world, 
    iai_tl, 
    carla2iai_tl
):

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

#---------
# Main
#---------
def main():

    args = argument_parser()

    # Setup CARLA client and world
    client, world = setup_carla_environment(
        args.host, 
        args.port, 
        args.location
    )

    # Specify the IAI API key
    try:
        api_key = os.environ.get("IAI_API_KEY", None)
        if api_key is None:
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

    seed = args.seed if args.seed is not None else int(time.time())
    random.seed(seed)
    
    vehicle_blueprints = get_actor_blueprints(
        world, 
        args.filterv, 
        args.generationv
    )
    if args.safe:
        vehicle_blueprints = [x for x in vehicle_blueprints if x.get_attribute('base_type') == 'car']   

    agent_data = []
    ego_agent_states, ego_agent_properties = initialize_ego_vehicle()
    agent_data += [AgentData(
        type = AgentType.EGO,
        state = state,
        properties = props,
        recurrent_state = RecurrentState()
    ) for state, props in zip(ego_agent_states,ego_agent_properties)]

    # Add Carla-driven pedestrians
    if num_pedestrians>0:
        world.set_pedestrians_seed(seed)
        blueprintsWalkers = get_actor_blueprints(world, args.filterw, args.generationw)
        if not blueprintsWalkers:
            raise ValueError("Couldn't find any walkers with the specified filters")
        pedestrians = spawn_pedestrians(client, world, num_pedestrians, blueprintsWalkers)
        iai_pedestrians_states, iai_pedestrians_properties = initialize_pedestrians(pedestrians)

        agent_data += [AgentData(
            type = AgentType.CARLA,
            state = iai_pedestrians_states[ind],
            properties = iai_pedestrians_properties[ind],
            carla_actor = pedestrians[ind],
            recurrent_state = RecurrentState()
        ) for ind in range(len(iai_pedestrians_states))]
    
    sim_pre_data = SimulationData(agent_data)
    # Initialize InvertedAI co-simulation
    response, carla2iai_tl, location_info_response, agent_data = initialize_simulation(
        args=args, 
        world=world,
        seed=seed,
        vehicle_blueprints=vehicle_blueprints,
        agent_data=agent_data,
        existing_agent_states=sim_pre_data.get_all_states(), 
        existing_agent_properties=sim_pre_data.get_all_properties()
    )
    sim_agent_data = SimulationData(agent_data)
    agent_properties = sim_agent_data.get_all_properties()
    # Map IAI agents to CARLA actors and update response properties and states
    print(f"Number of agents initialized: {len(response.agent_states)}")

    wp_manager = iai.WaypointManager(
        location_info_response = location_info_response,
        cfg = iai.WaypointManagerConfig(
            random_seed=seed,
            fail_soft=True
        )
    )

    # Write InvertedAI log file, which can be opened afterwards to visualize a gif and further analysis
    # See an example of usage here: https://github.com/inverted-ai/invertedai/blob/master/examples/scenario_log_example.py

    if args.iai_log or args.capture_video:
        sim_name = str(int(time.time()))
        iai_output_dir = os.path.join(os.getcwd(),sim_name)
        os.mkdir(iai_output_dir)
    
    if args.iai_log:
        log_writer = iai.LogWriter()
        log_writer.initialize(
            location=args.location,
            location_info_response=location_info_response,
            init_response=response
        )
        iai_log_output_dir = os.path.join(iai_output_dir,f"iai_log")
        os.mkdir(iai_log_output_dir)
        iai_output_data = os.path.join(iai_log_output_dir,f"{sim_name}_iai_log")

    # Perform CARLA simulation tick to spawn vehicles
    world.tick()
    
    sensor_manager = None
    try:
        vehicles = world.get_actors().filter('vehicle.*')

        if args.capture_video:
            sensor_manager = SensorManager(
                world = world,
                spectator_id = 0,
                camera_specs = [
                    CameraSpecification(
                        attachment_cfg = get_default_cam_attachment(
                            attachment = CameraAttachment.REARPOLE,
                            actor_to_attach = vehicles[random.randint(0,len(vehicles)-1)]
                        ),
                        type = CameraType.RGB,
                        fps = FPS,
                        save_path = iai_output_dir,
                        name = f"{sim_name}_carla_camera_0"
                    )
                ]
            )

            # Perform CARLA simulation tick to spawn sensors
            world.tick()
        for frame in tqdm(range(args.sim_length * FPS)):
            traffic_lights_states = assign_iai_traffic_lights_from_carla(world, response.traffic_lights_states, carla2iai_tl)
            agent_properties = wp_manager.update(
                response = response,
                agent_properties = agent_properties,
                agents_mask = [agent_type == AgentType.IAI for agent_type in sim_agent_data.get_all_types()]
            )

            #=================================================
            #Tick Ego

            updated_ego_agent_states, updated_ego_agent_properties = tick_ego_vehicle()

            #=================================================
            #=================================================
            #Tick Carla
            world.tick()
            time.sleep(1/FPS)

            # Update spectator view if there is hero vehicle
            if args.capture_video:
                sensor_manager.update_all_sensors()

            #=================================================
            #=================================================
            #Tick IAI

            response = iai.large_drive(
                location = args.location,
                agent_states = response.agent_states,
                agent_properties = agent_properties,
                recurrent_states = response.recurrent_states,
                traffic_lights_states = traffic_lights_states,
                single_call_agent_limit = args.capacity,
                async_api_calls = args.iai_async,
                api_model_version = args.api_model,
                random_seed = seed
            )

            #=================================================
            #=================================================
            #Update All Simulation Data

            # Update CARLA actors with new transforms from IAI agents
            for ind, state, props in zip(
                sim_agent_data.get_type_indexes(AgentType.EGO),
                updated_ego_agent_states,
                updated_ego_agent_properties
            ):
                response.agent_states[ind] = state
                agent_properties[ind] = props

            sim_agent_data.update_non_carla_iai_states(
                agent_states = response.agent_states,
                agent_properties = agent_properties,
                agent_recurrent_states = response.recurrent_states
            )

            sim_agent_data.update_carla_states_from_iai()
            sim_agent_data.update_iai_states_from_carla()

            response.agent_states = sim_agent_data.get_all_states()
            agent_properties = sim_agent_data.get_all_properties()

            if args.iai_log:
                log_writer.drive(
                    drive_response=response,
                    agent_properties=agent_properties
                )
            #=================================================

        time.sleep(0.5)

        if args.capture_video:
            print(f"Writing sensor videos to disk.")
            sensor_manager.write_videos(
                delete_images = True
            )
        
        if args.record:
            client.stop_recorder()

        if args.iai_log:
            print(f"Writing log data.")
            log_writer.export_to_file(log_path=f"{iai_output_data}.json")
            print(f"Generating birdview GIF.")
            log_writer.visualize(
                gif_path=f"{iai_output_data}.gif",
                fov = max(args.width,args.height),
                resolution = (2048,2048),
                dpi = 300,
                direction_vec = True,
                velocity_vec = False,
                plot_frame_number = True,
                map_center = args.map_center,
                left_hand_coordinates = True,
                agent_ids = list(range(len(agent_properties)))
            )
    except Exception as e:
        print(f"{e}")


if __name__ == '__main__':

    try:
        main()
    except KeyboardInterrupt:
        pass
    finally:
        print('\ndone.')
