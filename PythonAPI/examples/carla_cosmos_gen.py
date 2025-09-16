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
    logging.info("Starting CarlaCosmos-DataAcquisition parallel pipeline")

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
    try:
        while timestamp < args.start + total:
            idx = world.tick()
            frame_dict = {}
            for si in sensor_infos:
                res = si.capture_current_frame()
                if res:
                    img,_,_ = res
                    frame_dict[si.sensor_type] = img
            raw_q.put(FrameBundle(idx, frame_dict, timestamp))
            frame_count += 1
            if frame_count % 100 == 0:
                logging.info(f"Queued frame {frame_count}, timestamp={timestamp:.3f}, idx={idx}")
            timestamp += log_delta
    finally:
        for _ in workers: raw_q.put(None)
        for p in workers: p.join()
        proc_q.put(None); writer.join()
        client.stop_replayer(keep_actors=False)
        for si in sensor_infos: si.sensor.stop(); si.sensor.destroy()
        settings.synchronous_mode = False; settings.fixed_delta_seconds = None; world.apply_settings(settings)
        logging.info("Finished CarlaCosmos-DataAcquisition parallel pipeline")

if __name__ == '__main__':
    main()
