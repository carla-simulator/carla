import argparse
import json
import math
import carla
import pygame


def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument("-id", required=True, help="Collision scenario ID from scenario.json")
    parser.add_argument("-disable_choreography", action="store_true",
                        help="Disable post-collision choreography and just stop vehicles instead")
    parser.add_argument("-slowmo", type=float, default=1.0, metavar="FACTOR",
                        help="Slow-motion factor (e.g. 4.0 = 4x slower than real time). "
                             "Physics timestep is unchanged; only the wall-clock tick rate is reduced.")
    return parser.parse_args()


def load_config(path: str, scenario_id: str, blueprint_lib, map):
    with open(path) as f:
        cfg = json.load(f)

    if scenario_id not in cfg:
        raise KeyError(f"Scenario '{scenario_id}' not found in {path}")

    result = {}
    for key, vehicle_cfg in cfg[scenario_id].items():
        bp = blueprint_lib.find(vehicle_cfg["blueprint"])

        sp = vehicle_cfg["spawn_point"]
        spawn_wp = map.get_waypoint(
            carla.Location(x=sp["x"], y=sp["y"]),
            project_to_road=True
        )
        spawn_transform = spawn_wp.transform

        path_waypoints = []
        for pt in vehicle_cfg["path"]:
            wp = map.get_waypoint(
                carla.Location(x=pt["x"], y=pt["y"]),
                project_to_road=True
            )
            path_waypoints.append(wp.transform.location)

        result[key] = {
            "blueprint": bp,
            "spawn": spawn_transform,
            "path": path_waypoints,
            "speed_kmh": vehicle_cfg["speed_kmh"],
        }

    return result


def boost_collision_physics(
    vehicle_b: carla.Vehicle,
    normal_impulse_on_a: carla.Vector3D,
    low_friction: float = 0.1,
) -> carla.VehiclePhysicsControl:
    """
    Apply physically correct post-collision velocity to Vehicle B and temporarily
    drop tire friction so wheels don't immediately fight the new state.

    Returns the original VehiclePhysicsControl so the caller can restore it later
    (e.g. at the end of choreography phase 1).

    Coordinate note: event.normal_impulse is serialized from UE4 WITHOUT the
    Y-axis negation that every other CARLA Python vector uses (get_velocity,
    get_transform, …). We correct for this before computing in CARLA frame.

    Linear:  Δv_B  = −nimp / m_B
    Angular: Δω_B_z = −(lever × nimp)_z / I_zz
                       lever  = rear-left corner offset (CARLA frame)
                       I_zz   = m·(L²+W²)/12  (uniform-box approximation)
                       sign   negated: CARLA CCW+ → Python/UE4 CW+ convention
    """
    # Convert normal_impulse from UE4 frame → CARLA frame (negate Y).
    nimp = carla.Vector3D(
        normal_impulse_on_a.x,
        -normal_impulse_on_a.y,
        normal_impulse_on_a.z,
    )

    original_phys = vehicle_b.get_physics_control()
    mass_b = original_phys.mass   # kg

    # ── Drop tire friction so wheels don't resist the imposed velocity ────────
    slippery_phys = vehicle_b.get_physics_control()
    for wheel in slippery_phys.wheels:
        wheel.tire_friction = low_friction
    slippery_phys.wheels = slippery_phys.wheels
    vehicle_b.apply_physics_control(slippery_phys)

    # ── Linear ───────────────────────────────────────────────────────────────
    vel = vehicle_b.get_velocity()   # m/s, CARLA frame
    new_vel = carla.Vector3D(
        vel.x - nimp.x / mass_b,
        vel.y - nimp.y / mass_b,
        vel.z - nimp.z / mass_b,
    )
    vehicle_b.set_target_velocity(new_vel)

    # ── Angular ──────────────────────────────────────────────────────────────
    transform = vehicle_b.get_transform()
    bbox      = vehicle_b.bounding_box
    fwd   = transform.get_forward_vector()
    right = transform.get_right_vector()

    # Lever arm: rear-left corner of B (CARLA frame, extents are half-sizes).
    lever = carla.Vector3D(
        fwd.x * (-bbox.extent.x) + right.x * (-bbox.extent.y),
        fwd.y * (-bbox.extent.x) + right.y * (-bbox.extent.y),
        0.0,
    )

    # 2-D cross product Z: lever × (−nimp)  →  kg·m²·rad/s (CARLA CCW+)
    ang_impulse_z = lever.x * (-nimp.y) - lever.y * (-nimp.x)

    # Moment of inertia (yaw): uniform rectangular box approximation.
    L   = 2.0 * bbox.extent.x   # vehicle length  (m)
    W   = 2.0 * bbox.extent.y   # vehicle width   (m)
    Izz = mass_b * (L**2 + W**2) / 12.0   # kg·m²

    # Δω in deg/s.  Negate: CARLA cross-product is CCW+, but Python angular
    # velocity follows UE4 left-hand convention where Z+ = CW from above.
    delta_omega_z = math.degrees(-ang_impulse_z / Izz)

    ang_vel = vehicle_b.get_angular_velocity()   # deg/s
    vehicle_b.set_target_angular_velocity(
        carla.Vector3D(ang_vel.x, ang_vel.y, ang_vel.z + delta_omega_z)
    )

    dv = (new_vel.x - vel.x, new_vel.y - vel.y, new_vel.z - vel.z)
    print(f"  [boost] Δvel=({dv[0]:.2f}, {dv[1]:.2f}, {dv[2]:.2f}) m/s"
          f"  Δω_z={delta_omega_z:+.1f} deg/s  (Izz={Izz:.0f} kg·m²)"
          f"  tire_friction→{low_friction}")

    return original_phys


