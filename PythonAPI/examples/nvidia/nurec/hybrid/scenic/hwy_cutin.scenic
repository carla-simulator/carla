"""Highway cut-in, NuRec scene 7c2cf6cd (two lanes per direction, 100 km/h, 567 m recorded).

The ego drives the recorded right lane at 100 km/h with adaptive cruise on whatever is ahead in its lane. A faster car
comes up in the left lane, passes the ego, cuts into its lane a few car lengths ahead and slows down; the ego brakes to
its headway, the car speeds up again and pulls away. Ahead, a lead car in the ego lane closes on a slow car and moves
to the left lane to pass it, leaving the slow van for the ego to close on; a car cruises in the left lane and two fast
cars overtake the ego later in the clip. Only UE5-native vehicle models (Lincoln MKZ, Dodge Charger, Nissan Patrol, Mini
Cooper, Mercedes Sprinter); the legacy `ue4.*` imports are low-detail.
No oncoming traffic: this scene's opposite carriageway lies behind a guardrail and a wall of median bushes that the low
recorded camera cannot see past. Ends when the ego reaches the end of the recorded drive (the neural scene is only
reconstructed along it).

Run through hybrid_run.py --scenic hwy_cutin.scenic (hybrid_scenic.py sets map / ego_* / clip_end_* / timestep):
    hybrid_run.py --scene 7c2cf6cd --out RUN --scenic hwy_cutin.scenic [--scenic-param ego_speed=95 --seed 3]
"""
model scenic.simulators.carla.model
from nurec_lib import *

# --- globals set by hybrid_scenic.py (defaults only make the file compile on its own) ---
param ego_x = 0.0
param ego_y = 0.0
param ego_heading = 0.0
param clip_end_x = 0.0
param clip_end_y = 0.0
param ego_speed_kmh = 100.0
# --- scenario knobs (override with --scenic-param) ---
param ego_speed = 100          # km/h, the ego's cruise speed
param cutin_speed = 118        # km/h while passing
param cutin_slow = 84          # km/h after cutting in
param cutin_gap = 14           # m ahead of the ego where the cut-in starts
param cutin_start = -10        # m, where the cut-in car starts relative to the ego (left lane)

# lateral layout of this scene at the recorded start pose (measured on the OpenDRIVE: ego lane = rightmost of two)
LANE_LEFT = -3.9               # same-direction left lane

EGO_SPEED = globalParameters.ego_speed * KMH
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading

behavior EgoBehavior():
    do LaneKeep(EGO_SPEED, leads=[cutin, lead], headway=1.4)

behavior CutInBehavior():
    fast = globalParameters.cutin_speed * KMH
    do LaneKeep(fast) until fwd_lat(ego, self)[0] > globalParameters.cutin_gap
    target = ego.lane
    do LaneKeep(fast * 0.97, lane=target) until abs(fwd_lat(ego, self)[1]) < 0.4
    do LaneKeep(globalParameters.cutin_slow * KMH, leads=[lead]) for 4.5 seconds
    do LaneKeep(fast, leads=[lead])

behavior Cruise(speed_kmh, leads=()):
    do LaneKeep(speed_kmh * KMH, leads=leads)

behavior LeadBehavior(speed_kmh):
    # cruise behind the slow car until close, then move to the left lane and pass it
    do LaneKeep(speed_kmh * KMH, leads=[slow]) until fwd_lat(self, slow)[0] < 28
    target = self.laneSection._laneToLeft.lane
    do LaneKeep(speed_kmh * KMH, lane=target) until lateral_to_lane(self, target) < 0.3
    do LaneKeep((speed_kmh + 6) * KMH, leads=[left_ahead])

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10, 0.10, 0.12), with rolename 'hero',
    with speed EGO_SPEED, with behavior EgoBehavior()

cutin = new Car at (EGO_START offset by (LANE_LEFT @ globalParameters.cutin_start)),
    with blueprint 'vehicle.dodge.charger', with color Color(0.82, 0.82, 0.85),
    with speed globalParameters.cutin_speed * KMH, with behavior CutInBehavior()

lead = new Car at (follow roadDirection from EGO_START for 62),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.93, 0.93, 0.93),
    with speed 98 * KMH, with behavior LeadBehavior(98)

slow = new Car at (follow roadDirection from EGO_START for 135),
    with blueprint 'vehicle.sprinter.mercedes', with color Color(0.90, 0.90, 0.88),
    with speed 84 * KMH, with behavior Cruise(84)

left_ahead = new Car at ((follow roadDirection from EGO_START for 55) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.mini.cooper', with color Color(0.72, 0.10, 0.12),
    with speed 108 * KMH, with behavior Cruise(108)

passer = new Car at ((follow roadDirection from EGO_START for -60) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.24, 0.43, 0.80),
    with speed 128 * KMH, with behavior Cruise(128, leads=[left_ahead, cutin, lead])

passer2 = new Car at ((follow roadDirection from EGO_START for -115) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.nissan.patrol', with color Color(0.55, 0.57, 0.60),
    with speed 130 * KMH, with behavior Cruise(130, leads=[passer, left_ahead, cutin, lead])

require all(abs(relative heading of car from ego) < 10 deg for car in [cutin, lead, slow, left_ahead, passer, passer2])
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=6.0)
