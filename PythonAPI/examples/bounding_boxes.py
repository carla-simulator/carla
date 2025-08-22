#!/usr/bin/env python

# Copyright (c) 2019 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

# Generates 2D and 3D bounding boxes for a simulation and can save them as JSON
# Instructions:
# Press "r" to start recording images and bounding boxes
# Press "3" to visualize bounding boxes in 3D
# Press "2" to vizualize bounding boxes in 2D

import carla
import math
import json
import random
import time
import queue
import pygame
import numpy as np
from math import radians

from pygame.locals import K_ESCAPE
from pygame.locals import K_2
from pygame.locals import K_3
from pygame.locals import K_r

# Bounding box edge topology order
EDGES = [[0,1], [1,3], [3,2], [2,0], [0,4], [4,5], [5,1], [5,7], [7,6], [6,4], [6,2], [7,3]]

# Map for CARLA semantic labels to class names and colors
SEMANTIC_MAP = {0: ('unlabelled', (0,0,0)), 1: ('road', (128,64,0)),2: ('sidewalk', (244,35,232)),
                3: ('building', (70,70,70)), 4: ('wall', (102,102,156)), 5: ('fence', (190,153,153)),
                6: ('pole', (153,153,153)), 7: ('traffic light', (250,170,30)), 
                8: ('traffic sign', (220,220,0)), 9: ('vegetation', (107,142,35)),
                10: ('terrain', (152,251,152)), 11: ('sky', (70,130,180)), 
                12: ('pedestrian', (220,20,60)), 13: ('rider', (255,0,0)), 
                14: ('car', (0,0,142)), 15: ('truck', (0,0,70)), 16: ('bus', (0,60,100)), 
                17: ('train', (0,80,100)), 18: ('motorcycle', (0,0,230)), 
                19: ('bicycle', (119,11,32)), 20: ('static', (110,190,160)), 
                21: ('dynamic', (170,120,50)), 22: ('other', (55,90,80)), 
                23: ('water', (45,60,150)), 24: ('road line', (157,234,50)), 
                25: ('ground', (81,0,81)), 26: ('bridge', (150,100,100)), 
                27: ('rail track', (230,150,140)), 28: ('guard rail', (180,165,180))}

# Calculate the camera projection matrix
def build_projection_matrix(w, h, fov, is_behind_camera=False):
    focal = w / (2.0 * np.tan(fov * np.pi / 360.0))
    K = np.identity(3)

    if is_behind_camera:
        K[0, 0] = K[1, 1] = -focal
    else:
        K[0, 0] = K[1, 1] = focal

    K[0, 2] = w / 2.0
    K[1, 2] = h / 2.0
    return K

# Calculate 2D projection of 3D coordinate
def get_image_point(loc, K, w2c):
    
    # Format the input coordinate (loc is a carla.Position object)
    point = np.array([loc.x, loc.y, loc.z, 1])
    # transform to camera coordinates
    point_camera = np.dot(w2c, point)

    # New we must change from UE4's coordinate system to an "standard"
    # (x, y ,z) -> (y, -z, x)
    # and we remove the fourth componebonent also
    point_camera = [point_camera[1], -point_camera[2], point_camera[0]]

    # now project 3D->2D using the camera matrix
    point_img = np.dot(K, point_camera)
    # normalize
    point_img[0] /= point_img[2]
    point_img[1] /= point_img[2]

    return point_img[0:2]

# Verify that the point is within the image plane
def point_in_canvas(pos, img_h, img_w):
    """Return true if point is in canvas"""
    if (pos[0] >= 0) and (pos[0] < img_w) and (pos[1] >= 0) and (pos[1] < img_h):
        return True
    return False

# Decode the instance segmentation map into semantic labels and actor IDs
def decode_instance_segmentation(img_rgba: np.ndarray):
    semantic_labels = img_rgba[..., 2]  # R channel
    actor_ids = img_rgba[..., 1].astype(np.uint16) + (img_rgba[..., 0].astype(np.uint16) << 8)
    return semantic_labels, actor_ids

# Generate a 2D bounding box for an actor from the actor ID image
def bbox_2d_for_actor(actor, actor_ids: np.ndarray, semantic_labels: np.ndarray):
    mask = (actor_ids == actor.id)
    if not np.any(mask):
        return None  # actor not present
    ys, xs = np.where(mask)
    xmin, xmax = xs.min(), xs.max()
    ymin, ymax = ys.min(), ys.max()
    return {'actor_id': actor.id,
            'semantic_label': actor.semantic_tags[0],
            'bbox_2d': (xmin, ymin, xmax, ymax)}

