"""Urban stop-and-go, NuRec scene a2a4322c (one-way street, 47 m recorded in creeping traffic).

The ego rolls behind a lead car at 20 km/h; the lead brakes to a stop, the ego stops behind it, after a pause the
lead pulls away and the ego follows, while a second car ahead crawls in the lane that opens on the right. A car stands
still in the ego lane at the end of the route: the lead moves right to pass it and the ego stops behind it. Everything
happens inside the 47 m the scene was reconstructed on, so the whole scene is slow. Ends when the ego reaches the end
of the recorded drive. The standing car covers a ghost the reconstruction carries there (the recorded car that stood in
front of the recorded ego for most of the clip was baked into the static scene as a red smear; the temporal harmonizer
turns it into a phantom car).

Run through hybrid_run.py --scenic urban_lead_brake.scenic (hybrid_scenic.py sets map / ego_* / clip_end_* / timestep):
    hybrid_run.py --scene a2a4322c --out RUN --scenic urban_lead_brake.scenic [--scenic-param ego_speed=18]
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
param lead_gap = 11             # m, lead car ahead of the ego at the start
param lead_brake_after = 1.5    # s of driving before the lead brakes
param lead_stop_for = 2.5       # s the lead stands still
param crawler_dist = 40         # m ahead, in the lane that opens on the right
param standing_dist = 54        # m ahead, ego lane: the standing car (on the reconstruction's ghost)

LANE_RIGHT = 3.5                # the second lane, from ~35 m in (measured on the OpenDRIVE)

EGO_SPEED = globalParameters.ego_speed * KMH
EGO_START = new OrientedPoint at (globalParameters.ego_x @ globalParameters.ego_y), facing globalParameters.ego_heading

behavior EgoBehavior():
    do LaneKeep(EGO_SPEED, leads=[lead, crawler, standing], headway=1.2, min_gap=4.5)

behavior LeadBehavior():
    do LaneKeep(EGO_SPEED * 1.05) for globalParameters.lead_brake_after seconds
    do Stop(0.7) for globalParameters.lead_stop_for seconds
    do LaneKeep(EGO_SPEED * 1.1) until fwd_lat(self, standing)[0] < 24
    do LaneKeep(EGO_SPEED * 1.1, lane=crawler.lane, leads=[crawler])

ego = new Car at EGO_START, facing EGO_START.heading,
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.10, 0.10, 0.12), with rolename 'hero',
    with speed EGO_SPEED, with behavior EgoBehavior()

lead = new Car at (follow roadDirection from EGO_START for globalParameters.lead_gap),
    with blueprint 'vehicle.lincoln.mkz', with color Color(0.72, 0.10, 0.12),
    with speed EGO_SPEED, with behavior LeadBehavior()

crawler = new Car at ((follow roadDirection from EGO_START for globalParameters.crawler_dist) offset by (LANE_RIGHT @ 0)),
    with blueprint 'vehicle.mini.cooper', with color Color(0.93, 0.93, 0.93),
    with speed 8 * KMH, with behavior LaneKeep(8 * KMH)

standing = new Car at (follow roadDirection from EGO_START for globalParameters.standing_dist),
    with blueprint 'vehicle.nissan.patrol', with color Color(0.45, 0.06, 0.08),
    with behavior Stop(0.8)

require abs(relative heading of lead from ego) < 10 deg
require abs(relative heading of crawler from ego) < 15 deg
require abs(relative heading of standing from ego) < 25 deg
terminate when reached_point(ego, globalParameters.clip_end_x, globalParameters.clip_end_y, margin=3.0)
