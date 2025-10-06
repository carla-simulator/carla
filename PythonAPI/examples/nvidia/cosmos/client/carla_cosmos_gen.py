#!/usr/bin/env python3

import argparse
import multiprocessing as mp
import logging
from enum import Enum
from dataclasses import dataclass
from typing import Dict, List, Sequence, Tuple
from pathlib import Path
import yaml
import subprocess
import numpy as np
import cv2
import carla

import json
import tarfile
import tempfile
import math
import os
import sys
import shutil
import traceback
import glob

from PIL import Image

# === ENUMS AND DATA STRUCTURES ===
class AOV(Enum):
    RGB = 0
    DEPTH = 1
    SEMANTIC_SEGMENTATION = 2
    INSTANCE_SEGMENTATION = 3
    NORMALS = 4
    RDS_HQ = 5

@dataclass
class FrameBundle:
    index: int
    frames: Dict[AOV, np.ndarray]
    timestamp: float

def extract_between(input_string, left_delim, right_delim):
    try:
        start = input_string.index(left_delim) + len(left_delim)
        end = input_string.index(right_delim, start)
        return input_string[start:end]
    except ValueError:
        return None 


def parse_frames_duration(info):
    frames = extract_between(info, "Frames: ", "\n")
    duration = extract_between(info, "Duration: ", " seconds")

    if frames and duration:
        return int(frames), float(duration)
    else:
        return -1, -1.0

def compute_rotation_components(rotation):
    """Compute cosine and sine components for roll, pitch, yaw from CARLA rotation."""
    roll = math.radians(rotation.roll)
    pitch = math.radians(rotation.pitch)
    yaw = math.radians(rotation.yaw)

    cr, sr = math.cos(roll), math.sin(roll)
    cp, sp = math.cos(pitch), math.sin(pitch)
    cy, sy = math.cos(yaw), math.sin(yaw)

    return cr, sr, cp, sp, cy, sy

def build_transform_matrix(location, rotation, z_offset=0.0):
    """Build a 4x4 transformation matrix from CARLA location and rotation."""
    cr, sr, cp, sp, cy, sy = compute_rotation_components(rotation)
    adjusted_z = location.z + z_offset

    return [
        [cy * cp, cy * sp * sr - sy * cr, cy * sp * cr + sy * sr, location.x],
        [sy * cp, sy * sp * sr + cy * cr, sy * sp * cr - cy * sr, location.y],
        [-sp, cp * sr, cp * cr, adjusted_z],
        [0.0, 0.0, 0.0, 1.0]
    ]

# === CONFIGURATION LOADERS ===
CLASSES_TO_KEEP_SHADED_SEG: List[Sequence[int]] = []
CLASSES_TO_KEEP_CANNY: List[Sequence[int]] = []

# Semantic label to object type mapping for CARLA vehicles
SEMANTIC_LABEL_TO_OBJECT_TYPE = {
    14: "Automobile",
    15: "Truck",
    16: "Bus",
    17: "Train_or_tram_car",
    18: "Rider",
    19: "Rider"
}

def load_class_filter_config(path: str):
    path = Path(path).resolve()
    with open(path, 'r') as f:
        config = yaml.safe_load(f)
    global CLASSES_TO_KEEP_SHADED_SEG, CLASSES_TO_KEEP_CANNY
    CLASSES_TO_KEEP_SHADED_SEG = config.get('shaded_segmentation_classes', [])
    CLASSES_TO_KEEP_CANNY = config.get('canny_classes', [])

# === ORIGINAL POST-PROCESSING FUNCTIONS ===
def masked_edges_from_semseg(
    rgb_img: np.ndarray,
    semseg_img: np.ndarray,
    classes: List[Sequence[int]],
    *,
    gaussian_kernel: Tuple[int, int] = (5, 5),
    gaussian_sigma: float = 1.0,
    canny_thresh1: int = 100,
    canny_thresh2: int = 200,
) -> Tuple[np.ndarray, np.ndarray]:
    blurred_rgb = cv2.GaussianBlur(rgb_img, gaussian_kernel, gaussian_sigma)
    mask = np.zeros(semseg_img.shape[:2], dtype=np.uint8)
    for color in classes:
        lower = np.array(color, dtype=np.uint8)
        upper = np.array(color, dtype=np.uint8)
        mask |= cv2.inRange(semseg_img, lower, upper)
    mask_bool = mask.astype(bool)
    masked_rgb = np.zeros_like(rgb_img)
    masked_rgb[mask_bool] = blurred_rgb[mask_bool]
    gray = cv2.cvtColor(masked_rgb, cv2.COLOR_RGB2GRAY)
    edges = cv2.Canny(gray, canny_thresh1, canny_thresh2)
    return masked_rgb, edges