# Generate a 3D bounding box for an actor from the simulation
def bbox_3d_for_actor(actor, ego, camera_bp, camera):

    # Get the world to camera matrix
    world_2_camera = np.array(camera.get_transform().get_inverse_matrix())

     # Get the attributes from the camera
    image_w = camera_bp.get_attribute("image_size_x").as_int()
    image_h = camera_bp.get_attribute("image_size_y").as_int()
    fov = camera_bp.get_attribute("fov").as_float()

    # Calculate the camera projection matrix to project from 3D -> 2D
    K = build_projection_matrix(image_w, image_h, fov)
    K_b = build_projection_matrix(image_w, image_h, fov, is_behind_camera=True)

    ego_bbox_loc = ego.get_transform().location + ego.bounding_box.location
    ego_bbox_transform = carla.Transform(ego_bbox_loc, ego.get_transform().rotation)

    npc_bbox_loc = actor.get_transform().location + actor.bounding_box.location
    #npc_bbox_transform = carla.Transform(npc_bbox_loc, actor.get_transform().rotation)

    npc_loc_ego_space = ego_bbox_transform.inverse_transform(npc_bbox_loc)

    verts = [v for v in actor.bounding_box.get_world_vertices(actor.get_transform())]

    projection = []
    for edge in EDGES:
        p1 = get_image_point(verts[edge[0]], K, world_2_camera)
        p2 = get_image_point(verts[edge[1]],  K, world_2_camera)

        p1_in_canvas = point_in_canvas(p1, image_h, image_w)
        p2_in_canvas = point_in_canvas(p2, image_h, image_w)

        if not p1_in_canvas and not p2_in_canvas:
            continue

        ray0 = verts[edge[0]] - camera.get_transform().location
        ray1 = verts[edge[1]] - camera.get_transform().location
        cam_forward_vec = camera.get_transform().get_forward_vector()

        # One of the vertexes is behind the camera
        if not (cam_forward_vec.dot(ray0) > 0):
            p1 = get_image_point(verts[edge[0]], K_b, world_2_camera)
        if not (cam_forward_vec.dot(ray1) > 0):
            p2 = get_image_point(verts[edge[1]], K_b, world_2_camera)
        
        projection.append((int(p1[0]), int(p1[1]), int(p2[0]), int(p2[1])))

    return {'actor_id': actor.id,
            'semantic_label': actor.semantic_tags[0],
            'bbox_3d': {
                'center': {
                    'x': npc_loc_ego_space.x,
                    'y': npc_loc_ego_space.y,
                    'z': npc_loc_ego_space.z
                },
                'dimensions': {
                    'length': actor.bounding_box.extent.x*2,
                    'width': actor.bounding_box.extent.y*2,
                    'height': actor.bounding_box.extent.z*2,
                },
                'rotation_yaw': radians(actor.get_transform().rotation.yaw - ego.get_transform().rotation.yaw)
            },
            'projection': projection
    }

# Visualize 2D bounding boxes in Pygame
def visualize_2d_bboxes(surface, img, bboxes):

    rgb_img = img[:, :, :3][:, :, ::-1] 
    frame_surface = pygame.surfarray.make_surface(np.transpose(rgb_img[..., 0:3], (1,0,2)))
    surface.blit(frame_surface, (0, 0))

    font = pygame.font.SysFont("Arial", 18)

    for item in bboxes:
        bbox = item['2d']
        if bbox is not None:
            xmin, ymin, xmax, ymax = [int(v) for v in bbox['bbox_2d']]
            label = SEMANTIC_MAP[bbox['semantic_label']][0]
            color = SEMANTIC_MAP[bbox['semantic_label']][1]
            pygame.draw.rect(surface, color, pygame.Rect(xmin, ymin, xmax-xmin, ymax-ymin), 2)
            text_surface = font.render(label, True, (255,255,255), color) 
            text_rect = text_surface.get_rect(topleft=(xmin, ymin-20))
            surface.blit(text_surface, text_rect)

    return surface

