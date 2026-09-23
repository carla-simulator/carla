"""Unified NuRec world/camera backend for the Alpamayo closed-loop demo.

NuRec owns the generated OpenDRIVE world and this module is its sole tick
owner.  Running the standalone nurec_runner.py next to this module is invalid:
both applications would otherwise advance the same synchronous simulation.
"""

from __future__ import annotations

import queue
import sys
import time
from pathlib import Path
from typing import Any

import numpy as np
from PIL import Image

from alpamayo_control import (
    CapturedImage,
    ControlCommand,
    ModelService,
    ObservationBuffer,
    TelemetryRecorder,
    TrajectoryTracker,
    UserExit,
    current_state_in_plan_frame,
    default_telemetry,
    draw_or_exit,
    speed_mps,
)
from alpamayo_coordinates import alpamayo_points_to_carla_local, rear_axle_world_pose
from alpamayo_display import AlpamayoDisplay, NullDisplay
from alpamayo_rig import (
    CAMERA_NAMES,
    CARLA_THIRD_PERSON_CAMERA,
    DISPLAY_CAMERA_INDICES,
    TRAJECTORY_CAMERA_INDICES,
)


HERE = Path(__file__).resolve().parent
NUREC_DIR = HERE.parent.parent / "nvidia" / "nurec"


class NurecEpisodeComplete(Exception):
    """The finite NuRec timeline ended before another camera batch."""


def _put_latest(items: queue.Queue[CapturedImage], captured: CapturedImage) -> None:
    try:
        items.put_nowait(captured)
    except queue.Full:
        try:
            items.get_nowait()
        except queue.Empty:
            pass
        items.put_nowait(captured)


