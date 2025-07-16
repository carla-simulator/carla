# SPDX-FileCopyrightText: © 2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
#
# SPDX-License-Identifier: MIT

"""
NUREC Custom Camera Parameters Script

This script demonstrates how to configure NUREC cameras with custom intrinsic parameters
in CARLA. It showcases advanced camera setup including:
- Custom ftheta camera model configuration
- Precise intrinsic parameter specification (principal point, distortion polynomials)
- Custom camera transform matrices for positioning
- Rolling shutter simulation
- Real-time visualization using Pygame

This example is particularly useful for users who need to replicate specific camera
hardware characteristics or match real-world camera calibrations.

Example usage:
    python example_custom_camera.py --usdz-filename /path/to/scenario.usdz
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
logger = logging.getLogger("example_custom_camera")

from nurec_integration import NurecScenario, ShutterType
from pygame_display import PygameDisplay
from utils import handle_exception
from typing import Optional

def add_cameras(scenario: NurecScenario, client: carla.Client) -> PygameDisplay:
    # Set up pygame display for visualization
    pygame_displays = PygameDisplay(cell_width=1924, cell_height=1084)

    camera_params = {
        'camera_type': 'ftheta',
        'resolution_w': 3848,
        'resolution_h': 2168,
        'logical_id': "camera_front_wide_120fov",
        'principal_point_x': 1927.038818,
        'principal_point_y': 1099.315796,
        'pixeldist_to_angle_poly': [
            0.0,
            0.00053944590035826,
            2.46685849525363e-09,
            2.6309770334576e-12,
            -2.44043479933324e-16
        ],
        'reference_poly': 1,
        'angle_to_pixeldist_poly': [],
        'max_angle': np.pi,
        'shutter_type': ShutterType.ROLLING_TOP_TO_BOTTOM
    }

    transform_matrix = np.array([
        [ 0.0,  0.0,  1.0,   1.749],
        [-1.0,  0.0,  0.0,  -0.1],
        [ 0.0, -1.0,  0.0,   1.47],
        [ 0.0,  0.0,  0.0,   1.0]
      ])

    scenario.add_camera(
                camera_params,
                lambda image: pygame_displays.setImage(image, (1, 1), (0, 0)),
                transform=transform_matrix,
                framerate=30,
                resolution_ratio=0.5
            )
    
    return pygame_displays

def main() -> None:
    """
    Main function that parses command line arguments, connects to CARLA,
    loads the NUREC scenario, and starts the replay with custom camera configuration.
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
        '--move-spectator',
        action='store_true',
        help='move spectator camera')
    args = argparser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)

    # Create and load the NUREC scenario
    with NurecScenario(client, args.usdz_filename, port=args.nurec_port, move_spectator=args.move_spectator, fps=30) as scenario:
        display: Optional[PygameDisplay] = None
        try:
            display = add_cameras(scenario, client)
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
            if display is not None:
                display.destroy()


if __name__ == '__main__':
    main()