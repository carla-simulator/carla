# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
NUREC Multi-Camera Replay Script

This script demonstrates comprehensive camera setup for NUREC scenario replay in CARLA.
It showcases how to:
- Set up multiple NUREC cameras (front, left cross, right cross) simultaneously
- Configure different camera positions in a Pygame display grid
- Attach standard CARLA cameras to the ego vehicle for additional perspectives
- Coordinate multiple camera feeds with different framerates and resolutions

The script provides a complete multi-view visualization system, ideal for understanding
how to integrate various camera types and create comprehensive monitoring setups.

Example usage:
    python example_replay_recording.py --usdz-filename /path/to/scenario.usdz
"""
import numpy as np

import carla

import argparse
import os
import imageio
import sys
import yaml
from typing import Union
import logging

# set up logging with info for default logger
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    level=logging.INFO,
)
logger = logging.getLogger("example_replay_recording")


from nurec_integration import NurecScenario, ShutterType
from pygame_display import PygameDisplay
from constants import EGO_TRACK_ID
from utils import handle_exception
from typing import Tuple, Optional


def make_transform_matrix(rotation=None, translation=None):
    """
    Create a 4x4 compatible with your consumer:
    - Unreal/CARLA rotation: yaw(Z), pitch(Y), roll(X), in degrees.
    - Translation [x, y, z].
    - Adjust the axes so that the consumer's forward (Z column) points to +X of the world.
    """
    mat = np.eye(4, dtype=float)

    if rotation is not None:
        pitch_deg, yaw_deg, roll_deg = rotation
        yaw   = np.radians(yaw_deg)
        pitch = np.radians(pitch_deg)
        roll  = np.radians(roll_deg)

        # Basic rotations (right = +Y, up = +Z, forward = +X)
        Rz_yaw = np.array([
            [ np.cos(yaw), -np.sin(yaw), 0.0],
            [ np.sin(yaw),  np.cos(yaw), 0.0],
            [          0.,           0., 1.0]
        ])

        Ry_pitch = np.array([
            [ np.cos(pitch), 0.0, np.sin(pitch)],
            [           0.0, 1.0,           0.0],
            [-np.sin(pitch), 0.0, np.cos(pitch)]
        ])

        Rx_roll = np.array([
            [1.0,          0.0,           0.0],
            [0.0,  np.cos(roll), -np.sin(roll)],
            [0.0,  np.sin(roll),  np.cos(roll)]
        ])

        # Unreal Order: R = Rz(yaw) * Ry(pitch) * Rx(roll)
        R_unreal = Rz_yaw @ Ry_pitch @ Rx_roll

        # Change of basis matrix (columns = engine axes in Unreal coordinates):
        # col0 = engine_X = -Unreal_Y = (0,-1,0)
        # col1 = engine_Y = -Unreal_Z = (0, 0,-1)
        # col2 = engine_Z =  Unreal_X = (1, 0, 0)
        A = np.array([
            [ 0.0,  0.0, 1.0],
            [-1.0,  0.0, 0.0],
            [ 0.0, -1.0, 0.0]
        ])

        # Rotación final para tu consumidor
        R_engine = R_unreal @ A
        mat[:3, :3] = R_engine

    if translation is not None:
        mat[:3, 3] = translation

    return mat


def parse_camera_params(cam_cfg):
    params = cam_cfg["camera_params"].copy()

    # Convert "pi" string to np.pi
    if params.get("max_angle") == "pi":
        params["max_angle"] = np.pi

    # Convert shutter_type string to enum
    if isinstance(params.get("shutter_type"), str):
        params["shutter_type"] = getattr(ShutterType, params["shutter_type"])

    return params

def process_carla_image(
    display: PygameDisplay,
    pygame_dims: Tuple[int, int],
    image_pos: Tuple[int, int],
    image: carla.Image,
) -> None:
    """
    Callback function to process images from Carla's camera sensor
    """
    # Convert Carla raw image to a format suitable for pygame display
    array = np.frombuffer(image.raw_data, dtype=np.uint8)
    array = array.reshape((image.height, image.width, 4))
    array = array[:, :, :3]  # Remove alpha channel
    # bgr to rgb
    array = array[:, :, ::-1]
    display.setImage(array, pygame_dims, image_pos)

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
        image.save_to_disk(f"{output_dir}/{camera_name}/{next_index:05d}.jpg")
    else:
        array = image.astype(np.uint8)
        imageio.imwrite(f"{output_dir}/{camera_name}/{next_index:05d}.jpg", array)

def make_camera_callback(display, camera_name, pygame_pos, saveimages: bool, output_dir: str = "data"):
    def callback(image):
        display.setImage(image, (3, 2), pygame_pos)
        if saveimages:
            save_image(image, camera_name, output_dir)
    return callback


def add_cameras(
    scenario: NurecScenario, client: carla.Client, output_dir: str, saveimages: bool, resolution_ratio: float = 0.125
) -> Tuple[carla.Actor, PygameDisplay]:
    # Set up pygame display for visualization
    pygame_display = PygameDisplay()

    world = client.get_world()
    # Get the blueprint library to spawn cameras
    bp_library = world.get_blueprint_library()

    # Add cameras using the new flexible add_camera method

    with open("carla_example_camera_config.yaml", "r") as f:
        camera_configs = yaml.safe_load(f)

    grid_size =  (3, 2)
    grid_pos = (0, 0)
    for cam_cfg in camera_configs:
        # Case 1: Rich camera_params style
        if "camera_params" in cam_cfg:
            camera_params = parse_camera_params(cam_cfg)
            # --- Choose between transform_matrix or rot+trans ---
            if "transform_matrix" in cam_cfg:
                transform_matrix = np.array(cam_cfg["transform_matrix"], dtype=float)
            else:
                rotation = cam_cfg.get("rotation")         # [roll, pitch, yaw] in rad
                translation = cam_cfg.get("translation")   # [x, y, z]
                transform_matrix = make_transform_matrix(rotation, translation)

            cameraname = camera_params["logical_id"] + str(grid_pos[0]) + str(grid_pos[1])
            scenario.add_camera(
                camera_params,
                make_camera_callback(pygame_display, cameraname, grid_pos, saveimages, output_dir),
                transform=transform_matrix,
                framerate=30,
                resolution_ratio=0.125,
            )
        # Case 2: Simple CARLA sensor style
        elif "sensor" in cam_cfg:
            sensor_type = cam_cfg["sensor"]
            camera_bp = bp_library.find(f"sensor.camera.{sensor_type}")

            # Apply attributes
            for attr, value in cam_cfg.get("attributes", {}).items():
                camera_bp.set_attribute(attr, str(value))

            # Build transform
            loc = cam_cfg["transform"]["location"]
            rot = cam_cfg["transform"]["rotation"]
            camera_transform = carla.Transform(
                carla.Location(x=loc.get("x", 0.0), y=loc.get("y", 0.0), z=loc.get("z", 0.0)),
                carla.Rotation(pitch=rot.get("pitch", 0.0), yaw=rot.get("yaw", 0.0), roll=rot.get("roll", 0.0)),
            )

            camera = world.spawn_actor(camera_bp, camera_transform, attach_to=scenario.actor_mapping[EGO_TRACK_ID].actor_inst)


            camera.listen(
                lambda image, pos=grid_pos: process_carla_image(pygame_display, grid_size, pos, image)
            )

        else:
            raise ValueError(f"Unknown camera configuration format: {cam_cfg}")
        
        grid_pos = (grid_pos[0] + 1, grid_pos[1])
        if grid_pos[0] >= grid_size[0]:
            grid_pos = (1, grid_pos[1] + 1)
        
    return camera, pygame_display


def main() -> None:
    """
    Main function that parses command line arguments, connects to CARLA,
    loads the NUREC scenario, and starts the multi-camera replay with visualization.
    """
    argparser = argparse.ArgumentParser(description=__doc__)
    argparser.add_argument(
        "--host",
        metavar="H",
        default="127.0.0.1",
        help="IP of the host server (default: 127.0.0.1)",
    )
    argparser.add_argument(
        "-p",
        "--port",
        metavar="P",
        default=2000,
        type=int,
        help="TCP port to listen to (default: 2000)",
    )
    argparser.add_argument(
        '-o', '--output-dir',
        metavar='O',
        default="data",
        help='output directory (data)')
    argparser.add_argument(
        "-np",
        "--nurec-port",
        metavar="Q",
        default=46435,
        type=int,
        help="nurec port (default: 46435)",
    )
    argparser.add_argument(
        "-u",
        "--usdz-filename",
        metavar="U",
        required=True,
        help="Path to the USDZ file containing the NUREC scenario data",
    )
    argparser.add_argument(
        "--saveimages",
        action="store_true",
        help="Save images to disk (default: False)",
    )
    argparser.add_argument(
        "--move-spectator", action="store_true", help="move spectator camera"
    )
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)

    # Create and load the NUREC scenario
    with NurecScenario(
        client,
        args.usdz_filename,
        port=args.nurec_port,
        move_spectator=args.move_spectator,
        fps=30,
    ) as scenario:
        spectator: Optional[carla.Actor] = None
        display: Optional[PygameDisplay] = None
        try:
            # Add cameras, we need to refernce spectator to keep it alive
            spectator, display = add_cameras(scenario, client, args.output_dir, args.saveimages )

            logger.info("Starting replay")
            scenario.start_replay()

            should_apply_control = True

            # Keep the script running until the replay is done
            while not scenario.is_done():
                scenario.tick()
                if should_apply_control and scenario.seconds_since_start() > 1:
                    logger.info(
                        f"Applying control at time {scenario.seconds_since_start()} seconds."
                    )
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

    sim_world = client.get_world()
    # Reset the world to non-synchronous mode
    settings = sim_world.get_settings()
    settings.synchronous_mode = False
    sim_world.apply_settings(settings)

if __name__ == "__main__":
    main()
