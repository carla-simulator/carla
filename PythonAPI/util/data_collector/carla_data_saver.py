#!/usr/bin/env python
#
# Copyright (c) 2022 Intel Corporation.
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.


from __future__ import print_function
from config_schema import ConfigSchema
from omegaconf import DictConfig, OmegaConf
import hydra
import carla
import numpy as np
import random
from queue import Queue
import time
import os
import sys
import json
import logging
import coloredlogs
import fnmatch
from packaging import version

logger = logging.getLogger(__name__)
coloredlogs.install(level=logging.INFO)


class SensorDataCV():
    def __init__(self, actor_id, data):
        self.data = data
        self.actor_id = actor_id


class CarlaSyncMode(object):
    """
    Context manager to synchronize output from different sensors. Synchronous
    mode is enabled as long as we are inside this context
        with CarlaSyncMode(world, sensors) as sync_mode:
            while True:
                data = sync_mode.tick(timeout=1.0)
    """

    def __init__(self, client, *sensors, tm_port=8000, fps=30, respawn=True):
        self.world = client.get_world()
        self.sensors = sensors
        logger.info(f"Init sensors: {self.sensors}, kwargs: {fps}")
        self.frame = None
        self.delta_seconds = 1.0 / fps
        self._queues = []
        self._settings = None
        self.traffic_manager = client.get_trafficmanager(tm_port)
        self.traffic_manager.set_respawn_dormant_vehicles(respawn)

    def __enter__(self):
        self._settings = self.world.get_settings()
        self.frame = self.world.apply_settings(carla.WorldSettings(
            no_rendering_mode=False,
            synchronous_mode=True,
            fixed_delta_seconds=self.delta_seconds))
        self.traffic_manager.set_synchronous_mode(True)
        time.sleep(1)

        def make_queue(register_event, actor_id=None):
            q = Queue()

            def queue_put(x):
                q.put(SensorDataCV(actor_id, x))
            register_event(queue_put)
            self._queues.append(q)

        make_queue(self.world.on_tick)
        for sensor in self.sensors:
            logger.info(f"Making queue for sensor: {sensor}")
            make_queue(sensor.listen, actor_id=sensor.id)
        return self

    def tick(self, timeout):
        self.frame = self.world.tick()
        data = [self._retrieve_data(q, timeout) for q in self._queues]
        assert all(x.frame == self.frame for actor_id, x in data)
        return data

    def __exit__(self, *args, **kwargs):
        self.world.apply_settings(self._settings)
        self.traffic_manager.set_synchronous_mode(False)

    def _retrieve_data(self, sensor_queue, timeout):
        while True:
            x = sensor_queue.get(timeout=timeout)
            if x.data.frame == self.frame:
                return x.actor_id, x.data


def conf_to_carla_transform(conf):
    loc, rot = conf.get("location",{'x': 0.0, 'y': 0.0, 'z': 0.0}), conf.get("rotation", {'roll': 0.0, 'pitch': 0.0, 'yaw': 0.0})
    location = carla.Location(x=loc.get('x', 0.0), y=loc.get('y', 0.0), z=loc.get('z', 0.0))
    rotation = carla.Rotation(roll=rot.get('roll', 0.0), pitch=rot.get('pitch', 0.0), yaw=rot.get('yaw', 0.0))
    transform = carla.Transform(location, rotation)

    return transform


def spawn_actors(conf, world, parent=None):
    actors = {} #{id: (actor, should_destroy)}

    for actor_conf in conf:
        # Pass parent because...
        actor_list = conf_to_actor(actor_conf, world, parent=parent)
        logger.info(f"Spawned actor: {actor_list[0].type_id} with id: {actor_list[0].id}")
        
        for actor in actor_list:
            # Save actor to dictionary
            # For pedestrians, the ai walker controller is returned as a tuple (walker_controller, destination_transform)
            if type(actor) is tuple and fnmatch.fnmatch(actor[0].type_id, "controller.ai.walker"):
                actors[actor[0].id] = (actor[0], True, actor[1])
                derived_parent = actor[0].parent
            else:
                actors[actor.id] = (actor, True)
                derived_parent = actor.parent
            if derived_parent is not None and (fnmatch.fnmatch(derived_parent.type_id, "vehicle.*") or fnmatch.fnmatch(derived_parent.type_id, "walker.pedestrian.*")):
                # If actor has a parent and it's parent is not the default parent, then it was spawned by another script and we should not destroy this actor
                actors[derived_parent.id] = (derived_parent, derived_parent == parent)
                logger.info(f"Found actor : {derived_parent.type_id} with id: {derived_parent.id} to enable autopilot")

        attached_actors = spawn_actors(actor_conf.get("sensors", []), world, parent=actor)
        actors.update(attached_actors)

    return actors


