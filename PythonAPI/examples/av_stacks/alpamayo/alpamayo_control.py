#!/usr/bin/env python3
"""Closed-loop CARLA demo for NVIDIA Alpamayo 2 Super.

The CARLA-camera backend attaches to the current world without loading a map or
deleting traffic. The NuRec backend loads the recording's OpenDRIVE world.
Either way, this process is the sole synchronous tick owner, captures canonical
Alpamayo camera/egomotion history, asks a Python 3.12 model service for a
trajectory, and executes a short path prefix.
"""

from __future__ import annotations

import argparse
import collections
import concurrent.futures
import json
import math
import os
import queue
import socket
import subprocess
import sys
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any

import numpy as np

from alpamayo_controller import ControlCommand, TrajectoryTracker
from alpamayo_coordinates import (
    alpamayo_points_to_carla_local,
    history_to_alpamayo,
    rear_axle_world_pose,
)
from alpamayo_display import AlpamayoDisplay, NullDisplay
from alpamayo_protocol import connect, recv_message, send_message
from alpamayo_rig import (
    BODY_CLEARANCE_CAMERA_INDICES,
    CAMERA_INDICES,
    CAMERA_NAMES,
    DISPLAY_CAMERA_INDICES,
    DISPLAY_CAMERA_RING,
    body_clearance_z,
)
from alpamayo_schema import validate_request


HERE = Path(__file__).resolve().parent
DEFAULT_MODEL_PYTHON = HERE / ".venv-model" / "bin" / "python"


class UserExit(Exception):
    pass


class TelemetryRecorder:
    """Line-buffered raw-prediction and applied-control JSONL recorder."""

    def __init__(self, path: str | None) -> None:
        self.path = Path(path).expanduser().resolve() if path else None
        self._stream: Any | None = None
        if self.path is not None:
            self.path.parent.mkdir(parents=True, exist_ok=True)
            self._stream = self.path.open("w", encoding="utf-8", buffering=1)
            print(f"Recording Alpamayo telemetry to {self.path}")

    def write(self, event: str, **values: Any) -> None:
        if self._stream is not None:
            self._stream.write(json.dumps({"event": event, **values}) + "\n")

    def close(self) -> None:
        if self._stream is not None:
            self._stream.close()
            self._stream = None


@dataclass(frozen=True)
class CapturedImage:
    frame: int
    timestamp_us: int
    rgb_chw: np.ndarray