def created_shaded_composition(
    sem: np.ndarray, inst: np.ndarray, nor: np.ndarray, classes_to_keep: List[Sequence[int]]
) -> np.ndarray:
    semantics = sem[..., ::-1]
    instances = inst[..., ::-1]
    normals = nor[..., ::-1]
    light_source = np.array([1.0, 0.0, 0.0])
    mask = np.zeros(semantics.shape[:2], dtype=bool)
    for color in classes_to_keep:
        mask |= (semantics == np.array(color)).all(-1)
    mask_exp = mask[..., None]
    composed = np.where(mask_exp, semantics, instances)
    normals_f = normals.astype(np.float32) / 255.0
    shading = np.dot(normals_f, light_source)
    shaded_seg = (composed.astype(np.float32) * shading[..., None]).astype(np.uint8)
    return shaded_seg


def create_shuffled_colormap(
    size=65536, base_cmap_name='prism', seed=None, fix_zero=True
) -> np.ndarray:
    import matplotlib.pyplot as plt
    if seed is not None:
        np.random.seed(seed)
    try:
        cmap_func = plt.get_cmap(base_cmap_name)
    except ValueError:
        cmap_func = plt.get_cmap('turbo')
    base_colors = cmap_func(np.linspace(0, 1, size))[:, :3]
    indices = np.arange(size)
    if fix_zero:
        shuffled = np.concatenate(([0], np.random.permutation(indices[1:])))
    else:
        shuffled = np.random.permutation(indices)
    shuffled_colors = base_colors[shuffled]
    colormap_uint8 = (shuffled_colors * 255).astype(np.uint8)
    if fix_zero:
        colormap_uint8[0] = [0, 0, 0]
    return colormap_uint8


def reconstruct_ids_vectorized(image_data_uint8: np.ndarray) -> np.ndarray:
    low = image_data_uint8[:, :, 1].astype(np.uint16)
    high = image_data_uint8[:, :, 2].astype(np.uint16)
    return (high << 8) | low


def apply_colormap_vectorized(ids_uint16: np.ndarray, colormap: np.ndarray) -> np.ndarray:
    return colormap[ids_uint16]


def depth_to_log_grayscale(
    depth_map: np.ndarray,
    near_clip=0.01,
    far_clip=1000.0,
    inverted_depth=True
) -> Image.Image:
    clipped = np.clip(depth_map, near_clip, far_clip)
    log_depth = np.log(clipped)
    norm_log = (log_depth - np.log(near_clip)) / (np.log(far_clip) - np.log(near_clip))
    if inverted_depth:
        norm_log = 1.0 - norm_log
    gray_img = (norm_log * 255).astype(np.uint8)
    return Image.fromarray(gray_img)

# Pre-generate colormap for instance segmentation
colormap_uint8 = create_shuffled_colormap(seed=140)

# === SENSOR INFO WRAPPER ===
class SensorInfo:
    def __init__(self, sensor, stype: AOV):
        self.sensor = sensor
        self.sensor_type = stype
        self.queue = mp.Queue()
        sensor.listen(self._callback)

    def _callback(self, data):
        conv_map = {
            AOV.RGB: carla.ColorConverter.Raw,
            AOV.NORMALS: carla.ColorConverter.Raw,
            AOV.SEMANTIC_SEGMENTATION: carla.ColorConverter.CityScapesPalette
        }
        conv = conv_map.get(self.sensor_type, carla.ColorConverter.Raw)
        data.convert(conv)
        arr = np.frombuffer(data.raw_data, dtype=np.uint8)
        h, w = data.height, data.width
        raw = arr.reshape((h, w, 4))
        img = raw if self.sensor_type == AOV.DEPTH else raw[:, :, :3]
        self.queue.put((img.copy(), data.frame, data.timestamp))

    def capture_current_frame(self):
        try:
            return self.queue.get(timeout=1.0)
        except Exception:
            return None

# === WORKERS ===

