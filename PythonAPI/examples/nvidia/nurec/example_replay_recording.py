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

import logging

# set up logging with info for default logger
logging.basicConfig(
    format="%(asctime)s %(levelname)-8s %(message)s",
    datefmt="%Y-%m-%d %H:%M:%S",
    level=logging.INFO,
)
logger = logging.getLogger("example_replay_recording")


from nurec_integration import NurecScenario
from pygame_display import PygameDisplay
from constants import EGO_TRACK_ID
from utils import handle_exception
from typing import Tuple, Optional


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


def add_cameras(
    scenario: NurecScenario, client: carla.Client, resolution_ratio: float = 0.125
) -> Tuple[carla.Actor, PygameDisplay]:
    # Set up pygame display for visualization
    pygame_display = PygameDisplay()
    # Add cameras using the new flexible add_camera method

    scenario.add_camera(
        "camera_front_wide_120fov",
        lambda image: pygame_display.setImage(image, (3, 2), (1, 0)),
        framerate=10,
        resolution_ratio=0.125,
    )
    scenario.add_camera(
        "camera_cross_left_120fov",
        lambda image: pygame_display.setImage(image, (3, 2), (0, 0)),
        framerate=10,
        resolution_ratio=0.125,
    )
    scenario.add_camera(
        "camera_cross_right_120fov",
        lambda image: pygame_display.setImage(image, (3, 2), (2, 0)),
        framerate=10,
        resolution_ratio=0.125,
    )

    # Add a standard Carla camera attached to the ego vehicle
    world = client.get_world()
    bp_library = world.get_blueprint_library()
    camera_bp = bp_library.find("sensor.camera.rgb")
    camera_bp.set_attribute("image_size_x", "481")
    camera_bp.set_attribute("image_size_y", "271")
    camera_bp.set_attribute("fov", "100")

    # Get the ego vehicle instance
    ego_vehicle = scenario.actor_mapping[EGO_TRACK_ID].actor_inst

    # Attach camera to ego vehicle with a rear view
    camera_transform = carla.Transform(
        carla.Location(x=0, z=10), carla.Rotation(pitch=-90)
    )
    camera = world.spawn_actor(camera_bp, camera_transform, attach_to=ego_vehicle)

    # Set up the callback function to process and display images
    # Display in position 1,1 (center of the display grid)
    camera.listen(
        lambda image: process_carla_image(pygame_display, (3, 2), (1, 1), image)
    )
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
            spectator, display = add_cameras(scenario, client)

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


if __name__ == "__main__":
    main()
