"""Highway brake wave, NuRec scene 7c2cf6cd (two lanes per direction, 100 km/h, 567 m recorded).

The ego follows a platoon of three cars in the recorded right lane at 100 km/h. The car at the head of the platoon
brakes hard to 40 km/h and holds it for a few seconds; the braking propagates back through the platoon to the ego
(each car keeps a time headway on the one ahead), then the head accelerates again and the platoon stretches out. Cars
in the left lane keep passing at cruising speed throughout, so the wave is visible against undisturbed traffic.
No oncoming traffic (median bushes hide the opposite carriageway). Ends when the ego reaches the end of the recorded
drive.

Run through hybrid_run.py --scenic hwy_brake_wave.scenic (hybrid_scenic.py sets map / ego_* / clip_end_* / timestep):
    hybrid_run.py --scene 7c2cf6cd --out RUN --scenic hwy_brake_wave.scenic [--scenic-param brake_to=30]
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
param ego_speed = 100          # km/h, platoon cruise speed
param gap = 36                 # m between platoon cars at the start (centre to centre; above the headway distance,
                               # headway x speed + car lengths ~33 m at 100 km/h, so the platoon cruises until the head brakes)
param brake_after = 2.0        # s of cruising before the head of the platoon brakes
param brake_to = 40            # km/h the head slows down to
param brake_hold = 3.0         # s it holds that speed
param headway = 1.0            # s time headway of the platoon cars

LANE_LEFT = -3.9               # same-direction left lane at the recorded start pose

EGO_SPEED = globalParameters.ego_speed * KMH
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading

behavior HeadBehavior():
    do LaneKeep(EGO_SPEED) for globalParameters.brake_after seconds
    do LaneKeep(globalParameters.brake_to * KMH, max_brake=0.9) for globalParameters.brake_hold seconds
    do LaneKeep(EGO_SPEED * 1.05)

behavior Platoon(leads):   # not "Follow": a behaviour of that name breaks Scenic's `follow` specifier (every behaviour then binds with no parameters)
    do LaneKeep(EGO_SPEED * 1.03, leads=leads, headway=globalParameters.headway, min_gap=5.0, max_brake=0.9)

behavior Cruise(speed_kmh, leads=()):
    do LaneKeep(speed_kmh * KMH, leads=leads)

p3 = new Car at (follow roadDirection from EGO_START for 3 * globalParameters.gap),
    with blueprint 'vehicle.nissan.patrol', with color Color(0.45, 0.06, 0.08),
    with speed EGO_SPEED, with behavior HeadBehavior()

p2 = new Car at (follow roadDirection from EGO_START for 2 * globalParameters.gap),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.93, 0.93, 0.93),
    with speed EGO_SPEED, with behavior Platoon([p3])

p1 = new Car at (follow roadDirection from EGO_START for globalParameters.gap),
    with blueprint 'vehicle.dodge.charger', with color Color(0.82, 0.82, 0.85),
    with speed EGO_SPEED, with behavior Platoon([p2, p3])

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10, 0.10, 0.12), with rolename 'hero',
    with speed EGO_SPEED, with behavior LaneKeep(EGO_SPEED, leads=[p1, p2, p3], headway=1.1, max_brake=0.9)

left_ahead = new Car at ((follow roadDirection from EGO_START for 70) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.mini.cooper', with color Color(0.72, 0.10, 0.12),
    with speed 112 * KMH, with behavior Cruise(112)

passer = new Car at ((follow roadDirection from EGO_START for -35) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.24, 0.43, 0.80),
    with speed 118 * KMH, with behavior Cruise(118, leads=[left_ahead])

passer2 = new Car at ((follow roadDirection from EGO_START for -100) offset by (LANE_LEFT @ 0)),
    with blueprint 'vehicle.sprinter.mercedes', with color Color(0.90, 0.90, 0.88),
    with speed 116 * KMH, with behavior Cruise(116, leads=[passer, left_ahead])

require all(abs(relative heading of car from ego) < 10 deg for car in [p1, p2, p3, left_ahead, passer, passer2])
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=6.0)