class SensorRing:
    def __init__(
        self,
        world: Any,
        vehicle: Any,
        width: int,
        height: int,
        rear_axle_x: float,
        camera_projection: str,
        enable_dlss: bool,
        chase_dlss_screen_percentage: float,
        rt_lens_samples: int,
        camera_body_clearance: float,
    ) -> None:
        import carla

        self._queues: dict[int, queue.Queue[CapturedImage]] = {}
        self.actors: list[Any] = []
        blueprints = world.get_blueprint_library()
        for spec in DISPLAY_CAMERA_RING:
            is_model_camera = spec.index in CAMERA_INDICES
            use_rt_lens = is_model_camera and camera_projection == "rt-lens"
            use_cubemap = is_model_camera and camera_projection == "cubemap"
            blueprint_id = "sensor.camera.rgb"
            if use_rt_lens:
                blueprint_id = "sensor.camera.rt_lens"
            elif use_cubemap:
                blueprint_id = "sensor.camera.rgb_fisheye"
            blueprint = blueprints.find(blueprint_id)
            blueprint.set_attribute("image_size_x", str(width))
            blueprint.set_attribute("image_size_y", str(height))
            blueprint.set_attribute("fov", str(spec.fov_deg))
            blueprint.set_attribute("sensor_tick", "0.0")
            blueprint.set_attribute("enable_postprocess_effects", "true")
            if is_model_camera and (use_rt_lens or use_cubemap):
                if (
                    spec.focal_length_px is None
                    or spec.kannala_brandt is None
                    or spec.principal_point_px is None
                ):
                    raise RuntimeError(f"missing lens calibration for {spec.name}")
            if use_rt_lens:
                blueprint.set_attribute("camera_model", "kannala_brandt")
                blueprint.set_attribute(
                    "distortion_coeffs",
                    ",".join(str(value) for value in spec.kannala_brandt),
                )
                blueprint.set_attribute("fx", str(spec.focal_length_px / 1920.0))
                blueprint.set_attribute("fy", str(spec.focal_length_px / 1080.0))
                blueprint.set_attribute("cx", str(spec.principal_point_px[0] / 1920.0))
                blueprint.set_attribute("cy", str(spec.principal_point_px[1] / 1080.0))
                blueprint.set_attribute("samples_per_pixel", str(rt_lens_samples))
                # DefaultEngine.ini selects DLSS Ray Reconstruction when the
                # path-tracing denoiser is enabled.  Disabling it keeps the
                # calibrated lens and path tracer but bypasses DLSS entirely.
                blueprint.set_attribute(
                    "enable_denoiser", "true" if enable_dlss else "false"
                )
            elif use_cubemap:
                # The public PhysicalAI calibration is 1920x1080. The CLI
                # requires the same 16:9 aspect ratio, so one scale preserves
                # the calibrated pixel focal length at other resolutions.
                scale = height / 1080.0
                blueprint.set_attribute("camera_model", "kannala-brandt")
                blueprint.set_attribute(
                    "focal_length", str(spec.focal_length_px * scale)
                )
                for coefficient_index, coefficient in enumerate(spec.kannala_brandt):
                    blueprint.set_attribute(f"k{coefficient_index}", str(coefficient))
            else:
                # The chase view (and optional pinhole fallback) uses DLSS-SR;
                # the rt_lens cameras above use DLSS Ray Reconstruction.
                blueprint.set_attribute(
                    "enable_dlss", "true" if enable_dlss else "false"
                )
                blueprint.set_attribute(
                    "dlss_screen_percentage", str(chase_dlss_screen_percentage)
                )
            x, y, z, pitch, yaw, roll = spec.carla_pose(rear_axle_x)
            if spec.index in BODY_CLEARANCE_CAMERA_INDICES:
                bounding_box = vehicle.bounding_box
                corrected_z = body_clearance_z(
                    z,
                    float(bounding_box.location.z),
                    float(bounding_box.extent.z),
                    camera_body_clearance,
                )
                if corrected_z > z:
                    print(
                        f"Raised {spec.name} from z={z:.3f} m to "
                        f"z={corrected_z:.3f} m to clear the vehicle body"
                    )
                    z = corrected_z
            transform = carla.Transform(
                carla.Location(x=x, y=y, z=z),
                carla.Rotation(pitch=pitch, yaw=yaw, roll=roll),
            )
            sensor = world.spawn_actor(
                blueprint,
                transform,
                attach_to=vehicle,
                attachment_type=carla.AttachmentType.Rigid,
            )
            image_queue: queue.Queue[CapturedImage] = queue.Queue(maxsize=2)
            sensor.listen(self._callback(image_queue))
            self._queues[spec.index] = image_queue
            self.actors.append(sensor)

    @staticmethod
    def _callback(image_queue: queue.Queue[CapturedImage]):
        def on_image(image: Any) -> None:
            bgra = np.frombuffer(image.raw_data, dtype=np.uint8).reshape(
                image.height, image.width, 4
            )
            rgb_chw = bgra[:, :, :3][:, :, ::-1].transpose(2, 0, 1).copy()
            captured = CapturedImage(
                frame=int(image.frame),
                timestamp_us=int(round(float(image.timestamp) * 1_000_000)),
                rgb_chw=rgb_chw,
            )
            try:
                image_queue.put_nowait(captured)
            except queue.Full:
                try:
                    image_queue.get_nowait()
                except queue.Empty:
                    pass
                image_queue.put_nowait(captured)

        return on_image

    def capture(self, frame: int, timeout: float) -> dict[int, CapturedImage]:
        deadline = time.monotonic() + timeout
        result: dict[int, CapturedImage] = {}
        for camera_index in DISPLAY_CAMERA_INDICES:
            image_queue = self._queues[camera_index]
            while True:
                remaining = deadline - time.monotonic()
                if remaining <= 0.0:
                    raise TimeoutError(f"camera {camera_index} did not deliver frame {frame}")
                captured = image_queue.get(timeout=remaining)
                if captured.frame < frame:
                    continue
                if captured.frame > frame:
                    raise RuntimeError(
                        f"camera {camera_index} skipped frame {frame}; got {captured.frame}"
                    )
                result[camera_index] = captured
                break
        return result

    def destroy(self) -> None:
        for sensor in self.actors:
            if sensor.is_alive:
                sensor.stop()
                sensor.destroy()


