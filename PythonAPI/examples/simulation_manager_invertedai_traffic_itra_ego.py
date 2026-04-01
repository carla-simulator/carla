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
import uuid 
import carla
import argparse
import logging
import math
import random
import invertedai as iai
import numpy as np
import imageio
import traceback

from tqdm import tqdm
from enum import Enum
from dataclasses import dataclass
from invertedai.common import AgentProperties, AgentState, TrafficLightState, Point, RecurrentState, AgentType
from invertedai.common import AgentData as IaiAgentData
from invertedai import SimulationManager, RegionsConfig, LogWriterConfig 
from carla import command, Location
from typing import List, Tuple, Any, Optional, Dict

#---------
# CARLA Utils
#---------

IAI_TIME_STEP = 0.1

# Argument parser
def argument_parser():

    argparser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server'
    )
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to'
    )
    argparser.add_argument(
        '-n', '--number-of-vehicles',
        metavar='N',
        default=50,
        type=int,
        help='Number of vehicles spawned by InvertedAI'
    )
    argparser.add_argument(
        '-w', '--number-of-walkers',
        metavar='W',
        default=0,
        type=int,
        help='Number of walkers'
    )
    argparser.add_argument(
        '--safe',
        type=bool,
        default=True,
        help='Avoid spawning vehicles prone to accidents'
    )
    argparser.add_argument(
        '--filterv',
        metavar='PATTERN',
        default='vehicle.*',
        help='Filter vehicle model'
    )
    argparser.add_argument(
        '--generationv',
        metavar='G',
        default='All',
        help='restrict to certain vehicle generation'
    )
    argparser.add_argument(
        '--filterw',
        metavar='PATTERN',
        default='walker.pedestrian.*',
        help='Filter pedestrian type'
    )
    argparser.add_argument(
        '--generationw',
        metavar='G',
        default='All',
        help='restrict to certain pedestrian generation'
    )
    argparser.add_argument(
        '-s', '--seed',
        metavar='S',
        type=int,
        help='Set random seed'
    )
    argparser.add_argument(
        '--iai-key',
        type=str,
        help="InvertedAI API key."
    )
    argparser.add_argument(
        '--record',
        action='store_true',
        help="Record the simulation using the CARLA recorder",
        default=False
    )
    argparser.add_argument(
        '--sim-length',
        type=int,
        default=60,
        help="Length of the simulation in seconds"
    )
    argparser.add_argument(
        '--location',
        type=str,
        help=f"IAI formatted map on which to create simulate (default: carla:Town10HD, only tested there)",
        default='carla:Town10HD'
    )
    argparser.add_argument(
        '--width',
        type=int,
        help=f"Full width of the area to initialize",
        default=250
    )
    argparser.add_argument(
        '--height',
        type=int,
        help=f"Full height of the area to initialize",
        default=250
    )
    argparser.add_argument(
        '--map-center',
        type=int,
        nargs='+',
        help=f"Center of the area to initialize",
        default=tuple([-50,20])
    )
    argparser.add_argument(
        '--api-model',
        type=str,
        help=f"IAI API model version",
        default="nBu1"
    )
    argparser.add_argument(
        '--iai-log',
        action="store_true",
        help=f"Export a log file for the InvertedAI cosimulation, which can be replayed afterwards"
    )
    argparser.add_argument(
        '--capture-video',
        action="store_true",
        help=f"Capture video within Carla."
    )
    argparser.add_argument(
        '--interpolation-steps',
        type=int,
        help=f"Number of interpolation steps between default IAI time steps of 100ms (e.g. interpolation steps value of 3 is equivalent to 33.3ms = 30 FPS)",
        default=1
    )
    argparser.add_argument(
        '--scenario-path',
        type=str,
        help=f"Path to an IAI foramtted scenario log to be used for the ego vehicle(s).",
        default=''
    )
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
            print(f"Generating video for camera: {cam}")
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

class AgentSourceType(Enum):
    CARLA = 0
    IAI = 1
    EGO = 2

