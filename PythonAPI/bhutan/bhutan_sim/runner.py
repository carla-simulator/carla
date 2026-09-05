# Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de
# Barcelona (UAB).
#
# This work is licensed under the terms of the MIT license.
# For a copy, see <https://opensource.org/licenses/MIT>.

"""Execute a scenario template inside a running CARLA server.

The runner is the only module that imports ``carla``. It:

1. Picks a spawn point whose road geometry best matches the template's
   requested curvature and grade (route archetype selection).
2. Applies the weather preset and sensor degradation.
3. Spawns the ego vehicle, the template's actors and background traffic.
4. Ticks in synchronous mode, writing telemetry, events and bird's-eye ground
   truth through :class:`bhutan_sim.telemetry.TelemetryLogger`.
5. Runs the safety rules and the quality gates, and writes ``run.json``.

The ego vehicle is driven by the Traffic Manager. Nothing here sends control
messages to a real vehicle: the real-world truck workflow stays read-only.
"""

from __future__ import annotations

import math
import os
import random
import time
import uuid
from typing import Any, Dict, List, Optional, Tuple

from . import weather as weather_mod
from .quality import assess_run
from .safety_rules import check_samples, classify_events
from .scenario import ActorSpec, ScenarioTemplate
from .telemetry import Event, RunManifest, Sample, TelemetryLogger

EGO_BLUEPRINTS = {
    "truck": ("vehicle.carlacola.actors", "vehicle.firetruck.actors", "vehicle.sprinter.mercedes"),
    "shuttle": ("vehicle.sprinter.mercedes", "vehicle.carlacola.actors"),
    "car": ("vehicle.lincoln.mkz", "vehicle.mini.cooper", "vehicle.dodge.charger"),
}

ROLE_BLUEPRINTS = {
    "lead_vehicle": ("vehicle.nissan.patrol", "vehicle.lincoln.mkz"),
    "lead_vehicle_stopping": ("vehicle.nissan.patrol", "vehicle.lincoln.mkz"),
    "stalled_vehicle": ("vehicle.taxi.ford", "vehicle.lincoln.mkz"),
    "parked_vehicle": ("vehicle.mini.cooper", "vehicle.lincoln.mkz"),
    "following_vehicle": ("vehicle.dodge.charger", "vehicle.lincoln.mkz"),
    "oncoming_vehicle": ("vehicle.lincoln.mkz", "vehicle.nissan.patrol"),
    "oncoming_truck": ("vehicle.carlacola.actors", "vehicle.firetruck.actors", "vehicle.sprinter.mercedes"),
    "motorcycle": ("vehicle.*motorcycle*", "vehicle.*bike*", "vehicle.mini.cooper"),
}

WALKER_ROLES = ("pedestrian", "pedestrian_crossing", "worker", "livestock")
PROP_ROLES = {
    "debris": ("static.prop.dirtdebris01", "static.prop.dirtdebris02", "static.prop.box01"),
    "cones": ("static.prop.trafficcone01", "static.prop.trafficcone01", "static.prop.streetbarrier", "static.prop.warningconstruction"),
}

GT_CLASS_BY_ROLE = {
    "motorcycle": "motorcycle",
    "livestock": "animal",
    "debris": "obstacle",
    "cones": "obstacle",
    "pedestrian": "pedestrian",
    "pedestrian_crossing": "pedestrian",
    "worker": "pedestrian",
}


def _pick_blueprint(bp_lib, candidates) -> Optional[Any]:
    for pattern in candidates:
        found = bp_lib.filter(pattern)
        if found:
            return found[0]
    return None