def conf_to_actor(conf, world, parent=None):
    blueprint_library = world.get_blueprint_library()
    blueprint = conf_to_blueprint(conf.blueprint, blueprint_library)
    if "transform" not in conf:
        transform = random.choice(world.get_map().get_spawn_points())
        logger.info(
            f"Selected random spawn point (none provided) x: {transform.location.x}, y: {transform.location.y}, z: {transform.location.z} \n \
                roll: {transform.rotation.roll}, pitch: {transform.rotation.pitch}, yaw: {transform.rotation.yaw}")
    else:
        transform = conf_to_carla_transform(conf.transform)
    attach_to = conf_to_attach_to(conf, world, default_attach_to=parent)
    attachment = conf_to_attachment(conf)
    logger.info("Config: "+str(conf))
    if "destination_transform" not in conf:
        dest_transform = world.get_random_location_from_navigation()
        logger.info(
            f"Selected random destination point (none provided) x: {dest_transform.x}, y: {dest_transform.y}, z: {dest_transform.z} \n")
    else:
        dest_transform = conf_to_carla_transform(conf.destination_transform).location

    actor = world.spawn_actor(blueprint, transform, attach_to=attach_to, attachment_type=attachment)
    # Spawn a walker controller to control the walker
    if fnmatch.fnmatch(actor.type_id, "walker.pedestrian.*"):
        walker_controller_bp = world.get_blueprint_library().find('controller.ai.walker')
        walker_controller = world.spawn_actor(walker_controller_bp, carla.Transform(), attach_to=actor)
        if conf.blueprint.get("pace") == "run":
            print("Speed run: ", blueprint.get_attribute('speed').recommended_values[2])
            print("Speed walk: ", blueprint.get_attribute('speed').recommended_values[1])
            walker_controller.set_max_speed(float(blueprint.get_attribute('speed').recommended_values[2]))
        else:
            walker_controller.set_max_speed(float(blueprint.get_attribute('speed').recommended_values[1]))
        
        return [actor, (walker_controller, dest_transform)]
    
    return [actor]

def conf_to_blueprint(conf, library):
    blueprints = library.filter(conf.name)
    blueprint = random.choice(blueprints)
    logger.info("Selected %s blueprint from: %s", blueprint, blueprints)
    for name, value in conf.get("attr", {}).items():
        if blueprint.has_attribute(name):
            blueprint.set_attribute(name, value)

    return blueprint


def conf_to_attachment(conf):
    # TODO: Would be nice if we could iterate through carla.AttachmenType and compare
    #       stringified version of the enumerations so this turns into a loop that is future proof.
    if conf.get("attachment", "rigid") == "rigid":
        return carla.AttachmentType.Rigid
    elif conf.get("attachment") == "spring_arm":
        return carla.AttachmentType.SpringArm
    else:
        raise ValueError("Invalid attachment type in config")


def conf_to_attach_to(conf, world, default_attach_to=None):
    # NOTE: attach_to will always override!
    # attach_to is present, and parent is none
    attach_to = conf.get("attach_to", default_attach_to)

    # Treat attach_to as a role name, so find that actor in the world
    if isinstance(attach_to, str):
        attach_to = wait_for_actor_with_attribute_to_spawn(world, "role_name", attach_to)

    return attach_to


def wait_for_actor_with_attribute_to_spawn(world, attribute_name, attribute_value):
    logger.info("Waiting for actor with role_name %s to spawn...", attribute_value)

    actor = None

    while actor is None:
        # Filter actors in world by attribute_name with attribute_value
        actors = world.get_actors()
        actors = list(filter(lambda actor: actor.attributes.get(attribute_name) == attribute_value, actors))

        # If we found any actors with matching attribute, choose a random one
        if len(actors) > 0:
            actor = random.choice(actors)
            logger.info("Selected %s actor from: %s", actor, actors)
            break

        # Wait until we search for actors again
        time.sleep(1)

    return actor