# Visualize 3D bounding boxes in Pygame
def visualize_3d_bboxes(surface, img, bboxes):

    rgb_img = img[:, :, :3][:, :, ::-1] 
    frame_surface = pygame.surfarray.make_surface(np.transpose(rgb_img[..., 0:3], (1,0,2)))
    surface.blit(frame_surface, (0, 0))

    for item in bboxes:
        bbox = item['3d']
        color = SEMANTIC_MAP[bbox['semantic_label']][1]

        n = 0
        mean_x = 0
        mean_y = 0
        for line in bbox['projection']:
            mean_x += line[0]
            mean_y += line[1]
            n += 1
            pygame.draw.line(surface, color, (line[0], line[1]), (line[2],line[3]), 2)

        if n > 0:
            mean_x /= n
            mean_y /= n

            # --- Render label ---
            font = pygame.font.SysFont("Arial", 18)
            text_surface = font.render(SEMANTIC_MAP[bbox['semantic_label']][0], True, (255,255,255), color)  # black text, filled bg
            text_rect = text_surface.get_rect(topleft=(mean_x, mean_y))
            surface.blit(text_surface, text_rect)

def calculate_relative_velocity(actor, ego):
    # Calculate the relative velocity in world frame
    rel_vel = actor.get_velocity() - ego.get_velocity()
    # Now convert to local frame of ego
    vel_ego_frame = ego.get_transform().inverse_transform(rel_vel)

    return {
        'x': vel_ego_frame.x,
        'y': vel_ego_frame.y,
        'z': vel_ego_frame.z
    }

def vehicle_light_state_to_dict(vehicle: carla.Vehicle):
    state = vehicle.get_light_state()
    return {
        "position":     bool(state & carla.VehicleLightState.Position),
        "low_beam":     bool(state & carla.VehicleLightState.LowBeam),
        "high_beam":    bool(state & carla.VehicleLightState.HighBeam),
        "brake":        bool(state & carla.VehicleLightState.Brake),
        "reverse":      bool(state & carla.VehicleLightState.Reverse),
        "left_blinker": bool(state & carla.VehicleLightState.LeftBlinker),
        "right_blinker":bool(state & carla.VehicleLightState.RightBlinker),
        "fog":          bool(state & carla.VehicleLightState.Fog),
        "interior":     bool(state & carla.VehicleLightState.Interior),
        "special1":     bool(state & carla.VehicleLightState.Special1),
        "special2":     bool(state & carla.VehicleLightState.Special2),
    }