class CollisionChoreography:
    """
    Human-like post-collision behaviour: damped oscillating counter-steer + full brake.

    Models the "tank slapper": the driver yanks the wheel hard against the spin,
    overshoots to the other side, then corrects back, each swing smaller than the
    last — like a decaying sine wave — until the car straightens and stops.

      steer(t) = initial_sign * exp(-DECAY * t) * cos(OMEGA * t)

    Once the envelope drops below STEER_DONE_THRESHOLD the steering is zeroed and
    the vehicle brakes to a full stop before handing back to autopilot.
    """

    # Envelope decay: ~6 s to reach 5% of the original deflection.
    DECAY = 0.5
    # Oscillation frequency: 2 s period (one full left-right-left cycle every 2 s).
    OMEGA = math.pi
    # Stop steering once the amplitude envelope falls below this fraction.
    STEER_DONE_THRESHOLD = 0.05
    # cm/s below which we consider the vehicle stopped.
    MIN_SPEED_CMS = 50.0

    def __init__(
        self,
        vehicle: carla.Vehicle,
        normal_impulse: carla.Vector3D,
        dt: float = 0.05,
        original_physics: carla.VehiclePhysicsControl | None = None,
    ):
        self.vehicle = vehicle
        self.normal_impulse = normal_impulse
        self.dt = dt
        self.elapsed = 0.0
        self.active = True
        self._original_physics = original_physics
        self._friction_restored = original_physics is None

        ang_vel = vehicle.get_angular_velocity()
        self.impact_yaw_rate = ang_vel.z  # deg/s, UE4 Z-up left-hand

        # First steer direction: oppose the yaw spin.
        # Fall back to the lateral impulse component when spin is negligible at impact.
        if abs(self.impact_yaw_rate) >= 1.0:
            self.initial_sign = -1.0 if self.impact_yaw_rate > 0 else 1.0
        else:
            self.initial_sign = -1.0 if normal_impulse.y > 0 else 1.0

    def tick(self) -> None:
        if not self.active:
            return

        self.elapsed += self.dt

        # Restore tire friction at the end of phase 1 — the spin is established
        # by then and the brakes need normal grip to slow the vehicle.
        if not self._friction_restored and self.elapsed >= self.PHASE1_END:
            self.vehicle.apply_physics_control(self._original_physics)
            self._friction_restored = True

        envelope = math.exp(-self.DECAY * self.elapsed)

        if envelope >= self.STEER_DONE_THRESHOLD:
            # Damped oscillation: full deflection at t=0, decaying sinusoid afterwards.
            steer = self.initial_sign * envelope * math.cos(self.OMEGA * self.elapsed)
            steer = max(-1.0, min(1.0, steer))
        else:
            # Oscillation settled — centre the wheel and brake to a stop.
            steer = 0.0
            vel = self.vehicle.get_velocity()
            speed = math.sqrt(vel.x ** 2 + vel.y ** 2 + vel.z ** 2)
            if speed < self.MIN_SPEED_CMS:
                self.vehicle.apply_control(carla.VehicleControl(brake=1.0))
                self.active = False
                return

        self.vehicle.apply_control(
            carla.VehicleControl(throttle=0.0, steer=steer, brake=1.0)
        )

    @property
    def done(self) -> bool:
        return not self.active


