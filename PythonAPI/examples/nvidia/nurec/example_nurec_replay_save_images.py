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
- Render a NuRec neural lidar with a live bird's-eye-view panel (NRE >= 26.04)
- Vary the scenario with the asset-editing API (swap / insert rendered assets)
- Hand the ego and/or every scene vehicle over to CARLA control (Traffic
  Manager physics), streaming the resulting poses back into the neural render

Runtime behaviour (renderer backend, harmonizer, control modes, lidar, asset
editing) is read from a settings file, nurec_demo_config.yaml next to this
script by default; see that file for all options.

Example usage:
    python example_nurec_replay_save_images.py -u /path/to/scenario.usdz
    python example_nurec_replay_save_images.py -u scenario.usdz --config my_settings.yaml
"""
import copy
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
from nre.grpc.protos.sensorsim_pb2 import ReplaceAssetAction, DynamicObjectTrack
from pygame_display import PygameDisplay
from constants import EGO_TRACK_ID
from utils import handle_exception
from typing import Any, Dict, Tuple, Optional, List


# Defaults for the demo settings file (nurec_demo_config.yaml); any key may be
# omitted there and falls back to these values.
DEFAULT_DEMO_CONFIG: Dict[str, Any] = {
    "renderer": {
        "backend": None,
        "harmonizer": False,
        "image_format": "planar",
        "extra_server_args": [],
    },
    "control": {
        "ego": "trajectory",
        "handoff_seconds": 1.0,
        "actors": "replay",
    },
    "asset_editing": {
        "enabled": False,
        "demo_swap": False,
        "demo_insert": False,
        "insert_offset_m": 4.0,
    },
    "lidar": {
        "enabled": False,
        "type": "PANDAR128",
        "framerate": 10,
        "visualize": True,
        "bev_range_m": 25.0,
        "bev_size_px": 480,
    },
}


def _merge_config(defaults: Dict[str, Any], overrides: Dict[str, Any]) -> Dict[str, Any]:
    merged = dict(defaults)
    for key, value in (overrides or {}).items():
        if isinstance(value, dict) and isinstance(merged.get(key), dict):
            merged[key] = _merge_config(merged[key], value)
        else:
            merged[key] = value
    return merged


def load_demo_config(path: Optional[str]) -> Dict[str, Any]:
    """
    Load the demo settings file, merged over DEFAULT_DEMO_CONFIG. A missing
    default file falls back to the defaults; a missing explicit --config is
    an error.
    """
    default_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "nurec_demo_config.yaml"
    )
    explicit = path is not None
    path = path or default_path
    if not os.path.exists(path):
        if explicit:
            raise FileNotFoundError(f"Config file not found: {path}")
        logger.info("No demo config file found, using defaults")
        return copy.deepcopy(DEFAULT_DEMO_CONFIG)
    with open(path, "r") as f:
        overrides = yaml.safe_load(f) or {}
    cfg = _merge_config(copy.deepcopy(DEFAULT_DEMO_CONFIG), overrides)

    if cfg["control"]["ego"] not in ("replay", "trajectory", "autopilot"):
        raise ValueError(f"control.ego must be replay|trajectory|autopilot, got {cfg['control']['ego']!r}")
    if cfg["control"]["actors"] not in ("replay", "carla"):
        raise ValueError(f"control.actors must be replay|carla, got {cfg['control']['actors']!r}")
    if cfg["renderer"]["backend"] not in (None, "nrend", "gsplat"):
        raise ValueError(f"renderer.backend must be null|nrend|gsplat, got {cfg['renderer']['backend']!r}")
    logger.info(f"Demo config loaded from {path}")
    return cfg


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


def lidar_to_bev_image(
    points: np.ndarray,
    intensities: np.ndarray,
    size_px: int = 480,
    range_m: float = 50.0,
) -> np.ndarray:
    """
    Rasterize a lidar sweep into a bird's-eye-view RGB image: +x (forward) up,
    +y left; green from intensity, red from height, white ego marker.
    """
    img = np.zeros((size_px, size_px, 3), dtype=np.uint8)
    if points.shape[0] > 0:
        # The server may return no intensities (observed with NRE 26.04);
        # fall back to a constant brightness then.
        if intensities.shape[0] != points.shape[0]:
            intensities = np.full(points.shape[0], 0.75, dtype=np.float32)
        x, y, z = points[:, 0], points[:, 1], points[:, 2]
        mask = (np.abs(x) < range_m) & (np.abs(y) < range_m)
        x, y, z, inten = x[mask], y[mask], z[mask], intensities[mask]
        rows = ((1 - (x + range_m) / (2 * range_m)) * (size_px - 1)).astype(np.int32)
        cols = ((1 - (y + range_m) / (2 * range_m)) * (size_px - 1)).astype(np.int32)
        if inten.size and inten.max() <= 1.0:
            inten = inten * 255.0
        img[rows, cols, 1] = np.clip(inten, 48, 255).astype(np.uint8)
        img[rows, cols, 0] = np.clip((z + 2.0) / 4.0 * 255.0, 0, 255).astype(np.uint8)
        img[rows, cols, 2] = 64
    center = size_px // 2
    img[center - 4 : center + 5, center - 1 : center + 2] = (255, 255, 255)
    img[center - 1 : center + 2, center - 4 : center + 5] = (255, 255, 255)
    return img


def make_lidar_callback(
    display: Optional[PygameDisplay],
    pygame_pos: Tuple[int, int],
    saveimages: bool,
    output_dir: str,
    lidar_cfg: Dict[str, Any],
):
    size_px = int(lidar_cfg["bev_size_px"])
    range_m = float(lidar_cfg["bev_range_m"])

    def callback(points: np.ndarray, intensities: np.ndarray) -> None:
        bev = lidar_to_bev_image(points, intensities, size_px, range_m)
        if display is not None:
            display.setImage(bev, (3, 2), pygame_pos)
        if saveimages:
            save_image(bev, "lidar_bev", output_dir)

    return callback


def _shift_trajectory_sideways(track: DynamicObjectTrack, offset_m: float) -> None:
    """
    Shift a DynamicObjectTrack's whole trajectory laterally (perpendicular,
    in the ground plane, to its direction of travel). Parked objects fall
    back to a +x shift.
    """
    poses = track.trajectory.poses
    lateral = np.array([1.0, 0.0])
    positions = np.array([[p.pose.vec[0], p.pose.vec[1]] for p in poses])
    if len(positions) >= 2:
        deltas = positions[-1] - positions[0]
        norm = np.linalg.norm(deltas)
        if norm > 1e-3:
            direction = deltas / norm
            lateral = np.array([-direction[1], direction[0]])
    for pose_at_time in poses:
        pose_at_time.pose.vec[0] += float(lateral[0] * offset_m)
        pose_at_time.pose.vec[1] += float(lateral[1] * offset_m)


def run_asset_editing_demo(scenario: NurecScenario, edit_cfg: Dict[str, Any]) -> None:
    """
    Show the NRE scenario-variation API: list the scene's dynamic objects and
    optionally swap one rendered asset for another (demo_swap) and/or insert
    a rendered-only clone of an object on a laterally shifted trajectory
    (demo_insert). Both demos are self-contained: they only use assets that
    already exist in the loaded scene.
    """
    renderer = scenario.renderer
    objects = renderer.get_dynamic_objects()
    swappable_ids = set(renderer.get_external_asset_objects())
    logger.info(
        f"Scene has {len(objects)} dynamic object tracks, "
        f"{len(swappable_ids)} backed by swappable assets"
    )
    for obj in objects:
        marker = "*" if obj.id in swappable_ids else " "
        logger.info(
            f" {marker} track {obj.id}: class={obj.semantic_class} asset={obj.asset_id!r} "
            f"size=({obj.object_size.size_x:.1f}, {obj.object_size.size_y:.1f}, {obj.object_size.size_z:.1f})"
        )

    swappable = [o for o in objects if o.id in swappable_ids]
    replace: List[ReplaceAssetAction] = []
    insert: List[DynamicObjectTrack] = []

    if edit_cfg["demo_swap"]:
        pair = next(
            (
                (a, b)
                for a in swappable
                for b in swappable
                if a.id != b.id and a.asset_id != b.asset_id
            ),
            None,
        )
        if pair is None:
            logger.warning("demo_swap: no two swappable objects with distinct assets; skipping")
        else:
            original, donor = pair
            replace.append(
                ReplaceAssetAction(
                    original_id=original.id,
                    replacement_id=donor.asset_id,
                    object_size=donor.object_size,
                )
            )
            logger.info(
                f"demo_swap: track {original.id} will be rendered with asset "
                f"{donor.asset_id!r} (borrowed from track {donor.id})"
            )

    if edit_cfg["demo_insert"]:
        if not swappable:
            logger.warning("demo_insert: no swappable object to clone; skipping")
        else:
            source = swappable[0]
            clone = DynamicObjectTrack()
            clone.CopyFrom(source)
            clone.id = f"{source.id}_demo_insert"
            _shift_trajectory_sideways(clone, float(edit_cfg["insert_offset_m"]))
            insert.append(clone)
            logger.info(
                f"demo_insert: inserting clone {clone.id} of track {source.id} "
                f"shifted {edit_cfg['insert_offset_m']} m sideways"
            )

    if replace or insert:
        renderer.edit_assets(replace=replace or None, insert=insert or None)
        logger.info("edit_assets applied")


def apply_control_modes(scenario: NurecScenario, control_cfg: Dict[str, Any]) -> None:
    """Hand over ego / scene actors per the control section of the config."""
    ego_mode = control_cfg["ego"]
    actors_mode = control_cfg["actors"]
    logger.info(
        f"Control handoff at t={scenario.seconds_since_start():.2f}s: "
        f"ego={ego_mode}, actors={actors_mode}"
    )
    if ego_mode == "trajectory":
        scenario.set_ego_simple_trajectory_following()
    elif ego_mode == "autopilot":
        scenario.set_ego_autopilot(True)
    if actors_mode == "carla":
        scenario.set_all_actors_carla_controlled()


def add_cameras(
    scenario: NurecScenario, client: carla.Client, output_dir: str, saveimages: bool, resolution_ratio: float = 0.125
) -> Tuple[List[carla.Actor], PygameDisplay]:
    # Set up pygame display for visualization
    pygame_display = PygameDisplay()

    world = client.get_world()
    # Get the blueprint library to spawn cameras
    bp_library = world.get_blueprint_library()

    # Standard CARLA cameras spawned alongside the NuRec ones (may be empty)
    carla_cameras: List[carla.Actor] = []

    # Add cameras using the new flexible add_camera method

    config_path = os.path.join(os.path.dirname(os.path.abspath(__file__)),
                               "carla_example_camera_config.yaml")
    with open(config_path, "r") as f:
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
            carla_cameras.append(camera)

        else:
            raise ValueError(f"Unknown camera configuration format: {cam_cfg}")

        grid_pos = (grid_pos[0] + 1, grid_pos[1])
        if grid_pos[0] >= grid_size[0]:
            grid_pos = (0, grid_pos[1] + 1)

    return carla_cameras, pygame_display


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
        default=None,
        type=int,
        help="nurec gRPC port (default: auto-pick a free port)",
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
    argparser.add_argument(
        "-c",
        "--config",
        metavar="C",
        default=None,
        help="demo settings file (default: nurec_demo_config.yaml next to this script)",
    )
    args = argparser.parse_args()

    cfg = load_demo_config(args.config)
    renderer_cfg = cfg["renderer"]
    edit_cfg = cfg["asset_editing"]
    lidar_cfg = cfg["lidar"]
    asset_editing_active = (
        edit_cfg["enabled"] or edit_cfg["demo_swap"] or edit_cfg["demo_insert"]
    )

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)

    # Create and load the NUREC scenario
    with NurecScenario(
        client,
        args.usdz_filename,
        port=args.nurec_port,
        move_spectator=args.move_spectator,
        fps=30,
        renderer_backend=renderer_cfg["backend"],
        image_format=renderer_cfg["image_format"],
        harmonizer=renderer_cfg["harmonizer"],
        extra_server_args=renderer_cfg["extra_server_args"],
        enable_asset_editing=asset_editing_active,
    ) as scenario:
        carla_cameras: List[carla.Actor] = []
        display: Optional[PygameDisplay] = None
        try:
            # Add cameras; keep references to the CARLA camera actors alive
            carla_cameras, display = add_cameras(scenario, client, args.output_dir, args.saveimages )

            if lidar_cfg["enabled"]:
                scenario.add_lidar(
                    make_lidar_callback(
                        display if lidar_cfg["visualize"] else None,
                        (1, 1),  # free cell in the 3x2 pygame grid
                        args.saveimages,
                        args.output_dir,
                        lidar_cfg,
                    ),
                    lidar_type=lidar_cfg["type"],
                    framerate=lidar_cfg["framerate"],
                )
                logger.info(f"NuRec lidar enabled: {lidar_cfg['type']} @ {lidar_cfg['framerate']} Hz")

            if asset_editing_active:
                run_asset_editing_demo(scenario, edit_cfg)

            logger.info("Starting replay")
            scenario.start_replay()

            should_apply_control = (
                cfg["control"]["ego"] != "replay" or cfg["control"]["actors"] != "replay"
            )

            # Keep the script running until the replay is done
            while not scenario.is_done():
                scenario.tick()
                if (
                    should_apply_control
                    and scenario.seconds_since_start() > cfg["control"]["handoff_seconds"]
                ):
                    apply_control_modes(scenario, cfg["control"])
                    should_apply_control = False
        except KeyboardInterrupt:
            logger.info("Keyboard interrupt detected, exiting gracefully.")
            return
        except Exception as e:
            handle_exception(e)

        finally:
            for camera in carla_cameras:
                camera.stop()
                camera.destroy()
            if display is not None:
                display.destroy()

    sim_world = client.get_world()
    # Reset the world to non-synchronous mode
    settings = sim_world.get_settings()
    settings.synchronous_mode = False
    sim_world.apply_settings(settings)

if __name__ == "__main__":
    main()