def measure_route(waypoint, length_m: float = 60.0, step_m: float = 5.0) -> Tuple[float, float]:
    """Mean absolute curvature (1/m) and grade (%) of the road ahead."""
    points = [waypoint]
    wp = waypoint
    travelled = 0.0
    while travelled < length_m:
        nxt = wp.next(step_m)
        if not nxt:
            break
        wp = nxt[0]
        points.append(wp)
        travelled += step_m
    if len(points) < 3:
        return 0.0, 0.0
    curvature_sum = 0.0
    for a, b in zip(points, points[1:]):
        dyaw = abs((b.transform.rotation.yaw - a.transform.rotation.yaw + 180.0) % 360.0 - 180.0)
        curvature_sum += math.radians(dyaw) / step_m
    curvature = curvature_sum / (len(points) - 1)
    dz = points[-1].transform.location.z - points[0].transform.location.z
    grade = dz / travelled * 100.0 if travelled > 0 else 0.0
    return curvature, grade


def select_spawn_point(world_map, spawn_points, template: ScenarioTemplate, rng: random.Random, sample: int = 40):
    """Choose the spawn point whose road ahead best matches the template."""
    want_curv = template.params.road_curvature
    want_grade = template.params.grade_pct
    candidates = list(spawn_points)
    rng.shuffle(candidates)
    best, best_score, best_measure = None, float("inf"), (0.0, 0.0)
    for transform in candidates[:sample]:
        wp = world_map.get_waypoint(transform.location)
        if wp is None:
            continue
        curv, grade = measure_route(wp)
        score = abs(curv - want_curv) / 0.05 + abs(grade - want_grade) / 8.0
        if score < best_score:
            best, best_score, best_measure = transform, score, (curv, grade)
    return best or candidates[0], best_measure