class NurecSensorRing:
    """Six native NuRec cameras, one explicit context duplicate, and a chase view."""

    def __init__(
        self,
        scenario: Any,
        vehicle: Any,
        width: int,
        height: int,
        rear_axle_x: float,
        resolution_ratio: float,
        enable_dlss: bool,
        chase_dlss_screen_percentage: float,
    ) -> None:
        import carla

        self.scenario = scenario
        self.width = width
        self.height = height
        self._queues: dict[int, queue.Queue[CapturedImage]] = {
            index: queue.Queue(maxsize=8) for index in TRAJECTORY_CAMERA_INDICES
        }
        self._chase_queue: queue.Queue[CapturedImage] = queue.Queue(maxsize=8)
        self._collision_queue: queue.Queue[dict[str, Any]] = queue.Queue()
        self.chase_actor: Any | None = None
        self.collision_actor: Any | None = None

        available = set(scenario.get_available_cameras())
        missing = [CAMERA_NAMES[index] for index in TRAJECTORY_CAMERA_INDICES
                   if CAMERA_NAMES[index] not in available]
        if missing:
            raise RuntimeError(
                f"NuRec scene lacks Alpamayo cameras {missing}; available={sorted(available)}"
            )

        for index in TRAJECTORY_CAMERA_INDICES:
            scenario.add_camera(
                CAMERA_NAMES[index],
                self._neural_callback(index),
                framerate=10,
                resolution_ratio=resolution_ratio,
            )

        spec = CARLA_THIRD_PERSON_CAMERA
        blueprint = scenario.get_world().get_blueprint_library().find("sensor.camera.rgb")
        blueprint.set_attribute("image_size_x", str(width))
        blueprint.set_attribute("image_size_y", str(height))
        blueprint.set_attribute("fov", str(spec.fov_deg))
        blueprint.set_attribute("sensor_tick", "0.0")
        blueprint.set_attribute("enable_postprocess_effects", "true")
        blueprint.set_attribute("enable_dlss", "true" if enable_dlss else "false")
        blueprint.set_attribute(
            "dlss_screen_percentage", str(chase_dlss_screen_percentage)
        )
        x, y, z, pitch, yaw, roll = spec.carla_pose(rear_axle_x)
        transform = carla.Transform(
            carla.Location(x=x, y=y, z=z),
            carla.Rotation(pitch=pitch, yaw=yaw, roll=roll),
        )
        self.chase_actor = scenario.get_world().spawn_actor(
            blueprint,
            transform,
            attach_to=vehicle,
            attachment_type=carla.AttachmentType.Rigid,
        )
        self.chase_actor.listen(self._carla_callback)

        collision_bp = scenario.get_world().get_blueprint_library().find(
            "sensor.other.collision"
        )
        self.collision_actor = scenario.get_world().spawn_actor(
            collision_bp, carla.Transform(), attach_to=vehicle
        )
        self.collision_actor.listen(self._collision_callback)

    def _neural_callback(self, index: int):
        def on_image(image_hwc: np.ndarray) -> None:
            image_hwc = np.asarray(image_hwc, dtype=np.uint8)
            if image_hwc.shape[:2] != (self.height, self.width):
                image_hwc = np.asarray(
                    Image.fromarray(image_hwc).resize(
                        (self.width, self.height), Image.Resampling.BILINEAR
                    )
                )
            captured = CapturedImage(
                frame=int(self.scenario.last_render_frame),
                timestamp_us=int(self.scenario.last_render_timestamp_us),
                rgb_chw=image_hwc.transpose(2, 0, 1).copy(),
            )
            _put_latest(self._queues[index], captured)

        return on_image

    def _carla_callback(self, image: Any) -> None:
        bgra = np.frombuffer(image.raw_data, dtype=np.uint8).reshape(
            image.height, image.width, 4
        )
        captured = CapturedImage(
            frame=int(image.frame),
            timestamp_us=int(round(float(image.timestamp) * 1_000_000)),
            rgb_chw=bgra[:, :, :3][:, :, ::-1].transpose(2, 0, 1).copy(),
        )
        _put_latest(self._chase_queue, captured)

    def _collision_callback(self, event: Any) -> None:
        other = event.other_actor
        impulse = event.normal_impulse
        self._collision_queue.put_nowait(
            {
                "carla_frame": int(event.frame),
                "other_actor_id": int(other.id),
                "other_actor_type": str(other.type_id),
                "other_role_name": str(other.attributes.get("role_name", "")),
                "normal_impulse": [
                    float(impulse.x), float(impulse.y), float(impulse.z)
                ],
                "impulse_magnitude": float(
                    np.linalg.norm([impulse.x, impulse.y, impulse.z])
                ),
            }
        )

    def drain_collisions(self) -> list[dict[str, Any]]:
        events: list[dict[str, Any]] = []
        while True:
            try:
                events.append(self._collision_queue.get_nowait())
            except queue.Empty:
                return events

    @staticmethod
    def _take_after(
        items: queue.Queue[CapturedImage], frame: int, deadline: float, label: str
    ) -> CapturedImage:
        while True:
            remaining = deadline - time.monotonic()
            if remaining <= 0.0:
                raise TimeoutError(f"{label} did not deliver a frame after {frame}")
            captured = items.get(timeout=remaining)
            if captured.frame > frame:
                return captured

    def has_complete_batch_after(self, frame: int) -> bool:
        return all(
            any(item.frame > frame for item in list(self._queues[index].queue))
            for index in TRAJECTORY_CAMERA_INDICES
        )

    def capture_after(self, frame: int, timeout: float) -> tuple[int, dict[int, CapturedImage]]:
        deadline = time.monotonic() + timeout
        result = {
            index: self._take_after(
                self._queues[index], frame, deadline, f"NuRec camera {CAMERA_NAMES[index]}"
            )
            for index in TRAJECTORY_CAMERA_INDICES
        }
        neural_frames = {captured.frame for captured in result.values()}
        if len(neural_frames) != 1:
            raise RuntimeError(f"NuRec camera batch was not synchronized: {neural_frames}")
        batch_frame = neural_frames.pop()

        # The public sample has the exact six cameras consumed by Alpamayo's
        # trajectory profile.  The canonical rear-tele input is validated by
        # upstream but discarded by that profile, so mark its context source
        # explicitly by duplicating the synchronized rear-left neural frame.
        rear_left = result[3]
        result[4] = CapturedImage(
            frame=rear_left.frame,
            timestamp_us=rear_left.timestamp_us,
            rgb_chw=rear_left.rgb_chw,
        )
        result[7] = self._take_after(
            self._chase_queue, frame, deadline, "CARLA chase camera"
        )
        return batch_frame, result

    def destroy(self) -> None:
        if self.collision_actor is not None and self.collision_actor.is_alive:
            self.collision_actor.stop()
            self.collision_actor.destroy()
        self.collision_actor = None
        if self.chase_actor is not None and self.chase_actor.is_alive:
            self.chase_actor.stop()
            self.chase_actor.destroy()
        self.chase_actor = None