def main():

    pygame.init()

    # State variables
    record = False
    display_3d = False
    run_simulation = True

    clock = pygame.time.Clock()
    pygame.display.set_caption("Bounding Box Visualization")
    display = pygame.display.set_mode(
            (1280, 720),
            pygame.HWSURFACE | pygame.DOUBLEBUF)
    display.fill((0,0,0))
    pygame.display.flip()

    # Connect to the CARLA server and get the world object
    client = carla.Client('localhost', 2000)
    world  = client.get_world()

    # Set up the simulator in synchronous mode
    settings = world.get_settings()
    settings.synchronous_mode = True # Enables synchronous mode
    settings.fixed_delta_seconds = 0.05
    world.apply_settings(settings)

    # Set the traffic manager to Synchronous mode
    traffic_manager = client.get_trafficmanager()
    traffic_manager.set_synchronous_mode(True)

    bp_lib = world.get_blueprint_library()

    # Get the map spawn points
    spawn_points = world.get_map().get_spawn_points()

    # spawn vehicle
    vehicle_bp =bp_lib.find('vehicle.lincoln.mkz_2020')
    ego_vehicle = world.try_spawn_actor(vehicle_bp, random.choice(spawn_points))

    # spawn RGB camera
    camera_bp = bp_lib.find('sensor.camera.rgb')
    camera_bp.set_attribute('image_size_x', '1280')
    camera_bp.set_attribute('image_size_y', '720')
    camera_init_trans = carla.Transform(carla.Location(z=2))
    camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=ego_vehicle)

    # spawn instance segmentation camera
    inst_camera_bp = bp_lib.find('sensor.camera.instance_segmentation')
    inst_camera_bp.set_attribute('image_size_x', '1280')
    inst_camera_bp.set_attribute('image_size_y', '720')
    camera_init_trans = carla.Transform(carla.Location(z=2))
    inst_camera = world.spawn_actor(inst_camera_bp, camera_init_trans, attach_to=ego_vehicle)

    ego_vehicle.set_autopilot(True)

    # Add some traffic
    npcs = []
    for i in range(100):
        vehicle_bp = random.choice(bp_lib.filter('vehicle'))
        npc = world.try_spawn_actor(vehicle_bp, random.choice(spawn_points))
        if npc:
            npc.set_autopilot(True)
            npcs.append(npc)

    # Create queues to store and retrieve the sensor data
    image_queue = queue.Queue()
    camera.listen(image_queue.put)

    inst_queue = queue.Queue()
    inst_camera.listen(inst_queue.put)

    try:
        while run_simulation:
            for event in pygame.event.get():
                if event.type == pygame.KEYUP:
                    if event.key == K_r:
                        record = True
                    if event.key == K_2:
                        display_3d = False
                    if event.key == K_3:
                        display_3d = True
                    if event.key == K_ESCAPE:
                        run_simulation = False
                if event.type == pygame.QUIT:
                    run_simulation = False

            world.tick()
            snapshot = world.get_snapshot()

            json_frame_data = {
                'frame_id': snapshot.frame,
                'timestamp': snapshot.timestamp.elapsed_seconds,
                'objects': [] 
            }

            image = image_queue.get()
            img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))

            if record:
                image.save_to_disk('_out/%08d' % image.frame)

            inst_seg_image = inst_queue.get()
            inst_seg = np.reshape(np.copy(inst_seg_image.raw_data), (inst_seg_image.height, inst_seg_image.width, 4))

            # Empty list to collect bounding boxes for this frame
            frame_bboxes = []

            # Loop through the NPCs in the simulation
            for npc in world.get_actors().filter('*vehicle*'):

                # Filter out the ego vehicle
                if npc.id !=ego_vehicle.id:

                    npc_bbox = npc.bounding_box
                    dist = npc.get_transform().location.distance(ego_vehicle.get_transform().location)

                    # Filter for the vehicles within 100m
                    if dist < 50:

                        # Limit to vehicles in front of the camera
                        forward_vec = camera.get_transform().get_forward_vector()
                        inter_vehicle_vec = npc.get_transform().location - camera.get_transform().location

                        if forward_vec.dot(inter_vehicle_vec) > 0:
                            
                            # Generate 2D and 2D bounding boxes for each actor
                            semantic_labels, actor_ids = decode_instance_segmentation(inst_seg)
                            npc_bbox_2d = bbox_2d_for_actor(npc, actor_ids, semantic_labels)
                            npc_bbox_3d = bbox_3d_for_actor(npc, ego_vehicle, camera_bp, camera)

                            frame_bboxes.append({'3d': npc_bbox_3d, '2d': npc_bbox_2d})

                            json_frame_data['objects'].append({
                                'id': npc.id,
                                'class': SEMANTIC_MAP[npc.semantic_tags[0]][0],
                                'blueprint_id': npc.type_id,
                                'velocity': calculate_relative_velocity(npc, ego_vehicle),
                                'bbox_3d': npc_bbox_3d['bbox_3d'],
                                'bbox_2d': {
                                    'xmin': int(npc_bbox_2d['bbox_2d'][0]),
                                    'ymin': int(npc_bbox_2d['bbox_2d'][1]),
                                    'xmax': int(npc_bbox_2d['bbox_2d'][2]),
                                    'ymax': int(npc_bbox_2d['bbox_2d'][3]),
                                } if npc_bbox_2d else None,
                                'light_state': vehicle_light_state_to_dict(npc)

                            })

            # Draw the scene in Pygame
            display.fill((0,0,0))
            if display_3d:
                visualize_3d_bboxes(display, img, frame_bboxes)
            else:
                visualize_2d_bboxes(display, img, frame_bboxes)
            pygame.display.flip()
            clock.tick(30)  # 30 FPS              
            if record:
                with open(f"_out/{snapshot.frame}.json", 'w') as f:
                    json.dump(json_frame_data, f)

    except KeyboardInterrupt:
        pass
    finally:
        
        ego_vehicle.destroy()
        camera.stop()
        camera.destroy()
        inst_camera.stop()
        inst_camera.destroy()
        for npc in npcs:
            npc.set_autopilot(False)
            npc.destroy()

        world.tick()

        # Set up the simulator in synchronous mode
        settings = world.get_settings()
        settings.synchronous_mode = False # Disables synchronous mode
        settings.fixed_delta_seconds = None
        world.apply_settings(settings)

        # Set the traffic manager to Synchronous mode
        traffic_manager.set_synchronous_mode(False)

        pygame.quit()

        print('\ndone.')


if __name__ == '__main__':
    print('Bounding boxes script instructions:')
    print('Press "r" to start recording images as PNG and bounding boxes as JSON')
    print('Press "3" to see the bounding boxes in 3D')
    print('Press "2" to see the bounding boxes in 2D')
    main()