class ObservationBuffer:
    def __init__(self) -> None:
        self.transforms: collections.deque[Any] = collections.deque(maxlen=16)
        self.camera_frames: dict[int, collections.deque[CapturedImage]] = {
            index: collections.deque(maxlen=4) for index in DISPLAY_CAMERA_INDICES
        }

    @property
    def ready(self) -> bool:
        return len(self.transforms) == 16 and all(
            len(frames) == 4 for frames in self.camera_frames.values()
        )

    def append(self, transform: Any, images: dict[int, CapturedImage]) -> None:
        self.transforms.append(transform)
        for index in DISPLAY_CAMERA_INDICES:
            self.camera_frames[index].append(images[index])

    def latest_images(self) -> dict[int, np.ndarray]:
        return {
            index: frames[-1].rgb_chw
            for index, frames in self.camera_frames.items()
            if frames
        }

    def request(
        self,
        rear_axle_x: float,
        map_name: str,
        frame: int,
    ) -> tuple[dict[str, Any], dict[str, np.ndarray]]:
        if not self.ready:
            raise RuntimeError("observation history is not ready")
        image_frames = np.stack(
            [
                np.stack([item.rgb_chw for item in self.camera_frames[index]])
                for index in CAMERA_INDICES
            ]
        )
        absolute_timestamps = np.stack(
            [
                np.asarray(
                    [item.timestamp_us for item in self.camera_frames[index]],
                    dtype=np.int64,
                )
                for index in CAMERA_INDICES
            ]
        )
        camera_tmin = int(absolute_timestamps.min())
        relative_timestamps = (absolute_timestamps - camera_tmin).astype(np.float32) * 1e-6
        history_xyz, history_rot = history_to_alpamayo(
            list(self.transforms), rear_axle_x
        )
        ego_t0 = np.asarray([int(absolute_timestamps[:, -1].max())], dtype=np.int64)
        arrays = {
            "image_frames": image_frames,
            "camera_indices": np.asarray(CAMERA_INDICES, dtype=np.int64),
            "ego_history_xyz": history_xyz,
            "ego_history_rot": history_rot,
            "relative_timestamps": relative_timestamps,
            "absolute_timestamps": absolute_timestamps,
            "ego_t0": ego_t0,
            "ego_t0_relative": (ego_t0 - camera_tmin).astype(np.float32) * 1e-6,
            "ego_t0_frame_idx": np.asarray([3], dtype=np.int64),
        }
        metadata = {
            "camera_names": list(CAMERA_NAMES),
            "carla_map": map_name,
            "carla_frame": frame,
            "time_step_seconds": 0.1,
        }
        validate_request(metadata, arrays)
        return metadata, arrays


class ModelService:
    def __init__(self, args: argparse.Namespace) -> None:
        self.args = args
        self.socket: socket.socket | None = None
        self.process: subprocess.Popen[bytes] | None = None
        self.executor = concurrent.futures.ThreadPoolExecutor(max_workers=1)

    @staticmethod
    def _probe(host: str, port: int, timeout: float = 2.0) -> socket.socket | None:
        try:
            candidate = connect(host, port, timeout=timeout)
            candidate.settimeout(timeout)
            send_message(candidate, "ping")
            response = recv_message(candidate)
            candidate.settimeout(None)
            if response.kind == "pong":
                return candidate
            candidate.close()
        except (OSError, EOFError, RuntimeError):
            pass
        return None

    def start(self) -> None:
        self.socket = self._probe(self.args.model_host, self.args.model_port)
        if self.socket is not None:
            print(f"Reusing model server at {self.args.model_host}:{self.args.model_port}")
            return
        if self.args.external_model_server:
            raise RuntimeError(
                f"no Alpamayo model server is ready at "
                f"{self.args.model_host}:{self.args.model_port}"
            )
        # Do not resolve this path: virtual-environment Python executables are
        # commonly symlinks, and following the symlink would silently discard
        # the environment's site-packages.
        model_python = Path(
            os.path.abspath(os.path.expanduser(self.args.model_python))
        )
        if not model_python.is_file():
            raise FileNotFoundError(
                f"Alpamayo Python not found at {model_python}; run install_alpamayo.sh"
            )
        command = [
            str(model_python),
            str(HERE / "alpamayo_model_server.py"),
            "--host",
            self.args.model_host,
            "--port",
            str(self.args.model_port),
            "--model-id",
            self.args.model_id,
            "--diffusion-steps",
            str(self.args.diffusion_steps),
            "--seed",
            str(self.args.seed),
        ]
        if self.args.mock_model:
            command.extend(("--mock", "--mock-speed", str(self.args.mock_speed)))
        environment = os.environ.copy()
        environment["CUDA_VISIBLE_DEVICES"] = self.args.model_gpus
        environment.setdefault("PYTORCH_CUDA_ALLOC_CONF", "expandable_segments:True")
        environment["PYTHONUNBUFFERED"] = "1"
        print("Starting Alpamayo model service...")
        self.process = subprocess.Popen(
            command,
            cwd=HERE,
            env=environment,
            start_new_session=True,
        )
        deadline = time.monotonic() + self.args.model_startup_timeout
        while time.monotonic() < deadline:
            if self.process.poll() is not None:
                raise RuntimeError(f"model server exited with code {self.process.returncode}")
            self.socket = self._probe(self.args.model_host, self.args.model_port)
            if self.socket is not None:
                return
            time.sleep(1.0)
        raise TimeoutError("timed out waiting for the Alpamayo model server")

    def submit(
        self,
        metadata: dict[str, Any],
        arrays: dict[str, np.ndarray],
    ) -> concurrent.futures.Future[Any]:
        if self.socket is None:
            raise RuntimeError("model service is not connected")

        def request():
            assert self.socket is not None
            send_message(self.socket, "predict", metadata, arrays)
            response = recv_message(self.socket)
            if response.kind == "error":
                raise RuntimeError(response.metadata.get("error", "model server error"))
            if response.kind != "prediction" or "pred_xyz" not in response.arrays:
                raise RuntimeError(f"unexpected model response {response.kind!r}")
            return response

        return self.executor.submit(request)

    def close(self) -> None:
        if self.socket is not None:
            self.socket.close()
            self.socket = None
        self.executor.shutdown(wait=False, cancel_futures=True)
        if self.process is not None and not self.args.keep_model_server:
            self.process.terminate()
            try:
                self.process.wait(timeout=15)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait(timeout=5)


