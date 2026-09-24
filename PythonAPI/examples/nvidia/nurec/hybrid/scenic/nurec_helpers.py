"""Plain-Python geometry and control helpers for the NuRec hybrid Scenic behaviours (nurec_lib.scenic imports them; classes
cannot live in a .scenic file, where every class is a Scenic object class).

Frames: Scenic heading 0 = +y, counter-clockwise positive; CARLA steer +1 = full right lock.
"""
import math
from shapely.geometry import LineString, Point

MAX_STEER_DEG = 70.0     # steer = 1 -> full lock of CARLA's cars
WHEELBASE = 2.9
KMH = 1.0 / 3.6


def fwd_lat(me, other):
    """(longitudinal, lateral) offset of `other` from `me` in me's frame: forward and right positive, metres."""
    d = other.position - me.position; h = me.heading
    return (-d.x * math.sin(h) + d.y * math.cos(h), d.x * math.cos(h) + d.y * math.sin(h))


def fwd_lat_xy(me, x, y):
    h = me.heading; dx = x - me.position.x; dy = y - me.position.y
    return (-dx * math.sin(h) + dy * math.cos(h), dx * math.cos(h) + dy * math.sin(h))


def ahead_in_lane(me, other, max_d=150.0, lat_tol=1.8):
    lon, lat = fwd_lat(me, other)
    return 0.0 < lon < max_d and abs(lat) < lat_tol


def nearest_ahead(me, cars, max_d=150.0, lat_tol=1.8):
    best = None; best_d = max_d
    for car in cars:
        if car is me: continue
        lon, lat = fwd_lat(me, car)
        if 0.0 < lon < best_d and abs(lat) < lat_tol: best, best_d = car, lon
    return best


def reached_point(me, x, y, margin=0.0):
    """True once `me` is within `margin` metres of (x, y) along its heading, or past it."""
    return fwd_lat_xy(me, x, y)[0] < margin


class LanePath:
    """Centreline of a lane followed by its successors as one shapely line; rebuilt when the lane changes."""
    def __init__(self): self.lane = None; self.line = None

    def update(self, lane):
        if lane is None or lane is self.lane: return
        pts = []; l = lane
        for _ in range(6):
            coords = list(l.centerline.lineString.coords)
            pts.extend(coords if not pts else coords[1:])
            l = l._successor
            if l is None or not hasattr(l, "centerline"): break
        self.lane = lane; self.line = LineString(pts)

    def lookahead(self, pos, dist):
        s = self.line.project(Point(pos.x, pos.y)); p = self.line.interpolate(min(s + dist, self.line.length)); return p.x, p.y

    def lateral(self, pos): return self.line.distance(Point(pos.x, pos.y))


def carla_lane_path(actor, distance=300.0):
    """A fixed 3D-selected route, avoiding 2D lane ambiguity at overpasses.

    Geometry comes from CARLA's map; Scenic still controls every action. Refuse
    forks rather than silently choosing an unrelated exit. The caller chooses
    the starting actor/lane explicitly.
    """
    import carla
    kinds = carla.LaneType.Driving | carla.LaneType.OnRamp | carla.LaneType.OffRamp | carla.LaneType.Entry | carla.LaneType.Exit
    wp = actor.get_world().get_map().get_waypoint(actor.get_location(), lane_type=kinds)
    if wp is None:
        raise ValueError("No drivable lane at actor's 3D position")
    pts = []
    for _ in range(int(distance / 2) + 1):
        loc = wp.transform.location
        pts.append((loc.x, -loc.y))
        # Exported junctions can list the exact same successor twice.
        successors = list({(p.road_id, p.section_id, p.lane_id, round(p.s, 4)): p
                           for p in wp.next(2.0)}.values())
        if not successors:
            break
        if len(successors) != 1:
            raise ValueError(f"Route needs an explicit branch at road {wp.road_id}, lane {wp.lane_id}")
        wp = successors[0]
    if len(pts) < 2:
        raise ValueError("Lane route has no forward extent")
    route = LanePath()
    route.line = LineString(pts)
    return route


def lateral_to_lane(me, lane):
    """Distance (m) from `me` to the centreline of `lane` and its successors (for `until` conditions of a lane change)."""
    p = LanePath(); p.update(lane); return p.lateral(me.position)


def pure_pursuit(me, path, speed, min_ld=6.0, k_ld=0.55):
    ld = max(min_ld, k_ld * speed)
    tx, ty = path.lookahead(me.position, ld)
    lon, lat = fwd_lat_xy(me, tx, ty)
    alpha = math.atan2(lat, max(lon, 0.1))
    delta = math.atan(2.0 * WHEELBASE * math.sin(alpha) / ld)
    return max(-1.0, min(1.0, math.degrees(delta) / MAX_STEER_DEG))


class SpeedControl:
    """throttle = feed-forward(v_des) + PI(error); brake proportional to the excess speed."""
    def __init__(self, dt, kp=0.10, ki=0.05, ff=0.019, max_brake=0.7):
        self.dt = dt; self.kp = kp; self.ki = ki; self.ff = ff; self.i = 0.0; self.max_brake = max_brake

    def step(self, v_des, v):
        err = v_des - v
        if err < -0.6:
            self.i = 0.0
            return 0.0, min(self.max_brake, 0.2 * -err)
        self.i = max(-8.0, min(8.0, self.i + err * self.dt))
        return max(0.0, min(1.0, self.ff * v_des + self.kp * err + self.ki * self.i)), 0.0