class ScenarioRunner:
    def __init__(self, client, template: ScenarioTemplate, out_dir: str, seed: Optional[int] = None,
                 tm_port: int = 8000, fixed_delta: float = 0.05, save_camera: bool = False, map_name: Optional[str] = None):
        import carla  # noqa: F401  (validated early so errors are clear)

        self.client = client
        self.template = template
        self.out_dir = out_dir
        self.seed = template.seed if seed is None else seed
        self.rng = random.Random(self.seed)
        self.tm_port = tm_port
        self.fixed_delta = fixed_delta
        self.save_camera = save_camera
        self.map_name = map_name
        self.actors: List[Any] = []
        self.sensors: List[Any] = []
        self.walker_controllers: List[Any] = []
        self.roles: Dict[int, str] = {}
        self.role_actors: Dict[str, List[Any]] = {}
        self.state: Dict[str, Any] = {"gnss": None, "imu": None, "collisions": [], "lane": [], "obstacle": None}
        self.crossing_pending: List[Tuple[Any, Any, Any]] = []
        self.emitted: set = set()
        self.dropped_samples = 0

    # ------------------------------------------------------------------ setup

    def _setup_world(self):
        if self.map_name:
            self.world = self.client.load_world(self.map_name)
        else:
            self.world = self.client.get_world()
        self.map = self.world.get_map()
        self.original_settings = self.world.get_settings()
        settings = self.world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = self.fixed_delta
        self.world.apply_settings(settings)
        self.tm = self.client.get_trafficmanager(self.tm_port)
        self.tm.set_synchronous_mode(True)
        self.tm.set_random_device_seed(self.seed)
        self.tm.set_global_distance_to_leading_vehicle(3.0)
        self.weather = self.template.params.resolved_weather()
        self.world.set_weather(weather_mod.to_carla(self.weather))
        self.bp_lib = self.world.get_blueprint_library()

    def _spawn_ego(self):
        import carla

        params = self.template.params
        bp = _pick_blueprint(self.bp_lib, EGO_BLUEPRINTS[params.vehicle_class]) or self.bp_lib.filter("vehicle.*")[0]
        bp.set_attribute("role_name", "hero")
        spawn_points = self.map.get_spawn_points()
        transform, measured = select_spawn_point(self.map, spawn_points, self.template, self.rng)
        self.measured_route = {"curvature": round(measured[0], 4), "grade_pct": round(measured[1], 2)}
        ego = None
        for attempt in range(10):
            ego = self.world.try_spawn_actor(bp, transform)
            if ego is not None:
                break
            transform = self.rng.choice(spawn_points)
        if ego is None:
            raise RuntimeError("could not spawn ego vehicle")
        self.ego = ego
        self.actors.append(ego)
        self.ego_wp = self.map.get_waypoint(transform.location)
        ego.set_autopilot(True, self.tm_port)
        self.tm.ignore_lights_percentage(ego, 0.0)
        self.tm.auto_lane_change(ego, False)
        self._attach_sensors()

    def _attach_sensors(self):
        import carla

        deg = self.template.params.sensor_degradation
        deg_per_m = 1.0 / 111320.0

        gnss_bp = self.bp_lib.find("sensor.other.gnss")
        for name in ("noise_lat_stddev", "noise_lon_stddev"):
            gnss_bp.set_attribute(name, str(deg.gnss_noise_m * deg_per_m))
        gnss_bp.set_attribute("noise_alt_stddev", str(deg.gnss_noise_m * 1.5))
        gnss_bp.set_attribute("noise_seed", str(self.seed))
        gnss = self.world.spawn_actor(gnss_bp, carla.Transform(carla.Location(z=2.0)), attach_to=self.ego)
        gnss.listen(lambda e: self.state.__setitem__("gnss", e))
        self.sensors.append(gnss)

        imu_bp = self.bp_lib.find("sensor.other.imu")
        for axis in ("x", "y", "z"):
            imu_bp.set_attribute("noise_accel_stddev_" + axis, str(deg.imu_accel_noise))
            imu_bp.set_attribute("noise_gyro_stddev_" + axis, str(deg.imu_gyro_noise))
        imu_bp.set_attribute("noise_seed", str(self.seed))
        imu = self.world.spawn_actor(imu_bp, carla.Transform(), attach_to=self.ego)
        imu.listen(lambda e: self.state.__setitem__("imu", e))
        self.sensors.append(imu)

        col = self.world.spawn_actor(self.bp_lib.find("sensor.other.collision"), carla.Transform(), attach_to=self.ego)
        col.listen(lambda e: self.state["collisions"].append(e))
        self.sensors.append(col)

        lane = self.world.spawn_actor(self.bp_lib.find("sensor.other.lane_invasion"), carla.Transform(), attach_to=self.ego)
        lane.listen(lambda e: self.state["lane"].append(e))
        self.sensors.append(lane)

        obs_bp = self.bp_lib.find("sensor.other.obstacle")
        obs_bp.set_attribute("distance", "60")
        obs_bp.set_attribute("hit_radius", "0.6")
        obs_bp.set_attribute("only_dynamics", "False")
        obs = self.world.spawn_actor(obs_bp, carla.Transform(carla.Location(x=2.5, z=1.0)), attach_to=self.ego)
        obs.listen(lambda e: self.state.__setitem__("obstacle", e))
        self.sensors.append(obs)

        self.has_video = False
        if self.save_camera:
            cam_bp = self.bp_lib.find("sensor.camera.rgb")
            cam_bp.set_attribute("image_size_x", "1280")
            cam_bp.set_attribute("image_size_y", "720")
            cam_bp.set_attribute("fov", "90")
            if cam_bp.has_attribute("lens_flare_intensity"):
                cam_bp.set_attribute("lens_flare_intensity", str(0.1 + 0.9 * deg.camera_blur))
            if cam_bp.has_attribute("motion_blur_intensity"):
                cam_bp.set_attribute("motion_blur_intensity", str(min(1.0, 0.3 + deg.camera_blur)))
            cam = self.world.spawn_actor(cam_bp, carla.Transform(carla.Location(x=1.5, z=2.2)), attach_to=self.ego)
            cam_dir = os.path.join(self.out_dir, "camera")
            os.makedirs(cam_dir, exist_ok=True)
            cam.listen(lambda image: image.save_to_disk(os.path.join(cam_dir, "%06d.png" % image.frame)))
            self.sensors.append(cam)
            self.has_video = True

    def _lane_waypoint(self, lane: str, distance_m: float):
        """Waypoint for an actor lane specification relative to the ego."""
        if lane == "ego_behind":
            prev = self.ego_wp.previous(distance_m)
            return prev[0] if prev else self.ego_wp
        ahead = self.ego_wp.next(max(1.0, distance_m))
        wp = ahead[0] if ahead else self.ego_wp
        if lane in ("opposite", "opposite_encroaching"):
            import carla

            left = wp.get_left_lane()
            if left is not None and left.lane_type == carla.LaneType.Driving and left.lane_id * wp.lane_id < 0:
                return left
            right = wp.get_right_lane()
            if right is not None and right.lane_type == carla.LaneType.Driving and right.lane_id * wp.lane_id < 0:
                return right
            # Single-lane road: place the actor facing the ego in its own lane.
            return wp
        return wp

    @staticmethod
    def _offset(transform, forward_m: float = 0.0, right_m: float = 0.0, up_m: float = 0.0):
        import carla

        fwd = transform.get_forward_vector()
        right = transform.get_right_vector()
        loc = carla.Location(
            x=transform.location.x + fwd.x * forward_m + right.x * right_m,
            y=transform.location.y + fwd.y * forward_m + right.y * right_m,
            z=transform.location.z + up_m,
        )
        return carla.Transform(loc, transform.rotation)

    def _spawn_actor_spec(self, spec: ActorSpec):
        import carla

        wp = self._lane_waypoint(spec.lane, spec.distance_m)
        base = wp.transform
        lane_width = wp.lane_width or self.template.params.lane_width_m
        if spec.lane in ("opposite", "opposite_encroaching") and wp.lane_id * self.ego_wp.lane_id > 0:
            base = carla.Transform(base.location, carla.Rotation(yaw=base.rotation.yaw + 180.0, pitch=base.rotation.pitch, roll=base.rotation.roll))
        right_m = spec.lateral_m
        if spec.lane == "shoulder":
            right_m += lane_width / 2.0 + 0.8
        elif spec.lane == "crossing":
            right_m += lane_width / 2.0 + 0.8
        elif spec.lane == "opposite_encroaching":
            # Pull the actor toward the centreline so it intrudes into the ego lane.
            right_m += lane_width * 0.35 * (1 if wp.lane_id * self.ego_wp.lane_id < 0 else -1)

        role = spec.role
        if role in WALKER_ROLES:
            self._spawn_walker(spec, self._offset(base, right_m=right_m, up_m=1.0), lane_width)
        elif role in PROP_ROLES:
            self._spawn_props(spec, base, right_m)
        else:
            bp = _pick_blueprint(self.bp_lib, ROLE_BLUEPRINTS.get(role, ("vehicle.lincoln.mkz",))) or self.bp_lib.filter("vehicle.*")[0]
            if role == "parked_vehicle":
                right_m += lane_width / 2.0 + 0.6
            transform = self._offset(base, right_m=right_m, up_m=0.5)
            actor = self.world.try_spawn_actor(bp, transform)
            if actor is None:
                return
            self._register(actor, role)
            if role in ("lead_vehicle", "lead_vehicle_stopping", "oncoming_vehicle", "oncoming_truck", "motorcycle", "following_vehicle"):
                actor.set_autopilot(True, self.tm_port)
                self.tm.auto_lane_change(actor, False)
                if role == "following_vehicle":
                    self.tm.vehicle_percentage_speed_difference(actor, -30.0)
                elif role in ("lead_vehicle", "lead_vehicle_stopping"):
                    self.tm.vehicle_percentage_speed_difference(actor, 40.0)
            else:
                control = carla.VehicleControl(hand_brake=True)
                actor.apply_control(control)
                if role == "stalled_vehicle":
                    actor.set_light_state(carla.VehicleLightState(carla.VehicleLightState.Position | carla.VehicleLightState.Special1))

    def _spawn_walker(self, spec: ActorSpec, transform, lane_width: float):
        import carla

        walkers = self.bp_lib.filter("walker.pedestrian.*")
        if not walkers:
            return
        bp = self.rng.choice(walkers)
        if bp.has_attribute("is_invincible"):
            bp.set_attribute("is_invincible", "false")
        walker = self.world.try_spawn_actor(bp, transform)
        if walker is None:
            return
        self._register(walker, spec.role)
        controller_bp = self.bp_lib.find("controller.ai.walker")
        controller = self.world.spawn_actor(controller_bp, carla.Transform(), attach_to=walker)
        self.walker_controllers.append(controller)
        speed = {"livestock": 0.3, "worker": 0.6}.get(spec.role, 1.2)
        if spec.role == "pedestrian_crossing":
            target = self._offset(transform, right_m=-(lane_width * 2.0 + 1.6)).location
            self.crossing_pending.append((walker, controller, target))
        else:
            target = self._offset(transform, forward_m=60.0 if spec.role != "livestock" else 4.0).location
            controller.start()
            controller.go_to_location(target)
            controller.set_max_speed(speed)

    def _spawn_props(self, spec: ActorSpec, base, right_m: float):
        names = PROP_ROLES[spec.role]
        spacing = 3.5 if spec.role == "cones" else 1.5
        for index, name in enumerate(names):
            bp = _pick_blueprint(self.bp_lib, (name,))
            if bp is None:
                continue
            lateral = right_m + (0.9 if spec.role == "cones" else self.rng.uniform(-0.8, 0.8))
            transform = self._offset(base, forward_m=index * spacing, right_m=lateral, up_m=0.3)
            actor = self.world.try_spawn_actor(bp, transform)
            if actor is not None:
                self._register(actor, spec.role)

    def _spawn_traffic(self):
        density = self.template.params.traffic_density
        count = int(round(density * 20))
        if count <= 0:
            return
        spawn_points = [sp for sp in self.map.get_spawn_points() if sp.location.distance(self.ego.get_transform().location) > 30.0]
        self.rng.shuffle(spawn_points)
        vehicles = self.bp_lib.filter("vehicle.*")
        for transform in spawn_points[:count]:
            bp = self.rng.choice(vehicles)
            actor = self.world.try_spawn_actor(bp, transform)
            if actor is None:
                continue
            self._register(actor, "traffic")
            actor.set_autopilot(True, self.tm_port)

    def _register(self, actor, role: str) -> None:
        self.actors.append(actor)
        self.roles[actor.id] = role
        self.role_actors.setdefault(role, []).append(actor)

    # ------------------------------------------------------------------- loop

    def _ego_frame(self, location) -> Tuple[float, float]:
        """Project a world location into the ego bird's-eye frame (x forward, y right)."""
        transform = self.ego.get_transform()
        fwd = transform.get_forward_vector()
        right = transform.get_right_vector()
        dx = location.x - transform.location.x
        dy = location.y - transform.location.y
        return dx * fwd.x + dy * fwd.y, dx * right.x + dy * right.y

    def _gt_class(self, actor) -> Optional[str]:
        role = self.roles.get(actor.id)
        if role in GT_CLASS_BY_ROLE:
            return GT_CLASS_BY_ROLE[role]
        type_id = actor.type_id
        if type_id.startswith("walker."):
            return "pedestrian"
        if type_id.startswith("vehicle."):
            wheels = actor.attributes.get("number_of_wheels", "4")
            return "motorcycle" if wheels == "2" else "vehicle"
        if type_id.startswith("static.prop."):
            return "obstacle"
        return None

    def _ground_truth(self, frame: int, max_range: float = 80.0) -> Dict[str, Any]:
        objects = []
        for actor in self.world.get_actors():
            if actor.id == self.ego.id:
                continue
            cls = self._gt_class(actor)
            if cls is None:
                continue
            if actor.type_id.startswith("static.prop.") and actor.id not in self.roles:
                continue
            x, y = self._ego_frame(actor.get_transform().location)
            if abs(x) > max_range or abs(y) > max_range:
                continue
            bb = actor.bounding_box.extent
            objects.append({"id": actor.id, "cls": cls, "x": round(x, 2), "y": round(y, 2),
                            "length": round(bb.x * 2, 2), "width": round(bb.y * 2, 2), "role": self.roles.get(actor.id, "background")})
        return {"frame": frame, "objects": objects}

    def _emit_once(self, key: str, event: Event) -> None:
        if key in self.emitted:
            return
        self.emitted.add(key)
        self.logger.event(event)

    def _scenario_events(self, t: float, frame: int, lat: float, lon: float, sample: Sample) -> None:
        params = self.template.params
        for role, actors in self.role_actors.items():
            for actor in actors:
                x, y = self._ego_frame(actor.get_transform().location)
                if role in ("oncoming_vehicle", "oncoming_truck", "motorcycle"):
                    if -8.0 < x < 8.0:
                        self._emit_once("enc-%d" % actor.id, Event(t, frame, "oncoming_encounter", "info", lat, lon,
                                                                       "%s passed at lateral %.1f m" % (role, y), data={"lateral_m": round(y, 2)}))
                    if 0.0 < x < 40.0 and abs(y) < params.lane_width_m / 2.0:
                        self._emit_once("encroach-%d" % actor.id, Event(t, frame, "oncoming_encroachment", "warning", lat, lon,
                                                                            "%s inside ego lane %.1f m ahead" % (role, x), data={"ahead_m": round(x, 1), "lateral_m": round(y, 2)}))
                elif role in WALKER_ROLES:
                    if -2.0 < x < 6.0 and abs(y) < 2.0:
                        cls = "vru_crossing" if role == "pedestrian_crossing" else "vru_close_pass"
                        self._emit_once("vru-%d" % actor.id, Event(t, frame, cls, "warning", lat, lon,
                                                                   "%s within %.1f m laterally" % (role, abs(y)), data={"lateral_m": round(y, 2)}))
                elif role == "following_vehicle":
                    if -12.0 < x < 0.0 and abs(y) > params.lane_width_m * 0.5 and sample.speed_mps > 2.0:
                        self._emit_once("overtake-%d" % actor.id, Event(t, frame, "overtaken_close", "info", lat, lon,
                                                                        "overtaken with lateral gap %.1f m" % abs(y), data={"lateral_m": round(y, 2)}))
        obstacle = self.state.get("obstacle")
        if obstacle is not None and obstacle.frame >= frame - 1:
            other = obstacle.other_actor
            role = self.roles.get(other.id) if other is not None else None
            if role in ("debris", "cones", "livestock"):
                self._emit_once("obs-%d" % other.id, Event(t, frame, "obstacle_ahead", "warning", lat, lon,
                                                          "%s detected %.1f m ahead" % (role, obstacle.distance), data={"distance_m": round(obstacle.distance, 1)}))
            elif role in ("stalled_vehicle", "parked_vehicle"):
                self._emit_once("stop-%d" % other.id, Event(t, frame, "stopped_vehicle_ahead", "warning", lat, lon,
                                                           "%s detected %.1f m ahead" % (role, obstacle.distance), data={"distance_m": round(obstacle.distance, 1)}))

    def _lead_info(self, frame: int) -> Tuple[Optional[float], Optional[float]]:
        obstacle = self.state.get("obstacle")
        if obstacle is None or obstacle.frame < frame - 1:
            return None, None
        ego_v = self.ego.get_velocity()
        fwd = self.ego.get_transform().get_forward_vector()
        ego_speed = ego_v.x * fwd.x + ego_v.y * fwd.y
        other = obstacle.other_actor
        other_speed = 0.0
        if other is not None and hasattr(other, "get_velocity"):
            try:
                ov = other.get_velocity()
                other_speed = ov.x * fwd.x + ov.y * fwd.y
            except RuntimeError:
                other_speed = 0.0
        return float(obstacle.distance), float(other_speed - ego_speed)

    def _tick_sample(self, frame: int, t: float) -> Sample:
        transform = self.ego.get_transform()
        velocity = self.ego.get_velocity()
        accel = self.ego.get_acceleration()
        ang = self.ego.get_angular_velocity()
        control = self.ego.get_control()
        fwd = transform.get_forward_vector()
        right = transform.get_right_vector()
        geo = self.map.transform_to_geolocation(transform.location)
        gnss = self.state.get("gnss")
        imu = self.state.get("imu")
        if gnss is not None:
            lat, lon, alt = gnss.latitude, gnss.longitude, gnss.altitude
        else:
            lat, lon, alt = geo.latitude, geo.longitude, geo.altitude
        if imu is not None:
            accel_x, accel_y, accel_z = imu.accelerometer.x, imu.accelerometer.y, imu.accelerometer.z - 9.81
            yaw_rate = imu.gyroscope.z
        else:
            accel_x = accel.x * fwd.x + accel.y * fwd.y
            accel_y = accel.x * right.x + accel.y * right.y
            accel_z = accel.z
            yaw_rate = math.radians(ang.z)
        lead_distance, lead_rel = self._lead_info(frame)
        return Sample(
            t=t,
            frame=frame,
            lat=lat,
            lon=lon,
            alt=alt,
            speed_mps=math.sqrt(velocity.x ** 2 + velocity.y ** 2 + velocity.z ** 2),
            heading_deg=transform.rotation.yaw % 360.0,
            accel_x=accel_x,
            accel_y=accel_y,
            accel_z=accel_z,
            yaw_rate=yaw_rate,
            throttle=control.throttle,
            brake=control.brake,
            steer=control.steer,
            grade_pct=math.tan(math.radians(transform.rotation.pitch)) * 100.0,
            lead_distance_m=lead_distance,
            lead_rel_speed_mps=lead_rel,
            source="sim",
            x=transform.location.x,
            y=transform.location.y,
        )

    def _scheduled(self, elapsed: float) -> None:
        import carla

        params = self.template.params
        for actor in self.role_actors.get("lead_vehicle_stopping", []):
            if elapsed >= params.duration_s / 3.0 and actor.id not in self.emitted:
                actor.set_autopilot(False, self.tm_port)
                actor.apply_control(carla.VehicleControl(brake=1.0))
                self.emitted.add(actor.id)
        if self.crossing_pending:
            still = []
            for walker, controller, target in self.crossing_pending:
                x, _ = self._ego_frame(walker.get_transform().location)
                if 0.0 < x < max(15.0, params.speed_limit_kph / 3.6 * 2.5):
                    controller.start()
                    controller.go_to_location(target)
                    controller.set_max_speed(1.5)
                else:
                    still.append((walker, controller, target))
            self.crossing_pending = still

    def _apply_speed_target(self) -> None:
        limit = self.ego.get_speed_limit() or 30.0
        target = self.template.params.speed_limit_kph
        pct = (1.0 - target / limit) * 100.0
        self.tm.vehicle_percentage_speed_difference(self.ego, pct)

    def run(self, upload_client=None) -> Dict[str, Any]:
        run_id = "run-" + uuid.uuid4().hex[:12]
        self._setup_world()
        try:
            self._spawn_ego()
            for spec in self.template.actors:
                self._spawn_actor_spec(spec)
            self._spawn_traffic()
            manifest = RunManifest(
                run_id=run_id,
                source="sim",
                vehicle_class=self.template.params.vehicle_class,
                route_id="%s:%s" % (self.map.name, self.template.params.route_class),
                scenario_id=self.template.id,
                scenario_hash=self.template.content_hash(),
                started_at=time.time(),
                map_name=self.map.name,
                odd_zone="sim",
                streams={"video": self.has_video, "gnss": True, "imu": True, "can": False, "events": False},
                notes="measured route curvature=%.4f grade=%.2f" % (self.measured_route["curvature"], self.measured_route["grade_pct"]),
            )
            self.logger = TelemetryLogger(self.out_dir, manifest)
            samples: List[Dict[str, Any]] = []
            events_raw: List[Dict[str, Any]] = []
            self.world.tick()
            self._apply_speed_target()
            deg = self.template.params.sensor_degradation
            steps = int(self.template.params.duration_s / self.fixed_delta)
            t0 = time.time()
            for step in range(steps):
                self.world.tick()
                snapshot = self.world.get_snapshot()
                frame = snapshot.frame
                t = t0 + step * self.fixed_delta
                elapsed = step * self.fixed_delta
                self._scheduled(elapsed)
                sample = self._tick_sample(frame, t)
                if deg.dropout_probability > 0 and self.rng.random() < deg.dropout_probability:
                    self.dropped_samples += 1
                    if self.dropped_samples == 1:
                        self.logger.event(Event(t, frame, "sensor_dropout", "info", sample.lat, sample.lon, "telemetry sample dropped (simulated)"))
                else:
                    self.logger.sample(sample)
                    samples.append(sample.to_dict())
                self.logger.ground_truth(self._ground_truth(frame))
                for col in self.state["collisions"]:
                    other = col.other_actor.type_id if col.other_actor is not None else "unknown"
                    ev = Event(t, frame, "collision", "critical", sample.lat, sample.lon, "collision with %s" % other,
                               data={"other": other, "role": self.roles.get(col.other_actor.id, "environment") if col.other_actor is not None else "environment"})
                    self.logger.event(ev)
                    events_raw.append(ev.to_dict())
                self.state["collisions"] = []
                for inv in self.state["lane"]:
                    markings = ",".join(str(m.type) for m in inv.crossed_lane_markings)
                    ev = Event(t, frame, "lane_departure", "warning", sample.lat, sample.lon, "crossed %s" % markings, data={"markings": markings})
                    self.logger.event(ev)
                    events_raw.append(ev.to_dict())
                self.state["lane"] = []
                self._scenario_events(t, frame, sample.lat, sample.lon, sample)
            manifest.ended_at = time.time()
            violations = check_samples(samples, self.template.params.vehicle_class, self.template.params.speed_limit_kph)
            violations += classify_events(events_raw)
            for v in violations:
                self.logger.event(v)
            self.logger.close()
            all_events = events_raw + [v.to_dict() for v in violations]
            quality = assess_run(run_id, samples, all_events, manifest.streams, expected_rate_hz=1.0 / self.fixed_delta,
                                 privacy_status="not_required")
            report = quality.to_dict()
            result = {
                "run_id": run_id,
                "scenario_id": self.template.id,
                "samples": len(samples),
                "dropped_samples": self.dropped_samples,
                "events": len(all_events),
                "critical_violations": sum(1 for v in violations if v.severity == "critical"),
                "quality": report,
                "measured_route": self.measured_route,
                "out_dir": self.out_dir,
            }
            if upload_client is not None:
                result["upload"] = upload_client.upload_run_dir(self.out_dir, report)
            return result
        finally:
            self._cleanup()

    def _cleanup(self) -> None:
        import carla

        for controller in self.walker_controllers:
            try:
                controller.stop()
            except RuntimeError:
                pass
        for sensor in self.sensors:
            try:
                sensor.stop()
                sensor.destroy()
            except RuntimeError:
                pass
        ids = [a.id for a in self.walker_controllers + self.actors]
        if ids:
            self.client.apply_batch([carla.command.DestroyActor(i) for i in ids])
        try:
            self.tm.set_synchronous_mode(False)
            self.world.apply_settings(self.original_settings)
        except RuntimeError:
            pass
