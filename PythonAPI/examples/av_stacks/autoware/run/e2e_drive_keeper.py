#!/usr/bin/env python3
"""e2e_drive_keeper.py -- instrumentation + wedge recovery for the VAD e2e demo.

Started by run_carla_autoware.sh in --mode e2e (under CARLA_PY). A non-ticking
CARLA client that:

  1. attaches a `sensor.other.collision` to the ego and logs every contact
     (the only ground truth for "clean driving" -- position heartbeats alone
     cannot see scrapes, and wheel-speed VelocityReport keeps reporting
     4-7 m/s while the car is pinned against an obstacle);
  2. logs a heartbeat every 60 s of sim time: position, speed, odometer,
     collision count;
  3. detects a terminal wedge (< WEDGE_DIST m of displacement over
     WEDGE_WINDOW s of sim time after the car has first moved) and -- unless
     --no-recover -- teleports the ego to a nearby lane spawn point roughly
     aligned with its heading, so a LANE_FOLLOW-only demo keeps looping.
     VAD has no route input (`default_command` is a fixed LANE_FOLLOW), so
     dead ends and the occasional junction fumble are expected terminal
     states; teleporting is the demo-appropriate recovery (ground-truth
     localization follows the jump instantly and VAD resumes next frame).

Never ticks; observes via world.wait_for_tick() (the demo is the one ticker).
"""

import argparse
import collections
import math
import sys

import carla

WEDGE_WINDOW = 20.0   # s of sim time with no displacement = wedged
WEDGE_DIST = 0.5      # m
MOVE_THRESHOLD = 2.0  # m of total travel before wedge detection arms
RECOVER_MIN_DIST = 12.0   # teleport target: not right where we crashed...
RECOVER_MAX_DIST = 80.0   # ...but close enough to keep the demo local
RECOVER_MAX_ANGLE = 70.0  # deg between ego heading and target lane heading
EGO_ROLE_NAMES = ("hero", "ego")


def log(msg):
    print("[keeper] %s" % msg, flush=True)


def find_hero(world):
    for actor in world.get_actors().filter("vehicle.*"):
        if actor.attributes.get("role_name") in EGO_ROLE_NAMES:
            return actor
    return None


def angle_diff_deg(a, b):
    return abs((a - b + 180.0) % 360.0 - 180.0)


def pick_recovery_spawn(spawn_points, transform):
    loc, yaw = transform.location, transform.rotation.yaw
    best, best_d = None, float("inf")
    for sp in spawn_points:
        d = loc.distance(sp.location)
        if not (RECOVER_MIN_DIST <= d <= RECOVER_MAX_DIST):
            continue
        if angle_diff_deg(sp.rotation.yaw, yaw) > RECOVER_MAX_ANGLE:
            continue
        if d < best_d:
            best, best_d = sp, d
    return best, best_d


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--host", default="localhost")
    parser.add_argument("--port", type=int, default=2000)
    parser.add_argument("--no-recover", action="store_true",
                        help="log wedges but do not teleport-recover")
    args = parser.parse_args()

    client = carla.Client(args.host, args.port)
    client.set_timeout(20.0)
    world = client.get_world()
    world.wait_for_tick(20.0)  # populate this client's actor registry

    hero = None
    for _ in range(30):  # the rig/demo may still be spawning the ego
        hero = find_hero(world)
        if hero:
            break
        world.wait_for_tick(20.0)
    if not hero:
        sys.exit("[keeper] no ego (role hero/ego) found -- aborting")
    log("watching ego id %d (%s)" % (hero.id, hero.type_id))

    collisions = []

    def on_collision(event):
        loc = event.actor.get_transform().location
        other = event.other_actor.type_id if event.other_actor else "unknown"
        impulse = event.normal_impulse.length()
        collisions.append((event.timestamp, other))
        log("COLLISION #%d t=%.1fs with %s at (%.1f,%.1f) impulse=%.0f"
            % (len(collisions), event.timestamp, other, loc.x, loc.y, impulse))

    bp = world.get_blueprint_library().find("sensor.other.collision")
    col_sensor = world.spawn_actor(bp, carla.Transform(), attach_to=hero)
    col_sensor.listen(on_collision)
    log("collision sensor attached (id %d)" % col_sensor.id)

    spawn_points = world.get_map().get_spawn_points()
    history = collections.deque()  # (sim_t, x, y)
    odometer = 0.0
    last_pos = None
    last_beat = None
    recoveries = 0

    try:
        while True:
            snap = world.wait_for_tick(30.0)
            t = snap.timestamp.elapsed_seconds
            tf = hero.get_transform()
            x, y = tf.location.x, tf.location.y

            if last_pos is not None:
                odometer += math.hypot(x - last_pos[0], y - last_pos[1])
            last_pos = (x, y)

            history.append((t, x, y))
            while history and history[0][0] < t - WEDGE_WINDOW - 1.0:
                history.popleft()

            if last_beat is None or t - last_beat >= 60.0:
                v = hero.get_velocity()
                log("heartbeat t=%.0fs pos=(%.1f,%.1f) v=%.2f odo=%.0fm "
                    "collisions=%d recoveries=%d"
                    % (t, x, y, math.hypot(v.x, v.y), odometer,
                       len(collisions), recoveries))
                last_beat = t

            # wedge detection (armed only after the car has actually moved)
            if odometer > MOVE_THRESHOLD and history[-1][0] - history[0][0] >= WEDGE_WINDOW:
                d = math.hypot(x - history[0][1], y - history[0][2])
                if d < WEDGE_DIST:
                    log("WEDGED: %.2fm displacement over %.0fs at (%.1f,%.1f)"
                        % (d, WEDGE_WINDOW, x, y))
                    if args.no_recover:
                        history.clear()  # re-arm; keep logging every window
                        continue
                    target, td = pick_recovery_spawn(spawn_points, tf)
                    if target is None:
                        log("no aligned spawn point within %.0fm -- cannot recover"
                            % RECOVER_MAX_DIST)
                        history.clear()
                        continue
                    hero.set_target_velocity(carla.Vector3D(0, 0, 0))
                    hero.set_target_angular_velocity(carla.Vector3D(0, 0, 0))
                    hero.set_transform(target)
                    recoveries += 1
                    log("RECOVERED #%d: teleported %.0fm to (%.1f,%.1f) yaw=%.0f"
                        % (recoveries, td, target.location.x,
                           target.location.y, target.rotation.yaw))
                    history.clear()
    finally:
        col_sensor.stop()
        col_sensor.destroy()
        log("collision sensor destroyed; %d collisions, %d recoveries, %.0fm total"
            % (len(collisions), recoveries, odometer))


if __name__ == "__main__":
    main()
