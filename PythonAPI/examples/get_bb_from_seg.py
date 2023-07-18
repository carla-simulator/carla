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
import json

try:
    import numpy as np
except ImportError:
    raise RuntimeError('cannot import numpy, make sure numpy package is installed')


OUT_FOLDER = "_ground_truth_bb_from_seg"


# ==============================================================================
# -- functions -----------------------------------------------------------------
# ==============================================================================

def get_bb_from_seg(seg_im, object_color):
    """
    The input image should be a RGB image with dim (wxhx3).
    """
    object_mask = np.where((seg_im[:,:,:3] == np.flip(object_color)).all(axis=2), 1,0)
    # cv2.imshow("seg im", seg_im)
    # object_mask = object_mask.astype(np.uint8)

    contours = cv2.findContours(object_mask, cv2.RETR_FLOODFILL, cv2.CHAIN_APPROX_SIMPLE)
    cnts = contours[0] if len(contours) == 2 else contours[1]
    cnts = list(cnts)
    bounding_boxes = []
    
    for cnt in cnts:
        rect = cv2.minAreaRect(cnt)
        box = cv2.boxPoints(rect)
        bounding_boxes.append(np.int0(box))

    return bounding_boxes


def render_bounding_boxes(bb, img):
    
    x_min = bb[0][0]
    y_min = bb[0][1]
    x_max = bb[1][0]
    y_max = bb[2][1]

    img = cv2.line(img, (int(x_min),int(y_min)), (int(x_max),int(y_min)), (0,0,255, 255), 1)
    img = cv2.line(img, (int(x_min),int(y_max)), (int(x_max),int(y_max)), (0,0,255, 255), 1)
    img = cv2.line(img, (int(x_min),int(y_min)), (int(x_min),int(y_max)), (0,0,255, 255), 1)
    img = cv2.line(img, (int(x_max),int(y_min)), (int(x_max),int(y_max)), (0,0,255, 255), 1)
            
    return img


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

# spawn "segmentation sensor"
segmentation_bp = bp_lib.find('sensor.camera.semantic_segmentation')
segmentation_init_trans = carla.Transform(carla.Location(x=1.5, z=2.4))
segmentation = world.spawn_actor(segmentation_bp, segmentation_init_trans, attach_to=vehicle)
colorConverter = carla.ColorConverter.CityScapesPalette
ego_vehicle_list.append(segmentation)
vehicle.set_autopilot(True)

# Set up the simulator in synchronous mode
settings = world.get_settings()
settings.synchronous_mode = True # Enables synchronous mode
settings.fixed_delta_seconds = 0.05
world.apply_settings(settings)

# Create a queue to store and retrieve the sensor data
image_queue = queue.Queue()
segmentation_queue = queue.Queue()
camera.listen(image_queue.put)
segmentation.listen(segmentation_queue.put)

# Get the world to camera transformation matrix
world_2_camera = np.array(camera.get_transform().get_inverse_matrix())

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
seg_image = segmentation_queue.get()
seg_image.convert(colorConverter)

# Reshape the raw data into an RGB array
img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))
# print(type(seg_image))
seg_img = np.reshape(np.copy(seg_image.raw_data), (seg_image.height, seg_image.width, 4))
# print(type(seg_img))

# Display the image in an OpenCV display window
cv2.namedWindow('BoundingBoxes', cv2.WINDOW_AUTOSIZE)
cv2.imshow('BoundingBoxes',img)
cv2.waitKey(1)

palette = {
    # "rider": np.array([255, 0, 0]),       # 13
    "car": np.array([0, 0, 142]),         # 14
    "truck": np.array([0, 0, 70]),       # 15
    "bus": np.array([0, 60, 100]),         # 16
    # "train": np.array([0, 80, 100]),       # 17
    # "motorcycle": np.array([0, 0, 230]),  # 18
    # "bicycle": np.array([119, 11, 32])      # 19
}

try:
# main loop
    while True:
        frame_number += 1
        # Retrieve and reshape the image
        world.tick()
        image = image_queue.get()
        seg_image = segmentation_queue.get()
        seg_image.convert(colorConverter)


        img = np.reshape(np.copy(image.raw_data), (image.height, image.width, 4))
        seg_img = np.reshape(np.copy(seg_image.raw_data), (seg_image.height, seg_image.width, 4))

        frame_file = "{:05d}.png".format(frame_number)

        ground_truth_annotations["images"].append({
        "file_name": frame_file,
        "height": image.height,
        "width": image.width,
        "id": frame_number})

        # Get the camera matrix 
        world_2_camera = np.array(camera.get_transform().get_inverse_matrix())

        for key in palette:
            object_color = palette[key]

            bounding_boxes = get_bb_from_seg(seg_img, object_color)

            for bb in bounding_boxes:
                x_min = bb[0][0]
                y_min = bb[0][1]
                width = bb[1][0]-bb[0][0]
                height = bb[2][1]-bb[0][1]
                # print("AREA: ",width*height)
                if width*height > 500:
                    img = render_bounding_boxes(bb, img)
                    

                    ground_truth_annotations["annotations"].append({
                        "segmentation": [],
                        "area": (bb[1][0]-bb[0][0])*(bb[2][1]-bb[0][1]),
                        "iscrowd": 0,
                        "category_id": i+1,
                        "image_id": frame_number,
                        "bbox": [bb[0][0], bb[0][1], bb[1][0]-bb[0][0], bb[2][1]-bb[0][1]]
                    })

        cv2.imwrite(os.path.join(OUT_FOLDER , frame_file), img)
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
    with open(OUT_FOLDER+'/annotations.json', 'w') as json_file:
        json.dump(ground_truth_annotations, json_file)


cv2.destroyAllWindows()
