import glob
import os
import sys

try:
    sys.path.append(glob.glob('../carla/dist/carla-*%d.%d-%s.egg' % (
        sys.version_info.major,
        sys.version_info.minor,
        'win-amd64' if os.name == 'nt' else 'linux-x86_64'))[0])
except IndexError:
    pass


# ==============================================================================
# -- imports -------------------------------------------------------------------
# ==============================================================================

import carla

import weakref
import random
import math
import time
import queue
import cv2
import os
import json

try:
    import pygame
    from pygame.locals import K_ESCAPE
    from pygame.locals import K_SPACE
    from pygame.locals import K_a
    from pygame.locals import K_d
    from pygame.locals import K_s
    from pygame.locals import K_w
except ImportError:
    raise RuntimeError('cannot import pygame, make sure pygame package is installed')

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')

VIEW_WIDTH = 1920//2
VIEW_HEIGHT = 1080//2
VIEW_FOV = 90

BB_COLOR = (248, 64, 24)

# ==============================================================================
# -- functions -----------------------------------------------------------------
# ==============================================================================
def get_camera_matrix(w, h, fov):
    """
    Calculate projection matrix of the camera (intrinsics).

    Input:
        w: width of image (px)
        h: height of image (px)
        fov: field of view (deg)
    Output:
        K: 3x3 camera matrix
    """
    focal = w / (2.0 * np.tan(fov * np.pi / 360.0))
    K = np.identity(3)
    K[0, 0] = K[1, 1] = focal
    K[0, 2] = w / 2.0
    K[1, 2] = h / 2.0
    return K

def get_image_point(loc, K, w2c):
    """
    Calculate 2D projection of 3D coordinate.

    Input:
        loc: 3D coordinate of object (carla.Position object)
        K: camera projection matrix
        w2c: transformation matrix from point in worl to point in camera coord. system
    Output:
        2D point in image. 
    """

    # Format the input coordinate
    point = np.array([loc.x, loc.y, loc.z, 1])
    # transform to camera coordinates
    point_camera = np.dot(w2c, point)

    # New we must change from UE4's coordinate system to an "standard"
    # (x, y ,z) -> (y, -z, x)
    # and we remove the fourth component also
    point_camera = [point_camera[1], -point_camera[2], point_camera[0]]

    # now project 3D->2D using the camera matrix
    point_img = np.dot(K, point_camera)
    # normalize
    point_img[0] /= point_img[2]
    point_img[1] /= point_img[2]

    return point_img[0:2]


def render_bounding_boxes(bb, ego_vehicle, w2c, img, actor=None, dim="3D"):
    edges = [[0,1], [1,3], [3,2], [2,0], [0,4], [4,5], [5,1], [5,7], [7,6], [6,4], [6,2], [7,3]]
    bbox_params = []

    if actor:
        dist = actor.get_transform().location.distance(ego_vehicle.get_transform().location)
    else:
        dist = bb.location.distance(ego_vehicle.get_transform().location)

    # Filter for the vehicles within 50m
    if dist < 50:

    # Calculate the dot product between the forward vector
    # of the vehicle and the vector between the vehicle
    # and the other vehicle. We threshold this dot product
    # to limit to drawing bounding boxes IN FRONT OF THE CAMERA
        forward_vec = ego_vehicle.get_transform().get_forward_vector()
        if actor:
            ray = actor.get_transform().location - ego_vehicle.get_transform().location
        else:
            ray = bb.location - ego_vehicle.get_transform().location

        if forward_vec.dot(ray) > 1:
            p1 = get_image_point(bb.location, K, w2c)
            if actor is not None:
                verts = [v for v in bb.get_world_vertices(actor.get_transform())]
            else:
                verts = [v for v in bb.get_world_vertices(carla.Transform())]

            if dim == "3D":
                for edge in edges:
                    p1 = get_image_point(verts[edge[0]], K, world_2_camera)
                    p2 = get_image_point(verts[edge[1]],  K, world_2_camera)
                    img = cv2.line(img, (int(p1[0]),int(p1[1])), (int(p2[0]),int(p2[1])), (0,0,255, 255), 1)
            elif dim == "2D":
                x_max = -10000
                x_min = 10000
                y_max = -10000
                y_min = 10000

                for vert in verts:
                    p = get_image_point(vert, K, world_2_camera)
                    # Find the rightmost vertex
                    if p[0] > x_max:
                        x_max = p[0]
                    # Find the leftmost vertex
                    if p[0] < x_min:
                        x_min = p[0]
                    # Find the highest vertex
                    if p[1] > y_max:
                        y_max = p[1]
                    # Find the lowest  vertex
                    if p[1] < y_min:
                        y_min = p[1]

                # make sure bb is in image!
                if x_min > 0 and x_max < image_w and y_min > 0 and y_max < image_h:
                    bbox_params = [x_min, y_min, x_max-x_min, y_max-y_min]

                # img = cv2.line(img, (int(x_min),int(y_min)), (int(x_max),int(y_min)), (0,0,255, 255), 1)
                # img = cv2.line(img, (int(x_min),int(y_max)), (int(x_max),int(y_max)), (0,0,255, 255), 1)
                # img = cv2.line(img, (int(x_min),int(y_min)), (int(x_min),int(y_max)), (0,0,255, 255), 1)
                # img = cv2.line(img, (int(x_max),int(y_min)), (int(x_max),int(y_max)), (0,0,255, 255), 1)
            
            elif type(dim) == str:
                ValueError("Invalid argument value for parameter 'dim', can only be '2D' or '3D'!")
            else:
                TypeError
            
    return img, bbox_params