def _report_collisions(
    sensors: NurecSensorRing,
    recorder: TelemetryRecorder,
    telemetry: dict[str, Any],
    phase: str,
    cycle: int | None = None,
    control_step: int | None = None,
) -> None:
    for collision in sensors.drain_collisions():
        telemetry["collision_count"] = int(telemetry.get("collision_count", 0)) + 1
        telemetry["last_collision"] = collision
        print(
            "COLLISION: "
            f"frame={collision['carla_frame']} "
            f"actor={collision['other_actor_id']} "
            f"type={collision['other_actor_type']} "
            f"impulse={collision['impulse_magnitude']:.1f}"
        )
        recorder.write(
            "collision",
            sensor_backend="nurec",
            phase=phase,
            cycle=cycle,
            control_step=control_step,
            **collision,
        )


def _tick_and_capture(
    scenario: Any,
    vehicle: Any,
    sensors: NurecSensorRing,
    history: ObservationBuffer,
    previous_frame: int,
    timeout: float,
) -> int:
    # NuRec runs CARLA at 20 Hz and renders its cameras at 10 Hz.  Advance
    # until one complete new neural batch exists; this function is the only
    # place in the combined application that calls world.tick().
    for _ in range(4):
        scenario.tick()
        if sensors.has_complete_batch_after(previous_frame):
            frame, images = sensors.capture_after(previous_frame, timeout)
            history.append(vehicle.get_transform(), images)
            return frame
        if not scenario.is_running():
            raise NurecEpisodeComplete
    raise TimeoutError("NuRec did not produce a 10 Hz camera batch within four ticks")