def find_hero(world: Any, role_name: str) -> Any | None:
    matches = [
        actor
        for actor in world.get_actors().filter("vehicle.*")
        if actor.attributes.get("role_name") == role_name
    ]
    return min(matches, key=lambda actor: actor.id) if matches else None


def spawn_vehicle(world: Any, blueprint_filter: str, role_name: str) -> Any:
    candidates = world.get_blueprint_library().filter(blueprint_filter)
    if not candidates:
        raise RuntimeError(f"no vehicle blueprint matches {blueprint_filter!r}")
    blueprint = candidates[0]
    if blueprint.has_attribute("role_name"):
        blueprint.set_attribute("role_name", role_name)
    for spawn_point in world.get_map().get_spawn_points():
        vehicle = world.try_spawn_actor(blueprint, spawn_point)
        if vehicle is not None:
            return vehicle
    raise RuntimeError("unable to find a free spawn point for the ego vehicle")


def acquire_vehicle(world: Any, args: argparse.Namespace) -> tuple[Any, bool]:
    if args.ego_mode != "spawn":
        deadline = time.monotonic() + args.hero_timeout
        while True:
            hero = find_hero(world, args.role_name)
            if hero is not None:
                print(f"Attached to existing hero {hero.id}: {hero.type_id}")
                return hero, False
            if args.ego_mode == "auto" or time.monotonic() >= deadline:
                break
            time.sleep(0.25)
        if args.ego_mode == "attach":
            raise RuntimeError(
                f"no vehicle with role_name={args.role_name!r} appeared within "
                f"{args.hero_timeout:.1f}s"
            )
    vehicle = spawn_vehicle(world, args.vehicle_filter, args.role_name)
    print(f"Spawned hero {vehicle.id}: {vehicle.type_id}")
    return vehicle, True


def speed_mps(vehicle: Any) -> float:
    velocity = vehicle.get_velocity()
    return math.sqrt(velocity.x**2 + velocity.y**2 + velocity.z**2)


def current_state_in_plan_frame(
    vehicle: Any,
    plan_xyz_world: np.ndarray,
    plan_rotation_world: np.ndarray,
    rear_axle_x: float,
) -> tuple[np.ndarray, float]:
    current_xyz, current_rotation = rear_axle_world_pose(vehicle.get_transform(), rear_axle_x)
    position = plan_rotation_world.T @ (current_xyz - plan_xyz_world)
    relative_rotation = plan_rotation_world.T @ current_rotation
    yaw = math.atan2(relative_rotation[1, 0], relative_rotation[0, 0])
    return position, yaw


def tick_and_capture(
    world: Any,
    vehicle: Any,
    sensors: SensorRing,
    history: ObservationBuffer,
    timeout: float,
) -> int:
    frame = int(world.tick())
    history.append(vehicle.get_transform(), sensors.capture(frame, timeout))
    return frame


def default_telemetry(map_name: str) -> dict[str, Any]:
    return {
        "map": map_name.rsplit("/", 1)[-1],
        "cycle": 0,
        "inference_seconds": 0.0,
        "speed_mps": 0.0,
        "target_speed_mps": 0.0,
        "throttle": 0.0,
        "brake": 0.0,
        "steer": 0.0,
    }


def draw_or_exit(
    display: Any,
    history: ObservationBuffer,
    trajectory: np.ndarray | None,
    reasoning: str,
    status: str,
    telemetry: dict[str, Any],
    record_frame: bool = False,
) -> None:
    if not display.draw(
        images=history.latest_images(),
        trajectory=trajectory,
        reasoning=reasoning,
        status=status,
        telemetry=telemetry,
        record_frame=record_frame,
    ):
        raise UserExit


