# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
NUREC Image Capture Script

This script replays a NUREC (NVIDIA Neural Reconstruction Engine) scenario in CARLA
and captures images from both NUREC cameras and standard CARLA cameras, saving them
to disk. The script demonstrates how to:
- Set up NUREC cameras with custom framerates and resolutions
- Attach standard CARLA cameras to the ego vehicle
- Save captured images as JPEG files to organized directories
- Display real-time camera feeds using Pygame

The captured images are saved to the specified output directory with separate
folders for each camera type.

Example usage:
    python example_save_images.py --usdz-filename /path/to/scenario.usdz --output-dir ./captured_images
"""
import numpy as np

import carla

import argparse
import logging

# set up logging with info for default logger
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    level=logging.INFO,
)
logger = logging.getLogger("example_save_images")

from nurec_integration import NurecScenario
from pygame_display import PygameDisplay
from constants import EGO_TRACK_ID
from utils import handle_exception
from typing import Union, Tuple, Optional

    
def process_carla_image(display: PygameDisplay, image_pos: Tuple[int, int], image: carla.Image) -> None:
    """
    Callback function to process images from Carla's camera sensor
    """
    # Convert Carla raw image to a format suitable for pygame display
    array = np.frombuffer(image.raw_data, dtype=np.uint8)
    array = array.reshape((image.height, image.width, 4))
    array = array[:, :, :3]  # Remove alpha channel
    # bgr to rgb
    array = array[:, :, ::-1]
    display.setImage(array, (1,2), image_pos)

import os
try:
    import imageio
except ImportError as e:
    raise ImportError("imageio not found, this example requires it") from e

name_to_index = {}
def save_image(image: Union[carla.Image, np.ndarray], camera_name: str, output_dir: str) -> None:
    """
    Save the image to a file
    """
    next_index = name_to_index.get(camera_name, 0)
    name_to_index[camera_name] = next_index + 1

    # write as jpeg to disk under data/camera_name/
    os.makedirs(f"{output_dir}/{camera_name}", exist_ok=True)

    if isinstance(image, carla.Image):
        image.save_to_disk(f"{output_dir}/{camera_name}/{next_index}.jpg")
    else:
        array = image.astype(np.uint8)
        imageio.imwrite(f"{output_dir}/{camera_name}/{next_index}.jpg", array)

def add_cameras(scenario: NurecScenario, client: carla.Client, output_dir: str) -> Tuple[carla.Actor, carla.Actor, PygameDisplay]:
    # Set up pygame display for visualization
    pygame_displays = PygameDisplay()
    # Add cameras using the new flexible add_camera method
    
    scenario.add_camera("camera_front_wide_120fov", lambda image: pygame_displays.setImage(image, (1, 2), (0, 0)), framerate=10, resolution_ratio=0.125)
    

    write_to_disk = True
    scenario.add_camera("camera_front_wide_120fov", lambda image: save_image(image, "camera_front_wide_120fov", output_dir), framerate=30, resolution_ratio=0.5)
    # scenario.add_camera("camera_cross_left_120fov", lambda image: save_image(image, "camera_cross_left_120fov"), framerate=20, resolution_ratio=1)
    # scenario.add_camera("camera_cross_right_120fov", lambda image: save_image(image, "camera_cross_right_120fov"), framerate=20, resolution_ratio=1)

    # Add a standard Carla camera attached to the ego vehicle
    world = client.get_world()
    bp_library = world.get_blueprint_library()
    camera_bp = bp_library.find('sensor.camera.rgb')
    camera_bp.set_attribute('image_size_x', '481')
    camera_bp.set_attribute('image_size_y', '271')
    camera_bp.set_attribute('fov', '100')
    
    # Get the ego vehicle instance
    ego_vehicle = scenario.actor_mapping[EGO_TRACK_ID].actor_inst
    
    # Attach camera to ego vehicle with a rear view
    camera_transform = carla.Transform(carla.Location(x=0, z=10), carla.Rotation(pitch=-90))
    camera = world.spawn_actor(camera_bp, camera_transform, attach_to=ego_vehicle)

    camera.listen(lambda image: process_carla_image(pygame_displays, (0, 1), image))

    return camera, pygame_displays

def main() -> None:
    """
    Main function that parses command line arguments, connects to CARLA,
    loads the NUREC scenario, and starts the replay with visualization.
    """
    argparser = argparse.ArgumentParser(
        description=__doc__)
    argparser.add_argument(
        '--host',
        metavar='H',
        default='127.0.0.1',
        help='IP of the host server (default: 127.0.0.1)')
    argparser.add_argument(
        '-p', '--port',
        metavar='P',
        default=2000,
        type=int,
        help='TCP port to listen to (default: 2000)')
    argparser.add_argument(
        '-np', '--nurec-port',
        metavar='Q',
        default=46435,
        type=int,
        help='nurec port (default: 46435)')
    argparser.add_argument(
        '-u', '--usdz-filename',
        metavar='U',
        required=True,
        help='Path to the USDZ file containing the NUREC scenario data')
    argparser.add_argument(
        '-o', '--output-dir',
        metavar='O',
        default="data",
        help='output directory (data)')
    argparser.add_argument(
        '--move-spectator',
        action='store_true',
        help='move spectator camera')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)

    # Create and load the NUREC scenario
    with NurecScenario(client, args.usdz_filename, port=args.nurec_port, move_spectator=args.move_spectator, fps=30) as scenario:
        spectator: Optional[carla.Actor] = None
        display: Optional[PygameDisplay] = None
        try:
            spectator, display = add_cameras(scenario, client, args.output_dir)
            # Start the replay
            logger.info("Starting replay")
            scenario.start_replay()
            
            should_apply_control = True
            # Keep the script running until the replay is done
            while not scenario.is_done():
                scenario.tick()
                if should_apply_control and scenario.seconds_since_start() > 1:
                    logger.info(f"Applying control at time {scenario.seconds_since_start()} seconds.")
                    scenario.set_ego_simple_trajectory_following()
                    should_apply_control = False
            
        except KeyboardInterrupt:
            logger.info("Keyboard interrupt detected, exiting gracefully.")
            return
        except Exception as e:
            handle_exception(e)

        finally:
            if spectator is not None:
                spectator.stop()
                spectator.destroy()
            if display is not None:
                display.destroy()

if __name__ == '__main__':
    main()