# ==============================================================================
# -- prepare client ------------------------------------------------------------
# ==============================================================================
client = carla.Client('localhost', 2000)
world  = client.get_world()
bp_lib = world.get_blueprint_library()

# get spawn points
spawn_points = world.get_map().get_spawn_points()

ego_vehicle_list = []
# spawn vehicle
vehicle_bp =bp_lib.find('vehicle.lincoln.mkz_2020')
vehicle = world.try_spawn_actor(vehicle_bp, random.choice(spawn_points))
if vehicle:
    ego_vehicle_list.append(vehicle)

# spawn camera
camera_bp = bp_lib.find('sensor.camera.rgb')
camera_init_trans = carla.Transform(carla.Location(x=1.5, z=2.4))
camera = world.spawn_actor(camera_bp, camera_init_trans, attach_to=vehicle)
ego_vehicle_list.append(camera)
vehicle.set_autopilot(True)

# Set up the simulator in synchronous mode
settings = world.get_settings()
settings.synchronous_mode = True # Enables synchronous mode
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

# Create a queue to store and retrieve the sensor data
image_queue = queue.Queue()
camera.listen(image_queue.put)

# Get the world to camera transformation matrix
world_2_camera = np.array(camera.get_transform().get_inverse_matrix())

# Get the attributes from the camera
image_w = camera_bp.get_attribute("image_size_x").as_int()
image_h = camera_bp.get_attribute("image_size_y").as_int()
fov = camera_bp.get_attribute("fov").as_float()

# Calculate the camera projection matrix to project from 3D -> 2D
K = get_camera_matrix(image_w, image_h, fov)

edges = [[0,1], [1,3], [3,2], [2,0], [0,4], [4,5], [5,1], [5,7], [7,6], [6,4], [6,2], [7,3]]

vehicle_list = []
for i in range(50):
    vehicle_bp = random.choice(bp_lib.filter('vehicle'))
    npc = world.try_spawn_actor(vehicle_bp, random.choice(spawn_points))
    if npc:
        vehicle_list.append(npc)
        npc.set_autopilot(True)

ground_truth_annotations = {
    "info": {},

    "licenses": {},

    "images": [],

    "categories": [
        {"supercategory": "vehicle", "id": 1, "name": "car" },
        {"supercategory": "vehicle", "id": 2, "name": "truck" },
        {"supercategory": "vehicle", "id": 3, "name": "motorcycle" },
        {"supercategory": "vehicle", "id": 4, "name": "bicycle" },
        {"supercategory": "vehicle", "id": 5, "name": "bus" },
        {"supercategory": "vehicle", "id": 6, "name": "rider" },
        {"supercategory": "vehicle", "id": 7, "name": "train" },
    ],

    "annotations": []
}

frame_number = 0

# Retrieve the first image
world.tick()
image = image_queue.get()

# Reshape the raw data into an RGB array
img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4)) 

# Display the image in an OpenCV display window
cv2.namedWindow('BoundingBoxes', cv2.WINDOW_AUTOSIZE)
cv2.imshow('BoundingBoxes',img)
cv2.waitKey(1)

try:
# main loop
    while True:
        world.tick()
        frame_number += 1

        # Retrieve and reshape the image
        image = image_queue.get()
        img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))

        # Save frame to annotations json
        frame_file = "{:05d}.png".format(frame_number)

        ground_truth_annotations["images"].append({
        "file_name": frame_file,
        "height": image.height,
        "width": image.width,
        "id": frame_number})

        # get bounding boxes of all types of vehicles
        all_vehicles_bbs = []
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Car)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Truck)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Motorcycle)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Bicycle)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Bus)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Rider)))
        all_vehicles_bbs.append(list(world.get_level_bbs(carla.CityObjectLabel.Train)))
        

        # Get the camera matrix 
        world_2_camera = np.array(camera.get_transform().get_inverse_matrix())
        
        for i, vehicle_class in enumerate(all_vehicles_bbs):
            for bb in vehicle_class:
                img_bb, bb_params = render_bounding_boxes(bb, vehicle, world_2_camera, img, dim='2D')
                if bb_params:

                    ground_truth_annotations["annotations"].append({
                        "segmentation": [],
                        "area": bb_params[2]*bb_params[3],
                        "iscrowd": 0,
                        "category_id": i+1,
                        "image_id": frame_number,
                        "bbox": bb_params
                    })

        cv2.imwrite(os.path.join("out" , frame_file), img)
        cv2.imshow('BoundingBoxes',img)
        if cv2.waitKey(1) == ord('q'):
            break

except KeyboardInterrupt:
    pass
    
finally:
    print('Destroying {} vehicles!!!'.format(len(vehicle_list)))
    # destroy all actors
    for npc in vehicle_list:
        npc.destroy()
    for part in ego_vehicle_list:
        part.destroy()

    print("Saving annotations to json file.")
    with open('out/annotations.json', 'w') as json_file:
        json.dump(ground_truth_annotations, json_file)


cv2.destroyAllWindows()
