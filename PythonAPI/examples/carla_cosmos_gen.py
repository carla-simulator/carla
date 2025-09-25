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

from PIL import Image

# === ENUMS AND DATA STRUCTURES ===
class AOV(Enum):
    RGB = 0
    DEPTH = 1
    SEMANTIC_SEGMENTATION = 2
    INSTANCE_SEGMENTATION = 3
    NORMALS = 4
    COSMOS_VISUALIZATION = 5

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

# === CONFIGURATION LOADERS ===
CLASSES_TO_KEEP_SHADED_SEG: List[Sequence[int]] = []
CLASSES_TO_KEEP_CANNY: List[Sequence[int]] = []

def load_class_filter_config(path: str):
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
            AOV.SEMANTIC_SEGMENTATION: carla.ColorConverter.CityScapesPalette,
            AOV.COSMOS_VISUALIZATION: carla.ColorConverter.Raw
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
        if AOV.COSMOS_VISUALIZATION in frames:
            processed['COSMOS_VISUALIZATION'] = frames[AOV.COSMOS_VISUALIZATION]
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

# === DYNAMIC OBJECT EXTRACTION (COSMOS FORMAT) ===
def extract_dynamic_objects_cosmos_format(world, frame_number):
    objects_data = {}

    # Get all vehicles
    vehicles = world.get_actors().filter('vehicle.*')
    for vehicle in vehicles:
        bbox = vehicle.bounding_box
        transform = vehicle.get_transform()

        # Convert to transformation matrix (object_to_world)
        loc = transform.location
        rot = transform.rotation

        # Convert degrees to radians
        roll = math.radians(rot.roll)
        pitch = math.radians(rot.pitch)
        yaw = math.radians(rot.yaw)

        # Create rotation matrix from Euler angles
        cos_roll = math.cos(roll)
        sin_roll = math.sin(roll)
        cos_pitch = math.cos(pitch)
        sin_pitch = math.sin(pitch)
        cos_yaw = math.cos(yaw)
        sin_yaw = math.sin(yaw)

        adjusted_z = loc.z + bbox.extent.z  # Move up by half the height

        object_to_world = [
            [cos_yaw * cos_pitch, cos_yaw * sin_pitch * sin_roll - sin_yaw * cos_roll, cos_yaw * sin_pitch * cos_roll + sin_yaw * sin_roll, loc.x],
            [sin_yaw * cos_pitch, sin_yaw * sin_pitch * sin_roll + cos_yaw * cos_roll, sin_yaw * sin_pitch * cos_roll - cos_yaw * sin_roll, loc.y],
            [-sin_pitch, cos_pitch * sin_roll, cos_pitch * cos_roll, adjusted_z],
            [0.0, 0.0, 0.0, 1.0]
        ]

        # Get velocity to determine if moving
        velocity = vehicle.get_velocity()
        speed = math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2)
        is_moving = speed > 0.1  # Moving if speed > 0.1 m/s

        # Object dimensions: [length, width, height]
        object_lwh = [
            bbox.extent.x * 2.0,  # Length
            bbox.extent.y * 2.0,  # Width
            bbox.extent.z * 2.0   # Height
        ]

        objects_data[str(vehicle.id)] = {
            "object_to_world": object_to_world,
            "object_lwh": object_lwh,
            "object_is_moving": is_moving,
            "object_type": "Automobile",
            "aux_info": {
                "trackline_id": str(vehicle.id),
                "category": "automobile",
                "egomotion_label_class_id": "carla:generated:v0",
                "mounted": False,
                "has_trailer": False,
                "has_protrusion": False,
                "automobile_type": "other",
                "truck_type": "",
                "bus_type": "",
                "puller_type": "",
                "rider_type": "",
                "alive": True,
                "parent_obstacle_label_id": "",
                "lidar_sensor": "",
                "blueprint_id": vehicle.type_id
            }
        }

    # Get all pedestrians
    walkers = world.get_actors().filter('walker.pedestrian.*')
    for walker in walkers:
        bbox = walker.bounding_box
        transform = walker.get_transform()

        # Convert to transformation matrix
        loc = transform.location
        rot = transform.rotation

        # Convert degrees to radians and create transformation matrix
        roll = math.radians(rot.roll)
        pitch = math.radians(rot.pitch)
        yaw = math.radians(rot.yaw)

        cos_roll = math.cos(roll)
        sin_roll = math.sin(roll)
        cos_pitch = math.cos(pitch)
        sin_pitch = math.sin(pitch)
        cos_yaw = math.cos(yaw)
        sin_yaw = math.sin(yaw)

        adjusted_z = loc.z + bbox.extent.z  # Move up by half the height

        object_to_world = [
            [cos_yaw * cos_pitch, cos_yaw * sin_pitch * sin_roll - sin_yaw * cos_roll, cos_yaw * sin_pitch * cos_roll + sin_yaw * sin_roll, loc.x],
            [sin_yaw * cos_pitch, sin_yaw * sin_pitch * sin_roll + cos_yaw * cos_roll, sin_yaw * sin_pitch * cos_roll - cos_yaw * sin_roll, loc.y],
            [-sin_pitch, cos_pitch * sin_roll, cos_pitch * cos_roll, adjusted_z],
            [0.0, 0.0, 0.0, 1.0]
        ]

        # Get velocity to determine if moving
        velocity = walker.get_velocity()
        speed = math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2)
        is_moving = speed > 0.05  # Moving if speed > 0.05 m/s

        object_lwh = [
            bbox.extent.x * 2.0,  # Length
            bbox.extent.y * 2.0,  # Width
            bbox.extent.z * 2.0   # Height
        ]

        objects_data[str(walker.id)] = {
            "object_to_world": object_to_world,
            "object_lwh": object_lwh,
            "object_is_moving": is_moving,
            "object_type": "Pedestrian",
            "aux_info": {
                "trackline_id": str(walker.id),
                "category": "pedestrian",
                "egomotion_label_class_id": "carla:generated:v0",
                "mounted": False,
                "has_trailer": False,
                "has_protrusion": False,
                "automobile_type": "",
                "truck_type": "",
                "bus_type": "",
                "puller_type": "",
                "rider_type": "pedestrian",
                "alive": True,
                "parent_obstacle_label_id": "",
                "lidar_sensor": "",
                "blueprint_id": walker.type_id
            }
        }

    return objects_data