@dataclass
class AgentData:
    type: AgentSourceType
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
        agent_type: AgentSourceType
    ) -> List[Optional[Any]]:
        return [agent.other for agent in self.all_agent_data if agent.type == agent_type]
    
    def update_non_carla_iai_states(
        self,
        agent_states: List[AgentState],
        agent_properties: List[AgentProperties],
        agent_recurrent_states: List[RecurrentState]
    ):
        for agent_id in range(len(self.all_agent_data)):
            if not self.all_agent_data[agent_id].type == AgentSourceType.CARLA:
                self.all_agent_data[agent_id].state = agent_states[agent_id]
                self.all_agent_data[agent_id].properties = agent_properties[agent_id]
                self.all_agent_data[agent_id].recurrent_state = agent_recurrent_states[agent_id]

    # Update transforms of CARLA agents driven by IAI and tick the world
    def update_carla_states_from_iai(
        self,
        agent_data_list: Optional[List[AgentData]] = None
    ):
        """
        Tick the carla simulation forward one time step
        Assume carla_actors is a list of carla actors controlled by IAI
        """
        agent_list = agent_data_list if agent_data_list is not None else self.all_agent_data
        
        for agent in agent_list:
            if not agent.type == AgentSourceType.CARLA:
                agent_transform = transform_iai_to_carla(agent.state)
                try:     
                    agent.carla_actor.set_transform(agent_transform)
                except:
                    pass

    def update_iai_states_from_carla(self):
        for agent_id, agent in enumerate(self.all_agent_data):
            if agent.type == AgentSourceType.CARLA:
                state, properties = initialize_iai_agent(agent.carla_actor, agent.properties.agent_type)
                self.all_agent_data[agent_id].state = state
                self.all_agent_data[agent_id].properties = properties

    def get_type_indexes(
        self,
        agent_type: AgentSourceType
    ) -> List[int]:
        return [ind for ind in range(len(self.all_agent_data)) if self.all_agent_data[ind].type == agent_type]
    
    def get_all_types(self) -> List[AgentSourceType]:
        return [agent.type for agent in self.all_agent_data]

#---------
# Carla Environment Setup
#---------

# Setup CARLA client and world
def setup_carla_environment(
    host, 
    port, 
    location,
    step_length = 0.1
):
    map_name = location.split(":")[-1]

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
        if not data.type == AgentSourceType.CARLA:
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