def post_processing_worker(raw_q: mp.Queue, proc_q: mp.Queue):
    logging.info(f"[{mp.current_process().name}] starting")
    while True:
        bundle = raw_q.get()
        if bundle is None:
            break
        processed = {}
        frames = bundle.frames
        if AOV.RGB in frames:
            processed['RGB'] = frames[AOV.RGB]
        if AOV.RGB in frames and AOV.SEMANTIC_SEGMENTATION in frames:
            masked, edges = masked_edges_from_semseg(
                frames[AOV.RGB], frames[AOV.SEMANTIC_SEGMENTATION], CLASSES_TO_KEEP_CANNY
            )
            processed['RGB_MASKED'] = masked
            processed['RGB_EDGES'] = cv2.cvtColor(edges, cv2.COLOR_GRAY2RGB)
        if AOV.DEPTH in frames:
            depth_bgra = frames[AOV.DEPTH]
            scales = np.array([65536.0, 256.0, 1.0, 0.0]) / (256**3 - 1) * 1000
            depth_map = np.dot(depth_bgra, scales).astype(np.float32)
            gray_img = depth_to_log_grayscale(depth_map)
            processed['DEPTH'] = np.array(gray_img.convert('RGB'))
        if AOV.SEMANTIC_SEGMENTATION in frames:
            processed['SEMANTIC_SEGMENTATION'] = frames[AOV.SEMANTIC_SEGMENTATION]
        if AOV.INSTANCE_SEGMENTATION in frames:
            ids = reconstruct_ids_vectorized(frames[AOV.INSTANCE_SEGMENTATION])
            colored = apply_colormap_vectorized(ids, colormap_uint8)
            processed['INSTANCE_SEGMENTATION'] = colored
        proc_q.put((bundle.index, processed))
    logging.info(f"[{mp.current_process().name}] exiting")