def export_dynamic_objects_cosmos_format(dynamic_frames, session_id, output_dir):
    # Create all_object_info directory
    objects_dir = output_dir / "all_object_info"
    objects_dir.mkdir(parents=True, exist_ok=True)

    # Create temporary directory for JSON files
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        # Create individual JSON files for each frame
        json_files = []
        for frame_idx, frame_data in enumerate(dynamic_frames):
            filename = f"{session_id}.{frame_idx:06d}.all_object_info.json"
            json_file = temp_path / filename

            with open(json_file, 'w') as f:
                json.dump(frame_data, f, separators=(',', ':'))  # Compact format

            json_files.append((filename, str(json_file)))

        # Create tar archive
        tar_filename = f"{session_id}.tar"
        tar_path = objects_dir / tar_filename

        with tarfile.open(tar_path, 'w') as tar:
            for filename, filepath in json_files:
                tar.add(filepath, arcname=filename)

        logging.info(f"Exported {len(dynamic_frames)} frames of dynamic objects to {tar_path}")
        return True

def extract_camera_poses(world, frame_number, camera_actor_id, camera_sensor=None):
    """Extract camera pose data for a single frame"""
    import numpy as np

    if camera_sensor is not None:
        # Use the actual camera sensor transform - this is the correct camera pose
        camera_transform = camera_sensor.get_transform()
        loc = camera_transform.location
        rot = camera_transform.rotation
    else:
        # Fallback: Get the ego vehicle (camera actor)
        ego_vehicle = world.get_actor(camera_actor_id)
        if ego_vehicle is None:
            return None

        # Get vehicle transform (fallback only)
        vehicle_transform = ego_vehicle.get_transform()
        loc = vehicle_transform.location
        rot = vehicle_transform.rotation

    import math
    roll = math.radians(rot.roll)
    pitch = math.radians(rot.pitch)
    yaw = math.radians(rot.yaw)

    # Create rotation matrix from Euler angles
    cos_roll = math.cos(roll)
    sin_roll = math.sin(roll)
    cos_pitch = math.cos(pitch)
    sin_pitch = math.sin(pitch)
    cos_yaw = math.cos(yaw)
    sin_yaw = math.sin(yaw)

    # Rotation matrix (ZYX Euler angle convention)
    R = np.array([
        [cos_yaw * cos_pitch, cos_yaw * sin_pitch * sin_roll - sin_yaw * cos_roll, cos_yaw * sin_pitch * cos_roll + sin_yaw * sin_roll],
        [sin_yaw * cos_pitch, sin_yaw * sin_pitch * sin_roll + cos_yaw * cos_roll, sin_yaw * sin_pitch * cos_roll - cos_yaw * sin_roll],
        [-sin_pitch, cos_pitch * sin_roll, cos_pitch * cos_roll]
    ])

    # Use CARLA coordinate system directly, same as C++ exporters and objects
    # Create 4x4 transformation matrix
    pose_matrix = np.eye(4)
    pose_matrix[:3, :3] = R
    pose_matrix[:3, 3] = [loc.x, loc.y, loc.z]  # Python API already returns values in meters

    return pose_matrix