def initialize_simulation(
    args,
    world,
    seed,
    vehicle_blueprints,
    existing_agent_data,
    existing_agent_ids,        
    traffic_lights_states=None,
    iai_log_path=None, 
):
    traffic_lights_states, carla2iai_tl = initialize_tl_states(world)

    map_center = args.map_center
    print(f"Call location info.")
    location_info_response = iai.location_info(
        location=args.location,
        include_map_source=True,
        rendering_center=map_center
    )
    print(f"Begin initialization.")

    # Build waypoint config using lanelet map from location_info_response
    waypoint_cfg = iai.WaypointManagerConfig(
        lanelet_map=location_info_response.get_lanelet_map(),
        random_seed=seed,
        fail_soft=True,
    )

    # Optionally build log writer config (replaces manual iai.LogWriter setup in main)
    log_writer_cfg = None
    if iai_log_path is not None:
        log_writer_cfg = LogWriterConfig(
            log_path=iai_log_path,
            location=args.location,
            location_info_response=location_info_response,
        )

    # Create SimulationManager — manages IAI agents internally, handles waypoints and logging
    simulation_manager = SimulationManager(
        waypoint_cfg=waypoint_cfg,
        log_writer_cfg=log_writer_cfg,
    )

    # Form regions for IAI-managed agents via SimulationManager wrapper
    regions_config = RegionsConfig(
        location=args.location,
        agent_count_dict={AgentType.car: args.number_of_vehicles},
        area_shape=(int(args.width/2), int(args.height/2)),
        map_center=map_center,
    )
    regions = simulation_manager.form_regions(regions_config)

    # Wrap ego + CARLA pedestrian agents as external_agent_data dict keyed by stable IDs
    external_agent_data = {
        existing_agent_ids[i]: IaiAgentData(
            state=existing_agent_data[i].state,
            properties=existing_agent_data[i].properties,
            recurrent=None,
        )
        for i in range(len(existing_agent_data))
    }

    # Initialize via SimulationManager (wraps large_initialize; stores IAI agents in agents_dict)
    response = simulation_manager.initialize(
        location=args.location,
        regions=regions,
        external_agent_data=external_agent_data,
        traffic_light_state_history=[traffic_lights_states],
        random_seed=seed,
    )

    # Spawn CARLA actors for ego-type (non-CARLA-driven) external agents
    ego_indices = [i for i, d in enumerate(existing_agent_data) if d.type != AgentSourceType.CARLA]
    ego_subset = assign_carla_blueprints_to_agents(world, vehicle_blueprints,
                                                    [existing_agent_data[i] for i in ego_indices])
    for j, i in enumerate(ego_indices):
        existing_agent_data[i] = ego_subset[j]

    # Spawn CARLA actors for IAI-managed (internal) agents; update SimulationManager properties
    # with actual CARLA bounding box dimensions and build iai_carla_actors dict
    iai_agent_ids = simulation_manager.get_agent_ids()
    iai_states = simulation_manager.get_states()
    iai_properties = simulation_manager.get_properties()
    iai_carla_actors = {}
    agents_to_remove = []

    for i, agent_id in enumerate(iai_agent_ids):
        blueprint = random.choice(vehicle_blueprints)
        if blueprint.has_attribute('color'):
            color = random.choice(blueprint.get_attribute('color').recommended_values)
            blueprint.set_attribute('color', color)
        actor = world.try_spawn_actor(blueprint, transform_iai_to_carla(iai_states[i]))

        if actor is not None:
            bb = actor.bounding_box.extent
            actor.set_simulate_physics(False)
            prop = iai_properties[i]
            prop.length = 2 * bb.x
            prop.width = 2 * bb.y
            prop.rear_axis_offset = 2 * bb.x / 3
            simulation_manager.set_property(agent_id, prop)
            iai_carla_actors[agent_id] = actor
        else:
            agents_to_remove.append(agent_id)

    if agents_to_remove:
        simulation_manager.remove_agents(agents_to_remove)

    if len(iai_carla_actors) == 0:
        raise Exception("No vehicles could be placed in Carla environment.")

    print(f"Number of agents initialized: {len(iai_carla_actors) + len(existing_agent_data)}")

    return simulation_manager, iai_carla_actors, carla2iai_tl, location_info_response, response

#---------
# Synchronize InvertedAI and CARLA traffic lights
#---------

# Mapping between CARLA and IAI traffic lights IDs
def get_traffic_lights_mapping(world):
    tls = world.get_actors().filter('traffic.traffic_light*')
    tl_ids = sorted([tl.id for tl in list(tls)])
    carla2iai_tl = {}
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
# Ego Simulator-in-the-Loop Integration
# EDIT THESE FUNCTIONS HERE TO INTEGRATE YOUR OWN EGO VEHICLE
# Every "pass" is where your own code should be added
#---------
def convert_ego_states_to_iai_format(
    ego_states: List
) -> List[AgentState]:
    
    return ego_states

def convert_ego_properties_to_iai_format(
    ego_properties: List
) -> List[AgentProperties]:

    return ego_properties