def main():
    args = parse_args()

    client = carla.Client()
    world = client.get_world()
    map = world.get_map()
    tm = client.get_trafficmanager()
    blueprint_lib = world.get_blueprint_library()

    vehicle_1 = None
    vehicle_2 = None
    collision_sensor_1 = None
    collision_sensor_2 = None

    try:
        settings = world.get_settings()
        settings.synchronous_mode = True
        settings.fixed_delta_seconds = 0.05
        world.apply_settings(settings)
        clock = pygame.time.Clock()

        # Slow-motion: keep fixed_delta_seconds (physics unchanged) but cap how
        # many world.tick() calls happen per real second.  At slowmo=1.0 the
        # target is 1/fixed_delta_seconds = 20 fps (real time == sim time).
        # At slowmo=4.0 the target is 5 fps, so the simulation appears 4x slower.
        _sim_hz = 1.0 / settings.fixed_delta_seconds
        _slowmo_fps = _sim_hz / max(args.slowmo, 1.0)
        if args.slowmo > 1.0:
            print(f"Slow-motion enabled: {args.slowmo}x (tick rate capped at {_slowmo_fps:.1f} fps)")

        tm.set_synchronous_mode(True)

        cfg = load_config("collision.json", args.id, blueprint_lib, map)

        cfg["vehicle_1"]["spawn"].location.z += 0.5
        cfg["vehicle_2"]["spawn"].location.z += 0.5

        colors = {"vehicle_1": carla.Color(255, 0, 0), "vehicle_2": carla.Color(0, 0, 255)}
        for key in ("vehicle_1", "vehicle_2"):
            for i, loc in enumerate(cfg[key]["path"]):
                world.debug.draw_point(
                    carla.Location(loc.x, loc.y, loc.z + 0.5),
                    size=0.1,
                    color=colors[key],
                    life_time=60.0,
                )
                world.debug.draw_string(
                    carla.Location(loc.x, loc.y, loc.z + 1.0),
                    f"{key}[{i}]",
                    color=colors[key],
                    life_time=60.0,
                )

        vehicle_1 = world.spawn_actor(cfg["vehicle_1"]["blueprint"], cfg["vehicle_1"]["spawn"])
        vehicle_2 = world.spawn_actor(cfg["vehicle_2"]["blueprint"], cfg["vehicle_2"]["spawn"])

        for _ in range(50):
            world.tick()

        vehicle_1.set_autopilot(True)
        vehicle_2.set_autopilot(True)
        tm.auto_lane_change(vehicle_1, False)
        tm.auto_lane_change(vehicle_2, False)

        tm.set_path(vehicle_1, cfg["vehicle_1"]["path"])
        tm.set_path(vehicle_2, cfg["vehicle_2"]["path"])

        tm.ignore_vehicles_percentage(vehicle_1, 100)
        tm.ignore_vehicles_percentage(vehicle_2, 100)

        for key, vehicle in [("vehicle_1", vehicle_1), ("vehicle_2", vehicle_2)]:
            tm.set_desired_speed(vehicle, cfg[key]["speed_kmh"])
            # Force vehicle to its target speed immediately in local space (X = forward).
            # This bypasses engine/gear acceleration constraints while TM still steers.
            speed_ms = cfg[key]["speed_kmh"] / 3.6
            vehicle.enable_constant_velocity(carla.Vector3D(speed_ms, 0.0, 0.0))

        collision_bp = blueprint_lib.find("sensor.other.collision")
        collision_sensor_1 = world.spawn_actor(collision_bp, carla.Transform(), attach_to=vehicle_1)
        collision_sensor_2 = world.spawn_actor(collision_bp, carla.Transform(), attach_to=vehicle_2)

        # actor_id -> CollisionChoreography  (vehicle_2 only, when choreography is enabled)
        choreographies: dict[int, CollisionChoreography] = {}
        # actor ids that should receive a plain stop control every tick
        stopped_actors: set[int] = set()
        # permanent record — once an actor's post-collision handling starts it never re-triggers
        handled_actors: set[int] = set()
        # original physics of vehicle_2, saved by the vehicle_1 callback so the
        # vehicle_2 choreography can restore tire friction after phase 1
        vehicle_2_original_phys: list[carla.VehiclePhysicsControl | None] = [None]

        stop = carla.VehicleControl(brake=1.0)

        def on_collision(event: carla.CollisionEvent) -> None:
            if args.disable_choreography:
                return  # let autopilot and physics handle it naturally

            actor_id = event.actor.id

            if actor_id in handled_actors:
                return  # only handle each vehicle's first collision

            if not handled_actors:
                # first collision overall — print the summary line once
                print(f"Collision: {event.actor.type_id} hit {event.other_actor.type_id}")
                print(
                    f"  impulse=({event.normal_impulse.x:.0f}, {event.normal_impulse.y:.0f},"
                    f" {event.normal_impulse.z:.0f}) N·s"
                )

            handled_actors.add(actor_id)
            event.actor.disable_constant_velocity()
            event.actor.set_autopilot(False)

            # Vehicle A (vehicle_1) always stops — it hit the other car head-on.
            # Vehicle B (vehicle_2) gets the damped counter-steer choreography.
            if actor_id == vehicle_1.id:
                event.actor.apply_control(carla.VehicleControl(brake=1.0))
                stopped_actors.add(actor_id)
                print(f"  → full brake applied to {event.actor.type_id}")
            else:
                event.actor.apply_control(carla.VehicleControl(brake=1.0))
                choreo = CollisionChoreography(
                    event.actor, event.normal_impulse, dt=0.05,
                    original_physics=vehicle_2_original_phys[0],
                )
                choreographies[actor_id] = choreo
                print(
                    f"  → choreography started for {event.actor.type_id}"
                    f"  (yaw_rate_at_impact={choreo.impact_yaw_rate:+.1f} deg/s,"
                    f"  counter_steer={'left' if choreo.impact_yaw_rate > 0 else 'right'})"
                )
            

        collision_sensor_1.listen(on_collision)
        collision_sensor_2.listen(on_collision)

        while True:
            clock.tick_busy_loop(_slowmo_fps)
            world.tick()

            if handled_actors:
                for actor_id in list(stopped_actors):
                    vehicle = vehicle_1 if actor_id == vehicle_1.id else vehicle_2
                    vel = vehicle.get_velocity()
                    speed = math.sqrt(vel.x ** 2 + vel.y ** 2 + vel.z ** 2)
                    if speed < CollisionChoreography.MIN_SPEED_CMS:
                        stopped_actors.discard(actor_id)
                        #vehicle.set_autopilot(True)
                        vehicle.apply_control(stop)
                        print(f"  {vehicle.type_id} stopped, autopilot re-enabled")
                    else:
                        vehicle.apply_control(stop)

                for actor_id, choreo in list(choreographies.items()):
                    choreo.tick()
                    if choreo.done:
                        del choreographies[actor_id]
                        #choreo.vehicle.set_autopilot(True)
                        print(f"  {choreo.vehicle.type_id} recovered, autopilot re-enabled")

    except KeyboardInterrupt:
        print("Cancelled")

    finally:
        if collision_sensor_1 is not None and collision_sensor_1.is_alive:
            collision_sensor_1.destroy()
        if collision_sensor_2 is not None and collision_sensor_2.is_alive:
            collision_sensor_2.destroy()
        if vehicle_1 is not None and vehicle_1.is_alive:
            vehicle_1.destroy()
        if vehicle_2 is not None and vehicle_2.is_alive:
            vehicle_2.destroy()

        tm.set_synchronous_mode(False)

        settings = world.get_settings()
        settings.synchronous_mode = False
        settings.fixed_delta_seconds = None
        world.apply_settings(settings)


if __name__ == '__main__':
    main()