def actors_still_alive(snapshot, actor_dict):
    actors_spawned = actor_dict.keys()
    actors_alive = [id for id in actors_spawned if snapshot.has_actor(id)]
    logger.debug(f"Number of vehicles alive: {len(actors_alive)}")
    logger.debug(f"Number of vehicles spawned: {len(actors_spawned)}")
    if len(actors_alive) == len(actors_spawned):
        return True

    dead_actors = [actor_dict[id] for id in set(actors_spawned) - set(actors_alive)]
    for actor in dead_actors:
        logger.info(f"Actor {actor.type_id} with ID {actor.id} is dead.")

    return False


def save_sensor_static_metadata(sensors):
    sensors_metadata = {}
    for sensor in sensors:
        if 'camera' in sensor.type_id:
            camera_data = {}
            camera_data['type'] = sensor.type_id
            camera_data['fov'] = sensor.attributes['fov']
            sensors_metadata[sensor.id] = camera_data
            
    return sensors_metadata


def save_sensor_dynamic_metadata(sensor_name, sensorsnapshot, sensor):
    sensor_metadata = {}
    sensor_metadata['type'] = sensor_name.rsplit('.', 1)[0]
    # Note that there is a small discrepancy between 'Actor' data and 'Actorsnapshot', so here we save both in case postprocessing needs any of them.
    snapshot_transform = sensorsnapshot.get_transform()
    sensor_metadata['actorsnapshot_location'] = (snapshot_transform.location.x, snapshot_transform.location.y, snapshot_transform.location.z)
    sensor_metadata['actorsnapshot_rotation'] = (snapshot_transform.rotation.pitch, snapshot_transform.rotation.yaw, snapshot_transform.rotation.roll)
    sensor_metadata['actorsnapshot_transform_get_matrix'] = snapshot_transform.get_matrix()
    sensor_metadata['actorsnapshot_transform_get_inverse_matrix'] = snapshot_transform.get_inverse_matrix()

    sensor_transform = sensor.get_transform()
    sensor_metadata['actor_location'] = (sensor_transform.location.x, sensor_transform.location.y, sensor_transform.location.z)
    sensor_metadata['actor_rotation'] = (sensor_transform.rotation.pitch, sensor_transform.rotation.yaw, sensor_transform.rotation.roll)
    sensor_metadata['actor_transform_get_matrix'] = sensor_transform.get_matrix()
    sensor_metadata['actor_transform_get_inverse_matrix'] = sensor_transform.get_inverse_matrix()

    return sensor_metadata


def save_actors_static_metadata(actors):
    actors_data = {}
    for actor in actors:
        actor_type = actor.type_id.split('.')[0]
        if actor_type not in ['vehicle', 'traffic', 'walker']:
            continue

        actor_data = {}
        actor_data['type'] = actor.type_id
        bbox = actor.bounding_box
        actor_data['3D_BoundingBox'] = {}
        actor_data['3D_BoundingBox']['location'] = (bbox.location.x, bbox.location.y, bbox.location.z)
        actor_data['3D_BoundingBox']['rotation'] = (bbox.rotation.pitch, bbox.rotation.yaw, bbox.rotation.roll)
        actor_data['3D_BoundingBox']['extent'] = (bbox.extent.x, bbox.extent.y, bbox.extent.z)
        actors_data[actor.id] = actor_data

    return actors_data