def video_writer_worker(proc_q: mp.Queue, out_dir: Path, fps: float):
    logging.info("[Writer] starting")
    writers = {}
    paths = {}
    write_count = 0

    def get_writer(key: str, shape: Tuple[int, int]):
        if key not in writers:
            tmp = out_dir / f"{key.lower()}_tmp.mp4"
            final = out_dir / f"{key.lower()}.mp4"
            fourcc = cv2.VideoWriter_fourcc(*'mp4v')
            w = cv2.VideoWriter(str(tmp), fourcc, fps, (shape[1], shape[0]))
            writers[key] = w
            paths[key] = (tmp, final)
        return writers[key]

    while True:
        item = proc_q.get()
        if item is None:
            break
        idx, frames = item
        for key, img in frames.items():
            get_writer(key, img.shape[:2]).write(img)
            write_count += 1
            if write_count % 100 == 0:
                logging.info(f"[Writer] wrote {write_count} frames total")

    for key, w in writers.items():
        w.release()
        tmp, final = paths[key]
        try:
            subprocess.run(['ffmpeg', '-i', str(tmp), '-r', '24', '-c:v', 'libx264', 
            '-y',  '-loglevel', 'error', str(final)], check=True, 
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        except subprocess.CalledProcessError as e:
            logging.error(f"FFmpeg failed for {key}: {e}")
        tmp.unlink(missing_ok=True)
    logging.info("[Writer] exiting")

# === DYNAMIC OBJECT EXTRACTION (RDS-HQ FORMAT) ===
def extract_dynamic_objects_data(world, ego_vehicle_id=None):
    objects_data = {}

    vehicles = world.get_actors().filter('vehicle.*')
    for vehicle in vehicles:
        if ego_vehicle_id is not None and vehicle.id == ego_vehicle_id:
            continue

        bbox = vehicle.bounding_box
        transform = vehicle.get_transform()

        object_to_world = build_transform_matrix(
            transform.location, transform.rotation, bbox.extent.z
        )

        object_lwh = [bbox.extent.x * 2.0, bbox.extent.y * 2.0, bbox.extent.z * 2.0]
        semantic_label = vehicle.semantic_tags[0] if vehicle.semantic_tags else 14  # Default to Car
        object_type = SEMANTIC_LABEL_TO_OBJECT_TYPE.get(semantic_label, "Automobile")

        objects_data[str(vehicle.id)] = {
            "object_to_world": object_to_world,
            "object_lwh": object_lwh,
            "object_is_moving": True,
            "object_type": object_type
        }

    walkers = world.get_actors().filter('walker.pedestrian.*')
    for walker in walkers:
        bbox = walker.bounding_box
        transform = walker.get_transform()

        object_to_world = build_transform_matrix(
            transform.location, transform.rotation, bbox.extent.z
        )

        object_lwh = [bbox.extent.x * 2.0, bbox.extent.y * 2.0, bbox.extent.z * 2.0]

        objects_data[str(walker.id)] = {
            "object_to_world": object_to_world,
            "object_lwh": object_lwh,
            "object_is_moving": True,
            "object_type": "Pedestrian"
        }

    return objects_data

def export_dynamic_objects_data(dynamic_frames, session_id, output_dir):
    objects_dir = output_dir / "all_object_info"
    objects_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        json_files = []
        for frame_idx, frame_data in enumerate(dynamic_frames):
            filename = f"{session_id}.{frame_idx:06d}.all_object_info.json"
            json_file = temp_path / filename

            with open(json_file, 'w') as f:
                json.dump(frame_data, f, separators=(',', ':'))  # Compact format

            json_files.append((filename, str(json_file)))

        tar_filename = f"{session_id}.tar"
        tar_path = objects_dir / tar_filename

        with tarfile.open(tar_path, 'w') as tar:
            for filename, filepath in json_files:
                tar.add(filepath, arcname=filename)

        return True

def extract_camera_poses(world, frame_number, camera_sensor=None):
    if camera_sensor is None:
        logging.warning(f"Frame {frame_number}: No camera sensor provided, skipping pose extraction")
        return None

    transform = camera_sensor.get_transform()

    loc = transform.location
    cr, sr, cp, sp, cy, sy = compute_rotation_components(transform.rotation)

    R = np.array([
        [cy * cp, cy * sp * sr - sy * cr, cy * sp * cr + sy * sr],
        [sy * cp, sy * sp * sr + cy * cr, sy * sp * cr - cy * sr],
        [-sp, cp * sr, cp * cr]
    ])

    pose_carla = np.eye(4)
    pose_carla[:3, :3] = R
    pose_carla[:3, 3] = [loc.x, loc.y, loc.z]

    pose_opencv = np.concatenate([
        pose_carla[:, 1:2],   # X = Y_carla (right)
        -pose_carla[:, 2:3],  # Y = -Z_carla (down)
        pose_carla[:, 0:1],   # Z = X_carla (forward)
        pose_carla[:, 3:4]
    ], axis=1)

    return pose_opencv

def export_camera_pose_data(pose_frames, session_id, output_dir):
    pose_dir = output_dir / "pose"
    pose_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        npy_files = []
        for frame_idx, pose_matrix in enumerate(pose_frames):
            if pose_matrix is not None:
                filename = f"{session_id}.{frame_idx:06d}.pose.rds_hq.npy"
                npy_file = temp_path / filename

                np.save(npy_file, pose_matrix)
                npy_files.append((filename, str(npy_file)))

        tar_filename = f"{session_id}.tar"
        tar_path = pose_dir / tar_filename

        with tarfile.open(tar_path, 'w') as tar:
            for filename, filepath in npy_files:
                tar.add(filepath, arcname=filename)

        logging.info(f"Exported {len(npy_files)} frames of camera pose data to {tar_path}")
        return True

# === CAMERA INTRINSICS EXPORT ===
def extract_camera_intrinsics_from_sensor(sensor):
    attributes = sensor.attributes

    image_width = int(attributes.get('image_size_x', 1920))
    image_height = int(attributes.get('image_size_y', 1080))
    fov = float(attributes.get('fov', 90.0))

    focal = image_width / (2.0 * np.tan(fov * np.pi / 360.0))

    K = np.identity(3)
    K[0, 0] = focal  # fx
    K[1, 1] = focal  # fy
    K[0, 2] = image_width / 2.0   # cx
    K[1, 2] = image_height / 2.0  # cy

    return K, image_width, image_height, fov

def export_camera_intrinsincs_pinhole(K, width, height, session_id, output_dir):
    pinhole_dir = output_dir / "pinhole_intrinsic"
    pinhole_dir.mkdir(parents=True, exist_ok=True)

    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        # PinholeCamera expects [fx, fy, cx, cy, w, h]
        intrinsic_data = np.array([
            K[0,0],    # fx
            K[1,1],    # fy
            K[0,2],    # cx
            K[1,2],    # cy
            width,     # w
            height     # h
        ], dtype=np.float32)

        filename = f"{session_id}.pinhole_intrinsic.rds_hq.npy"
        npy_file = temp_path / filename
        np.save(npy_file, intrinsic_data)

        tar_path = pinhole_dir / f"{session_id}.tar"
        with tarfile.open(tar_path, 'w') as tar:
            tar.add(npy_file, arcname=filename)

    logging.info(f"Exported pinhole camera intrinsics to {tar_path}")

    return True


# === DATASET CONFIG EXPORT ===
def export_dataset_config(session_id, output_dir, rds_hq_camera_name="rds_hq", input_fps=30, target_render_fps=24, total_frames=None, chunk_frames=None, image_width=1280, image_height=1080):
    """
    Export dataset configuration JSON for RDS-HQ renderer.

    Args:
        session_id: Session identifier
        output_dir: Output directory path
        rds_hq_camera_name: Name of the RDS-HQ camera sensor
        input_fps: FPS of the exported pose data (native CARLA recording FPS)
        target_render_fps: Desired output video FPS (renderer will downsample)
        total_frames: Total number of frames exported (used to calculate chunk size)
        chunk_frames: Target chunk frame size (None = use full video, -1 = default 121, or specify exact value)
        image_width: Camera sensor width from sensor config (default: 1280)
        image_height: Camera sensor height from sensor config (default: 1080)

    Returns:
        Path to exported config file
    """
    if total_frames is not None:
        downsample_ratio = input_fps // target_render_fps
        downsampled_frames = total_frames // downsample_ratio

        if chunk_frames is None:
            chunk_frame = downsampled_frames
        else:
            chunk_frame = min(chunk_frames, downsampled_frames)
    else:
        chunk_frame = 121

    config = {
        "CAMERAS": [rds_hq_camera_name],
        "MINIMAP_TYPES": [
            "lanelines",
            "road_boundaries",
            "crosswalks",
            "road_markings",
            "wait_lines",
            "poles",
            "traffic_signs",
            "traffic_lights"
        ],
        "INPUT_POSE_FPS": input_fps,  # Native recording FPS
        "INPUT_LIDAR_FPS": 10,
        "GT_VIDEO_FPS": input_fps,
        "COSMOS_RESOLUTION": [image_width, image_height],
        "NOT_POST_TRAINING": {
            "RESIZE_RESOLUTION": [image_width, image_height],
            "TO_COSMOS_RESOLUTION": "resize",
            "TARGET_CHUNK_FRAME": chunk_frame,
            "OVERLAP_FRAME": 0,
            "TARGET_RENDER_FPS": target_render_fps  # Output video FPS
        },
        "POST_TRAINING": {
            "RESIZE_RESOLUTION": [image_width, image_height],
            "TO_COSMOS_RESOLUTION": "center-crop",
            "TARGET_CHUNK_FRAME": chunk_frame,
            "OVERLAP_FRAME": 0,
            "TARGET_RENDER_FPS": target_render_fps
        }
    }

    config_path = output_dir / f"dataset_{session_id}.json"
    with open(config_path, 'w') as f:
        json.dump(config, f, indent=2)

    logging.info(f"Exported dataset config to {config_path}")
    return str(config_path)


# === HD MAP RENDERING ===
def render_hdmap_video(session_id, rds_hq_dir, output_dir):
    """
    Render HD map video using the Cosmos-Drive-Dreams toolkit.

    Args:
        session_id: Session identifier
        rds_hq_dir: Directory containing RDS-HQ export data
        output_dir: Output directory for rendered videos

    Returns:
        True if rendering succeeded, False otherwise
    """
    try:
        script_dir = Path(__file__).parent.resolve()
        # Go up one level from client/ to cosmos/, then into utils/
        toolkit_path = script_dir.parent / "utils" / "cosmos-drive-dreams" / "cosmos-drive-dreams-toolkits"
        render_script = toolkit_path / "render_from_rds_hq.py"

        if not toolkit_path.exists() or not render_script.exists():
            logging.error(f"Cosmos-Drive-Dreams toolkit not found at {toolkit_path}")
            logging.error("Please initialize the submodule: git submodule update --init --recursive")
            return False

        config_path = rds_hq_dir / f"dataset_{session_id}.json"
        if not config_path.exists():
            logging.error(f"Dataset config not found: {config_path}")
            return False

        toolkit_config_dir = toolkit_path / "config"
        toolkit_config_dir.mkdir(exist_ok=True)
        toolkit_config_path = toolkit_config_dir / f"dataset_{session_id}.json"

        shutil.copy(str(config_path), str(toolkit_config_path))
        logging.info(f"Copied dataset config to {toolkit_config_path}")

        logging.info(f"Starting HD map video rendering for session: {session_id}")
        logging.info(f"Input directory: {rds_hq_dir}")
        logging.info(f"Output directory: {output_dir}")

        cmd = [
            sys.executable,
            str(render_script),
            '-i', str(rds_hq_dir),
            '-o', str(output_dir),
            '-cj', session_id,
            '-d', session_id,
            '-c', 'pinhole',
            '-s', 'lidar'  # Skip lidar rendering (we don't export lidar data)
        ]

        result = subprocess.run(
            cmd,
            cwd=str(toolkit_path),  # Run from toolkit directory so relative paths work
            check=True,
            capture_output=True,
            text=True
        )

        if result.stdout:
            logging.info(f"Renderer output:\n{result.stdout}")

        logging.info(f"Successfully rendered HD map video for session: {session_id}")
        return True

    except subprocess.CalledProcessError as e:
        logging.error(f"Renderer failed with exit code {e.returncode}")
        if e.stdout:
            logging.error(f"Stdout: {e.stdout}")
        if e.stderr:
            logging.error(f"Stderr: {e.stderr}")
        return False
    except Exception as e:
        logging.error(f"Failed to render HD map video: {e}")
        logging.error(f"Full traceback: {traceback.format_exc()}")
        return False


# === RDS-HQ EXPORT ===
def export_rds_hq_clip(world, args, log_frames, log_duration, dynamic_frames=None, pose_frames=None, camera_intrinsics=None):
    rds_hq_dir = Path(args.output_dir) / "rds-hq"
    rds_hq_dir.mkdir(parents=True, exist_ok=True)

    log_file_base = Path(args.recorder_filename).stem
    log_file_base_sanitized = log_file_base.replace('.', '_')

    start_time_us = int(args.start * 1000000)
    end_time_us = int((args.start + (args.duration if args.duration > 0 else log_duration)) * 1000000)
    session_id = f"{log_file_base_sanitized}_{start_time_us}_{end_time_us}"

    logging.info(f"Exporting RDS-HQ clip with session ID: {session_id}")
    logging.info(f"Output directory: {rds_hq_dir}")

    recording_fps = round(1.0 / (log_duration / log_frames))

    try:
        logging.info("Starting export of all static cosmos elements...")
        static_exports = [
            ("crosswalks", world.export_cosmos_crosswalks, "3d_crosswalks"),
            ("road_boundaries", world.export_cosmos_road_boundaries, "3d_road_boundaries"),
            ("lane_lines", world.export_cosmos_lane_lines, "3d_lanelines"),  # Note: no underscore for compatibility
            ("traffic_signs", world.export_cosmos_traffic_signs, "3d_traffic_signs"),
            ("wait_lines", world.export_cosmos_wait_lines, "3d_wait_lines"),
            ("road_markings", world.export_cosmos_road_markings, "3d_road_markings")
        ]

        successful_exports = []
        failed_exports = []
        static_directories = {}

        for export_name, export_func, dir_name in static_exports:
            try:
                logging.info(f"Attempting to export {export_name}...")
                result = export_func(session_id, str(rds_hq_dir) + "/")
                logging.info(f"Successfully exported {export_name}: {result}")
                successful_exports.append(export_name)
                static_directories[export_name] = dir_name
            except Exception as e:
                logging.error(f"Failed to export {export_name}: {e}")
                logging.error(f"Full traceback: {traceback.format_exc()}")
                failed_exports.append((export_name, str(e)))

        for dir_path in rds_hq_dir.glob("3d_*"):
            if dir_path.is_dir():
                try:
                    json_files = list(dir_path.glob("*.json"))

                    if json_files:
                        tar_filename = f"{session_id}.tar"
                        tar_path = dir_path / tar_filename

                        with tarfile.open(tar_path, 'w') as tar:
                            for json_file in json_files:
                                tar.add(json_file, arcname=json_file.name)

                        logging.info(f"Created tar archive {tar_path} with {len(json_files)} JSON files")

                        for json_file in json_files:
                            json_file.unlink()
                        logging.info(f"Removed {len(json_files)} JSON files from {dir_path.name}")
                    else:
                        logging.debug(f"No JSON files found in {dir_path}")
                except Exception as e:
                    logging.error(f"Failed to create tar archive for {dir_path.name}: {e}")

        if dynamic_frames:
            try:
                actual_frame_count = len(dynamic_frames)
                calculated_fps = actual_frame_count / (args.duration if args.duration > 0 else log_duration)
                logging.info(f"Exporting {actual_frame_count} frames of dynamic objects (calculated fps: {calculated_fps:.2f})...")
                if export_dynamic_objects_data(dynamic_frames, session_id, rds_hq_dir):
                    successful_exports.append("dynamic_objects")
            except Exception as e:
                logging.error(f"Failed to export dynamic objects: {e}")
                failed_exports.append(("dynamic_objects", str(e)))

        if pose_frames:
            try:
                actual_frame_count = len(pose_frames)
                calculated_fps = actual_frame_count / (args.duration if args.duration > 0 else log_duration)
                logging.info(f"Exporting {actual_frame_count} frames of camera pose data (calculated fps: {calculated_fps:.2f})...")
                if export_camera_pose_data(pose_frames, session_id, rds_hq_dir):
                    successful_exports.append("camera_poses")
            except Exception as e:
                logging.error(f"Failed to export camera poses: {e}")
                failed_exports.append(("camera_poses", str(e)))

        if camera_intrinsics:
            try:
                logging.info("Exporting camera intrinsics...")
                K, width, height, fov = camera_intrinsics
                if export_camera_intrinsincs_pinhole(K, width, height, session_id, rds_hq_dir):
                    successful_exports.append("camera_intrinsics")
            except Exception as e:
                logging.error(f"Failed to export camera intrinsics: {e}")
                failed_exports.append(("camera_intrinsics", str(e)))

        # Create metadata file with actual exported frame counts
        actual_exported_frames = len(dynamic_frames) if dynamic_frames else 0
        actual_duration = args.duration if args.duration > 0 else log_duration
        actual_fps = actual_exported_frames / actual_duration if actual_duration > 0 else 0

        metadata = {
            "session_id": session_id,
            "carla_version": "0.9.15",
            "recorder_file": args.recorder_filename,
            "start_time": args.start,
            "duration": actual_duration,
            "total_frames_in_recording": log_frames,
            "exported_frames": actual_exported_frames,
            "recording_fps": round(1.0 / (log_duration / log_frames)),
            "exported_fps": round(actual_fps),
            "camera_actor_id": args.camera,
            "time_factor": args.time_factor,
            "output_structure": {
                "3d_crosswalks": f"{session_id}.tar",
                "3d_road_boundaries": f"{session_id}.tar",
                "3d_lanelines": f"{session_id}.tar",
                "3d_traffic_signs": f"{session_id}.tar",
                "3d_wait_lines": f"{session_id}.tar",
                "3d_road_markings": f"{session_id}.tar",
                "all_object_info": f"{session_id}.tar",
                "pose": f"{session_id}.tar",
                "pinhole_intrinsic": f"{session_id}.tar"
            }
        }

        metadata_file = rds_hq_dir / f"{session_id}_metadata.json"
        with open(metadata_file, 'w') as f:
            json.dump(metadata, f, indent=2)

        logging.info("Exporting dataset config for RDS-HQ renderer...")
        # Extract image dimensions from camera intrinsics
        K, width, height, fov = camera_intrinsics if camera_intrinsics else (None, 1280, 1080, 90.0)
        export_dataset_config(
            session_id=session_id,
            output_dir=rds_hq_dir,
            rds_hq_camera_name="rds_hq",
            input_fps=recording_fps,
            target_render_fps=24,
            total_frames=actual_exported_frames,
            image_width=int(width),
            image_height=int(height)
        )

    except Exception as e:
        logging.error(f"Failed to export RDS-HQ clip: {e}")
        raise

# === MAIN ===
def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--sensors', type=str, required=True)
    parser.add_argument('--class-filter-config', type=str)
    parser.add_argument('-f','--recorder-filename', type=str, required=True)
    parser.add_argument('-o','--output-dir', type=str, required=True)
    parser.add_argument('-s','--start', type=float, default=0.0)
    parser.add_argument('-d','--duration', type=float, default=0.0)
    parser.add_argument('--host', type=str, default='127.0.0.1')
    parser.add_argument('--port', type=int, default=2000)
    parser.add_argument('-c','--camera', type=int, default=0)
    parser.add_argument('--time-factor', type=float, default=1.0)
    parser.add_argument('--ignore-hero', action='store_true')
    parser.add_argument('--move-spectator', action='store_true')
    parser.add_argument('--spawn-sensors', action='store_true')
    parser.add_argument('--num-post-workers', type=int, default=max(1, mp.cpu_count()-1))
    parser.add_argument('--skip-render-hdmap', action='store_true', help='Skip automatic HD map video rendering')
    args = parser.parse_args()

    logging.basicConfig(
        level=logging.INFO,
        format='%(asctime)s %(levelname)s %(processName)s: %(message)s'
    )
    logging.info("Starting CarlaCosmos-DataAcquisition with RDS-HQ export")

    if args.class_filter_config:
        load_class_filter_config(args.class_filter_config)

    client = carla.Client(args.host, args.port)
    client.set_timeout(60.0)
    client.reload_world()
    
    recorder_filename = Path(args.recorder_filename).resolve()
    info = client.show_recorder_file_info(str(recorder_filename), False)
    log_frames, log_duration = parse_frames_duration(info)

    log_delta = log_duration / log_frames
    fps = round(1.0 / log_delta)
    logging.info(f"Recorder: {log_frames} frames, {log_duration:.2f}s, fps={fps}")

    client.set_replayer_time_factor(args.time_factor)
    client.set_replayer_ignore_hero(args.ignore_hero)
    client.set_replayer_ignore_spectator(not args.move_spectator)
    client.replay_file(
        str(recorder_filename), args.start, args.duration, args.camera, args.spawn_sensors
    )

    world = client.get_world()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = log_delta
    world.apply_settings(settings)

    sensors_filepath = Path(args.sensors.replace('file:','')).resolve()
    with open(sensors_filepath, 'r') as f:
        sensor_cfg = yaml.safe_load(f)
    vehicle = world.get_actor(args.camera)
    sensor_infos = []
    for entry in sensor_cfg:
        sensor_type = entry['sensor']
        carla_sensor_type = 'rgb' if sensor_type == 'rds_hq' else sensor_type

        sensor_name = f"sensor.camera.{carla_sensor_type}"

        if entry.get('wide_angle_lens', False):
            sensor_name += '.wide_angle_lens'

        bp = world.get_blueprint_library().find(sensor_name)

        attributes = entry.get('attributes', {})
        for k,v in attributes.items():
            bp.set_attribute(k, str(v))

        tf = entry.get('transform',{})
        transform = carla.Transform(
            carla.Location(**tf.get('location',{})),
            carla.Rotation(**tf.get('rotation',{}))
        )
        sensor = world.spawn_actor(bp, transform, attach_to=vehicle)

        sensor_infos.append(SensorInfo(sensor, AOV[sensor_type.upper()]))

    raw_q = mp.Queue()
    proc_q = mp.Queue()
    workers = []
    for i in range(args.num_post_workers):
        p = mp.Process(
            target=post_processing_worker,
            args=(raw_q, proc_q),
            name=f"PostProc-{i}"
        )
        p.start(); workers.append(p)

    out_dir = Path(args.output_dir).resolve()
    out_dir.mkdir(parents=True, exist_ok=True)
    writer = mp.Process(
        target=video_writer_worker,
        args=(proc_q, out_dir, fps),
        name="Writer"
    )
    writer.start()

    timestamp = args.start
    total = log_duration if args.duration == 0.0 else args.duration
    frame_count = 0
    dynamic_frames = []
    pose_frames = []

    has_rds_hq_sensor = any(si.sensor_type == AOV.RDS_HQ for si in sensor_infos)
    camera_intrinsics = None

    if has_rds_hq_sensor:
        for si in sensor_infos:
            if si.sensor_type == AOV.RDS_HQ:
                K, width, height, fov = extract_camera_intrinsics_from_sensor(si.sensor)
                camera_intrinsics = (K, width, height, fov)
                break
        logging.info("RDS-HQ sensor found - will export RDS-HQ data")
    else:
        logging.info("No RDS-HQ sensor found - skipping RDS-HQ export")

    try:
        while timestamp < args.start + total:
            idx = world.tick()

            if has_rds_hq_sensor:
                dynamic_objects = extract_dynamic_objects_data(world, args.camera)
                dynamic_frames.append(dynamic_objects)

                main_camera_sensor = None
                for si in sensor_infos:
                    if si.sensor_type == AOV.RDS_HQ:
                        main_camera_sensor = si.sensor
                        break

                camera_pose = extract_camera_poses(world, frame_count, main_camera_sensor)
                pose_frames.append(camera_pose)

            frame_dict = {}
            for si in sensor_infos:
                res = si.capture_current_frame()
                if res:
                    img,_,_ = res
                    frame_dict[si.sensor_type] = img
            raw_q.put(FrameBundle(idx, frame_dict, timestamp))

            frame_count += 1
            if frame_count % 100 == 0:
                rds_frames_collected = len(dynamic_frames)
                logging.info(f"Queued frame {frame_count}, timestamp={timestamp:.3f}, idx={idx}, RDS-HQ frames={rds_frames_collected}")
            timestamp += log_delta
    finally:
        for _ in workers: raw_q.put(None)
        for p in workers: p.join()
        proc_q.put(None); writer.join()

        if has_rds_hq_sensor:
            export_rds_hq_clip(world, args, log_frames, log_duration, dynamic_frames, pose_frames, camera_intrinsics)

            # Automatically render HD map video unless --skip-render-hdmap is specified
            if not args.skip_render_hdmap:
                rds_hq_dir = Path(args.output_dir) / "rds-hq"
                log_file_base = Path(args.recorder_filename).stem
                log_file_base_sanitized = log_file_base.replace('.', '_')
                start_time_us = int(args.start * 1000000)
                end_time_us = int((args.start + (args.duration if args.duration > 0 else log_duration)) * 1000000)
                session_id = f"{log_file_base_sanitized}_{start_time_us}_{end_time_us}"

                render_hdmap_video(session_id, rds_hq_dir, Path(args.output_dir))
            else:
                logging.info("Skipping HD map video rendering (--skip-render-hdmap flag set)")
        else:
            logging.info("Skipping RDS-HQ export (no rds_hq sensor defined)")

        client.stop_replayer(keep_actors=False)
        for si in sensor_infos: si.sensor.stop(); si.sensor.destroy()
        settings.synchronous_mode = False; settings.fixed_delta_seconds = None; world.apply_settings(settings)

if __name__ == '__main__':
    main()