def run_nurec(args: Any) -> None:
    import carla

    if str(NUREC_DIR) not in sys.path:
        sys.path.insert(0, str(NUREC_DIR))
    from nurec_integration import NurecScenario

    client = carla.Client(args.host, args.port)
    client.set_timeout(30.0)
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
    sensors: NurecSensorRing | None = None
    scenario: Any | None = None
    vehicle: Any | None = None

    try:
        model.start()
        scenario = NurecScenario(
            client,
            str(Path(args.nurec_usdz).expanduser().resolve()),
            port=args.nurec_port,
            move_spectator=False,
            fps=10,
            image=args.nurec_image,
            reuse_container=args.nurec_reuse_container,
            renderer_backend=args.nurec_renderer,
            harmonizer=args.nurec_harmonizer,
            cuda_visible_devices=args.nurec_gpu,
            traffic_manager_port=args.traffic_manager_port,
        )
        with scenario:
            world = scenario.get_world()
            if args.nurec_carla_weather:
                weather = getattr(carla.WeatherParameters, args.nurec_carla_weather, None)
                if weather is None:
                    raise ValueError(
                        f"unknown CARLA weather preset {args.nurec_carla_weather!r}"
                    )
                world.set_weather(weather)
            ego = scenario.get_ego_actor()
            if ego is None:
                raise RuntimeError("NuRec scenario did not create an ego actor")
            vehicle = ego.actor_inst
            sensors = NurecSensorRing(
                scenario,
                vehicle,
                args.width,
                args.height,
                args.rear_axle_x,
                args.nurec_resolution_ratio,
                args.enable_dlss,
                args.chase_dlss_screen_percentage,
            )
            scenario.start_replay(synchronous_mode=True)
            if args.nurec_actors == "carla":
                handed_over = scenario.set_all_actors_carla_controlled(
                    collision_aware=True
                )
                print(
                    "NuRec rigid traffic replay disabled: "
                    f"{handed_over} current vehicles handed to CARLA physics; "
                    "future vehicles will be handed over on entry"
                )
            elif args.nurec_actors == "disabled":
                disabled = scenario.disable_all_traffic()
                print(
                    "NuRec traffic disabled: "
                    f"{disabled} current vehicles held collision-free; "
                    "future vehicles will be held on entry"
                )
            history = ObservationBuffer()
            map_name = f"NuRec/{Path(args.nurec_usdz).stem}"
            telemetry = default_telemetry(map_name)
            bootstrap_throttle = args.bootstrap_throttle
            if bootstrap_throttle is None:
                # NuRec always spawns its own ego.  Give Alpamayo the same
                # short moving history as the standard spawned-vehicle mode;
                # a stationary 16-frame bootstrap can collapse the first plan.
                bootstrap_throttle = 0.25
            scenario.apply_ego_control(carla.VehicleControl(throttle=bootstrap_throttle))
            print("Collecting 1.6 seconds of synchronized NuRec history...")
            last_frame = -1
            for index in range(16):
                last_frame = _tick_and_capture(
                    scenario, vehicle, sensors, history, last_frame, args.sensor_timeout
                )
                _report_collisions(
                    sensors, telemetry_recorder, telemetry, "bootstrap"
                )
                telemetry["speed_mps"] = speed_mps(vehicle)
                draw_or_exit(
                    display,
                    history,
                    None,
                    "",
                    f"Collecting NuRec history {index + 1}/16",
                    telemetry,
                    record_frame=True,
                )
            scenario.apply_ego_control(carla.VehicleControl())

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
            episode_complete = False
            while ((args.max_cycles == 0 or cycle < args.max_cycles)
                   and scenario.is_running()):
                metadata, arrays = history.request(args.rear_axle_x, map_name, last_frame)
                metadata["sensor_backend"] = "nurec"
                metadata["nurec_context_camera_4"] = "duplicate_of_camera_3"
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
                        "Alpamayo inference — NuRec simulation paused",
                        telemetry,
                    )
                    time.sleep(1.0 / 30.0)
                response = future.result()
                trajectory_alpamayo = np.asarray(
                    response.arrays["pred_xyz"], dtype=np.float32
                )
                path_carla = alpamayo_points_to_carla_local(trajectory_alpamayo)
                reasoning = str(response.metadata.get("cot", ""))
                telemetry["inference_seconds"] = float(
                    response.metadata.get(
                        "inference_seconds", time.monotonic() - inference_started
                    )
                )
                endpoint = trajectory_alpamayo[-1]
                print(
                    f"NuRec cycle {cycle}: {telemetry['inference_seconds']:.2f}s, "
                    f"endpoint=({endpoint[0]:.2f}, {endpoint[1]:.2f})m, "
                    f"reasoning={reasoning!r}"
                )
                telemetry_recorder.write(
                    "prediction",
                    sensor_backend="nurec",
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
                        "NuRec observe-only prediction complete",
                        telemetry,
                        record_frame=True,
                    )
                    break

                command = ControlCommand(0.0, 0.0, 0.0, 0.0, 0)
                for control_index in range(args.control_steps):
                    position, yaw = current_state_in_plan_frame(
                        vehicle, plan_xyz_world, plan_rotation_world, args.rear_axle_x
                    )
                    speed_before_control = speed_mps(vehicle)
                    command = tracker.command(
                        path_carla,
                        position,
                        yaw,
                        speed_before_control,
                        trajectory_time_index=control_index,
                    )
                    scenario.apply_ego_control(
                        carla.VehicleControl(
                            throttle=command.throttle,
                            brake=command.brake,
                            steer=command.steer,
                        )
                    )
                    try:
                        last_frame = _tick_and_capture(
                            scenario,
                            vehicle,
                            sensors,
                            history,
                            last_frame,
                            args.sensor_timeout,
                        )
                    except NurecEpisodeComplete:
                        episode_complete = True
                        print("NuRec episode complete.")
                        break
                    _report_collisions(
                        sensors,
                        telemetry_recorder,
                        telemetry,
                        "control",
                        cycle=cycle,
                        control_step=control_index,
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
                        sensor_backend="nurec",
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
                        f"Executing NuRec prediction {control_index + 1}/{args.control_steps}",
                        telemetry,
                        record_frame=True,
                    )
                if episode_complete:
                    break
                cycle += 1
    except UserExit:
        print("User requested shutdown.")
    finally:
        if vehicle is not None and vehicle.is_alive:
            vehicle.apply_control(carla.VehicleControl(brake=1.0, hand_brake=True))
        if sensors is not None:
            sensors.destroy()
        model.close()
        display.close()
        telemetry_recorder.close()