def save_actors_dynamic_metadata(actors, worldsnapshot, world):
    actors_data = {}
    for id, actor in actors.items():
        actorsnapshot = worldsnapshot.find(id)
        if actorsnapshot is None:
            continue

        actor_data = {}
        actor_data['type'] = actor['type']
        actorsnapshot_transform = actorsnapshot.get_transform()
        actor_data['actorsnapshot_location'] = (actorsnapshot_transform.location.x, actorsnapshot_transform.location.y, actorsnapshot_transform.location.z)
        actor_data['actorsnapshot_rotation'] = (actorsnapshot_transform.rotation.pitch, actorsnapshot_transform.rotation.yaw, actorsnapshot_transform.rotation.roll)
        actor_data['actorsnapshot_transform_get_matrix'] = actorsnapshot_transform.get_matrix()
        actor_data['actorsnapshot_transform_get_inverse_matrix'] = actorsnapshot_transform.get_inverse_matrix()

        # "get_bones" method is only supported by "Actor", but not "ActorSnapshot". However, there is a small delta between the data from "Actor" and "ActorSnapshot". So here transform from "Actor" is also saved for "walker" in case correction is needed in postprocessing.
        actor_transform = world.get_actor(id).get_transform()
        actor_data['actor_location'] = (actor_transform.location.x, actor_transform.location.y, actor_transform.location.z)
        actor_data['actor_rotation'] = (actor_transform.rotation.pitch, actor_transform.rotation.yaw, actor_transform.rotation.roll)
        actor_data['actor_transform_get_matrix'] = actor_transform.get_matrix()
        actor_data['actor_transform_get_inverse_matrix'] = actor_transform.get_inverse_matrix()

        if 'walker' in actor['type']:
            bones = world.get_actor(id).get_bones().bone_transforms
            actor_data['bones'] = []
            for bone in bones:
                bone_data = {}
                bone_data['name'] = bone.name
                bone_data['world'] = {}
                bone_data['world']['location'] = (bone.world.location.x, bone.world.location.y, bone.world.location.z)
                bone_data['world']['rotation'] = (bone.world.rotation.pitch, bone.world.rotation.yaw, bone.world.rotation.roll)
                bone_data['component'] = {}
                bone_data['component']['location'] = (bone.component.location.x, bone.component.location.y, bone.component.location.z)
                bone_data['component']['rotation'] = (bone.component.rotation.pitch, bone.component.rotation.yaw, bone.component.rotation.roll)
                bone_data['relative'] = {}
                bone_data['relative']['location'] = (bone.relative.location.x, bone.relative.location.y, bone.relative.location.z)
                bone_data['relative']['rotation'] = (bone.relative.rotation.pitch, bone.relative.rotation.yaw, bone.relative.rotation.roll)
                actor_data['bones'].append(bone_data)

        actors_data[id] = actor_data

    return actors_data

# ==============================================================================
# -- data_saver_loop() ---------------------------------------------------------------
# ==============================================================================