def initialize_ego_vehicle(
    location: str,
    scenario_log_path: Optional[str] = None
) -> Tuple[List[AgentState],List[AgentProperties],List[RecurrentState]]:
    log_reader = None
    if os.path.exists(scenario_log_path):
        log_reader = iai.LogReader(
            log_path = scenario_log_path
        )
        log_reader.initialize()

        ego_agent_states = log_reader.agent_states
        ego_agent_properties = log_reader.agent_properties
        ego_recurrent_states = [RecurrentState() for _ in range(len(ego_agent_properties))]
        traffic_lights_states = [log_reader.traffic_lights_states] if log_reader.traffic_lights_states is not None else None

    else:
        response = iai.initialize(
            location=location,
            agent_properties=iai.utils.get_default_agent_properties({AgentType.car:1}),
        )
        ego_agent_states = response.agent_states
        ego_agent_properties = response.agent_properties
        ego_recurrent_states = response.recurrent_states
        traffic_lights_states = response.traffic_lights_states

    return ego_agent_states, ego_agent_properties, ego_recurrent_states, traffic_lights_states, log_reader

def tick_ego_vehicle(
    args,
    location: str,
    num_ego_agents: int,
    agent_states: List[AgentState],
    agent_properties: List[AgentProperties],
    agent_recurrent_states: List[RecurrentState],
    traffic_lights_states: Optional[Dict[int, TrafficLightState]] = None,
    log_reader: Optional[Any] = None
) -> Tuple[List[AgentState],List[AgentProperties],List[RecurrentState]]:
    if log_reader is not None:
        is_within_log = log_reader.drive()
        updated_ego_agent_states = log_reader.agent_states
        updated_ego_agent_properties = log_reader.agent_properties
        updated_ego_recurrent_states = [RecurrentState() for _ in range(len(updated_ego_agent_properties))]
        traffic_lights_states = log_reader.traffic_lights_states if log_reader.traffic_lights_states is not None else None
    
    if log_reader is None or (log_reader is not None and not is_within_log):
        ego_response = iai.large_drive(
            location=location,
            agent_properties=agent_properties,
            agent_states=agent_states,
            recurrent_states=agent_recurrent_states,
            traffic_lights_states=traffic_lights_states,
            api_model_version = args.api_model,
            random_seed = args.seed
        )
        
        updated_ego_agent_states = convert_ego_states_to_iai_format(ego_response.agent_states[:num_ego_agents])
        updated_ego_agent_properties= convert_ego_properties_to_iai_format(agent_properties[:num_ego_agents])
        updated_ego_recurrent_states = ego_response.recurrent_states[:num_ego_agents]

    return updated_ego_agent_states, updated_ego_agent_properties, updated_ego_recurrent_states, log_reader

def wrap_angle(angle):
    ang_wrap = angle % (2*math.pi)
    if ang_wrap > math.pi:
        ang_wrap = ang_wrap - 2*math.pi
    elif ang_wrap < -math.pi:
        ang_wrap = ang_wrap + 2*math.pi

    return ang_wrap

def interpolate_state(
    state,
    state_prev,
    t_interp,
    t_total
):
    return AgentState(
        center = Point(
            x = state_prev.center.x + (state.center.x-state_prev.center.x)*(t_interp+1)/t_total,
            y = state_prev.center.y + (state.center.y-state_prev.center.y)*(t_interp+1)/t_total
        ),
        speed = state_prev.speed + (state.speed-state_prev.speed)*(t_interp+1)/t_total,
        orientation = wrap_angle(state_prev.orientation + wrap_angle(state.orientation-state_prev.orientation)*(t_interp+1)/t_total)
    )

