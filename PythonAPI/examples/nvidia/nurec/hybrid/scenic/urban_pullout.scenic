"""Urban pull-out, NuRec scene a2a4322c (one-way street, 47 m recorded in creeping traffic).

The ego drives the street at 20 km/h. Where it widens, a car waits at the kerb in the lane that opens on the right; as
the ego comes up it pulls out into the ego lane right in front of it, the ego brakes hard to its headway and follows it
up to the car standing in the ego lane at the end of the route (which covers the ghost the reconstruction carries there,
see urban_lead_brake.scenic). The pulled-out car stops behind the standing car and the ego behind it, short of the route
end, so the clip ends on a time limit (`clip_seconds`). There is deliberately no lead car: the 16 m between the kerb and
the standing car are all the room the kerb car has, and a lead car queued behind the standing one leaves it none.

Run through hybrid_run.py --scenic urban_pullout.scenic (hybrid_scenic.py sets map / ego_* / clip_end_* / timestep):
    hybrid_run.py --scene a2a4322c --out RUN --scenic urban_pullout.scenic [--scenic-param pullout_trigger=18]
"""
model scenic.simulators.carla.model
from nurec_lib import *

param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param clip_end_x = 0.0
param clip_end_y = 0.0
param ego_speed_kmh = 20.0
param ego_speed = 20            # km/h
param pullout_dist = 38         # m ahead, at the kerb of the lane that opens on the right
param pullout_trigger = 22      # m: the kerb car pulls out when the ego is this close (~4 s away at 20 km/h; from rest it needs ~2.5 s to get its nose into the lane)
param pullout_speed = 14        # km/h it drives at once in the lane
param standing_dist = 54        # m ahead, ego lane: the standing car (on the reconstruction's ghost)
param clip_seconds = 15         # s: the clip ends here at the latest (the ego ends up stopped behind the pulled-out car, short of the route end)

LANE_RIGHT = 3.5                # the second lane, from ~35 m in (measured on the OpenDRIVE)

EGO_SPEED = globalParameters.ego_speed * KMH
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading
KERB = new OrientedPoint at ((follow roadDirection from EGO_START for globalParameters.pullout_dist) offset by (LANE_RIGHT @ 0))

behavior EgoBehavior():
    # lat_tol 3.0: the kerb car counts as ahead as soon as it moves off the kerb (3.5 m out), not only once it is in the lane
    do LaneKeep(EGO_SPEED, leads=[pullout, standing], headway=1.2, min_gap=4.5, max_brake=0.8, lat_tol=3.0)

behavior PulloutBehavior():
    do Stop(0.8) until fwd_lat(ego, self)[0] < globalParameters.pullout_trigger
    target = ego.lane
    do LaneKeep(globalParameters.pullout_speed * KMH, lane=target, leads=[standing], min_gap=2.5, max_throttle=0.7) until lateral_to_lane(self, target) < 0.4
    do LaneKeep(globalParameters.pullout_speed * KMH, leads=[standing], headway=1.2, min_gap=3.0)

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10, 0.10, 0.12), with rolename 'hero',
    with speed EGO_SPEED, with behavior EgoBehavior()

pullout = new Car at KERB,
    with blueprint 'vehicle.dodge.charger', with color Color(0.82, 0.82, 0.85),
    with behavior PulloutBehavior()

standing = new Car at (follow roadDirection from EGO_START for globalParameters.standing_dist),
    with blueprint 'vehicle.nissan.patrol', with color Color(0.45, 0.06, 0.08),
    with behavior Stop(0.8)

require abs(relative heading of pullout from ego) < 15 deg
require abs(relative heading of standing from ego) < 25 deg
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=3.0)
terminate after globalParameters.clip_seconds seconds