def extract_vehicle_pose(world, frame_number, camera_actor_id, camera_sensor=None):
    """Extract ego vehicle pose data for a single frame"""
    # For now, vehicle pose is the same as camera pose since camera is mounted on ego vehicle
    return extract_camera_poses(world, frame_number, camera_actor_id, camera_sensor)

def export_pose_data(pose_frames, session_id, output_dir):
    """Export per-frame camera pose data in cosmos format (tar archive of .npy files)"""
    import numpy as np
    import tarfile
    import tempfile
    import os
    from io import BytesIO

    # Create pose directory
    pose_dir = output_dir / "pose"
    pose_dir.mkdir(parents=True, exist_ok=True)

    # Create temporary directory for npy files
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        # Create individual .npy files for each frame
        npy_files = []
        for frame_idx, pose_matrix in enumerate(pose_frames):
            if pose_matrix is not None:
                # Create webdataset compatible filename
                # Use session_id as prefix for webdataset compatibility
                filename = f"{session_id}.{frame_idx:06d}.pose.camera_front_wide_120fov.npy"
                npy_file = temp_path / filename

                # Save numpy array
                np.save(npy_file, pose_matrix)
                npy_files.append((filename, str(npy_file)))

        # Create tar archive
        tar_filename = f"{session_id}.tar"
        tar_path = pose_dir / tar_filename

        with tarfile.open(tar_path, 'w') as tar:
            for filename, filepath in npy_files:
                tar.add(filepath, arcname=filename)

        logging.info(f"Exported {len(npy_files)} frames of camera pose data to {tar_path}")
        return True

def export_vehicle_pose_data(vehicle_pose_frames, session_id, output_dir):
    """Export per-frame vehicle pose data in cosmos format (tar archive of .npy files)"""
    import numpy as np
    import tarfile
    import tempfile

    # Create vehicle_pose directory
    vehicle_pose_dir = output_dir / "vehicle_pose"
    vehicle_pose_dir.mkdir(parents=True, exist_ok=True)

    # Create temporary directory for npy files
    with tempfile.TemporaryDirectory() as temp_dir:
        temp_path = Path(temp_dir)

        # Create individual .npy files for each frame
        npy_files = []
        for frame_idx, pose_matrix in enumerate(vehicle_pose_frames):
            if pose_matrix is not None:
                # Use session_id as prefix for webdataset compatibility
                filename = f"{session_id}.{frame_idx:06d}.vehicle_pose.npy"
                npy_file = temp_path / filename

                # Save numpy array
                np.save(npy_file, pose_matrix)
                npy_files.append((filename, str(npy_file)))

        # Create tar archive
        tar_filename = f"{session_id}.tar"
        tar_path = vehicle_pose_dir / tar_filename

        with tarfile.open(tar_path, 'w') as tar:
            for filename, filepath in npy_files:
                tar.add(filepath, arcname=filename)

        logging.info(f"Exported {len(npy_files)} frames of vehicle pose data to {tar_path}")
        return True