def run(args: argparse.Namespace) -> None:
    if args.sensor_backend == "nurec":
        from alpamayo_nurec import run_nurec

        run_nurec(args)
        return

    import carla

    client = carla.Client(args.host, args.port)
    client.set_timeout(30.0)
    world = client.get_world()
    map_name = world.get_map().name
    map_short_name = map_name.rsplit("/", 1)[-1]
    if args.expected_map and args.expected_map not in (map_name, map_short_name):
        raise RuntimeError(
            f"expected map {args.expected_map!r}, but the existing world is {map_name!r}"
        )

    display: Any = (
        NullDisplay()
        if args.no_display
        else AlpamayoDisplay(
            screenshot=args.screenshot,
            record_video=args.record_video,
            record_fps=args.record_fps,
        )
    )
    telemetry_path = args.telemetry_log
    if telemetry_path is None and args.record_video:
        telemetry_path = str(Path(args.record_video).with_suffix(".telemetry.jsonl"))
    telemetry_recorder = TelemetryRecorder(telemetry_path)
    model = ModelService(args)
    original_settings = world.get_settings()
    original_world_was_sync = original_settings.synchronous_mode
    vehicle: Any | None = None
    spawned_vehicle = False
    sensors: SensorRing | None = None
    traffic_manager: Any | None = None
    try:
        model.start()
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.1
        settings.no_rendering_mode = False
        world.apply_settings(settings)
        if args.manage_traffic_manager:
            traffic_manager = client.get_trafficmanager(args.traffic_manager_port)
            traffic_manager.set_synchronous_mode(True)

        vehicle, spawned_vehicle = acquire_vehicle(world, args)
        vehicle.set_autopilot(False, args.traffic_manager_port)
        sensors = SensorRing(
            world,
            vehicle,
            args.width,
            args.height,
            args.rear_axle_x,
            args.camera_projection,
            args.enable_dlss,
            args.chase_dlss_screen_percentage,
            args.rt_lens_samples,
            args.camera_body_clearance,
        )
        history = ObservationBuffer()
        telemetry = default_telemetry(map_name)
        bootstrap_throttle = args.bootstrap_throttle
        if bootstrap_throttle is None:
            bootstrap_throttle = 0.25 if spawned_vehicle else 0.0
        vehicle.apply_control(carla.VehicleControl(throttle=bootstrap_throttle))
        print("Collecting 1.6 seconds of synchronized history...")
        last_frame = -1
        for index in range(16):
            last_frame = tick_and_capture(
                world, vehicle, sensors, history, args.sensor_timeout
            )
            telemetry["speed_mps"] = speed_mps(vehicle)
            draw_or_exit(
                display,
                history,
                None,
                "",
                f"Collecting history {index + 1}/16",
                telemetry,
                record_frame=True,
            )
        vehicle.apply_control(carla.VehicleControl())

        tracker = TrajectoryTracker(
            wheelbase_m=args.wheelbase,
            max_wheel_angle_deg=args.max_wheel_angle,
            lookahead_m=args.lookahead,
            speed_kp=args.speed_kp,
            speed_ki=args.speed_ki,
            speed_kd=args.speed_kd,
            speed_integral_limit=args.speed_integral_limit,
            stop_target_speed_mps=args.stop_target_speed,
            hold_speed_mps=args.hold_speed,
            minimum_stop_brake=args.minimum_stop_brake,
            hold_brake=args.hold_brake,
            steering_gain=args.steering_gain,
            max_steer_rate_per_s=args.max_steer_rate,
        )
        cycle = 0
        reasoning = ""
        trajectory_alpamayo: np.ndarray | None = None
        while args.max_cycles == 0 or cycle < args.max_cycles:
            metadata, arrays = history.request(args.rear_axle_x, map_name, last_frame)
            plan_xyz_world, plan_rotation_world = rear_axle_world_pose(
                history.transforms[-1], args.rear_axle_x
            )
            telemetry["cycle"] = cycle
            future = model.submit(metadata, arrays)
            inference_started = time.monotonic()
            while not future.done():
                telemetry["inference_seconds"] = time.monotonic() - inference_started
                draw_or_exit(
                    display,
                    history,
                    trajectory_alpamayo,
                    reasoning,
                    "Alpamayo inference — simulation paused",
                    telemetry,
                )
                time.sleep(1.0 / 30.0)
            response = future.result()
            trajectory_alpamayo = np.asarray(response.arrays["pred_xyz"], dtype=np.float32)
            path_carla = alpamayo_points_to_carla_local(trajectory_alpamayo)
            reasoning = str(response.metadata.get("cot", ""))
            telemetry["inference_seconds"] = float(
                response.metadata.get(
                    "inference_seconds", time.monotonic() - inference_started
                )
            )
            endpoint = trajectory_alpamayo[-1]
            print(
                f"Cycle {cycle}: {telemetry['inference_seconds']:.2f}s, "
                f"endpoint=({endpoint[0]:.2f}, {endpoint[1]:.2f})m, "
                f"reasoning={reasoning!r}"
            )
            telemetry_recorder.write(
                "prediction",
                cycle=cycle,
                carla_frame=last_frame,
                inference_seconds=telemetry["inference_seconds"],
                reasoning=reasoning,
                trajectory_alpamayo=trajectory_alpamayo.tolist(),
                trajectory_carla_local=path_carla.tolist(),
            )
            if args.observe_only:
                draw_or_exit(
                    display,
                    history,
                    trajectory_alpamayo,
                    reasoning,
                    "Observe-only prediction complete",
                    telemetry,
                    record_frame=True,
                )
                break

            command = ControlCommand(0.0, 0.0, 0.0, 0.0, 0)
            for control_index in range(args.control_steps):
                position, yaw = current_state_in_plan_frame(
                    vehicle,
                    plan_xyz_world,
                    plan_rotation_world,
                    args.rear_axle_x,
                )
                speed_before_control = speed_mps(vehicle)
                command = tracker.command(
                    path_carla,
                    position,
                    yaw,
                    speed_before_control,
                    trajectory_time_index=control_index,
                )
                vehicle.apply_control(
                    carla.VehicleControl(
                        throttle=command.throttle,
                        brake=command.brake,
                        steer=command.steer,
                    )
                )
                last_frame = tick_and_capture(
                    world, vehicle, sensors, history, args.sensor_timeout
                )
                telemetry.update(
                    {
                        "speed_mps": speed_mps(vehicle),
                        "target_speed_mps": command.target_speed_mps,
                        "throttle": command.throttle,
                        "brake": command.brake,
                        "steer": command.steer,
                        "requested_steer": command.requested_steer,
                    }
                )
                telemetry_recorder.write(
                    "control",
                    cycle=cycle,
                    control_step=control_index,
                    carla_frame=last_frame,
                    position_carla_local=position.tolist(),
                    yaw_carla_local_rad=float(yaw),
                    target_index=command.target_index,
                    steering_target_index=command.target_index,
                    target_point_alpamayo=trajectory_alpamayo[
                        command.target_index
                    ].tolist(),
                    speed_target_index=command.speed_index,
                    speed_mps_before_control=speed_before_control,
                    speed_mps=telemetry["speed_mps"],
                    target_speed_mps=command.target_speed_mps,
                    speed_error_mps=command.speed_error_mps,
                    longitudinal_effort=command.longitudinal_effort,
                    hold_active=command.hold_active,
                    throttle=command.throttle,
                    brake=command.brake,
                    requested_steer=command.requested_steer,
                    applied_steer=command.steer,
                )
                draw_or_exit(
                    display,
                    history,
                    trajectory_alpamayo,
                    reasoning,
                    f"Executing prediction {control_index + 1}/{args.control_steps}",
                    telemetry,
                    record_frame=True,
                )
            cycle += 1
    except UserExit:
        print("User requested shutdown.")
    finally:
        if vehicle is not None and vehicle.is_alive:
            vehicle.apply_control(carla.VehicleControl(brake=1.0, hand_brake=True))
        if sensors is not None:
            sensors.destroy()
        if spawned_vehicle and vehicle is not None and vehicle.is_alive:
            vehicle.destroy()
        elif (
            not spawned_vehicle
            and vehicle is not None
            and vehicle.is_alive
            and args.restore_autopilot
        ):
            vehicle.set_autopilot(True, args.traffic_manager_port)
        if traffic_manager is not None and not original_world_was_sync:
            traffic_manager.set_synchronous_mode(False)
        world.apply_settings(original_settings)
        model.close()
        display.close()
        telemetry_recorder.close()


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--host", default="127.0.0.1", help="CARLA server host")
    parser.add_argument("--port", type=int, default=2000, help="CARLA RPC port")
    parser.add_argument(
        "--sensor-backend",
        choices=("carla", "nurec"),
        default="carla",
        help="Use CARLA camera actors or a unified NuRec reconstructed world",
    )
    parser.add_argument(
        "--expected-map",
        default=None,
        help="Validate the existing map without loading or changing it",
    )
    parser.add_argument("--ego-mode", choices=("auto", "attach", "spawn"), default="auto")
    parser.add_argument("--role-name", default="hero")
    parser.add_argument("--hero-timeout", type=float, default=30.0)
    parser.add_argument("--vehicle-filter", default="vehicle.lincoln.mkz")
    parser.add_argument("--restore-autopilot", action="store_true")
    parser.add_argument("--traffic-manager-port", type=int, default=8001)
    parser.add_argument(
        "--no-manage-traffic-manager",
        action="store_false",
        dest="manage_traffic_manager",
    )
    parser.set_defaults(manage_traffic_manager=True)

    parser.add_argument(
        "--nurec-usdz",
        default=None,
        metavar="PATH",
        help="NuRec USDZ artifact (required with --sensor-backend nurec)",
    )
    parser.add_argument("--nurec-port", type=int, default=None, help="NRE gRPC port")
    parser.add_argument(
        "--nurec-image",
        default="nvcr.io/nvidia/nre/nre-ga:26.04.01",
        help="Pinned NRE container image",
    )
    parser.add_argument(
        "--nurec-gpu",
        default="0",
        help="Physical GPU(s) exposed inside the NRE container (default: 0)",
    )
    parser.add_argument(
        "--nurec-renderer", choices=("nrend", "gsplat"), default=None
    )
    parser.add_argument("--nurec-harmonizer", action="store_true")
    parser.add_argument(
        "--nurec-resolution-ratio",
        type=float,
        default=1.0,
        help="Native NuRec camera render scale before dashboard/model resize",
    )
    parser.add_argument(
        "--nurec-actors",
        choices=("replay", "carla", "disabled"),
        default="replay",
        help=(
            "Keep traffic on rigid replay or put vehicles under CARLA physics "
            "with collision-aware Traffic Manager where routes are usable"
        ),
    )
    parser.add_argument(
        "--nurec-no-replay-traffic",
        action="store_const",
        const="carla",
        dest="nurec_actors",
        help=(
            "Disable rigid non-ego vehicle replay: use collision-aware CARLA "
            "Traffic Manager for moving traffic and hold route-less vehicles"
        ),
    )
    parser.add_argument(
        "--nurec-disable-traffic",
        action="store_const",
        const="disabled",
        dest="nurec_actors",
        help=(
            "Disable rigid replay and Traffic Manager for non-ego vehicles; "
            "hold their CARLA proxies collision-free"
        ),
    )
    parser.add_argument(
        "--nurec-carla-weather",
        default="ClearNoon",
        help="Weather preset for the CARLA-only chase view (neural views are unaffected)",
    )
    parser.add_argument(
        "--stop-nurec-container",
        action="store_false",
        dest="nurec_reuse_container",
        help="Stop NRE at exit instead of leaving it warm for reuse",
    )
    parser.set_defaults(nurec_reuse_container=True)

    parser.add_argument("--model-host", default="127.0.0.1")
    parser.add_argument("--model-port", type=int, default=8767)
    parser.add_argument("--model-id", default="nvidia/Alpamayo2-Super")
    parser.add_argument("--model-python", default=str(DEFAULT_MODEL_PYTHON))
    parser.add_argument(
        "--model-gpus",
        default=os.environ.get("ALPAMAYO2_CUDA_DEVICES", "1,2,3"),
        help="Physical GPUs exposed to the model subprocess; keep CARLA's GPU excluded",
    )
    parser.add_argument("--model-startup-timeout", type=float, default=600.0)
    parser.add_argument("--external-model-server", action="store_true")
    parser.add_argument("--keep-model-server", action="store_true")
    parser.add_argument("--diffusion-steps", type=int, default=10)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--mock-model", action="store_true")
    parser.add_argument("--mock-speed", type=float, default=5.0)

    parser.add_argument("--width", type=int, default=1920)
    parser.add_argument("--height", type=int, default=1080)
    parser.add_argument(
        "--camera-projection",
        choices=("rt-lens", "cubemap", "pinhole"),
        default="rt-lens",
        help=(
            "Use the calibrated ray-traced Kannala-Brandt sensors, the older "
            "cubemap fisheye, or pinhole fallback (default: rt-lens)"
        ),
    )
    parser.add_argument(
        "--chase-dlss-screen-percentage",
        type=float,
        default=50.0,
        help="DLSS-SR percentage for the dashboard-only chase camera",
    )
    parser.add_argument(
        "--no-dlss",
        action="store_false",
        dest="enable_dlss",
        help=(
            "Disable the rt_lens DLSS-RR denoiser and chase-camera DLSS-SR; "
            "the calibrated path-traced sensors remain active"
        ),
    )
    parser.set_defaults(enable_dlss=True)
    parser.add_argument(
        "--rt-lens-samples",
        type=int,
        default=1,
        help="Path-tracing samples per frame before DLSS-RR (default: 1)",
    )
    parser.add_argument("--sensor-timeout", type=float, default=30.0)
    parser.add_argument(
        "--rear-axle-x",
        type=float,
        default=-1.3535001278,
        help="Rear axle x-coordinate relative to the vehicle actor origin",
    )
    parser.add_argument(
        "--camera-body-clearance",
        type=float,
        default=0.08,
        help="Clearance above the vehicle bounding box for forward center cameras",
    )
    parser.add_argument("--bootstrap-throttle", type=float, default=None)
    parser.add_argument("--wheelbase", type=float, default=2.8)
    parser.add_argument("--max-wheel-angle", type=float, default=70.0)
    parser.add_argument("--lookahead", type=float, default=3.0)
    parser.add_argument(
        "--speed-kp",
        "--speed-gain",
        dest="speed_kp",
        type=float,
        default=0.35,
        help="Longitudinal PID proportional gain (--speed-gain is an alias)",
    )
    parser.add_argument("--speed-ki", type=float, default=0.04)
    parser.add_argument("--speed-kd", type=float, default=0.02)
    parser.add_argument("--speed-integral-limit", type=float, default=5.0)
    parser.add_argument(
        "--stop-target-speed",
        type=float,
        default=0.25,
        help="Predicted speed at or below this requests a full stop",
    )
    parser.add_argument(
        "--hold-speed",
        type=float,
        default=0.15,
        help="Engage stationary brake hold below this ego speed",
    )
    parser.add_argument("--minimum-stop-brake", type=float, default=0.25)
    parser.add_argument("--hold-brake", type=float, default=1.0)
    parser.add_argument(
        "--steering-gain",
        type=float,
        default=1.0,
        help="Multiplier applied to pure-pursuit steering (default: 1.0)",
    )
    parser.add_argument(
        "--max-steer-rate",
        type=float,
        default=1.0,
        help="Maximum normalized steering-command change per second (default: 1.0)",
    )
    parser.add_argument("--control-steps", type=int, default=10)
    parser.add_argument("--max-cycles", type=int, default=0, help="0 runs until ESC/Ctrl-C")
    parser.add_argument("--observe-only", action="store_true")
    parser.add_argument("--no-display", action="store_true")
    parser.add_argument(
        "--screenshot",
        default=None,
        help="Save the first dashboard frame containing a prediction",
    )
    parser.add_argument(
        "--record-video",
        default=None,
        metavar="PATH",
        help="Record the fully composited dashboard to an MP4 file",
    )
    parser.add_argument(
        "--record-fps",
        type=float,
        default=20.0,
        help="Dashboard recording frame rate (default: 20)",
    )
    parser.add_argument(
        "--telemetry-log",
        default=None,
        metavar="PATH",
        help=(
            "Write raw trajectories and requested/applied controls as JSONL; "
            "defaults to a sidecar when --record-video is used"
        ),
    )
    args = parser.parse_args()
    if args.bootstrap_throttle is not None and not 0.0 <= args.bootstrap_throttle <= 1.0:
        parser.error("--bootstrap-throttle must be between 0 and 1")
    if args.control_steps < 1:
        parser.error("--control-steps must be positive")
    if args.record_fps <= 0.0:
        parser.error("--record-fps must be positive")
    if args.width * 9 != args.height * 16:
        parser.error("Alpamayo camera resolution must use a 16:9 aspect ratio")
    if args.steering_gain <= 0.0:
        parser.error("--steering-gain must be positive")
    if args.max_steer_rate <= 0.0:
        parser.error("--max-steer-rate must be positive")
    if args.speed_kp < 0.0 or args.speed_ki < 0.0 or args.speed_kd < 0.0:
        parser.error("longitudinal PID gains must be non-negative")
    if args.speed_integral_limit < 0.0:
        parser.error("--speed-integral-limit must be non-negative")
    if args.stop_target_speed < 0.0 or args.hold_speed < 0.0:
        parser.error("stop and hold speeds must be non-negative")
    if args.hold_speed > args.stop_target_speed:
        parser.error("--hold-speed cannot exceed --stop-target-speed")
    if not 0.0 <= args.minimum_stop_brake <= 1.0:
        parser.error("--minimum-stop-brake must be between 0 and 1")
    if not 0.0 <= args.hold_brake <= 1.0:
        parser.error("--hold-brake must be between 0 and 1")
    if args.camera_body_clearance < 0.0:
        parser.error("--camera-body-clearance cannot be negative")
    if not 25.0 <= args.chase_dlss_screen_percentage <= 100.0:
        parser.error("--chase-dlss-screen-percentage must be between 25 and 100")
    if args.rt_lens_samples < 1:
        parser.error("--rt-lens-samples must be positive")
    if args.no_display and args.record_video:
        parser.error("--record-video cannot be combined with --no-display")
    if args.sensor_backend == "nurec":
        if not args.nurec_usdz:
            parser.error("--nurec-usdz is required with --sensor-backend nurec")
        if args.expected_map:
            parser.error(
                "--expected-map applies to the CARLA backend; NuRec loads the "
                "OpenDRIVE map embedded in --nurec-usdz"
            )
        if args.nurec_resolution_ratio <= 0.0:
            parser.error("--nurec-resolution-ratio must be positive")
    return args


if __name__ == "__main__":
    try:
        run(parse_args())
    except KeyboardInterrupt:
        print("\nCancelled by user.")
    except Exception as exc:
        print(f"ERROR: {type(exc).__name__}: {exc}", file=sys.stderr)
        raise