#---------
# Main
#---------
def main():

    args = argument_parser()

    # Setup CARLA client and world
    client, world = setup_carla_environment(
        host = args.host, 
        port = args.port, 
        location = args.location,
        step_length = IAI_TIME_STEP/args.interpolation_steps
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

    args.seed = args.seed if args.seed is not None else int(time.time())
    random.seed(args.seed)
    
    vehicle_blueprints = get_actor_blueprints(
        world, 
        args.filterv, 
        args.generationv
    )
    if args.safe:
        vehicle_blueprints = [x for x in vehicle_blueprints if x.get_attribute('base_type') == 'car']   

    agent_data = []
    ego_agent_states, ego_agent_properties, ego_recurrent_states, traffic_lights_states, log_reader = initialize_ego_vehicle(
        location = args.location,
        scenario_log_path = args.scenario_path
    )
    agent_data += [AgentData(
        type = AgentSourceType.EGO,
        state = state,
        properties = props,
        recurrent_state = recurr
    ) for state, props, recurr in zip(ego_agent_states,ego_agent_properties,ego_recurrent_states)]
    num_ego_agents = len(agent_data)

    # Add Carla-driven pedestrians
    if num_pedestrians>0:
        world.set_pedestrians_seed(args.seed)
        blueprintsWalkers = get_actor_blueprints(world, args.filterw, args.generationw)
        if not blueprintsWalkers:
            raise ValueError("Couldn't find any walkers with the specified filters")
        pedestrians = spawn_pedestrians(client, world, num_pedestrians, blueprintsWalkers)
        iai_pedestrians_states, iai_pedestrians_properties = initialize_pedestrians(pedestrians)

        agent_data += [AgentData(
            type = AgentSourceType.CARLA,
            state = iai_pedestrians_states[ind],
            properties = iai_pedestrians_properties[ind],
            carla_actor = pedestrians[ind],
            recurrent_state = RecurrentState()
        ) for ind in range(len(iai_pedestrians_states))]
    
    if args.iai_log or args.capture_video:
        sim_name = str(int(time.time()))
        iai_output_dir = os.path.join(os.getcwd(), "output", sim_name)
        os.makedirs(iai_output_dir, exist_ok=True)
    if args.iai_log:
        iai_log_output_dir = os.path.join(iai_output_dir, "iai_log")
        os.mkdir(iai_log_output_dir)
        iai_output_data = os.path.join(iai_log_output_dir, f"{sim_name}_iai_log")
    else:
        iai_output_data = None

    existing_agent_ids = [str(uuid.uuid4()) for _ in range(len(agent_data))]
    simulation_manager, iai_carla_actors, carla2iai_tl, location_info_response, response = initialize_simulation(
        args=args,
        world=world,
        seed=args.seed,
        vehicle_blueprints=vehicle_blueprints,
        existing_agent_data=agent_data,
        existing_agent_ids=existing_agent_ids,
        traffic_lights_states=traffic_lights_states,
        iai_log_path=f"{iai_output_data}.json" if iai_output_data else None,
    )
    sim_agent_data = SimulationData(agent_data)
    cosim_agents = {
        existing_agent_ids[i]: IaiAgentData(
            state=agent_data[i].state,
            properties=agent_data[i].properties,
            recurrent=None,
        )
        for i in range(len(existing_agent_ids))
    }

    # Perform CARLA simulation tick to spawn vehicles
    world.tick()
    
    sensor_manager = None
    try:
        if args.capture_video:
            sensor_manager = SensorManager(
                world = world,
                spectator_id = 0,
                camera_specs = [
                    CameraSpecification(
                        attachment_cfg = get_default_cam_attachment(
                            attachment = CameraAttachment.REARPOLE,
                            actor_to_attach = sim_agent_data.all_agent_data[0].carla_actor
                        ),
                        type = CameraType.RGB,
                        fps = FPS,
                        save_path = iai_output_dir,
                        name = f"{sim_name}_carla_camera_0"
                    )
                ]
            )

            world.tick()
        for frame in tqdm(range(args.sim_length * int(1/IAI_TIME_STEP))):
            traffic_lights_states = assign_iai_traffic_lights_from_carla(world, response.traffic_lights_states, carla2iai_tl)
            iai_states_prev = simulation_manager.get_states()
            iai_agent_ids_ordered = simulation_manager.get_agent_ids()
            cosim_states_prev = {aid: data.state for aid, data in cosim_agents.items()}

            #=================================================
            #Tick IAI via SimulationManager
            # cosim_agents holds current CARLA-side states (ego + peds); drive returns updated cosim_agents
            response, cosim_agents = simulation_manager.drive(
                external_agent_data=cosim_agents,
                location=args.location,
                traffic_lights_states=traffic_lights_states,
                api_model_version=args.api_model,
                random_seed=args.seed,
                return_external_dict=True,
            )
            iai_states_new = simulation_manager.get_states()
            #=================================================
            #=================================================
            #Tick Ego
            ego_ids = existing_agent_ids[:num_ego_agents]
            ped_ids  = existing_agent_ids[num_ego_agents:]
            iai_props  = simulation_manager.get_properties()
            ped_states = [cosim_agents[pid].state      for pid in ped_ids]
            ped_props  = [cosim_agents[pid].properties for pid in ped_ids]
            recur_size = len(ego_recurrent_states[0].packed) if ego_recurrent_states else 64
            zero_recur = [RecurrentState.fromval([0.0] * recur_size)]
            ego_agent_states, ego_agent_properties, ego_recurrent_states, log_reader = tick_ego_vehicle(
                args = args,
                location = args.location,
                num_ego_agents = num_ego_agents,
                agent_states = ego_agent_states + iai_states_new + ped_states,
                agent_properties = ego_agent_properties + iai_props + ped_props,
                agent_recurrent_states = ego_recurrent_states + zero_recur * (len(iai_props) + len(ped_props)),
                traffic_lights_states = response.traffic_lights_states,
                log_reader = log_reader
            )
            # Update cosim_agents with new ego states so next drive() sees the ego tick result
            for i, aid in enumerate(ego_ids):
                cosim_agents[aid] = IaiAgentData(
                    state=ego_agent_states[i],
                    properties=ego_agent_properties[i],
                    recurrent=None,
                )
            #=================================================
            #=================================================
            #Tick Carla
            for t_interp in range(args.interpolation_steps):
                world.tick()
                time.sleep(1/FPS)

                # Update IAI CARLA actors with interpolated states
                for j, agent_id in enumerate(iai_agent_ids_ordered):
                    if agent_id in iai_carla_actors:
                        interp = interpolate_state(
                            state=iai_states_new[j],
                            state_prev=iai_states_prev[j],
                            t_interp=t_interp,
                            t_total=args.interpolation_steps,
                        )
                        try:
                            iai_carla_actors[agent_id].set_transform(transform_iai_to_carla(interp))
                        except:
                            pass

                # Update ego CARLA actors with interpolated states
                for i, d in enumerate(sim_agent_data.all_agent_data):
                    if d.type == AgentSourceType.EGO and d.carla_actor is not None:
                        interp = interpolate_state(
                            state=cosim_agents[existing_agent_ids[i]].state,
                            state_prev=cosim_states_prev[existing_agent_ids[i]],
                            t_interp=t_interp,
                            t_total=args.interpolation_steps,
                        )
                        try:
                            d.carla_actor.set_transform(transform_iai_to_carla(interp))
                        except:
                            pass

                if args.capture_video:
                    sensor_manager.update_all_sensors()

            #=================================================
            #=================================================
            # Update cosim_agents with actual CARLA positions for pedestrians
            for i, d in enumerate(sim_agent_data.all_agent_data):
                if d.type == AgentSourceType.CARLA and d.carla_actor is not None:
                    state, props = initialize_iai_agent(d.carla_actor, d.properties.agent_type)
                    cosim_agents[existing_agent_ids[i]] = IaiAgentData(
                        state=state, properties=props, recurrent=None,
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
            simulation_manager.export_log(path=f"{iai_output_data}.json")
            print(f"Generating birdview GIF.")
            simulation_manager.log_writer.visualize(
                gif_path=f"{iai_output_data}.gif",
                fov = max(args.width,args.height),
                resolution = (2048,2048),
                dpi = 300,
                direction_vec = True,
                velocity_vec = False,
                plot_frame_number = True,
                map_center = args.map_center,
                left_hand_coordinates = True,
                agent_ids = list(range(len(simulation_manager.get_agent_ids()) + len(agent_data)))
            )
    except Exception as e:
        print(f"{e}")
        print(traceback.format_exc())


if __name__ == '__main__':

    for _ in range(100):
        try:
            main()
        # except KeyboardInterrupt:
        #     pass
        finally:
            print('\ndone.')