# === RDS-HQ EXPORT ===
def export_rds_hq_clip(world, args, log_frames, log_duration, dynamic_frames=None, pose_frames=None, vehicle_pose_frames=None):
    """Export RDS-HQ compatible clip structure"""
    import os

    # Create RDS-HQ output directory
    rds_hq_dir = Path(args.output_dir) / "rds-hq"
    rds_hq_dir.mkdir(parents=True, exist_ok=True)

    # Generate session ID based on the log file name
    # Extract base name without extension from the recorder file
    log_file_base = Path(args.recorder_filename).stem

    # Sanitize the log file base name by replacing periods with underscores
    # This is crucial for webdataset compatibility as it uses periods as delimiters
    log_file_base_sanitized = log_file_base.replace('.', '_')

    # Create session ID with format: logfilename_starttime_endtime (in microseconds)
    start_time_us = int(args.start * 1000000)
    end_time_us = int((args.start + (args.duration if args.duration > 0 else log_duration)) * 1000000)
    session_id = f"{log_file_base_sanitized}_{start_time_us}_{end_time_us}"

    logging.info(f"Exporting RDS-HQ clip with session ID: {session_id}")
    logging.info(f"Output directory: {rds_hq_dir}")

    try:
        # Export static cosmos data
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
        static_directories = {}  # Track created directories for tar archiving

        for export_name, export_func, dir_name in static_exports:
            try:
                logging.info(f"Attempting to export {export_name}...")
                result = export_func(session_id, str(rds_hq_dir) + "/")
                logging.info(f"Successfully exported {export_name}: {result}")
                successful_exports.append(export_name)
                static_directories[export_name] = dir_name
            except Exception as e:
                logging.error(f"Failed to export {export_name}: {e}")
                import traceback
                logging.error(f"Full traceback: {traceback.format_exc()}")
                failed_exports.append((export_name, str(e)))

        # Create tar archives for all 3d_* directories with JSON files
        import tarfile
        import glob
        import os

        # Process all 3d_* directories
        for dir_path in rds_hq_dir.glob("3d_*"):
            if dir_path.is_dir():
                try:
                    # Find all JSON files in the directory
                    json_files = list(dir_path.glob("*.json"))

                    if json_files:
                        # Create tar archive
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

        # Export dynamic objects
        if dynamic_frames:
            try:
                logging.info(f"Exporting {len(dynamic_frames)} frames of dynamic objects...")
                if export_dynamic_objects_cosmos_format(dynamic_frames, session_id, rds_hq_dir):
                    successful_exports.append("dynamic_objects")
            except Exception as e:
                logging.error(f"Failed to export dynamic objects: {e}")
                failed_exports.append(("dynamic_objects", str(e)))

        # Export pose data
        if pose_frames:
            try:
                logging.info(f"Exporting {len(pose_frames)} frames of camera pose data...")
                if export_pose_data(pose_frames, session_id, rds_hq_dir):
                    successful_exports.append("camera_poses")
            except Exception as e:
                logging.error(f"Failed to export camera poses: {e}")
                failed_exports.append(("camera_poses", str(e)))

        # Export vehicle pose data
        if vehicle_pose_frames:
            try:
                logging.info(f"Exporting {len(vehicle_pose_frames)} frames of vehicle pose data...")
                if export_vehicle_pose_data(vehicle_pose_frames, session_id, rds_hq_dir):
                    successful_exports.append("vehicle_poses")
            except Exception as e:
                logging.error(f"Failed to export vehicle poses: {e}")
                failed_exports.append(("vehicle_poses", str(e)))

        # Create metadata file
        metadata = {
            "session_id": session_id,
            "carla_version": "0.9.15",
            "recorder_file": args.recorder_filename,
            "start_time": args.start,
            "duration": args.duration if args.duration > 0 else log_duration,
            "total_frames": log_frames,
            "fps": round(1.0 / (log_duration / log_frames)),
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
                "vehicle_pose": f"{session_id}.tar"
            }
        }

        import json
        metadata_file = rds_hq_dir / f"{session_id}_metadata.json"
        with open(metadata_file, 'w') as f:
            json.dump(metadata, f, indent=2)

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
    
    info = client.show_recorder_file_info(args.recorder_filename, False)
    log_frames, log_duration = parse_frames_duration(info)

    log_delta = log_duration / log_frames
    fps = round(1.0 / log_delta)
    logging.info(f"Recorder: {log_frames} frames, {log_duration:.2f}s, fps={fps}")

    client.set_replayer_time_factor(args.time_factor)
    client.set_replayer_ignore_hero(args.ignore_hero)
    client.set_replayer_ignore_spectator(not args.move_spectator)
    client.replay_file(
        args.recorder_filename, args.start, args.duration, args.camera, args.spawn_sensors
    )

    world = client.get_world()
    settings = world.get_settings()
    settings.synchronous_mode = True
    settings.fixed_delta_seconds = log_delta
    world.apply_settings(settings)

    with open(args.sensors.replace('file:',''), 'r') as f:
        sensor_cfg = yaml.safe_load(f)
    vehicle = world.get_actor(args.camera)
    sensor_infos = []
    for entry in sensor_cfg:
        bp = world.get_blueprint_library().find(f"sensor.camera.{entry['sensor']}")
        for k,v in entry.get('attributes',{}).items(): bp.set_attribute(k,str(v))
        tf = entry.get('transform',{})
        transform = carla.Transform(
            carla.Location(**tf.get('location',{})),
            carla.Rotation(**tf.get('rotation',{}))
        )
        sensor = world.spawn_actor(bp, transform, attach_to=vehicle)
        
        # If it's the cosmos visualization sensor, set it to ignore the ego vehicle
        if entry['sensor'].upper() == 'COSMOS_VISUALIZATION':
            sensor.set_ignored_vehicles([args.camera])  # Only this sensor ignores ego
        
        sensor_infos.append(SensorInfo(sensor, AOV[entry['sensor'].upper()]))

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

    out_dir = Path(args.output_dir)
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
    dynamic_frames = []  # Collect dynamic object data for each frame
    pose_frames = []     # Collect camera pose data for each frame
    vehicle_pose_frames = []  # Collect vehicle pose data for each frame

    try:
        while timestamp < args.start + total:
            idx = world.tick()

            # Extract dynamic objects for this frame (cosmos format)
            dynamic_objects = extract_dynamic_objects_cosmos_format(world, frame_count)
            dynamic_frames.append(dynamic_objects)

            # Find the main camera sensor (RGB camera for pose reference)
            main_camera_sensor = None
            for si in sensor_infos:
                if si.sensor_type == AOV.RGB:  # Use RGB camera as the pose reference
                    main_camera_sensor = si.sensor
                    break

            # Extract camera pose for this frame using the actual camera sensor
            camera_pose = extract_camera_poses(world, frame_count, args.camera, main_camera_sensor)
            pose_frames.append(camera_pose)

            # Extract vehicle pose for this frame using the same camera sensor
            vehicle_pose = extract_vehicle_pose(world, frame_count, args.camera, main_camera_sensor)
            vehicle_pose_frames.append(vehicle_pose)

            # Capture sensor frames
            frame_dict = {}
            for si in sensor_infos:
                res = si.capture_current_frame()
                if res:
                    img,_,_ = res
                    frame_dict[si.sensor_type] = img
            raw_q.put(FrameBundle(idx, frame_dict, timestamp))
            frame_count += 1
            if frame_count % 100 == 0:
                logging.info(f"Queued frame {frame_count}, timestamp={timestamp:.3f}, idx={idx}, objects={len(dynamic_objects)}")
            timestamp += log_delta
    finally:
        for _ in workers: raw_q.put(None)
        for p in workers: p.join()
        proc_q.put(None); writer.join()

        export_rds_hq_clip(world, args, log_frames, log_duration, dynamic_frames, pose_frames, vehicle_pose_frames)

        client.stop_replayer(keep_actors=False)
        for si in sensor_infos: si.sensor.stop(); si.sensor.destroy()
        settings.synchronous_mode = False; settings.fixed_delta_seconds = None; world.apply_settings(settings)
        logging.info("Finished CarlaCosmos-DataAcquisition parallel pipeline")

if __name__ == '__main__':
    main()
