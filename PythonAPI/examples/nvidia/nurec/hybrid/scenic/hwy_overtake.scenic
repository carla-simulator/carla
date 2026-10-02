"""Highway overtake, NuRec scene 7c2cf6cd (two lanes per direction, 100 km/h, 567 m recorded).

The ego closes on a slow van in the recorded right lane. A faster car is coming up in the left lane, so the ego waits
behind the van until that car has passed, pulls out behind it, overtakes the van and moves back into the right lane
ahead of it. Traffic ahead in both lanes gives the ego something to follow after the manoeuvre. The ego camera leaves the
recorded lane by one lane width for a few seconds; the reconstruction tolerates that (README).
No oncoming traffic (median bushes hide the opposite carriageway). Ends when the ego reaches the end of the recorded
drive.

Run through hybrid_run.py --scenic hwy_overtake.scenic (hybrid_scenic.py sets map / ego_* / clip_end_* / timestep):
    hybrid_run.py --scene 7c2cf6cd --out RUN --scenic hwy_overtake.scenic [--scenic-param van_speed=70]
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
param pass_speed = 112         # km/h while overtaking
param van_dist = 48            # m ahead of the ego where the van starts
param van_speed = 74           # km/h
param passer_start = -24       # m, the left-lane car that the ego has to let by
param passer_speed = 124       # km/h
param pull_out_gap = 42        # m (centre to centre) to the van at which the ego wants to pull out; must exceed the
                               # headway distance it would otherwise settle at (headway x speed + car lengths, ~34 m at 74 km/h)
param clear_behind = 16        # m the van has to be behind the ego before it moves back

LANE_LEFT = -3.9               # same-direction left lane at the recorded start pose

EGO_SPEED = globalParameters.ego_speed * KMH
PASS_SPEED = globalParameters.pass_speed * KMH
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading

behavior EgoBehavior():
    # close on the van, let the left-lane car by, pull out behind it, pass the van, move back
    do LaneKeep(EGO_SPEED, leads=[van], headway=1.2) until (fwd_lat(self, van)[0] < globalParameters.pull_out_gap and fwd_lat(self, passer)[0] > 10)
    left = self.laneSection._laneToLeft.lane
    do LaneKeep(PASS_SPEED, lane=left, leads=[passer, left_far]) until lateral_to_lane(self, left) < 0.3
    do LaneKeep(PASS_SPEED, leads=[passer, left_far]) until fwd_lat(self, van)[0] < -globalParameters.clear_behind
    right = self.laneSection._laneToRight.lane
    do LaneKeep(EGO_SPEED, lane=right, leads=[right_far]) until lateral_to_lane(self, right) < 0.3
    do LaneKeep(EGO_SPEED, leads=[right_far], headway=1.4)

behavior Cruise(speed_kmh, leads=()):
    do LaneKeep(speed_kmh * KMH, leads=leads)

right_far = new Car at (follow roadDirection from EGO_START for 175),
    with blueprint 'vehicle.nissan.patrol', with color Color(0.55, 0.57, 0.60),
    with speed 100 * KMH, with behavior Cruise(100)

left_far = new Car at ((follow roadDirection from EGO_START for 130) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.mini.cooper', with color Color(0.72, 0.10, 0.12),
    with speed 112 * KMH, with behavior Cruise(112)

van = new Car at (follow roadDirection from EGO_START for globalParameters.van_dist),
    with blueprint 'vehicle.sprinter.mercedes', with color Color(0.90, 0.90, 0.88),
    with speed globalParameters.van_speed * KMH, with behavior Cruise(globalParameters.van_speed, leads=[right_far])

passer = new Car at ((follow roadDirection from EGO_START for globalParameters.passer_start) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.dodge.charger', with color Color(0.82, 0.82, 0.85),
    with speed globalParameters.passer_speed * KMH, with behavior Cruise(globalParameters.passer_speed, leads=[left_far])

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10, 0.10, 0.12), with rolename 'hero',
    with speed EGO_SPEED, with behavior EgoBehavior()

trailer = new Car at ((follow roadDirection from EGO_START for -95) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.24, 0.43, 0.80),
    with speed 118 * KMH, with behavior Cruise(118, leads=[passer, ego, left_far])

require all(abs(relative heading of car from ego) < 10 deg for car in [van, passer, left_far, right_far, trailer])
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=6.0)