def data_saver_loop(conf):
    world = None
    if conf.carla.get("seed"):
        random.seed(conf.carla.seed)

    # Try connecting 10 times
    client = None
    for i in range(10):
        try:
            client = carla.Client(conf.carla.host, conf.carla.port)
            client.set_timeout(conf.carla.timeout)

            logger.info(f"CARLA server version: {client.get_server_version()}")
            logger.info(f"CARLA client version: {client.get_client_version()}")

            client_version = client.get_client_version().split('-')[0]
            server_version = client.get_server_version().split('-')[0]
            assert client_version == server_version, "CARLA server and client should have same version"
            assert version.parse(client_version) >= version.parse('0.9.13'), "CARLA version needs be >= '0.9.13'"

            if conf.carla.get("townmap"):
                logger.info(f"Loading map {conf.carla.townmap}.")
                world = client.load_world(conf.carla.townmap)
            world = client.get_world()

        except RuntimeError:
            client = None
            logger.info("CARLA connection failed on attempt {i+1} of 10")
            time.sleep(5)

    max_frames = conf.get("max_frames", sys.maxsize)
    tm_port = conf.carla.get("traffic_manager_port", 8000)
    fps = conf.carla.get("fps", 30)
    respawn = conf.carla.get("respawn", True)

    logger.info("Setting weather.")
    weather = carla.WeatherParameters(cloudiness=conf.weather.cloudiness, precipitation=conf.weather.precipitation, \
            precipitation_deposits=conf.weather.precipitation_deposits, wind_intensity=conf.weather.wind_intensity, \
            sun_azimuth_angle=conf.weather.sun_azimuth_angle, sun_altitude_angle=conf.weather.sun_altitude_angle, \
            fog_density=conf.weather.fog_density, fog_distance=conf.weather.fog_distance, wetness=conf.weather.wetness)
    world.set_weather(weather)

    actor_dict = {}
    try:
        actor_dict = spawn_actors(conf.spawn_actors, world)
        # actor_dict: {actor_id: (actor, should_destroy)}
        for actor_tuple in actor_dict.values():
            # Skip non-vehicle actors
            #if not fnmatch.fnmatch(actor.type_id, "vehicle.*"):
            #    continue
            actor = actor_tuple[0]
            if fnmatch.fnmatch(actor.type_id, "controller.ai.walker"):
                actor.start()
                destination = actor_tuple[2]
                actor.go_to_location(destination)
            if fnmatch.fnmatch(actor.type_id, "vehicle.*"):
                actor.set_autopilot(True, tm_port)
                actor.set_light_state(carla.VehicleLightState.NONE)

        sensor_list = list(map(lambda x: x[0], filter(lambda a: fnmatch.fnmatch(a[0].type_id, "sensor.*"), actor_dict.values())))
        sensor_id_to_sensor_type =  {actor.id: actor.type_id for actor in sensor_list}

        metadata_path = os.path.join(conf.output_dir, 'metadata')
        if not os.path.exists(metadata_path):
            os.makedirs(metadata_path)

        # Save metadata that does not change during the simulation
        metadata_static = {}
        metadata_static['sensors'] = save_sensor_static_metadata(sensor_list)
        actors = world.get_actors()
        metadata_static['actors'] = save_actors_static_metadata(actors)

        metadata_static_file = os.path.join(metadata_path, 'metadata_static.json')
        with open(metadata_static_file, 'w') as f:
            json.dump(metadata_static, f, indent=4)

        local_frame_num = 0
        # Create a synchronous mode context.
        with CarlaSyncMode(client, *sensor_list, tm_port=tm_port, fps=fps, respawn=respawn) as sync_mode:
            while True:
                local_frame_num += 1
                logging.info("Begin sync_mode.tick")

                if local_frame_num > max_frames:
                    logger.info(f"Max frames, {max_frames}, reached. Exiting...")
                    break
                # Advance the simulation and wait for the data.
                (_, snapshot), *sensor_data = sync_mode.tick(conf.carla.sync.get("timeout", 2.0))

                # Break if any vehicle in our actor list has disappeared
                if not actors_still_alive(snapshot, actor_dict):
                    logger.info("Actor(s) dead")
                    break
                logger.info("Snapshot: %s and length of sensor_data: %d", snapshot, len(sensor_data))

                # Save metadata for postprocessing
                metadata_dynamic = {}
                metadata_dynamic['frame_id'] = local_frame_num
                metadata_dynamic['timestamp'] = snapshot.timestamp.elapsed_seconds
                metadata_dynamic['actors'] = save_actors_dynamic_metadata(metadata_static['actors'], snapshot, world)
                metadata_dynamic['sensors'] = {}

                for sensor_id, data in sensor_data:
                    sensor = snapshot.find(sensor_id)
                    sensor_type = sensor_id_to_sensor_type[sensor_id]
                    sensor_name = f"{sensor_type}.{sensor.id}"
                    path = os.path.join(conf.output_dir, sensor_name, f"{local_frame_num:08}")

                    metadata_dynamic['sensors'][sensor_id] = save_sensor_dynamic_metadata(sensor_name, sensor, world.get_actor(sensor_id))

                    # CARLA's default file format for saving LIDAR data is ply, which can take a lot of disk space
                    data.save_to_disk(path)

                metadata_dynamic_file = f"{metadata_path}/{local_frame_num:08}.json"
                with open(metadata_dynamic_file, 'w') as f:
                    json.dump(metadata_dynamic, f, indent=4)

    except KeyboardInterrupt:
        print('\nCancelled by user. Bye!')
    except Exception as error:
        logger.exception(error)
    finally:
        logger.info("Destroying actors")
        for actor in actor_dict.values():
            should_destroy = actor[1]
            if not should_destroy:
                continue
            actor[0].destroy()

        print('done.')


# ==============================================================================
# -- main() --------------------------------------------------------------------
# ==============================================================================

@hydra.main(config_path="configs", config_name="config")
def main(conf: DictConfig):
    schema = OmegaConf.structured(ConfigSchema)
    OmegaConf.merge(schema, conf)
    logger.info('listening to server %s:%s', conf.carla.host, conf.carla.port)

    logger.info(OmegaConf.to_yaml(conf))

    data_saver_loop(conf)


if __name__ == '__main__':

    main()
